#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/xmlstring.h>

#include <jansson/jansson.h>

#include "bno.h"
#include "stringutils.h"

// internal bno as xml reader
#include "xmlparser.h"

// internal bno as json reader
#include "jsonparser.h"


#define STRING_EQUALS(str1, str2) (strcmp(str1, str2) == 0)
#define CHECK_OPTION_VALUE(value_string, jump_label) \
    if (value_string == NULL || value_string[0] == '\0') goto jump_label;
#define FILTER_SEPARATOR ':'

#define BNO_WRITE_PROPERTY_NAME(name, name_len, file, counter) \
    {                                                          \
        fwrite(                                                \
            &(BNO_EntryName){                                  \
                .name_length = name_len + 1,                   \
            },                                                 \
            sizeof(BNO_EntryName), 1, file                     \
        );                                                     \
        json_write_string(name, name_len, UINT8_MAX, file);    \
        counter += sizeof(BNO_EntryName) + name_len + 1;       \
    }
#define BNO_WRITE_ITEM_HEADER(bno_type, value_len, file) \
    fwrite(&(BNO_Entry){.type = bno_type, .value_length = value_len}, sizeof(BNO_Entry), 1, file)
#define BNO_WRITE_ITEM(bno_type, c_type, getter, file, counter) \
    {                                                           \
        BNO_WRITE_ITEM_HEADER(bno_type, sizeof(c_type), file);  \
        c_type value = getter;                                  \
        fwrite(&value, sizeof(c_type), 1, file);                \
        counter += sizeof(BNO_Entry) + sizeof(c_type);          \
    }
#define BNO_DUMP_JSON_VALUE(getter, expression, file) fprintf(file, expression, getter);

typedef struct {
    const char* input_filename;
    char* output_filename;
    bool encode;
    bool is_json;
    bool is_binary;
    const char* whitelist;
    const char* blacklist;
} Arguments;

typedef struct {
    BNO_Node bno_node;
    xmlNodePtr node;
    xmlNodePtr current_child;
    long offset;
} XmlStack;

typedef struct {
    XmlNode node;
    const char* node_name;
    int32_t child_index;
    int32_t child_count;
    bool last_node_was_text;
} BNOXmlStack;

typedef struct {
    char** allow;
    char** reject;
} XmlFilter;

typedef struct {
    JSONToken token;
    void* obj_iterator;
    size_t arr_index;
    size_t arr_length;
} JsonStack;

typedef struct {
    JSONTokenValue token;
    JSONTokenIterator iterator;
    bool is_array;
    bool first_value_dumped;
} BNOJsonStack;

typedef struct {
    uint8_t* data;
    size_t offset;
    size_t length;
} DataBuffer;


static size_t json_write_entry(FILE* out_file, json_t* item);
static size_t parse_json_to_bno_object(FILE* out_file, json_t* obj);
static size_t parse_json_to_bno_array(FILE* out_file, json_t* array);

static bool read_buffer(void* buffer, size_t element_size, size_t count, DataBuffer* data) {
    size_t total = element_size * count;
    size_t new_offset = data->offset + total;

    if (new_offset > data->length) {
        return true;
    }

    memcpy(buffer, data->data + data->offset, total);
    data->offset = new_offset;

    return false;
}

static size_t get_dot_index(const char* str, size_t* length) {
    size_t bar_idx = 0, dot_idx = 0;
    size_t len = 0;
    for (size_t i = 1; str[i] != '\0'; i++) {
        switch (str[i]) {
            case '/':
            case '\\':
                bar_idx = i;
                break;
            case '.':
                dot_idx = i;
                break;
        }
        len++;
    }

    if (bar_idx >= dot_idx && dot_idx != 0) {
        dot_idx = len;
    }

    *length = len;
    return dot_idx;
}

static size_t get_lowercase_ext(const char* str, char* ext, size_t max_ext_size) {
    size_t ext_length = 0;
    for (size_t i = 0, j = 0; i < max_ext_size; i++) {
        char c = str[i];
        if (c == '\0') break;
        ext[j++] = tolower(c);
        ext_length++;
    }
    return ext_length;
}


static char** xml_filter_build(const char* filter_contents) {
    if (filter_contents == NULL) {
        return NULL;
    } else if (filter_contents[0] == '\0') {
        char** empty_list = malloc(sizeof(char*));
        empty_list[0] = NULL;
        return empty_list;
    }

    size_t filter_contents_length = strlen(filter_contents) + 1;
    size_t count = 0;
    bool counting = true;
    char** list = NULL;

L_process:
    for (size_t i = 0, prev_index = 0; i < filter_contents_length; i++) {
        if (filter_contents[i] != FILTER_SEPARATOR && filter_contents[i] != '\0') {
            continue;
        }

        if (prev_index >= i) {
            // empty
            continue;
        }

        if (counting) {
            count++;
            continue;
        }

        size_t next_i = i + 1;
        size_t rule_length = i - prev_index;
        char* rule = malloc(rule_length + 1);
        assert(rule);

        memcpy(rule, filter_contents + prev_index, rule_length);
        rule[rule_length] = '\0';

        list[count++] = rule;
        prev_index = next_i;
    }

    if (counting && count > 0) {
        counting = false;
        list = malloc(sizeof(char*) * (count + 1));
        assert(list);

        list[count] = NULL; // end-of-list marker
        count = 0;
        goto L_process;
    }

    return list;
}

