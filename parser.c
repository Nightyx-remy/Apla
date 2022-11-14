#include "parser.h"

#define RETURN_EOF_ERR(msg) return parser_single_result_err(parser_error_create(PARSER_UNEXPECTED_EOF, string_from_const(msg)))
#define RETURN_EOF_ERR_STR(msg) return parser_single_result_err(parser_error_create(PARSER_UNEXPECTED_EOF, msg))
#define RETURN_UNEXPECTED_TOKEN_ERR_(fmt, ...) return parser_single_result_err(parser_error_create(PARSER_UNEXPECTED_TOKEN, string_format(fmt, __VA_ARGS__)))

#define EXPECT_CURRENT_SHOULD_BE(name0, name1, msg) \
    name0 = parser_get_current(self);\
    if (name0 == NULL) RETURN_EOF_ERR_STR(string_format("Unexpected EOF, should be {cs}", msg)); \
    name1 = name0->data

#define RETURN_UNEXPECTED_TOKEN_ERR(positioned, token, expected) \
    RETURN_UNEXPECTED_TOKEN_ERR_(\
        "Unexpected Token '{sf}' at {d}:{d}, should be {cs}\n{sf}",\
        token_to_string(token),\
        (int) positioned->start.line,\
        (int) positioned->start.column,\
        expected,\
        positioned_arrow_message(positioned, self->src)\
    )

#define EXPECT_TOKEN_SHOULD_BE(name0, name1, token, msg) \
    name0 = parser_get_current(self);\
    if (name0 == NULL) RETURN_EOF_ERR_STR(string_format("Unexpected EOF, should be {cs}", msg)); \
    name1 = name0->data; \
    if (name1->type != token) RETURN_UNEXPECTED_TOKEN_ERR(name0, name1, msg)

#define EXPECT_KEYWORD_SHOULD_BE(name0, name1, key, msg) \
    name0 = parser_get_current(self);\
    if (name0 == NULL) RETURN_EOF_ERR_STR(string_format("Unexpected EOF, should be {cs}", msg)); \
    name1 = name0->data; \
    if (name1->type != TOKEN_KEYWORD) RETURN_UNEXPECTED_TOKEN_ERR(name0, name1, msg); \
    if (name1->data.keyword != key) RETURN_UNEXPECTED_TOKEN_ERR(name0, name1, msg)

#define UNWRAP_SINGLE(expr, name0, name1) \
    name0 = expr; \
    if (name0.type == PARSER_ERROR) return name0; \
    name1 = name0.data.node

// End Of Statement
#define EXPECT_EOS(name0, name1) \
    name0 = parser_get_current(self); \
    if (name0 != NULL) { \
        name1 = name0->data;                         \
        if (name1->type != TOKEN_NEW_LINE) RETURN_UNEXPECTED_TOKEN_ERR(name0, name1, "EOF or NewLine"); \
        parser_advance(self);                             \
    }

////////////////////////////////////////////////////////////////////////////////
//                                Parser Error                                //
////////////////////////////////////////////////////////////////////////////////

ParserError parser_error_create(ParserErrorType type, String* message) {
    ParserError self;
    self.type = type;
    self.message = message;
    return self;
}

void parser_error_destroy(ParserError* self) {
    string_destroy(self->message);
}

////////////////////////////////////////////////////////////////////////////////
//                                Parser Result                               //
////////////////////////////////////////////////////////////////////////////////

ParserResult parser_result_ok(List* ast) {
    ParserResult self;
    self.type = PARSER_OK;
    self.data.ast = ast;
    return self;
}

ParserResult parser_result_err(ParserError error) {
    ParserResult self;
    self.type = PARSER_ERROR;
    self.data.error = error;
    return self;
}

void parser_result_destroy(ParserResult* self) {
    switch (self->type) {
        case PARSER_ERROR:
            parser_error_destroy(&self->data.error);
            break;
        case PARSER_OK: {
            uintptr_t i = 0;
            while (i < self->data.ast->length) {
                Positioned* pos = self->data.ast->ptr[i];
                node_destroy(pos->data);
                positioned_destroy(pos);
                i += 1;
            }
            list_destroy(self->data.ast);
        } break;
    }
}

////////////////////////////////////////////////////////////////////////////////
//                            Parser Single Result                            //
////////////////////////////////////////////////////////////////////////////////

ParserSingleResult parser_single_result_ok(Positioned* node) {
    ParserSingleResult self;
    self.type = PARSER_OK;
    self.data.node = node;
    return self;
}

