#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "huffman.h"


#define MAX_SYMBOLS 256

typedef struct Node_s Node;
typedef struct Node_s {
    uint8_t symbol;
    uint32_t frequency;

    Node* parent;
    Node* left;
    Node* right;
} Node;

typedef struct LightNode_s LightNode;
typedef struct LightNode_s {
    union {
        uint8_t symbol_l;
        uint16_t left_index;
    };
    union {
        uint8_t symbol_r;
        uint16_t right_index;
    };
} LightNode;

typedef struct __attribute__((__packed__)) {
    uint8_t left_index;
    uint8_t right_index;
} SerializedNode8;

typedef struct __attribute__((__packed__)) {
    uint16_t left_index;
    uint16_t right_index;
} SerializedNode16;

static int sort_nodes(const void* a, const void* b) {
    const Node* node_a = *(const Node**)(const void**)a;
    const Node* node_b = *(const Node**)(const void**)b;

    if (node_a->frequency == 0)
        return 1;
    else if (node_b->frequency == 0)
        return -1;
    else if (node_a->frequency == 0 && node_b->frequency == 0)
        return 1;
    else
        return (int)(node_a->frequency - node_b->frequency);
}

static uint16_t node_index(Node** array, Node** array_end, Node* node) {
    if (!node) {
        return 0;
    }

    for (Node** ptr = array; ptr < array_end; ptr++) {
        if (*ptr == node) {
            return ptr - array;
        }
    }

    // the node is not in the array
    node = NULL;
    assert(node);
    return 0;
}


uint8_t* huffman_compress(uint8_t* data, size_t data_size, size_t* compressed_size) {
    if (data_size < 1) {
        *compressed_size = 0;
        return NULL;
    }


    Node symbols[MAX_SYMBOLS];
    Node joints[MAX_SYMBOLS];
    Node* tree[MAX_SYMBOLS];

    memset(symbols, 0x00, sizeof(symbols));
    memset(joints, 0x00, sizeof(joints));

    for (uint16_t i = 0; i < MAX_SYMBOLS; i++) {
        symbols[i].symbol = i;
        tree[i] = &symbols[i];
    }

    size_t tree_size = MAX_SYMBOLS;
    for (size_t i = 0; i < data_size; i++) {
        symbols[data[i]].frequency++;
    }

    qsort(tree, tree_size, sizeof(Node*), sort_nodes);

    for (size_t i = 0; i < tree_size; i++) {
        if (tree[i]->frequency == 0) {
            tree_size = i;
        }
    }

    Node** tree_ptr = tree;
    Node* joint_ptr = joints;
    while (tree_size > 1) {
        joint_ptr->left = *tree_ptr++;
        joint_ptr->right = *tree_ptr;
        joint_ptr->frequency = joint_ptr->left->frequency + joint_ptr->right->frequency;

        if (joint_ptr->left) joint_ptr->left->parent = joint_ptr;
        if (joint_ptr->right) joint_ptr->right->parent = joint_ptr;

        *tree_ptr = joint_ptr++;
        tree_size--;

        qsort(tree_ptr, tree_size, sizeof(Node*), sort_nodes);
    }

    // root node has no parent
    (*tree_ptr)->parent = NULL;

    Node* used_nodes[MAX_SYMBOLS];
    Node** used_nodes_ptr = used_nodes;
    uint16_t symbol_count = 0;

    for (joint_ptr--; joint_ptr >= joints; joint_ptr--) {
        *used_nodes_ptr++ = joint_ptr;
    }
    for (Node *ptr = symbols, *end = &symbols[MAX_SYMBOLS]; ptr <= end; ptr++) {
        if (ptr->frequency > 0) {
            *used_nodes_ptr++ = ptr;
            symbol_count++;
        }
    }

    assert(symbol_count > 0 && (symbol_count - 1) <= MAX_SYMBOLS);

    void* serialized_tree_buffer;
    size_t serialized_tree_size;
    size_t used_nodes_count = (size_t)(used_nodes_ptr - used_nodes);

    if (used_nodes_count > (MAX_SYMBOLS - 1)) {
        SerializedNode16 serialized_tree[MAX_SYMBOLS * 2];
        SerializedNode16* ptr = serialized_tree;
        for (size_t i = 0; i < used_nodes_count; i++) {
            Node* node = used_nodes[i];

            if (node >= symbols && node <= &symbols[MAX_SYMBOLS]) {
                *ptr++ = (SerializedNode16){
                    .left_index = node->symbol,
                    .right_index = node->symbol
                };
            } else {
                *ptr++ = (SerializedNode16){
                    .left_index = node_index(used_nodes, used_nodes_ptr, node->left),
                    .right_index = node_index(used_nodes, used_nodes_ptr, node->right)
                };
            }
        }
        serialized_tree_buffer = serialized_tree;
        serialized_tree_size = used_nodes_count * sizeof(SerializedNode16);
    } else {
        SerializedNode8 serialized_tree[MAX_SYMBOLS * 2];
        SerializedNode8* ptr = serialized_tree;
        for (size_t i = 0; i < used_nodes_count; i++) {
            Node* node = used_nodes[i];

            if (node >= symbols && node <= &symbols[MAX_SYMBOLS]) {
                *ptr++ = (SerializedNode8){
                    .left_index = node->symbol,
                    .right_index = node->symbol
                };
            } else {
                *ptr++ = (SerializedNode8){
                    .left_index = (uint8_t)node_index(used_nodes, used_nodes_ptr, node->left),
                    .right_index = (uint8_t)node_index(used_nodes, used_nodes_ptr, node->right)
                };
            }
        }
        serialized_tree_buffer = serialized_tree;
        serialized_tree_size = used_nodes_count * sizeof(SerializedNode8);
    }


    size_t allocated_size = serialized_tree_size + data_size + sizeof(uint8_t) + sizeof(uint32_t);
    uint8_t* compressed_data = malloc(allocated_size);
    uint8_t* compressed_data_ptr = compressed_data;

    *compressed_data_ptr++ = (uint8_t)(symbol_count - 1);

    *((uint32_t*)compressed_data_ptr) = (uint32_t)data_size;
    compressed_data_ptr += sizeof(uint32_t);

    memcpy(compressed_data_ptr, serialized_tree_buffer, serialized_tree_size);
    compressed_data_ptr += serialized_tree_size;

    uint8_t accumulator = 0x00;
    uint8_t accumulator_size = 0;
    uint8_t codes[16];
    for (size_t i = 0; i < data_size; i++) {
        Node* node = &symbols[data[i]];
        uint8_t codes_index = sizeof(codes);

        // note: nodes are traversed in reverse. leaf --> root
        while (true) {
            Node* parent = node->parent;
            if (!parent) break;

            codes_index--;

            if (parent->right == node)
                codes[codes_index] = 0x01;
            else if (parent->left == node)
                codes[codes_index] = 0x00;
            else
                assert(parent->left == node || parent->right == node);

            node = parent;
        }

        uint8_t codes_used = sizeof(codes) - codes_index;

    L_write_word_to_accumulator:
        uint8_t available = 8 - accumulator_size;
        if (available > codes_used) available = codes_used;

        for (uint8_t j = 0; j < available; j++) {
            uint8_t bit = codes[codes_index];
            accumulator |= bit << (7 - accumulator_size);

            accumulator_size++;
            codes_index++;
        }

        if (accumulator_size >= 8) {
            *compressed_data_ptr++ = accumulator;

            accumulator = 0x00;
            accumulator_size = 0;

            codes_used -= available;

            if (codes_used > 0) {
                available = codes_used > 8 ? 8 : codes_used;
                goto L_write_word_to_accumulator;
            }
        }
    }

    if (accumulator_size > 0) {
        *compressed_data_ptr++ = accumulator;
    }

    size_t total_size = (size_t)(compressed_data_ptr - compressed_data);
    assert(total_size <= allocated_size);

    if (total_size < allocated_size) {
        compressed_data = realloc(compressed_data, total_size);
    }

    *compressed_size = total_size;
    return compressed_data;
}