static void xml_filters_destroy(XmlFilter* filters) {
    if (filters->allow) {
        for (size_t i = 0; filters->allow[i] != NULL; i++) {
            free(filters->allow[i]);
        }
        free(filters->allow);
    }

    if (filters->reject) {
        for (size_t i = 0; filters->reject[i] != NULL; i++) {
            free(filters->reject[i]);
        }
        free(filters->reject);
    }
}

static bool xml_filters_check(xmlNodePtr parent_node, XmlFilter* filters, xmlNodePtr node) {
    const char* tag = (const char*)parent_node->name;
    bool result = true;

    if (filters->allow) {
        result = false;
        for (size_t i = 0; filters->allow[i] != NULL; i++) {
            if (STRING_EQUALS(filters->allow[i], tag)) {
                result = true;
                break;
            }
        }
    }

    if (filters->reject) {
        for (size_t i = 0; filters->reject[i] != NULL; i++) {
            if (STRING_EQUALS(filters->reject[i], tag)) {
                result = false;
                break;
            }
        }
    }

    if (result) {
        return true;
    }

    xmlChar* text = node->content; // xmlNodeListGetString(node->doc, node, 1);
    if (text == NULL) return false;

    result = false;

    // check if the string is whitespaced
    Grapheme grapheme;
    while (*text != '\0') {
        if (string_decode_utf8_character((const char*)text, 0, &grapheme) && grapheme.size > 1) {
            // UNICODE character found
            result = true;
            break;
        }

        switch (*text) {
            case '\x20':
            case '\t':
            case '\r':
            case '\n':
                text++;
                continue;
        }

        // ASCII character found
        result = true;
        break;
    }

    // xmlFree(text);
    return result;
}

static int xml_InputRead_callback(void* context, char* buffer, int len) {
    size_t ret = fread(buffer, 1, (size_t)len, (FILE*)context);
    return (ret < 1 && ferror((FILE*)context) != 0) ? -1 : (int)ret;
}

static int xml_inputClose_callback(void* context) {
    return fclose((FILE*)context);
}

static void print_usage(int argc, char** argv) {
    const char* program = argc > 0 ? argv[0] : "bno";

    size_t idx = 0;
    for (size_t i = 0; program[i] != '\0'; i++) {
        if (program[i] == '/' || program[i] == '\\') {
            idx = i + 1;
        }
    }
    program = program + idx;

    printf("BNO encoder/decoder v0.1 by kapodamy\n");
    printf("Encodes XML and JSON files into BNO (binary object notation) format and viceversa.\n");
    printf("\n");
    printf("Usage:\n");
    printf("    Encode:\n");
    printf("        %s [encode options...] <input xml file> <output bno file>\n", program);
    printf("        %s <input json file> <output jbno file>\n", program);
    printf("    Decode:\n");
    printf("        %s <input jbno file> <output json file>\n", program);
    printf("        %s <input bno file> <output xml file>\n", program);
    printf("Encode XML options:\n");
    printf(" -w, --whitelist <tag names>         Nodes to allow whitespaced text, each tag name is separated by ':' character\n");
    printf(" -b, --blacklist <tag names>         Nodes to disallow whitespaced text, each tag name is separated by ':' character\n");
    printf(" -n, --no-spaces                     Reject all whitespaced text nodes\n");
    printf("\n");
}

