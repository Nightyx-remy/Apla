#ifndef APLA_COMPILER_TOKEN_H
#define APLA_COMPILER_TOKEN_H

#include "str.h"

typedef enum TokenTypeT {
    TOKEN_DECIMAL,
    TOKEN_STRING,
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_EQUAL,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_LEFT_PARENTHESIS,
    TOKEN_RIGHT_PARENTHESIS,
    TOKEN_RIGHT_ARROW,
    TOKEN_NEW_LINE,
    TOKEN_TAB
} TokenType;

typedef struct TokenDecimalT {
    String* data;
} TokenDecimal;

typedef struct TokenStringT {
    String* data;
} TokenString;

typedef struct TokenIdentifierT {
    String* data;
} TokenIdentifier;

typedef enum TokenKeywordT {
    TOKEN_KEYWORD_FN = 0,
    TOKEN_KEYWORD_CONST,
    TOKEN_KEYWORD_VAR,
    TOKEN_KEYWORD_RETURN,
    TOKEN_KEYWORD_EXTERN,
    TOKEN_KEYWORD_INCLUDE,
} TokenKeyword;

const char* token_keyword_to_string(TokenKeyword keyword);
int string_from_token_keyword(String* str);

typedef union TokenDataT {
    TokenDecimal decimal;
    TokenString string;
    TokenIdentifier identifier;
    TokenKeyword keyword;
    void* empty;
} TokenData;

typedef struct TokenT {
    TokenType type;
    TokenData data;
} Token;

Token* token_decimal(String* data);
Token* token_string(String* data);
Token* token_identifier(String* data);
Token* token_keyword(TokenKeyword keyword);
Token* token_plus();
Token* token_minus();
Token* token_star();
Token* token_slash();
Token* token_equal();
Token* token_colon();
Token* token_comma();
Token* token_left_parenthesis();
Token* token_right_parenthesis();
Token* token_right_arrow();
Token* token_new_line();
Token* token_tab();

String* token_to_string(Token* self);

void token_destroy(Token* self);

#endif //APLA_COMPILER_TOKEN_H