uint8_t* huffman_decompress(uint8_t* compressed, size_t compressed_size, size_t* data_size) {
    LightNode nodes[MAX_SYMBOLS * 2];
    uint8_t* compressed_end = compressed + compressed_size;

    uint16_t symbol_count = (uint16_t)(*compressed++) + 1;
    uint32_t uncompressed_size = *((uint32_t*)compressed);
    compressed += sizeof(uint32_t);

    LightNode* node_ptr = nodes;
    if (symbol_count > (MAX_SYMBOLS - 1)) {
        SerializedNode16* serialized_tree_ptr = (SerializedNode16*)compressed;
        uint16_t table_size = (symbol_count * 2) - 1;
        SerializedNode16* serialized_tree_ptr_end = serialized_tree_ptr + table_size;

        for (; serialized_tree_ptr < serialized_tree_ptr_end; serialized_tree_ptr++) {
            if (serialized_tree_ptr->left_index == serialized_tree_ptr->right_index) {
                uint16_t symbol = (uint8_t)serialized_tree_ptr->left_index;
                *node_ptr = (LightNode){
                    .symbol_l = symbol,
                    .symbol_r = symbol
                };
            } else {
                *node_ptr = (LightNode){
                    .left_index = serialized_tree_ptr->left_index,
                    .right_index = serialized_tree_ptr->right_index
                };
            }
            node_ptr++;
        }
        compressed = (uint8_t*)serialized_tree_ptr;
    } else {
        SerializedNode8* serialized_tree_ptr = (SerializedNode8*)compressed;
        uint16_t table_size = (symbol_count * 2) - 1;
        SerializedNode8* serialized_tree_ptr_end = serialized_tree_ptr + table_size;

        for (; serialized_tree_ptr < serialized_tree_ptr_end; serialized_tree_ptr++) {
            if (serialized_tree_ptr->left_index == serialized_tree_ptr->right_index) {
                uint16_t symbol = (uint8_t)serialized_tree_ptr->left_index;
                *node_ptr = (LightNode){
                    .symbol_l = symbol,
                    .symbol_r = symbol
                };
            } else {
                *node_ptr = (LightNode){
                    .left_index = serialized_tree_ptr->left_index,
                    .right_index = serialized_tree_ptr->right_index
                };
            }
            node_ptr++;
        }
        compressed = (uint8_t*)serialized_tree_ptr;
    }

    uint8_t* data = malloc(uncompressed_size);
    uint8_t* data_ptr = data;

    LightNode* current = &nodes[0]; // node n° is the root
    uint8_t* data_ptr_end = data + uncompressed_size;
    while (compressed <= compressed_end) {
        uint8_t accumulator = *compressed++;
        uint8_t mask = 0x80;

        while (mask) {
            uint8_t set = accumulator & mask;
            mask >>= 1;

            uint16_t index;
            if (set)
                index = current->right_index;
            else
                index = current->left_index;

            current = &nodes[index];

            if (current->left_index == current->right_index) {
                *data_ptr++ = (uint8_t)current->symbol_l;

                if (data_ptr >= data_ptr_end) {
                    goto L_assert_uncompressed;
                }
                current = &nodes[0];
            }
        }
    }

L_assert_uncompressed:
    assert(data_ptr >= data_ptr_end);

    *data_size = uncompressed_size;
    return data;
}