static void parse_arguments(int argc, char** argv, Arguments* values) {
    if (argc < 2 || STRING_EQUALS(argv[1], "-h") || STRING_EQUALS(argv[1], "--help")) {
        print_usage(argc, argv);
        exit(0);
    }

    const char* in_filename = NULL;
    const char* out_filename = NULL;
    size_t option_index = 0;
    int limit = argc - 1;

    for (size_t i = 1; i < argc; i++) {
        bool is_option = false;
        const char* option_name = argv[i];
        const char* option_value = NULL;

        if (option_name[0] == '\0') continue;

        if (i < limit) {
            if (option_name[0] == '-' && option_name[1] == '-') {
                option_name += 2;
                is_option = true;
            } else if (option_name[0] == '-') {
                option_name += 1;
                is_option = true;
            }
        }

        if (is_option) {
            option_index = i;
            size_t j = i + 1;

            if (j < argc) {
                option_value = argv[j];
            }
        } else {
            goto L_option_as_filename;
        }

        if (STRING_EQUALS(option_name, "h") || STRING_EQUALS(option_name, "help")) {
            print_usage(argc, argv);
            exit(0);
            return;
        } else if (STRING_EQUALS(option_name, "w") || STRING_EQUALS(option_name, "whitelist")) {
            CHECK_OPTION_VALUE(option_value, L_missing_value);
            i++;
            values->whitelist = option_value;
        } else if (STRING_EQUALS(option_name, "b") || STRING_EQUALS(option_name, "blacklist")) {
            CHECK_OPTION_VALUE(option_value, L_missing_value);
            i++;
            values->whitelist = option_value;
        } else if (STRING_EQUALS(option_name, "n") || STRING_EQUALS(option_name, "no-spaces")) {
            CHECK_OPTION_VALUE(option_value, L_missing_value);
            values->whitelist = "";
        } else {
            is_option = false;
        }

    L_option_as_filename:
        if (!is_option) {
            if (!in_filename) {
                in_filename = option_name;
            } else if (!out_filename) {
                out_filename = option_name;
            } else {
                printf("unknown option '%s'\n", option_name);
                print_usage(argc, argv);
                exit(1);
            }
        }
    }

    if (!out_filename && !in_filename) {
        printf("missing input and output filenames.\n");
        exit(1);
        return;
    }

    FILE* in_file = fopen(in_filename, "rb");
    if (in_file) {
        fclose(in_file);
    } else {
        printf("failed to open '%s' input file.\n", in_filename);
        exit(1);
        return;
    }

    char ext[5];
    size_t length, dot_idx, ext_length;
    bool check_in_filename = !out_filename;

    char* out_filename2 = NULL;
    if (out_filename) {
        dot_idx = get_dot_index(out_filename, &length);
        ext_length = get_lowercase_ext(out_filename + dot_idx, ext, sizeof(ext));

        if (memcmp(out_filename + dot_idx, ".json", ext_length) == 0) {
            values->is_json = true;
            values->encode = false;
        } else if (memcmp(out_filename + dot_idx, ".xml", ext_length) == 0) {
            values->is_json = false;
            values->encode = false;
        } else if (memcmp(out_filename + dot_idx, ".bno", ext_length) == 0) {
            check_in_filename = true;
            values->encode = true;
        } else if (memcmp(out_filename + dot_idx, ".jbno", ext_length) == 0) {
            values->is_json = true;
            values->encode = true;
        } else {
            printf("unknown output filename extension: %s\n", out_filename);
            exit(1);
        }

        out_filename2 = strdup(out_filename);
        assert(out_filename2);
    }

    if (check_in_filename) {
        dot_idx = get_dot_index(in_filename, &length);

        if (dot_idx >= length) {
            printf("unknown input file, missing filename extension: %s\n", in_filename);
            exit(1);
        }

        ext_length = get_lowercase_ext(in_filename + dot_idx, ext, sizeof(ext));
        if (memcmp(in_filename + dot_idx, ".json", ext_length) == 0) {
            values->is_json = true;
            values->encode = true;
        } else if (memcmp(in_filename + dot_idx, ".xml", ext_length) == 0) {
            values->is_json = false;
            values->encode = true;
        } else if (memcmp(in_filename + dot_idx, ".bno", ext_length) == 0) {
            values->is_json = false;
            values->encode = false;
        } else if (memcmp(in_filename + dot_idx, ".jbno", ext_length) == 0) {
            values->is_json = false;
            values->encode = false;
        } else {
            printf("unknown input filename extension: %s\n", in_filename);
            exit(1);
        }

        if (!out_filename2) {
            out_filename2 = malloc(dot_idx + sizeof(".\0\0\0\0"));
            assert(out_filename2);

            memcpy(out_filename2, in_filename, dot_idx);

            if (values->encode) {
                if (values->is_json) {
                    out_filename2[dot_idx + 0] = '.';
                    out_filename2[dot_idx + 1] = 'j';
                    out_filename2[dot_idx + 2] = 'b';
                    out_filename2[dot_idx + 3] = 'n';
                    out_filename2[dot_idx + 4] = 'o';
                    out_filename2[dot_idx + 5] = '\0';
                } else {
                    out_filename2[dot_idx + 0] = '.';
                    out_filename2[dot_idx + 1] = 'b';
                    out_filename2[dot_idx + 2] = 'n';
                    out_filename2[dot_idx + 3] = 'o';
                    out_filename2[dot_idx + 4] = '\0';
                    out_filename2[dot_idx + 5] = '\0';
                }
            } else if (values->is_json) {
                out_filename2[dot_idx + 0] = '.';
                out_filename2[dot_idx + 1] = 'j';
                out_filename2[dot_idx + 2] = 's';
                out_filename2[dot_idx + 3] = 'o';
                out_filename2[dot_idx + 4] = 'n';
                out_filename2[dot_idx + 5] = '\0';
            } else {
                out_filename2[dot_idx + 0] = '.';
                out_filename2[dot_idx + 1] = 'x';
                out_filename2[dot_idx + 2] = 'm';
                out_filename2[dot_idx + 3] = 'l';
                out_filename2[dot_idx + 4] = '\0';
                out_filename2[dot_idx + 5] = '\0';
            }
        }
    }

    if (!values->encode) {
        dot_idx = get_dot_index(out_filename, &length);
        ext_length = get_lowercase_ext(out_filename + dot_idx, ext, sizeof(ext));

        if (dot_idx == length || ext_length < 1) {
            values->is_binary = true;
        } else if (memcmp(out_filename + dot_idx, ".json", ext_length) == 0) {
            values->is_json = true;
        } else if (memcmp(out_filename + dot_idx, ".xml", ext_length) == 0) {
            values->is_json = false;
        } else if (memcmp(out_filename + dot_idx, ".data", ext_length) == 0) {
            values->is_binary = true;
        } else if (memcmp(out_filename + dot_idx, ".bin", ext_length) == 0) {
            values->is_binary = true;
        } else {
            printf("invalid output filename extension, must be json, xml, data or bin: %s\n", out_filename);
            exit(1);
        }
    }

    assert(out_filename2);
    FILE* out_file = fopen(out_filename2, "wb");
    if (out_file) {
        fclose(out_file);
    } else {
        printf("failed to open '%s' output file.\n", out_filename2);
        free(out_filename2);
        exit(1);
        return;
    }
    values->input_filename = in_filename;
    values->output_filename = out_filename2;
    return;

L_missing_value:
    printf("missing value for option '%s'\n", argv[option_index]);
    exit(1);
    return;
}


