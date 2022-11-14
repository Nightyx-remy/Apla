//
// Created by remy on 11/6/22.
//

#include "lexer.h"

LexerError lexer_error_create(LexerErrorType type, String* message) {
    LexerError self;
    self.type = type;
    self.message = message;
    return self;
}

void lexer_error_destroy(LexerError* self) {
    string_destroy(self->message);
}

LexerResult lexer_result_ok(List* tokens) {
    LexerResult self;
    self.type = LEXER_OK;
    self.data.tokens = tokens;
    return self;
}

LexerResult lexer_result_err(LexerError error) {
    LexerResult self;
    self.type = LEXER_ERROR;
    self.data.error = error;
    return self;
}

void lexer_result_destroy(LexerResult* self) {
    switch (self->type) {
        case LEXER_ERROR:
            lexer_error_destroy(&self->data.error);
            break;
        case LEXER_OK: {
            uintptr_t i = 0;
            while (i < self->data.tokens->length) {
                Positioned* pos = self->data.tokens->ptr[i];
                token_destroy(pos->data);
                positioned_destroy(pos);
                i += 1;
            }
            list_destroy(self->data.tokens);
        } break;
    }
}

LexerSingleResult lexer_single_result_ok(Positioned* pos) {
    LexerSingleResult self;
    self.type = LEXER_OK;
    self.data.pos = pos;
    return self;
}

LexerSingleResult lexer_single_result_err(LexerError error) {
    LexerSingleResult self;
    self.type = LEXER_ERROR;
    self.data.error = error;
    return self;
}

LexerResult lexer_single_result_to_result(LexerSingleResult self) {
    if (self.type == LEXER_ERROR) {
        return lexer_result_err(self.data.error);
    } else {
        List* l = list_create();
        list_push(l, self.data.pos);
        return lexer_result_ok(l);
    }
}

Lexer lexer_create(String* src) {
    Lexer self;
    self.src = src;
    self.pos = position_default();
    return self;
}

char lexer_get_current(Lexer* self) {
    if (self->pos.index < self->src->length) {
        return self->src->ptr[self->pos.index];
    } else {
        return '\0';
    }
}

char lexer_peek(Lexer* self, uintptr_t offset) {
    if (self->pos.index + offset < self->src->length) {
        return self->src->ptr[self->pos.index + offset];
    } else {
        return '\0';
    }
}

void lexer_advance(Lexer* self) {
    position_advance(&self->pos, lexer_get_current(self));
}

Positioned* lexer_single_empty(Lexer* self) {
    Position start = position_clone(self->pos);
    Position end = position_clone(start);
    position_advance(&end, lexer_get_current(self));
    return positioned_create(NULL, start, end);
}

Positioned* lexer_make_single(Lexer* self, Token* token) {
    Position start = position_clone(self->pos);
    Position end = position_clone(start);
    position_advance(&end, lexer_get_current(self));
    return positioned_create((void*) token, start, end);
}

Positioned* lexer_make_number(Lexer* self) {
    String* buf = string_empty();
    Position start = position_clone(self->pos);

    char current = lexer_get_current(self);
    while (isdigit(current)) {
        string_push(buf, current);
        lexer_advance(self);
        current = lexer_get_current(self);
    }

    Position end = position_clone(self->pos);

    return positioned_create((void*) token_decimal(buf), start, end);
}

LexerSingleResult lexer_make_string(Lexer* self) {
    String* buf = string_empty();
    Position start = position_clone(self->pos);

    char current = lexer_get_current(self);
    while (current != '"') {
        if (current == '\0') {
            return lexer_single_result_err(lexer_error_create(LEXER_UNEXPECTED_EOF, string_from_const("Unexpected EOF, missing '\"'!")));
        }
        string_push(buf, current);
        lexer_advance(self);
        current = lexer_get_current(self);
    }

    Position end = position_clone(self->pos);

    return lexer_single_result_ok(positioned_create((void*) token_string(buf), start, end));
}

