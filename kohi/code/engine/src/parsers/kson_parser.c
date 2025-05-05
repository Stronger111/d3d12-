#include "kson_parser.h"

#include "containers/darray.h"
#include "containers/stack.h"
#include "core/asserts.h"
#include "core/kmemory.h"
#include "core/kstring.h"
#include "core/logger.h"

b8 kson_parser_create(kson_parser* out_parser) {
    if (!out_parser) {
        KERROR("kson_parser_create requires valid pointer to out_parser, ya dingus.");
        return false;
    }

    out_parser->position = 0;
    out_parser->tokens = darray_create(kson_token);
    out_parser->file_content = 0;

    return true;
}

void kson_parser_destroy(kson_parser* parser) {
    if (parser) {
        if (parser->file_content) {
            string_free((char*)parser->file_content);
            parser->file_content = 0;
        }
        if (parser->tokens) {
            darray_destroy(parser->tokens);
            parser->tokens = 0;
        }
        parser->position = 0;
    }
}

typedef enum kson_tokenize_mode {
    KSON_TOKENIZE_MODE_UNKNOWN,
    KSON_TOKENIZE_MODE_DEFINING_IDENTIFIER,
    KSON_TOKENIZE_MODE_WHITESPACE,
    KSON_TOKENIZE_MODE_STRING_LITERAL,
    KSON_TOKENIZE_MODE_NUMERIC_LITERAL,
    KSON_TOKENIZE_MODE_BOOLEAN,
    KSON_TOKENIZE_MODE_OPERATOR,
} kson_tokenize_mode;

// Reset both the current token type and the tokenize mode to unknown.
static void RESET_CURRENT_TOKEN_AND_MODE(kson_token* current_token, kson_tokenize_mode* mode) {
    current_token->type = KSON_TOKEN_TYPE_UNKNOWN;
    current_token->start = 0;
    current_token->end = 0;
    *mode = KSON_TOKENIZE_MODE_UNKNOWN;
}

#ifdef KOHI_DEBUG
static void _populate_token_content(kson_token* t, const char* source) {
    KASSERT_MSG(t->start <= t->end, "Token start comes after token end, ya dingus!");
    char buffer[512] = {0};
    KASSERT_MSG((t->end - t->start) < 512, "token won't fit in buffer.");
    string_mid(buffer, source, t->start, t->end - t->start);
    t->content = string_duplicate(buffer);
}
#define POPULATE_TOKEN_CONTENT(t, source) _populate_token_content(t, source)
#else
// No-op
#define POPULATE_CURRENT_TOKEN_CONTENT(t, source)
#endif

// Pushes the current token, if not of unknown type.
static void PUSH_TOKEN(kson_token* t, kson_parser* parser) {
    if (t->type != KSON_TOKEN_TYPE_UNKNOWN) {
        POPULATE_TOKEN_CONTENT(t, parser->file_content);
        darray_push(parser->tokens, *t);
    }
}