static size_t xml_write_bno_value_string(const xmlChar* utf8_string, size_t max_size, FILE* file) {
    size_t string_length = strlen((const char*)utf8_string);

    if (string_length > max_size) {
        printf("can not write the string, length=%zu maximum=%zi: %s\n.", string_length, max_size, utf8_string);
        exit(2);
    }

    fwrite(utf8_string, sizeof(char), (size_t)string_length, file);
    fputc(0x00, file);
    return string_length + 1;
}

static uint32_t xml_write_bno_node_string(xmlNodePtr node, FILE* file) {
    xmlChar* text = node->content; // xmlNodeListGetString(node->doc, node, 1);
    if (text == NULL) {
        // nothing to write
        return 0;
    }

    size_t string_length = strlen((const char*)text);
    assert(string_length > 0 && (string_length + 1) <= INT32_MAX);

    BNO_Node bno_node = (BNO_Node){
        .type = BNO_Type_String,
        .name_length = 0,
        .attributes_count = 0,
        .attributes_length = 0,
        .value_length = (uint32_t)string_length + 1
    };

    fwrite(&bno_node, sizeof(BNO_Node), 1, file);
    fwrite(text, string_length, 1, file);
    fputc(0x00, file);

    // xmlFree(text);

    return sizeof(BNO_Node) + bno_node.value_length;
}

static void xml_write_bno_node_header(XmlStack* stack, xmlNodePtr node, FILE* file) {
    stack->node = node;
    stack->current_child = node->children;
    stack->offset = ftell(file);

    BNO_Node bno_node = (BNO_Node){
        .type = BNO_Type_Object, .value_length = 0, .attributes_count = 0, .attributes_length = 0
    };
    BNO_Attribute bno_attr;

    // step 1: reserve space for node header
    fwrite(&bno_node, sizeof(BNO_Node), 1, file);

    // step 2: write node name
    bno_node.name_length = (uint8_t)xml_write_bno_value_string(node->name, UINT8_MAX, file);

    // step 3: write attributes
    for (xmlAttrPtr attr = node->properties; attr != NULL; attr = attr->next) {
        if (bno_node.attributes_count > BNO_MAX_ATTRIBUTES) {
            printf(
                "can not encode the XML, the node %s contains more than %i attributes.\n",
                node->name, BNO_MAX_ATTRIBUTES
            );
            exit(2);
        }

        xmlChar* value = xmlNodeListGetString(attr->doc, attr->children, 1);

        // allocate space for the attribute header
        long bno_attr_offset = ftell(file);
        fwrite(&bno_attr, sizeof(BNO_Attribute), 1, file);

        bno_attr.name_length = (uint8_t)xml_write_bno_value_string(attr->name, UINT8_MAX, file);
        bno_attr.value_length = (uint8_t)xml_write_bno_value_string(value, UINT8_MAX, file);

        xmlFree(value);

        assert(bno_node.attributes_count <= BNO_MAX_ATTRIBUTES);
        assert(bno_node.attributes_length >= 0 && bno_node.attributes_length <= UINT16_MAX);

        bno_node.attributes_count++;
        bno_node.attributes_length += bno_attr.name_length + bno_attr.value_length;
        bno_node.attributes_length += sizeof(BNO_Attribute);

        // write attribute header
        long end_offset = ftell(file);
        fseek(file, bno_attr_offset, SEEK_SET);
        fwrite(&bno_attr, sizeof(BNO_Attribute), 1, file);
        fseek(file, end_offset, SEEK_SET);
    }

    // step 4: remember until all children nodes are written
    // stack->bno_node = bno_node;
    memcpy(&stack->bno_node, &bno_node, sizeof(BNO_Node));
}

static void xml_write_bno_node_trailer(XmlStack* stack, FILE* file) {
    assert(stack->node != NULL);

    long offset = ftell(file);

    fseek(file, stack->offset, SEEK_SET);
    fwrite(&stack->bno_node, sizeof(BNO_Node), 1, file);
    fseek(file, offset, SEEK_SET);

    stack->offset = -1;
    stack->node = stack->current_child = NULL;
}

