#ifndef _xmlparser_h
#define _xmlparser_h

#include <stdbool.h>
#include <stdint.h>

#include "bno.h"

#define XMLPARSER_ITERATOR_MAX_RECURSIVE_DEEP 32


typedef const BNO_Header* XmlParser;
typedef const BNO_Node* XmlNode;

typedef struct {
    bool __run;
    int32_t len;
    const char* tag_name;
    const uint8_t* ptr;
} XmlChildrenIterator;

typedef struct {
    bool __run;
    const char* tag_name;
    struct {const uint8_t* ptr; int32_t len;} stack[XMLPARSER_ITERATOR_MAX_RECURSIVE_DEEP];
    uint8_t stack_used;
} XmlRecursiveIterator;

typedef struct {
    bool __run;
    int32_t len;
    const uint8_t* ptr;
} XmlAttributesIterator;

typedef struct {
    const char* name;
    const char* value;
} XmlAttribute;


XmlParser xmlparser_init(const char* src);
void xmlparser_destroy(XmlParser* xmlparser);

XmlNode xmlparser_get_root(XmlParser xmlparser);

uint8_t xmlparser_get_attribute_count(XmlNode node);
const char* xmlparser_get_attribute_name(XmlNode node, int32_t index);
const char* xmlparser_get_attribute_value(XmlNode node, int32_t index);
const char* xmlparser_get_attribute_value2(XmlNode node, const char* attribute_name);
XmlAttribute xmlparser_get_attribute(XmlNode node, int32_t index);
bool xmlparser_has_attribute(XmlNode node, const char* attribute_name);

const char* xmlparser_get_tag_name(XmlNode node);
XmlNode xmlparser_get_first_children(XmlNode node, const char* tag_name);
int32_t xmlparser_get_children_count(XmlNode node);
XmlNode xmlparser_get_children_at(XmlNode node, int32_t index);

XmlChildrenIterator xmlparser_iterator_get_children(XmlNode node);
XmlChildrenIterator xmlparser_iterator_get_children2(XmlNode node, const char* tag_name);
XmlRecursiveIterator xmlparser_iterator_get_recursive(XmlNode node, const char* tag_name);
bool xmlparser_iterate_children(XmlChildrenIterator* iterator, XmlNode* node);
bool xmlparser_iterate_recursive_children(XmlRecursiveIterator* iterator, XmlNode* node);

XmlAttributesIterator xmlparser_iterator_get_attributes(XmlNode node);
bool xmlparser_iterate_attributes(XmlAttributesIterator* iterator, XmlAttribute* attribute);
const char* xmlparser_get_text(XmlNode text_node);
bool xmlparser_is_node_text(XmlNode node);

char* xmlparser_get_outerXml(XmlNode node);
char* xmlparser_get_textContext(XmlNode node);

#endif