b8 kson_parser_tokenize(kson_parser* parser, const char* source) {
    if (!parser) {
        KERROR("kson_parser_tokenize requires valid pointer to out_parser, ya dingus.");
        return false;
    }
    if (!source) {
        KERROR("kson_parser_tokenize requires valid pointer to source, ya dingus.");
        return false;
    }

    if (parser->file_content) {
        string_free((char*)parser->file_content);
    }
    parser->file_content = string_duplicate(source);

    // Ensure the parser's tokens array is empty.
    darray_clear(parser->tokens);

    u32 char_length = string_length(source);
    /* u32 text_length_utf8 = string_utf8_length(source); */

    kson_tokenize_mode mode = KSON_TOKENIZE_MODE_DEFINING_IDENTIFIER;
    kson_token current_token = {0};
    // The previous codepoint.
    i32 prev_codepoint = 0;
    // The codepoint from 2 iterations ago.
    i32 prev_codepoint2 = 0;

    b8 eof_reached = false;

    // Take the length in chars and get the correct codepoint from it.
    i32 codepoint = 0;
    for (u32 c = 0; c < char_length; prev_codepoint2 = prev_codepoint, prev_codepoint = codepoint) {
        if (eof_reached) {
            break;
        }

        codepoint = source[c];
        // How many bytes to advance.
        u8 advance = 1;
        // NOTE: UTF-8 codepoint handling.
        if (!bytes_to_codepoint(source, c, &codepoint, &advance)) {
            KWARN("Invalid UTF-8 found in string,using unknown codepoint of -1");
            codepoint = -1;
        }

        if (mode == KSON_TOKENIZE_MODE_STRING_LITERAL) {
            // Handle string literal parsing.
            // End the string if only if the previous codepoint was not a backslash OR the codepoint
            // previous codepoint was a backslash AND the one before that was also a backslash. I.e.
            // it needs to be confirmed that the backslash is not already escaped and that the quote is
            // also not escaped.
            if (codepoint == '"' && (prev_codepoint != '\\' || prev_codepoint2 == '\\')) {
                // Terminate the string,push the token onto the array, and revert modes.
                PUSH_TOKEN(&current_token, parser);
                RESET_CURRENT_TOKEN_AND_MODE(&current_token, &mode);
            } else {
                // Handle other characters as part of the string.
                current_token.end += advance;
            }
            // TODO: May need to handle other escape sequences read in here,like \t,\n, etc.

            // At this point,this codepoint has been handles so continue early.
            c += advance;
            continue;
        }

        // Not part of a string, identifier,numeric,etc., so try to figure out what to do next.
        switch (codepoint) {
            case '\n':
                PUSH_TOKEN(&current_token, parser);

                // Just create a new token and insert it.
                kson_token newline_token = {KSON_TOKEN_TYPE_NEWLINE, c, c + advance};

                PUSH_TOKEN(&newline_token, parser);  // old
                RESET_CURRENT_TOKEN_AND_MODE(&current_token, &mode);
                break;
            case '\t':
            case '\r':
            case ' ': {
                if (mode == KSON_TOKENIZE_MODE_WHITESPACE) {
                    // Track it onto the whitspace.
                    current_token.end++;
                } else {
                    // Before switching to the whitespace mode,push the current token.
                    PUSH_TOKEN(&current_token, parser);
                    mode = KSON_TOKENIZE_MODE_WHITESPACE;
                    current_token.type = KSON_TOKEN_TYPE_WHITESPACE;
                    current_token.start = c;
                    current_token.end = c + advance;
                }
            } break;
            case '{': {
                PUSH_TOKEN(&current_token, parser);

                // Create and push a new token for this.
                kson_token open_brace_token = {KSON_TOKEN_TYPE_CURLY_BRACE_OPEN, c, c + advance};
                PUSH_TOKEN(&open_brace_token, parser);

                RESET_CURRENT_TOKEN_AND_MODE(&current_token, &mode);
            } break;
            case '}': {
                PUSH_TOKEN(&current_token, parser);

                // Create and push a new token for this.
                kson_token close_brace_token = {KSON_TOKEN_TYPE_CURLY_BRACE_CLOSE, c, c + advance};
                PUSH_TOKEN(&close_brace_token, parser);

                RESET_CURRENT_TOKEN_AND_MODE(&current_token, &mode);
            } break;
            case '[': {
                PUSH_TOKEN(&current_token, parser);

                // Create and push a new token for this.
                kson_token open_bracket_token = {KSON_TOKEN_TYPE_BRACKET_OPEN, c, c + advance};
                PUSH_TOKEN(&open_bracket_token, parser);

                RESET_CURRENT_TOKEN_AND_MODE(&current_token, &mode);
            } break;
            case ']': {
                PUSH_TOKEN(&current_token, parser);

                // Create and push a new token for this.
                kson_token close_bracket_token = {KSON_TOKEN_TYPE_BRACKET_CLOSE, c, c + advance};
                PUSH_TOKEN(&close_bracket_token, parser);

                RESET_CURRENT_TOKEN_AND_MODE(&current_token, &mode);
            } break;
            case '"': {
                PUSH_TOKEN(&current_token, parser);

                RESET_CURRENT_TOKEN_AND_MODE(&current_token, &mode);

                // Change to string parsing mode.
                mode = KSON_TOKENIZE_MODE_STRING_LITERAL;
                current_token.start = c + advance;
                current_token.end = c + advance;
            } break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
                if (mode == KSON_TOKENIZE_MODE_NUMERIC_LITERAL) {
                    current_token.end++;
                } else {
                    // push the existing token.
                    PUSH_TOKEN(&current_token, parser);

                    // Switch to numeric parsing mode.
                    mode = KSON_TOKENIZE_MODE_NUMERIC_LITERAL;
                    current_token.type = KSON_TOKEN_TYPE_NUMERIC_LITERAL;
                    current_token.start = c;
                    current_token.end = c + advance;
                }
            } break;
            case '-': {
                // NOTE: Always treat the minus as a minus operator regardless of how it is used (except in
                // the string case above, which is already covered). It's then up to the grammar rules as to
                // whether this then gets used to negate a numeric literal or if it is used for subtraction, etc.
                PUSH_TOKEN(&current_token, parser);

                // Create and push a new token for this.
                kson_token minus_token = {KSON_TOKEN_TYPE_OPERATOR_MINUS, c, c + advance};
                PUSH_TOKEN(&minus_token, parser);

                RESET_CURRENT_TOKEN_AND_MODE(&current_token, &mode);
            } break;
            case '+': {
                // NOTE: Always treat the plus as a plus operator regardless of how it is used (except in
                // the string case above, which is already covered). It's then up to the grammar rules as to
                // whether this then gets used to ensure positivity of a numeric literal or if it is used for addition, etc.

                PUSH_TOKEN(&current_token, parser);

                // Create and push a new token for this.
                kson_token plus_token = {KSON_TOKEN_TYPE_OPERATOR_PLUS, c, c + advance};
                PUSH_TOKEN(&plus_token, parser);

                RESET_CURRENT_TOKEN_AND_MODE(&current_token, &mode);
            } break;
            case '/': {
                PUSH_TOKEN(&current_token, parser);
                RESET_CURRENT_TOKEN_AND_MODE(&current_token, &mode);
                if (source[c + 1] == '/') {
                    i32 cm = c + 2;
                    char ch = source[cm];
                    while (ch != '\n' && ch != '\0') {
                        cm++;
                        ch = source[cm];
                    }
                    if (cm > 0) {
                        // Skip to one char before the newline so the newline gets processed.
                        // This is done because the comment shouldn't be tokenized, but should
                        // instead be ignored.
                        c = cm;
                    }
                    continue;
                } else {
                    // Otherwise it should be treated as a slash operator.
                    // Create and push a new token for this.
                    kson_token slash_token = {KSON_TOKEN_TYPE_OPERATOR_SLASH, c, c + advance};
                    PUSH_TOKEN(&slash_token, parser);
                }
                RESET_CURRENT_TOKEN_AND_MODE(&current_token, &mode);
            } break;
            case '*': {
                PUSH_TOKEN(&current_token, parser);

                // Create and push a new token for this.
                kson_token asterisk_token = {KSON_TOKEN_TYPE_OPERATOR_ASTERISK, c, c + advance};
                PUSH_TOKEN(&asterisk_token, parser);

                RESET_CURRENT_TOKEN_AND_MODE(&current_token, &mode);
            } break;
            case '=': {
                PUSH_TOKEN(&current_token, parser);

                // Create and push a new token for this.
                kson_token equal_token = {KSON_TOKEN_TYPE_OPERATOR_EQUAL, c, c + advance};
                PUSH_TOKEN(&equal_token, parser);

                RESET_CURRENT_TOKEN_AND_MODE(&current_token, &mode);
            } break;
            case '.': {
                // NOTE: Always treat this as a dot token, regardless of use. It's up to the grammar
                // rules in the parser as to whether or not it's to be used as part of a numeric literal
                // or something else.

                PUSH_TOKEN(&current_token, parser);

                // Create and push a new token for this.
                kson_token dot_token = {KSON_TOKEN_TYPE_OPERATOR_DOT, c, c + advance};
                PUSH_TOKEN(&dot_token, parser);

                RESET_CURRENT_TOKEN_AND_MODE(&current_token, &mode);
            } break;
            case '\0': {
                // Reached the end of the file.
                PUSH_TOKEN(&current_token, parser);

                // Create and push a new token for this.
                kson_token eof_token = {KSON_TOKEN_TYPE_EOF, c, c + advance};
                PUSH_TOKEN(&eof_token, parser);

                RESET_CURRENT_TOKEN_AND_MODE(&current_token, &mode);

                eof_reached = true;
            } break;
            default: {
                // Identifiers may be made up of upper/lowercase a-z, underscores and numbers (although
                // a number cannot be the first character of an identifier). Note that the number cases
                // are handled above as numeric literals, and can/will be combined into identifiers
                // if there are identifiers without whitespace next to numerics.
                if ((codepoint >= 'A' && codepoint <= 'z') || codepoint == '_') {
                    if (mode == KSON_TOKENIZE_MODE_DEFINING_IDENTIFIER) {
                        // Start a new identifier token.
                        if (current_token.type == KSON_TOKEN_TYPE_UNKNOWN) {
                            current_token.type == KSON_TOKEN_TYPE_IDENTIFIER;
                            current_token.start = c;
                            current_token.end = c;
                        }
                        // Track onto the existing identifier.
                        current_token.end += advance;
                    } else {
                        // Check first to see if it's possibly a boolean definition.
                        const char* str = source + c;
                        u8 bool_advance = 0;
                        if (strings_nequali(str, "true", 4)) {
                            bool_advance = 4;
                        } else if (strings_nequali(str, "false", 5)) {
                            bool_advance = 5;
                        }

                        if (bool_advance) {
                            PUSH_TOKEN(&current_token, parser);

                            // Create and push boolean token.
                            kson_token bool_token = {KSON_TOKEN_TYPE_BOOLEAN, c, c + bool_advance};
                            PUSH_TOKEN(&bool_token, parser);

                            RESET_CURRENT_TOKEN_AND_MODE(&current_token, &mode);

                            // Move forward by the size of the token.
                            advance = bool_advance;
                        } else {
                            // Treat as the start of an identifier definition.
                            // Push the existing token.
                            PUSH_TOKEN(&current_token, parser);

                            // Switch to identifier parsing mode.
                            mode = KSON_TOKENIZE_MODE_DEFINING_IDENTIFIER;
                            current_token.type = KSON_TOKEN_TYPE_IDENTIFIER;
                            current_token.start = c;
                            current_token.end = c + advance;
                        }
                    }
                } else {
                    // If any other character is come across here that isn't part of a string, it's unknown
                    // what should happen here. So, throw an error regarding this and boot if this is the
                    // case.
                    KERROR("Unexpected character '%c' at position %u. Tokenization failed.", c + advance);
                    // Clear the tokens array, as there is nothing that can be done with them in this case.
                    darray_clear(parser->tokens);
                    return false;
                }
            } break;
        }
        // Now advance c
        c += advance;
    }
    PUSH_TOKEN(&current_token, parser);
    // Create and push a new token for this.
    kson_token eof_token = {KSON_TOKEN_TYPE_EOF, char_length, char_length + 1};
    PUSH_TOKEN(&eof_token, parser);

    return true;
}