static bool xml_to_bno(const char* in_filename, XmlFilter* filters, FILE* out_file) {
    FILE* in_file = fopen(in_filename, "rb");
    assert(in_file);

    uint8_t bom_mark[3];
    fread(bom_mark, sizeof(uint8_t), sizeof(bom_mark), in_file);

    if (bom_mark[0] != 0xEF || bom_mark[1] != 0xBB || bom_mark[2] != 0xBF) {
        fseek(in_file, 0, SEEK_SET);
    }

    xmlDocPtr doc = xmlReadIO(
        xml_InputRead_callback, xml_inputClose_callback, in_file,
        in_filename,
        "utf-8",
        XML_PARSE_NOENT | XML_PARSE_NONET | XML_PARSE_NOCDATA
    );

    if (doc == NULL) {
        fprintf(stderr, "failed to parse XML file: %s\n", in_filename);
        return false;
    }

    fwrite(
        &(BNO_Header){
            .signature = BNO_SIGNATURE,
            .content = BNO_CONTENT_XML
        },
        sizeof(BNO_Header), 1, out_file
    );

    size_t stack_length = 1024;
    ssize_t stack_index = 0;
    XmlStack* stack = malloc(sizeof(XmlStack) * stack_length);

    // write node header and add xml root to the stack
    xmlNodePtr root = xmlDocGetRootElement(doc);
    xml_write_bno_node_header(stack + stack_index, root, out_file);
    stack_index++;

    BNO_Node* bno_parent;
    XmlStack* parent;

L_Process:
    while (stack_index > 0) {
        parent = stack + (stack_index - 1);
        bno_parent = &parent->bno_node;

        for (xmlNodePtr node = parent->current_child; node != NULL; node = node->next) {
            if (node->type == XML_ELEMENT_NODE) {
                ssize_t new_stack_index = stack_index + 1;
                if (new_stack_index >= stack_length) {
                    // this never should happen (XML is too big)
                    stack_length += 1024;
                    stack = realloc(stack, sizeof(XmlStack) * stack_length);
                    assert(stack);
                }

                // resume on the next node
                parent->current_child = node->next;

                // write node and place node on the stack
                xml_write_bno_node_header(stack + stack_index, node, out_file);

                // change parent and write current node childrens
                parent = stack + stack_index;
                stack_index = new_stack_index;
                goto L_Process;
            } else if (node->type == XML_TEXT_NODE) {
                if (xml_filters_check(parent->node, filters, node)) {
                    bno_parent->value_length += xml_write_bno_node_string(node, out_file);
                }
            }
        }

        // add current bno node size to parent node "value length"
        if (stack_index > 1) {
            uint32_t total_length = sizeof(BNO_Node);
            total_length += bno_parent->name_length + bno_parent->value_length + bno_parent->attributes_length;

            BNO_Node* top_parent_bno = &stack[stack_index - 2].bno_node;
            top_parent_bno->value_length += total_length;
            assert(top_parent_bno->value_length <= INT32_MAX);
        }

        // close current node and return to parent
        xml_write_bno_node_trailer(parent, out_file);
        stack_index--;
    }

    // write EOF
    fwrite(
        &(BNO_Node){
            .type = BNO_Type_EOF,
            .name_length = 0,
            .attributes_count = 0,
            .attributes_length = 0,
            .value_length = 0
        },
        sizeof(BNO_Node),
        1,
        out_file
    );

    xmlFreeDoc(doc);
    if (stack) free(stack);
    return true;
}


static void bno_dump_padding(FILE* out_file, ssize_t level) {
    while (level--) {
        fputs("    ", out_file);
    }
}

static void escape_xml(const char* chars, FILE* out_file) {
    Grapheme grapheme;

    for (int32_t i = 0; chars[i] != '\0'; i++) {
        if (string_decode_utf8_character(chars, i, &grapheme) && grapheme.size > 1) {
            for (uint8_t j = 0; j < grapheme.size && chars[i] != '\0'; i++, j++) {
                fputc(chars[i], out_file);
            }
            i--;
            continue;
        }

        switch (chars[i]) {
            case '"':
                fputs("&quot;", out_file);
                break;
            case '\'':
                fputs("&apos;", out_file);
                break;
            case '<':
                fputs("&lt;", out_file);
                break;
            case '>':
                fputs("&gt;", out_file);
                break;
            case '&':
                fputs("&amp;", out_file);
                break;
            default:
                fputc(chars[i], out_file);
                break;
        }
    }
}

static void bno_dump_xml_header(XmlNode node, FILE* out_file) {
    if (xmlparser_is_node_text(node)) {
        const char* str = xmlparser_get_text(node);
        escape_xml(str, out_file);
        return;
    }

    fputc('<', out_file);
    escape_xml(xmlparser_get_tag_name(node), out_file);

    uint8_t attributes_count = xmlparser_get_attribute_count(node);

    if (attributes_count < 1) {
        goto L_write_end;
    }

    fputc(' ', out_file);

    // write attributes
    for (uint8_t i = 0; i < attributes_count; i++) {
        XmlAttribute attr = xmlparser_get_attribute(node, i);

        fputs(attr.name, out_file);
        fputs("=\"", out_file);
        fputs(attr.value, out_file);
        fputc('"', out_file);

        if ((i + 1) < attributes_count) {
            fputc(' ', out_file);
        }
    }

L_write_end:
    fputc('>', out_file);
}