ParserSingleResult parser_single_result_err(ParserError error) {
    ParserSingleResult self;
    self.type = PARSER_ERROR;
    self.data.error = error;
    return self;
}

ParserResult parser_single_result_to_result(ParserSingleResult self) {
    if (self.type == PARSER_ERROR) {
        return parser_result_err(self.data.error);
    } else {
        List* l = list_create();
        list_push(l, self.data.node);
        return parser_result_ok(l);
    }
}

////////////////////////////////////////////////////////////////////////////////
//                                   Parser                                   //
////////////////////////////////////////////////////////////////////////////////

ParserSingleResult parser_parse_current(Parser* self);
ParserSingleResult parser_parse_expr2(Parser* self);
ParserSingleResult parser_parse_function_call(Parser* self, String* name);

Parser parser_create(String* src, List* tokens) {
    Parser self;
    self.src = src;
    self.tokens = tokens;
    self.index = 0;
    return self;
}

Positioned* parser_get_current(Parser* self) {
    if (self->index >= self->tokens->length) {
        return NULL;
    } else {
        return self->tokens->ptr[self->index];
    }
}

void parser_advance(Parser* self) {
    self->index += 1;
}

void parser_advance_x(Parser* self, uintptr_t x) {
    self->index += x;
}

Positioned* parser_get_next(Parser* self) {
    parser_advance(self);
    return parser_get_current(self);
}

Positioned* parser_peek(Parser* self, uintptr_t offset) {
    if (self->index + offset >= self->tokens->length) {
        return NULL;
    } else {
        return self->tokens->ptr[self->index + offset];
    }
}

ParserSingleResult parser_parse_expr0(Parser* self) {
    Positioned* current;
    Token* tok;
    EXPECT_CURRENT_SHOULD_BE(current, tok, "EXPR_0");

    switch (tok->type) {
        case TOKEN_DECIMAL:
            return parser_single_result_ok(positioned_convert(current, node_value_decimal(string_clone(tok->data.decimal.data))));
        case TOKEN_STRING:
            return parser_single_result_ok(positioned_convert(current, node_value_string(string_clone(tok->data.string.data))));
        case TOKEN_IDENTIFIER:
            return parser_parse_function_call(self, tok->data.identifier.data);
        default: RETURN_UNEXPECTED_TOKEN_ERR(current, tok, "EXPR0");
    }
}

ParserSingleResult parser_parse_function_call(Parser* self, String* name) {
    Positioned* init = parser_get_current(self);

    Positioned* current = parser_peek(self, 1);
    if (current == NULL) return parser_single_result_ok(positioned_convert(init, node_variable_call(name)));
    Token* tok = current->data;

    if (tok->type != TOKEN_LEFT_PARENTHESIS) return parser_single_result_ok(positioned_convert(init, node_variable_call(name)));
    parser_advance_x(self, 2);
    EXPECT_CURRENT_SHOULD_BE(current, tok, ")");

    List* params = list_create();
    while (tok->type != TOKEN_RIGHT_PARENTHESIS) {
        if (params->length != 0) {
            EXPECT_TOKEN_SHOULD_BE(current, tok, TOKEN_COMMA, ",");
            parser_advance(self);
        }
        Positioned* value;
        ParserSingleResult res;
        UNWRAP_SINGLE(parser_parse_expr2(self), res, value);
        list_push(params, value);
        EXPECT_CURRENT_SHOULD_BE(current, tok, ")");
    }
    Position end = current->end;

    return parser_single_result_ok( positioned_create(node_function_call(name, params), init->start, end));
}

ParserSingleResult parser_parse_expr1(Parser* self) {
    ParserSingleResult res;
    Positioned* left;
    UNWRAP_SINGLE(parser_parse_expr0(self), res, left);

    while (true) {
        Positioned* current = parser_get_next(self);
        if (current == NULL) break;
        Token* tok = current->data;

        // Get the operator
        NodeBinaryOperationType op;
        if (tok->type == TOKEN_STAR) {
            op = NODE_BINARY_MUL;
        } else if (tok->type == TOKEN_SLASH) {
            op = NODE_BINARY_DIV;
        } else break;
        parser_advance(self);

        // Get right hand side
        Positioned* right;
        UNWRAP_SINGLE(parser_parse_expr0(self), res, right);

        left = positioned_create(node_binary_operation(left, op, right), left->start, right->end);
    }

    return parser_single_result_ok(left);
}