#define NEXT_TOKEN()                            \
    {                                           \
        index++;                                \
        current_token = &parser->tokens[index]; \
    }

#define ENSURE_IDENTIFIER(token_string)                                                                          \
    {                                                                                                            \
        if (expect_identifier) {                                                                                 \
            KERROR("Expected identifier, instead found '%s'. Position: %u", token_string, current_token->start); \
            return false;                                                                                        \
        }                                                                                                        \
    }

static kson_token* get_last_non_whitespace_token(kson_parser* parser, u32 current_index) {
    if (current_index == 0) {
        return 0;
    }
    kson_token* t = &parser->tokens[current_index - 1];
    while (current_index > 0 && t && t->type == KSON_TOKEN_TYPE_WHITESPACE) {
        current_index--;
        t = &parser->tokens[current_index];
    }

    // This means that the last token available in the file is whitespace.
    // Impossible to return non-whitespace token.
    if (t->type == KSON_TOKEN_TYPE_WHITESPACE) {
        return 0;
    }

    return t;
}

#define NUMERIC_LITERAL_STR_MAX_LENGTH 25

static char* string_from_kson_token(const char* file_content, const kson_token* token) {
    i32 length = (i32)token->end - (i32)token->start;
    KASSERT_MSG(length > 0, "Token length should be at one,ya dingus.");
    char* mid = kallocate(sizeof(char) * (length + 1), MEMORY_TAG_STRING);
    string_mid(mid, file_content, token->start, length);
    mid[length] = 0;
    return mid;
}

b8 kson_parser_parse(kson_parser* parser,kson_tree* out_tree){
    
}