static bool bno_to_xml(const char* in_filename, FILE* out_file) {
    XmlParser doc = xmlparser_init(in_filename);
    if (doc == NULL) {
        fprintf(stderr, "failed to parse BNO(XML) file: %s\n", in_filename);
        return false;
    }

    fputs("<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n", out_file);

    size_t stack_length = 1024;
    ssize_t stack_index = 0;
    BNOXmlStack* stack = malloc(sizeof(BNOXmlStack) * stack_length);

    // write node header and add xml root to the stack
    XmlNode root = xmlparser_get_root(doc);
    bno_dump_xml_header(root, out_file);

    // add root to stack
    stack[stack_index] = (BNOXmlStack){
        .child_index = 0,
        .child_count = xmlparser_get_children_count(root),
        .node = root,
        .node_name = xmlparser_get_tag_name(root),
        .last_node_was_text = xmlparser_is_node_text(root)
    };
    stack_index++;

    XmlNode node_parent;
    BNOXmlStack* parent;

L_Process:
    while (stack_index > 0) {
        parent = stack + (stack_index - 1);
        node_parent = parent->node;

        for (; parent->child_index < parent->child_count; parent->child_index++) {
            XmlNode node = xmlparser_get_children_at(node_parent, parent->child_index);

            if (xmlparser_is_node_text(node)) {
                parent->last_node_was_text = true;
                const char* str = xmlparser_get_text(node);
                escape_xml(str, out_file);
            } else {
                ssize_t new_stack_index = stack_index + 1;
                if (new_stack_index >= stack_length) {
                    // this never should happen (XML is too big)
                    stack_length += 1024;
                    stack = realloc(stack, sizeof(BNOXmlStack) * stack_length);
                    assert(stack);
                }

                // obligatory
                parent->child_index++;

                // write node header
                if (!parent->last_node_was_text) {
                    fputs("\r\n", out_file);
                    bno_dump_padding(out_file, stack_index);
                }
                bno_dump_xml_header(node, out_file);

                parent->last_node_was_text = false;

                // change parent and write current node childrens
                stack[stack_index] = (BNOXmlStack){
                    .child_index = 0,
                    .child_count = xmlparser_get_children_count(node),
                    .node = node,
                    .node_name = xmlparser_get_tag_name(node),
                    .last_node_was_text = false
                };

                stack_index = new_stack_index;
                parent = stack + stack_index;

                goto L_Process;
            }
        }

        // close current node and return to parent
        if (parent->node_name) {
            if (parent->child_count > 0) {
                if (!parent->last_node_was_text) {
                    fputs("\r\n", out_file);
                    bno_dump_padding(out_file, stack_index - 1);
                }
                fputs("</", out_file);
                fputs(parent->node_name, out_file);
                fputs(">", out_file);
            } else {
                fseek(out_file, -1, SEEK_END);
                fputs(" />", out_file);
            }
        }

        stack_index--;
    }

    xmlparser_destroy(&doc);
    if (stack) free(stack);
    return true;
}


static void json_write_string(const char* utf8_str, size_t utf8_str_length, size_t max_size, FILE* out_file) {
    if (utf8_str_length > max_size) {
        printf("can not write the string, length=%zu maximum=%zu: %s\n.", utf8_str_length, max_size, utf8_str);
        exit(2);
    }

    fwrite(utf8_str, sizeof(char), utf8_str_length, out_file);
    fputc(0x00, out_file);
}

static size_t parse_json_to_bno_object(FILE* out_file, json_t* obj) {
    size_t written_bytes = 0;

    for (void* iter = json_object_iter(obj); iter != NULL; iter = json_object_iter_next(obj, iter)) {
        const char* name = json_object_iter_key(iter);
        size_t name_length = json_object_iter_key_len(iter);
        assert(name);

        BNO_WRITE_PROPERTY_NAME(name, name_length, out_file, written_bytes);

        json_t* property = json_object_iter_value(iter);
        assert(property);

        written_bytes += json_write_entry(out_file, property);
    }

    return written_bytes;
}

static size_t parse_json_to_bno_array(FILE* out_file, json_t* array) {
    size_t written_bytes = 0;

    for (size_t i = 0, array_length = json_array_size(array); i < array_length; i++) {
        json_t* item = json_array_get(array, i);
        assert(item);

        written_bytes += json_write_entry(out_file, item);
    }

    return written_bytes;
}

static int json_to_bno(const char* in_filename, FILE* out_file) {
    FILE* in_file = fopen(in_filename, "rb");
    assert(in_file);

    uint8_t bom_mark[3];
    fread(bom_mark, sizeof(uint8_t), sizeof(bom_mark), in_file);

    if (bom_mark[0] != 0xEF || bom_mark[1] != 0xBB || bom_mark[2] != 0xBF) {
        fseek(in_file, 0, SEEK_SET);
    }

    json_error_t error;
    json_t* root = json_loadf(in_file, JSON_REJECT_DUPLICATES | JSON_DECODE_ANY, &error);
    fclose(in_file);

    if (!root) {
        printf("parse failed on line %d: %s\n", error.line, error.text);
        return false;
    }

    fwrite(
        &(BNO_Header){
            .signature = BNO_SIGNATURE,
            .content = BNO_CONTENT_JSON
        },
        sizeof(BNO_Header), 1, out_file
    );

    json_write_entry(out_file, root);

    return true;
}

