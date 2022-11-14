//
// Created by remy on 11/8/22.
//

#include "node.h"
#include <stdio.h>

NodeFunctionParameter* node_function_parameter_create(String* name, String* type) {
    NodeFunctionParameter* self = (NodeFunctionParameter*) malloc(sizeof(NodeFunctionParameter));
    self->name = name;
    self->type = type;
    return self;
}

void node_function_parameter_destroy(NodeFunctionParameter* self) {
    string_destroy(self->name);
    string_destroy(self->type);
    free(self);
}

Node* node_empty(NodeType type) {
    Node* self = (Node*) malloc(sizeof(Node));
    self->type = type;
    return self;
}

Node* node_binary_operation(Positioned* lhs, NodeBinaryOperationType operation, Positioned* rhs) {
    Node* self = node_empty(NODE_BINARY_OPERATION);
    self->data.binary_operation.lhs = lhs;
    self->data.binary_operation.operation_type = operation;
    self->data.binary_operation.rhs = rhs;
    return self;
}

Node* node_value_string(String* value) {
    Node* self = node_empty(NODE_VALUE);
    self->data.node_value.type = NODE_VALUE_STRING;
    self->data.node_value.data.string = value;
    return self;
}

Node* node_value_decimal(String* value) {
    Node* self = node_empty(NODE_VALUE);
    self->data.node_value.type = NODE_VALUE_DECIMAL;
    self->data.node_value.data.string = value;
    return self;
}

Node* node_variable_call(String* value) {
    Node* self = node_empty(NODE_VALUE);
    self->data.node_value.type = NODE_VARIABLE_CALL;
    self->data.node_value.data.variable_call = value;
    return self;
}

Node* node_variable_definition(NodeVariableDefinitionType var_type, String* name, String* type, Positioned* value) {
    Node* self = node_empty(NODE_VARIABLE_DEFINITION);
    self->data.variable_definition.var_type = var_type;
    self->data.variable_definition.name = name;
    self->data.variable_definition.type = type;
    self->data.variable_definition.value = value;
    return self;
}

Node* node_function_definition(String* name, List* parameters, String* type, List* body) {
    Node* self = node_empty(NODE_FUNCTION_DEFINITION);
    self->data.function_definition.name = name;
    self->data.function_definition.parameters = parameters;
    self->data.function_definition.type = type;
    self->data.function_definition.body = body;
    return self;
}

Node* node_return(Positioned* expr) {
    Node* self = node_empty(NODE_RETURN);
    self->data.node_return.expr = expr;
    return self;
}

Node* node_function_call(String* name, List* parameters) {
    Node* self = node_empty(NODE_FUNCTION_CALL);
    self->data.function_call.name = name;
    self->data.function_call.parameters = parameters;
    return self;
}

Node* node_include(String* path) {
    Node* self = node_empty(NODE_INCLUDE);
    self->data.node_include.path = path;
    return self;
}

