//
// Created by remy on 10/30/22.
//

#include "token.h"

const char* token_keyword_to_string(TokenKeyword keyword) {
    switch (keyword) {
        case TOKEN_KEYWORD_FN: return "fn";
        case TOKEN_KEYWORD_CONST: return "const";
        case TOKEN_KEYWORD_VAR: return "var";
        case TOKEN_KEYWORD_RETURN: return "return";
        case TOKEN_KEYWORD_EXTERN: return "extern";
        case TOKEN_KEYWORD_INCLUDE: return "include";
        default: return "-?-";
    }
}

int string_from_token_keyword(String* str) {
    if (string_equal_const(str, "fn")) {
        return TOKEN_KEYWORD_FN;
    } else if (string_equal_const(str, "const")) {
        return TOKEN_KEYWORD_CONST;
    } else if (string_equal_const(str, "var")) {
        return TOKEN_KEYWORD_VAR;
    } else if (string_equal_const(str, "return")) {
        return TOKEN_KEYWORD_RETURN;
    } else if (string_equal_const(str, "extern")) {
        return TOKEN_KEYWORD_EXTERN;
    } else if (string_equal_const(str, "include")) {
        return TOKEN_KEYWORD_INCLUDE;
    } else {
        return -1;
    }
}

Token* token_create(TokenType type) {
    Token* token = (Token*) malloc(sizeof(Token));
    token->type = type;
    token->data.empty = NULL;
    return token;
}

Token* token_decimal(String* data) {
    Token* token = token_create(TOKEN_DECIMAL);
    token->data.decimal.data = data;
    return token;
}

Token* token_string(String* data) {
    Token* token = token_create(TOKEN_STRING);
    token->data.string.data = data;
    return token;
}

Token* token_identifier(String* data) {
    Token* token = token_create(TOKEN_IDENTIFIER);
    token->data.identifier.data = data;
    return token;
}

Token* token_keyword(TokenKeyword keyword) {
    Token* token = token_create(TOKEN_KEYWORD);
    token->data.keyword = keyword;
    return token;
}

Token* token_plus() {
    return token_create(TOKEN_PLUS);
}

Token* token_minus() {
    return token_create(TOKEN_MINUS);
}

Token* token_star() {
    return token_create(TOKEN_STAR);
}

Token* token_slash() {
    return token_create(TOKEN_SLASH);
}

Token* token_equal() {
    return token_create(TOKEN_EQUAL);
}

Token* token_colon() {
    return token_create(TOKEN_COLON);
}

Token* token_comma() {
    return token_create(TOKEN_COMMA);
}

Token* token_left_parenthesis() {
    return token_create(TOKEN_LEFT_PARENTHESIS);
}

Token* token_right_parenthesis() {
    return token_create(TOKEN_RIGHT_PARENTHESIS);
}

Token* token_right_arrow() {
    return token_create(TOKEN_RIGHT_ARROW);
}

Token* token_new_line() {
    return token_create(TOKEN_NEW_LINE);
}

Token* token_tab() {
    return token_create(TOKEN_TAB);
}

String* token_to_string(Token* self) {
    String* str = string_empty();

    switch (self->type) {
        case TOKEN_DECIMAL:
            string_push_const(str, "Decimal(");
            string_push_string(str, self->data.decimal.data);
            string_push(str, ')');
            break;
        case TOKEN_STRING:
            string_push(str, '"');
            string_push_string(str, self->data.string.data);
            string_push(str, '\"');
            break;
        case TOKEN_IDENTIFIER:
            string_push_const(str, "ID(");
            string_push_string(str, self->data.identifier.data);
            string_push(str, ')');
            break;
        case TOKEN_KEYWORD:
            string_push_const(str, "Keyword(");
            string_push_const(str, token_keyword_to_string(self->data.keyword));
            string_push(str, ')');
            break;
        case TOKEN_PLUS:
            string_push_const(str, "+");
            break;
        case TOKEN_MINUS:
            string_push_const(str, "-");
            break;
        case TOKEN_STAR:
            string_push_const(str, "*");
            break;
        case TOKEN_SLASH:
            string_push_const(str, "/");
            break;
        case TOKEN_EQUAL:
            string_push_const(str, "=");
            break;
        case TOKEN_COLON:
            string_push_const(str, ":");
            break;
        case TOKEN_COMMA:
            string_push_const(str, ",");
            break;
        case TOKEN_LEFT_PARENTHESIS:
            string_push_const(str, "(");
            break;
        case TOKEN_RIGHT_PARENTHESIS:
            string_push_const(str, ")");
            break;
        case TOKEN_RIGHT_ARROW:
            string_push_const(str, "=>");
            break;
        case TOKEN_NEW_LINE:
            string_push_const(str, "NewLine");
            break;
        case TOKEN_TAB:
            string_push_const(str, "Tab");
            break;
    }

    return str;
}

void token_destroy(Token* self) {
    switch (self->type) {
        case TOKEN_DECIMAL:
            string_destroy(self->data.decimal.data);
            break;
        case TOKEN_STRING:
            string_destroy(self->data.string.data);
            break;
        default:
            break;
    }
    free(self);
}