static size_t json_write_entry(FILE* out_file, json_t* item) {
    long header_offset = 0;
    BNO_Type complex_type = BNO_Type_EOF;
    size_t ret = 0;
    size_t written_bytes = 0;

    switch (item->type) {
        case JSON_OBJECT:
            complex_type = BNO_Type_Object;
            break;
        case JSON_ARRAY:
            complex_type = BNO_Type_Array;
            break;
        default:
            break;
    }

    switch (item->type) {
        case JSON_OBJECT:
        case JSON_ARRAY:
            header_offset = ftell(out_file);
            BNO_WRITE_ITEM_HEADER(complex_type, 0, out_file);
            break;
        default:
            break;
    }

    switch (item->type) {
        case JSON_REAL:
            BNO_WRITE_ITEM(BNO_Type_NumberDouble, double, json_real_value(item), out_file, written_bytes);
            break;
        case JSON_INTEGER:
            BNO_WRITE_ITEM(BNO_Type_NumberLong, int64_t, json_integer_value(item), out_file, written_bytes);
            break;
        case JSON_STRING:
            size_t str_len = json_string_length(item);
            const char* str = json_string_value(item);

            BNO_WRITE_ITEM_HEADER(BNO_Type_String, str_len + 1, out_file);
            json_write_string(str, str_len, UINT32_MAX, out_file);

            written_bytes += sizeof(BNO_Entry) + str_len + 1;
            break;
        case JSON_TRUE:
            BNO_WRITE_ITEM(BNO_Type_Boolean, uint8_t, 0x01, out_file, written_bytes);
            break;
        case JSON_FALSE:
            BNO_WRITE_ITEM(BNO_Type_Boolean, uint8_t, 0x00, out_file, written_bytes);
            break;
        case JSON_ARRAY:
            ret = parse_json_to_bno_array(out_file, item);
            break;
        case JSON_OBJECT:
            ret = parse_json_to_bno_object(out_file, item);
            break;
        case JSON_NULL:
            BNO_WRITE_ITEM_HEADER(BNO_Type_Null, 0, out_file);
            written_bytes += sizeof(BNO_Entry);
            break;
        default:
            assert(false);
            break;
    }

    switch (item->type) {
        case JSON_OBJECT:
        case JSON_ARRAY:
            long offset = ftell(out_file);
            written_bytes += ret + sizeof(BNO_Entry);

            fseek(out_file, header_offset, SEEK_SET);
            BNO_WRITE_ITEM_HEADER(complex_type, (uint32_t)ret, out_file);
            fseek(out_file, offset, SEEK_SET);
            break;
        default:
            break;
    }

    return written_bytes;
}


static void escape_json(const char* chars, FILE* out_file) {
    Grapheme grapheme;

    fputc('"', out_file);
    for (; *chars != '\0'; chars++) {
        if (string_decode_utf8_character(chars, 0, &grapheme) && grapheme.size > 1) {
            for (uint8_t j = 0; j < grapheme.size && *chars != '\0'; chars++, j++) {
                fputc(*chars, out_file);
            }
            chars--;
            continue;
        }

        switch (*chars) {
            case '"':
                fputs("\\\"", out_file);
                continue;
            case '\\':
                fputs("\\\\", out_file);
                continue;
            /*case '/':
                fputs("\\/", out_file);
                continue;*/
            case '\b':
                fputs("\\\b", out_file);
                continue;
            case '\f':
                fputs("\\\f", out_file);
                continue;
            case '\n':
                fputs("\\\n", out_file);
                continue;
            case '\r':
                fputs("\\\r", out_file);
                continue;
            case '\t':
                fputs("\\\t", out_file);
                continue;
            default:
                fputc(*chars, out_file);
                break;
        }
    }
    fputc('"', out_file);
}

static void bno_dump_json_entry(JSONTokenValue* token_value, FILE* out_file) {
    switch (token_value->value_type) {
        case JSONTokenType_NumberDouble:
            BNO_DUMP_JSON_VALUE(*token_value->value_number_double, "%.1g", out_file);
            break;
        case JSONTokenType_NumberLong:
            BNO_DUMP_JSON_VALUE(*token_value->value_number_long, "%lli", out_file);
            break;
        case JSONTokenType_String:
            escape_json(token_value->value_string, out_file);
            break;
        case JSONTokenType_Boolean:
            BNO_DUMP_JSON_VALUE((*token_value->value_boolean) ? "true" : "false", "%s", out_file);
            break;
        case JSONTokenType_Null:
            BNO_DUMP_JSON_VALUE("null", "%s", out_file);
            break;
        default:
            assert(false);
            break;
    }
}