String* node_to_string(Node* self) {
    String* buf = string_empty();

    switch (self->type) {
        case NODE_BINARY_OPERATION: {
            string_push(buf, '(');
            String* lhs_str = node_to_string(self->data.binary_operation.lhs->data);
            string_push_string(buf, lhs_str);
            string_destroy(lhs_str);

            switch (self->data.binary_operation.operation_type) {
                case NODE_BINARY_PLUS:
                    string_push_const(buf, " + ");
                    break;
                case NODE_BINARY_MINUS:
                    string_push_const(buf, " - ");
                    break;
                case NODE_BINARY_MUL:
                    string_push_const(buf, " * ");
                    break;
                case NODE_BINARY_DIV:
                    string_push_const(buf, " / ");
                    break;
            }

            String* rhs_str = node_to_string(self->data.binary_operation.rhs->data);
            string_push_string(buf, rhs_str);
            string_destroy(rhs_str);
            string_push(buf, ')');
        } break;
        case NODE_VALUE:
            switch (self->data.node_value.type) {
                case NODE_VALUE_DECIMAL:
                    string_push_string(buf, self->data.node_value.data.decimal);
                    break;
                case NODE_VALUE_STRING:
                    string_push(buf, '"');
                    string_push_string(buf, self->data.node_value.data.string);
                    string_push(buf, '"');
                    break;
                case NODE_VARIABLE_CALL:
                    string_push_string(buf, self->data.node_value.data.variable_call);
                    break;
            }
            break;
        case NODE_VARIABLE_DEFINITION:
            switch (self->data.variable_definition.var_type) {
                case NODE_VARIABLE_VAR:
                    string_push_const(buf, "var ");
                    break;
                case NODE_VARIABLE_CONST:
                    string_push_const(buf, "const ");
                    break;
            }
            string_push_string(buf, self->data.variable_definition.name);
            if (self->data.variable_definition.type != NULL) {
                string_push_const(buf, ": ");
                string_push_string(buf, self->data.variable_definition.type);
            }
            if (self->data.variable_definition.value != NULL) {
                string_push_const(buf, " = ");
                String* node_str = node_to_string(self->data.variable_definition.value->data);
                string_push_string(buf, node_str);
                string_destroy(node_str);
            }
            break;
        case NODE_FUNCTION_DEFINITION:
            if (self->data.function_definition.body == NULL) {
                string_push_const(buf, "extern ");
            }
            string_push_const(buf, "fn ");

            string_push_string(buf, self->data.function_definition.name);

            string_push(buf, '(');
            uintptr_t i = 0;
            while (i < self->data.function_definition.parameters->length) {
                if (i != 0) string_push_const(buf, ", ");
                Positioned* pos = self->data.function_definition.parameters->ptr[i];
                NodeFunctionParameter* param = pos->data;
                string_push_string(buf, param->name);
                if (param->type != NULL) {
                    string_push_const(buf, ": ");
                    string_push_string(buf, param->type);
                }
                i += 1;
            }
            string_push(buf, ')');

            if (self->data.function_definition.type != NULL) {
                string_push_const(buf, ": ");
                string_push_string(buf, self->data.function_definition.type);
            }

            if (self->data.function_definition.body != NULL) {
                string_push_const(buf, " =>\n");
                i = 0;
                while (i < self->data.function_definition.body->length) {
                    Positioned* pos = self->data.function_definition.body->ptr[i];
                    String* str = node_to_string(pos->data);
                    List* str_split = string_lines(str);
                    uintptr_t j = 0;
                    while (j < str_split->length) {
                        if (i != 0 || j != 0) string_push(buf, '\n');
                        string_push(buf, '\t');
                        string_push_string(buf, str_split->ptr[j]);
                        string_destroy(str_split->ptr[j]);
                        j += 1;
                    }
                    list_destroy(str_split);
                    string_destroy(str);

                    i += 1;
                }
            }

            break;
        case NODE_RETURN:
            string_push_format(buf, "return {sf}", node_to_string(self->data.node_return.expr->data));
            break;
        case NODE_FUNCTION_CALL:
            string_push_string(buf, self->data.function_call.name);
            string_push(buf, '(');
            i = 0;
            while (i < self->data.function_call.parameters->length) {
                if (i != 0) string_push_const(buf, ", ");
                Positioned* pos = self->data.function_call.parameters->ptr[i];
                String* str = node_to_string(pos->data);
                string_push_string(buf, str);
                string_destroy(str);
                i += 1;
            }
            string_push(buf, ')');
            break;
        case NODE_INCLUDE:
            string_push_format(buf, "include \"{s}\"", self->data.node_include.path);
            break;
    }

    return buf;
}

void node_destroy(Node* self) {
    switch (self->type) {
        case NODE_BINARY_OPERATION:
            node_destroy(self->data.binary_operation.lhs->data);
            positioned_destroy(self->data.binary_operation.lhs);
            node_destroy(self->data.binary_operation.rhs->data);
            positioned_destroy(self->data.binary_operation.rhs);
            break;
        case NODE_VALUE:
            switch (self->data.node_value.type) {
                case NODE_VALUE_DECIMAL:
                    string_destroy(self->data.node_value.data.decimal);
                    break;
                case NODE_VALUE_STRING:
                    string_destroy(self->data.node_value.data.string);
                    break;
                case NODE_VARIABLE_CALL:
                    string_destroy(self->data.node_value.data.variable_call);
                    break;
            }
            break;
        case NODE_VARIABLE_DEFINITION:
            if (self->data.variable_definition.value != NULL) {
                node_destroy(self->data.variable_definition.value->data);
                positioned_destroy(self->data.variable_definition.value);
            }
            if (self->data.variable_definition.type != NULL) {
                string_destroy(self->data.variable_definition.type);
            }
            string_destroy(self->data.variable_definition.name);
            break;
        case NODE_FUNCTION_DEFINITION:
            string_destroy(self->data.function_definition.name);
            if (self->data.function_definition.type != NULL) {
                string_destroy(self->data.function_definition.type);
            }
            uintptr_t i = 0;
            while (i < self->data.function_definition.parameters->length) {
                Positioned* pos = self->data.function_definition.parameters->ptr[i];
                NodeFunctionParameter* param = pos->data;
                node_function_parameter_destroy(param);
                positioned_destroy(pos);
                i += 1;
            }
            list_destroy(self->data.function_definition.parameters);
            if (self->data.function_definition.body != NULL) {
                i = 0;
                while (i < self->data.function_definition.body->length) {
                    Positioned* pos = self->data.function_definition.body->ptr[i];
                    node_destroy(pos->data);
                    positioned_destroy(pos);
                    i += 1;
                }
                list_destroy(self->data.function_definition.body);
            }
            break;
        case NODE_RETURN:
            node_destroy(self->data.node_return.expr->data);
            positioned_destroy(self->data.node_return.expr);
            break;
        case NODE_FUNCTION_CALL:
            string_destroy(self->data.function_call.name);
            i = 0;
            while (i < self->data.function_call.parameters->length) {
                Positioned* pos = self->data.function_call.parameters->ptr[i];
                node_destroy(pos->data);
                positioned_destroy(pos);
                i += 1;
            }
            list_destroy(self->data.function_call.parameters);
            break;
        case NODE_INCLUDE:
            string_destroy(self->data.node_include.path);
            break;
    }
    free(self);
}