ParserSingleResult parser_parse_expr2(Parser* self) {
    ParserSingleResult res;
    Positioned* left;
    UNWRAP_SINGLE(parser_parse_expr1(self), res, left);

    while (true) {
        Positioned* current = parser_get_current(self);
        if (current == NULL) break;
        Token* tok = current->data;

        // Get the operator
        NodeBinaryOperationType op;
        if (tok->type == TOKEN_PLUS) {
            op = NODE_BINARY_PLUS;
        } else if (tok->type == TOKEN_MINUS) {
            op = NODE_BINARY_MINUS;
        } else break;
        parser_advance(self);

        // Get right hand side
        Positioned* right;
        UNWRAP_SINGLE(parser_parse_expr1(self), res, right);

        left = positioned_create(node_binary_operation(left, op, right), left->start, right->end);
    }

    return parser_single_result_ok(left);
}

ParserSingleResult parser_parse_variable_definition(Parser* self, Position start, NodeVariableDefinitionType var_type) {
    parser_advance(self);
    Positioned* current;
    Token* current_token;

    // Get Identifier
    EXPECT_TOKEN_SHOULD_BE(current, current_token, TOKEN_IDENTIFIER, "IDENTIFIER");
    String* name = current_token->data.identifier.data;
    Position end = current->end;

    current = parser_get_next(self);

    // Get Type (?)
    String* type = NULL;
    if (current != NULL && ((Token*) current->data)->type == TOKEN_COLON) {
        parser_advance(self);
        EXPECT_TOKEN_SHOULD_BE(current, current_token, TOKEN_IDENTIFIER, "IDENTIFIER");
        type = current_token->data.identifier.data;
        parser_advance(self);
        current = parser_get_current(self);
    }

    // Get Value (?)
    Positioned* value = NULL;
    parser_advance(self);
    if (current != NULL && ((Token*) current->data)->type == TOKEN_EQUAL) {
        ParserSingleResult res = parser_parse_expr2(self);
        if (res.type == PARSER_ERROR) return res;
        else value = res.data.node;
        end = res.data.node->end;
    }

    return parser_single_result_ok(positioned_create(node_variable_definition(var_type, name, type, value), start, end));
}

ParserSingleResult parser_parse_funciton_definition(Parser* self, Position start, bool external) {
    parser_advance(self);
    Positioned* current;
    Token* current_token;

    // Get Identifier
    EXPECT_TOKEN_SHOULD_BE(current, current_token, TOKEN_IDENTIFIER, "IDENTIFIER");
    String* name = current_token->data.identifier.data;
    Position end;
    parser_advance(self);

    // Get Parameters
    EXPECT_TOKEN_SHOULD_BE(current, current_token, TOKEN_LEFT_PARENTHESIS, "(");
    parser_advance(self);
    List* params = list_create();
    EXPECT_CURRENT_SHOULD_BE(current, current_token, ")");
    while (current_token->type != TOKEN_RIGHT_PARENTHESIS) {
        if (params->length != 0) {
            if (current_token->type != TOKEN_COMMA) break;
            parser_advance(self);
        }
        // Get Parameter Name
        EXPECT_TOKEN_SHOULD_BE(current, current_token, TOKEN_IDENTIFIER, "IDENTIFIER");
        String* param_name = current_token->data.identifier.data;
        Position param_start = current->start;
        parser_advance(self);
        // Get Parameter Type
        EXPECT_TOKEN_SHOULD_BE(current, current_token, TOKEN_COLON, ":");
        parser_advance(self);
        EXPECT_TOKEN_SHOULD_BE(current, current_token, TOKEN_IDENTIFIER, "IDENTIFIER");
        String* param_type = current_token->data.identifier.data;
        Position param_end = current->end;
        Positioned* param = positioned_create(node_function_parameter_create(param_name, param_type), param_start, param_end);
        list_push(params, param);
        parser_advance(self);
        EXPECT_CURRENT_SHOULD_BE(current, current_token, ")");
    }
    parser_advance(self);

    current = parser_get_current(self);

    String* type = NULL;
    if (current == NULL) {
        if (!external) RETURN_EOF_ERR("Unexpected EOF, should be =>");
    } else {
        current_token = current->data;
        // Get Type
        if (current_token->type == TOKEN_COLON) {
            parser_advance(self);
            EXPECT_TOKEN_SHOULD_BE(current, current_token, TOKEN_IDENTIFIER, "IDENTIFIER");
            type = current_token->data.identifier.data;
            parser_advance(self);
        }
    }

    List* body = NULL;
    if (!external) {
        // Get Body
        EXPECT_TOKEN_SHOULD_BE(current, current_token, TOKEN_RIGHT_ARROW, "=>");
        end = current->end;
        body = list_create();
        current = parser_get_next(self);
        while (current != NULL) {
            current_token = current->data;
            if (current_token->type == TOKEN_NEW_LINE) {
                current = parser_get_next(self);
                continue;
            }
            if (current_token->type != TOKEN_TAB) break;
            parser_advance(self);
            ParserSingleResult res;
            Positioned* node;
            UNWRAP_SINGLE(parser_parse_current(self), res, node);
            end = node->end;
            list_push(body,node);
            current = parser_get_current(self);
        }
    }

    return parser_single_result_ok(positioned_create(node_function_definition(name, params, type, body), start, end));
}