static bool bno_to_json(const char* in_file, FILE* out_file) {
    JSONToken root = json_load_from(in_file);
    if (!root) {
        return false;
    }

    size_t stack_length = 1024;
    ssize_t stack_index = 0;
    BNOJsonStack* stack = malloc(sizeof(BNOJsonStack) * stack_length);

    JSONTokenValue token_value = json_get_root_as_token_value(root);
    if (token_value.value_type == JSONTokenType_Object) {
        stack[stack_index] = (BNOJsonStack){
            .token = token_value,
            .iterator = json_iterator_get_object_properties(root),
            .is_array = false,
            .first_value_dumped = false
        };
        fputc('{', out_file);
    } else if (token_value.value_type == JSONTokenType_Array) {
        stack[stack_index] = (BNOJsonStack){
            .token = token_value,
            .iterator = json_iterator_get_array_items(root),
            .is_array = true,
            .first_value_dumped = false
        };
        fputc('[', out_file);
    } else {
        bno_dump_json_entry(&token_value, out_file);
        return true;
    }
    stack_index++;


    while (stack_index > 0) {
        BNOJsonStack* current = stack + (stack_index - 1);

        bool has_next_value;
        if (current->is_array)
            has_next_value = json_iterate_array(&current->iterator, &token_value);
        else
            has_next_value = json_iterate_object(&current->iterator, &token_value);

        if (!has_next_value) {
            fputs("\r\n", out_file);
            bno_dump_padding(out_file, stack_index);
            fputc(current->is_array ? ']' : '}', out_file);

            stack_index--;
            continue;
        }

        if ((stack_index + 1) > stack_length) {
            switch (token_value.value_type) {
                case JSONTokenType_Object:
                case JSONTokenType_Array:
                    // deep json tree, increase stack length
                    stack_length += 1024;
                    stack = realloc(stack, sizeof(BNOJsonStack) * stack_length);
                    assert(stack);
                    break;
                default:
                    break;
            }
        }

        if (token_value.value_type == JSONTokenType_Object) {
            stack[stack_index] = (BNOJsonStack){
                .token = token_value,
                .iterator = json_iterator_get_object_properties(token_value.token),
                .is_array = false,
                .first_value_dumped = false
            };
            stack_index++;

            if (current->first_value_dumped) {
                fputs(",", out_file);
            } else {
                current->first_value_dumped = true;
            }
            fputs("\r\n", out_file);
            bno_dump_padding(out_file, stack_index);
            if (token_value.property_name) {
                escape_json(token_value.property_name, out_file);
                fputs(": ", out_file);
            }
            fputs("{", out_file);
            continue;
        } else if (token_value.value_type == JSONTokenType_Array) {
            stack[stack_index] = (BNOJsonStack){
                .token = token_value,
                .iterator = json_iterator_get_array_items(token_value.token),
                .is_array = true,
                .first_value_dumped = false
            };
            stack_index++;

            if (current->first_value_dumped) {
                fputs(", ", out_file);
            } else {
                current->first_value_dumped = true;
            }
            fputs("\r\n", out_file);
            bno_dump_padding(out_file, stack_index);
            if (token_value.property_name) {
                escape_json(token_value.property_name, out_file);
                fputs(": ", out_file);
            }
            fputs("[", out_file);
            continue;
        }

        if (current->first_value_dumped) {
            fputc(',', out_file);
        } else {
            current->first_value_dumped = true;
        }

        fputs("\r\n", out_file);
        bno_dump_padding(out_file, stack_index + 1);

        if (token_value.property_name) {
            escape_json(token_value.property_name, out_file);
            fputs(": ", out_file);
        }
        bno_dump_json_entry(&token_value, out_file);
    }

    json_destroy(&root);
    return true;
}


static bool decode_bno(const char* in_filename, FILE* out_file, bool is_json, bool is_binary) {
    BNO_Header hdr;
    FILE* in_file = fopen(in_filename, "rb");
    assert(in_file);

    fseek(in_file, 0, SEEK_END);
    size_t in_file_length = (size_t)ftell(in_file);
    fseek(in_file, 0, SEEK_SET);

    DataBuffer data_buffer = (DataBuffer){
        .offset = 0,
        .length = in_file_length,
        .data = malloc(in_file_length)
    };

    assert(data_buffer.data);
    fread(data_buffer.data, sizeof(char), in_file_length, in_file);
    fclose(in_file);

    if (read_buffer(&hdr, sizeof(BNO_Header), 1, &data_buffer)) {
        printf("truncated file: %s", in_filename);
        goto L_failed;
    }

    if (hdr.signature != BNO_SIGNATURE) {
        printf("invalid bno file, missing signature: %s", in_filename);
        goto L_failed;
    }


    if (hdr.content == BNO_CONTENT_JSON) {
        if (!is_binary && !is_json) {
            printf("invalid output filename extension, the bno content is JSON\n");
            goto L_failed;
        }
        if (!bno_to_json(in_filename, out_file)) {
            goto L_failed;
        }
    } else if (hdr.content == BNO_CONTENT_XML) {
        if (!is_binary && is_json) {
            printf("invalid output filename extension, the bno content is XML\n");
            goto L_failed;
        }
        if (!bno_to_xml(in_filename, out_file)) {
            goto L_failed;
        }
    } else {
        char name[5] = {
            (hdr.content >> 24) & 0xFF,
            (hdr.content >> 16) & 0xFF,
            (hdr.content >> 8) & 0xFF,
            (hdr.content >> 0) & 0xFF,
            '\0'
        };
        printf("unsupported bno content: %s", name);
        goto L_failed;
    }

    free(data_buffer.data);
    return true;

L_failed:
    free(data_buffer.data);
    return false;
}


int main(int argc, char** argv) {
    bool success;
    Arguments values = (Arguments){
        .input_filename = NULL,
        .output_filename = NULL,
        .encode = false,
        .is_binary = false,
        .is_json = false
    };

    parse_arguments(argc, argv, &values);
    assert(values.input_filename && values.output_filename);

    FILE* out_file = fopen(values.output_filename, "wb");
    assert(out_file);

    if (values.encode) {
        if (values.is_json) {
            success = json_to_bno(values.input_filename, out_file);
        } else {
            XmlFilter filters = (XmlFilter){
                .allow = xml_filter_build(values.whitelist),
                .reject = xml_filter_build(values.blacklist)
            };

            success = xml_to_bno(values.input_filename, &filters, out_file);
            xml_filters_destroy(&filters);
        }
    } else {
        success = decode_bno(values.input_filename, out_file, values.is_json, values.is_binary);
    }

    if (success) {
        printf("Succesfully %s %s\n", values.encode ? "created" : "decoded", values.output_filename);
    }

    fclose(out_file);
    free(values.output_filename);
    return success ? 0 : 1;
}
