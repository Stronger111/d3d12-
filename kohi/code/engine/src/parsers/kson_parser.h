#ifndef _KSON_H_
#define _KSON_H_

#include "defines.h"

typedef enum kson_token_type {
    KSON_TOKEN_TYPE_UNKNOWN,
    KSON_TOKEN_TYPE_WHITESPACE,
    KSON_TOKEN_TYPE_COMMENT,
    KSON_TOKEN_TYPE_IDENTIFIER,
    KSON_TOKEN_TYPE_OPERATOR_EQUAL,
    KSON_TOKEN_TYPE_OPERATOR_MINUS,
    KSON_TOKEN_TYPE_OPERATOR_PLUS,
    KSON_TOKEN_TYPE_OPERATOR_SLASH,
    KSON_TOKEN_TYPE_OPERATOR_ASTERISK,  // 星号
    KSON_TOKEN_TYPE_OPERATOR_DOT,
    KSON_TOKEN_TYPE_STRING_LITERAL,
    KSON_TOKEN_TYPE_NUMERIC_LITERAL,
    KSON_TOKEN_TYPE_BOOLEAN,
    KSON_TOKEN_TYPE_CURLY_BRACE_OPEN,
    KSON_TOKEN_TYPE_CURLY_BRACE_CLOSE,
    KSON_TOKEN_TYPE_BRACKET_OPEN,
    KSON_TOKEN_TYPE_BRACKET_CLOSE,
    KSON_TOKEN_TYPE_NEWLINE,
    KSON_TOKEN_TYPE_EOF
} kson_token_type;

typedef struct kson_token {
    kson_token_type type;
    u32 start;
    u32 end;
#ifdef KOHI_DEBUG
    const char* content;
#endif
} kson_token;

typedef struct kson_parser {
    const char* file_content;
    u32 position;

    // darray
    kson_token* tokens;
} kson_parser;

typedef enum kson_property_type {
    // TODO: Do we want to support undefined/null types. If so, pick one and just use that, no defining both.
    KSON_PROPERTY_TYPE_UNKNOWN,
    KSON_PROPERTY_TYPE_INT,
    KSON_PROPERTY_TYPE_FLOAT,
    KSON_PROPERTY_TYPE_STRING,
    KSON_PROPERTY_TYPE_OBJECT,
    KSON_PROPERTY_TYPE_ARRAY,
    KSON_PROPERTY_TYPE_BOOLEAN,
} kson_property_type;

struct kson_property;

typedef enum kson_object_type {
    KSON_OBJECT_TYPE_OBJECT,
    KSON_OBJECT_TYPE_ARRAY
} kson_object_type;

// An object which can contain properties. Objects
// represent both "object" types as well as "array"
// types. These types are identical with one key
// difference: An object's properties are required to
// be named, whereas array properties are unnamed.
typedef struct kson_object {
    kson_object_type type;
    // darray
    struct kson_property* properties;
} kson_object;

// An alias to represent kson arrays, which are really just
// kson_objects that contain properties without names.
typedef kson_object kson_array;

// Represents a property value for a kson property
typedef union kson_property_value {
    // Signed 64-bit int value
    i64 i;
    // 32-bit float value
    f32 f;
    // String value
    const char* s;
    // Array or object value
    kson_object o;
    // Boolean value
    b8 b;
} kson_property_value;

// Represents a single property for a kson object or array.
typedef struct kson_property {
    // The type of property.
    kson_property_type type;
    // The name of the property. If this belongs to an array, it should be null.
    char* name;
    // The property value.
    kson_property_value value;
} kson_property;

// Represents a hierarchy of kson objects.
typedef struct kson_tree {
    // The root object, which always must exist.
    kson_object root;
} kson_tree;

/**
 * @brief Creates a kson parser. Note that it is generally recommended to use the
 * kson_tree_from_string() and kson_tree_to_string() functions instead of invoking
 * this manually, as these also handle cleanup of the parser object.
 *
 * @param out_parser A pointer to hold the newly-created parser.
 * @returns True on success; otherwise false.
 */
KAPI b8 kson_parser_create(kson_parser* out_parser);

/**
 * @brief Destroys the provided parser.
 *
 * @param parser A pointer to the parser to be destroyed.
 */
KAPI void kson_parser_destroy(kson_parser* parser);

/**
 * @brief Uses the given parser to tokenize the provided source string.
 * Note that it is recommended to use the kson_tree_from_string() function instead.
 *
 * @param parser A pointer to the parser to use. Required. Must be a valid parser.
 * @param source A constant pointer to the source string to tokenize.
 * @returns True on success; otherwise false.
 */
KAPI b8 kson_parser_tokenize(kson_parser* parser,const char* source);

/**
 * @brief Uses the given parser to build a kson_tree using the tokens previously
 * parsed. This means that kson_parser_tokenize() must have been called and completed
 * successfully for this function to work. It is recommended to use kson_tree_from_string() instead.
 *
 * @param parser A pointer to the parser to use. Required. Must be a valid parser that has already had kson_parser_tokenize() called against it successfully.
 * @param out_tree A pointer to hold the generated kson_tree. Required.
 * @returns True on success; otherwise false.
 */
KAPI b8 kson_parser_parse(kson_parser* parser,kson_tree* out_tree);

#endif