ParserSingleResult parser_parse_return(Parser* self, Position start) {
    parser_advance(self);
    ParserSingleResult res;
    Positioned* expr;
    UNWRAP_SINGLE(parser_parse_expr2(self), res, expr);
    Position end = expr->end;
    return parser_single_result_ok(positioned_create(node_return(expr), start, end));
}

ParserSingleResult parser_parse_include(Parser* self, Position start) {
    parser_advance(self);
    Positioned* current;
    Token* current_token;
    EXPECT_TOKEN_SHOULD_BE(current, current_token, TOKEN_STRING, "String");
    Position end = current->end;
    parser_advance(self);
    return parser_single_result_ok(positioned_create(node_include(string_clone(current_token->data.string.data)), start, end));
}

ParserSingleResult parser_handle_keyword(Parser* self) {
    Positioned* current;
    Token* current_token;
    EXPECT_TOKEN_SHOULD_BE(current, current_token, TOKEN_KEYWORD, "--001-Keyword-");

    switch (current_token->data.keyword) {
        case TOKEN_KEYWORD_FN: return parser_parse_funciton_definition(self, current->start, false);
        case TOKEN_KEYWORD_CONST: {
            ParserSingleResult res = parser_parse_variable_definition(self, current->start, NODE_VARIABLE_CONST);
            if (res.type == PARSER_ERROR) return res;
            EXPECT_EOS(current, current_token)
            return res;
        }
        case TOKEN_KEYWORD_VAR: {
            ParserSingleResult res = parser_parse_variable_definition(self, current->start, NODE_VARIABLE_VAR);
            if (res.type == PARSER_ERROR) return res;
            EXPECT_EOS(current, current_token)
            return res;
        }
        case TOKEN_KEYWORD_RETURN: {
            ParserSingleResult res = parser_parse_return(self, current->start);
            if (res.type == PARSER_ERROR) return res;
            EXPECT_EOS(current, current_token)
            return res;
        }
        case TOKEN_KEYWORD_EXTERN: {
            Position start = current->start;
            parser_advance(self);
            EXPECT_KEYWORD_SHOULD_BE(current, current_token, TOKEN_KEYWORD_FN, "fn");
            return parser_parse_funciton_definition(self, start, true);
        }
        case TOKEN_KEYWORD_INCLUDE: {
            ParserSingleResult res = parser_parse_include(self, current->start);
            if (res.type == PARSER_ERROR) return res;
            EXPECT_EOS(current, current_token)
            return res;
        }
    }

}

ParserSingleResult parser_parse_current(Parser* self) {
    Positioned* current = parser_get_current(self);
    if (current == NULL) {
        return parser_single_result_err(parser_error_create(PARSER_UNEXPECTED_EOF, string_from_const("Unexpected EOF!")));
    } else {
        Token* tok = current->data;
        switch (tok->type) {
            case TOKEN_DECIMAL:
            case TOKEN_STRING:
            case TOKEN_IDENTIFIER: {
                ParserSingleResult res = parser_parse_expr2(self);
                if (res.type == PARSER_ERROR) return res;
                EXPECT_EOS(current, tok)
                return res;
            }
            case TOKEN_KEYWORD: return parser_handle_keyword(self);
            case TOKEN_TAB:
            case TOKEN_NEW_LINE:
                parser_advance(self);
                return parser_parse_current(self);
            default: RETURN_UNEXPECTED_TOKEN_ERR(current, tok, "?");
        }
    }
}

ParserResult parser_parse(Parser* self) {
    List* nodes = list_create();

    while (true) {
        Positioned * current = parser_get_current(self);
        if (current == NULL) break;
        ParserSingleResult res = parser_parse_current(self);
        if (res.type == PARSER_ERROR) return parser_single_result_to_result(res);
        list_push(nodes, res.data.node);
    }

    return parser_result_ok(nodes);
}