Positioned* lexer_make_id(Lexer* self) {
    String* buf = string_empty();
    Position start = position_clone(self->pos);

    char current = lexer_get_current(self);
    while (isalnum(current) || current == '_') {
        string_push(buf, current);
        lexer_advance(self);
        current = lexer_get_current(self);
    }

    Position end = position_clone(self->pos);

    int keyword = string_from_token_keyword(buf);
    if (keyword == -1) {
        return positioned_create((void*) token_identifier(buf), start, end);
    } else {
        string_destroy(buf);
        return positioned_create((void*) token_keyword(keyword), start, end);
    }
}

LexerResult lexer_tokenize(Lexer* self) {
    List* tokens = list_create();

    Position space_start = position_clone(self->pos);
    uint8_t space_count = 0;

    char current = lexer_get_current(self);
    while (current != '\0') {
        // Replace 4 spaces by a tab token
        if (current == ' ') {
            space_count += 1;
            if (space_count == 4) {
                space_count = 0;
                Position end = self->pos;
                position_advance(&end, lexer_get_current(self));
                list_push(tokens, positioned_create((void*) token_tab(), space_start, end));
            } else if (space_count == 1) {
                space_start = position_clone(self->pos);
            }
            lexer_advance(self);
            goto SkipAdvance;
        } else {
            space_count = 0;
        }

        // Other
        if (isdigit(current)) {
            list_push(tokens, lexer_make_number(self));
            goto SkipAdvance;
        } else if (isalpha(current)) {
            list_push(tokens, lexer_make_id(self));
            goto SkipAdvance;
        } else {
            switch (current) {
                case '+':
                    list_push(tokens, lexer_make_single(self, token_plus()));
                    break;
                case '-':
                    list_push(tokens, lexer_make_single(self, token_minus()));
                    break;
                case '*':
                    list_push(tokens, lexer_make_single(self, token_star()));
                    break;
                case '/':
                    list_push(tokens, lexer_make_single(self, token_slash()));
                    break;
                case '(':
                    list_push(tokens, lexer_make_single(self, token_left_parenthesis()));
                    break;
                case ')':
                    list_push(tokens, lexer_make_single(self, token_right_parenthesis()));
                    break;
                case '"':
                    lexer_advance(self);
                    LexerSingleResult res = lexer_make_string(self);
                    if (res.type == LEXER_OK) {
                        list_push(tokens, res.data.pos);
                    } else {
                        return lexer_single_result_to_result(res);
                    }
                    break;
                case '=': {
                    Position start = self->pos;
                    if (lexer_peek(self, 1) == '>') {
                        lexer_advance(self);
                        Position end = self->pos;
                        position_advance(&end, lexer_get_current(self));
                        list_push(tokens, positioned_create((void*) token_right_arrow(), start, end));
                    } else {
                        list_push(tokens, lexer_make_single(self, token_equal()));
                    }
                    break; }
                case ':':
                    list_push(tokens, lexer_make_single(self, token_colon()));
                    break;
                case ',':
                    list_push(tokens, lexer_make_single(self, token_comma()));
                    break;
                case '\n':
                    list_push(tokens, lexer_make_single(self, token_new_line()));
                    break;
                case '\t':
                    list_push(tokens, lexer_make_single(self, token_tab()));
                    break;
                case '#':
                    while (current != '\n' && current != '\0') {
                        lexer_advance(self);
                        current = lexer_get_current(self);
                    }
                    break;
                default: {
                    String* str = string_from_const("Unexpected char '");
                    string_push(str, current);
                    string_push_const(str, "' at ");
                    string_push_int(str, (int) self->pos.line);
                    string_push(str, ':');
                    string_push_int(str, (int) self->pos.column);
                    string_push(str, '\n');
                    Positioned* pos = lexer_single_empty(self);
                    String* arrow = positioned_arrow_message(pos, self->src);
                    string_push_string(str, arrow);
                    string_destroy(arrow);
                    positioned_destroy(pos);
                    return lexer_result_err(lexer_error_create(LEXER_UNEXPECTED_CHAR, str));
                }
            }
        }

        lexer_advance(self);

        SkipAdvance:
        current = lexer_get_current(self);
    }

    return lexer_result_ok(tokens);
}