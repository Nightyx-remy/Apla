#ifndef APLA_COMPILER_NODE_H
#define APLA_COMPILER_NODE_H

#include "str.h"
#include "position.h"

typedef enum NodeTypeT {
    NODE_BINARY_OPERATION,
    NODE_VALUE,
    NODE_VARIABLE_DEFINITION,
    NODE_FUNCTION_DEFINITION,
    NODE_RETURN,
    NODE_FUNCTION_CALL,
    NODE_INCLUDE
} NodeType;

typedef enum NodeBinaryOperationTypeT {
    NODE_BINARY_PLUS,
    NODE_BINARY_MINUS,
    NODE_BINARY_MUL,
    NODE_BINARY_DIV
} NodeBinaryOperationType;

typedef struct NodeBinaryOperationT {
    Positioned* lhs;
    NodeBinaryOperationType operation_type;
    Positioned* rhs;
} NodeBinaryOperation;

typedef enum NodeValueTypeT {
    NODE_VALUE_DECIMAL,
    NODE_VALUE_STRING,
    NODE_VARIABLE_CALL
} NodeValueType;

typedef union NodeValueDataT {
    String* decimal;
    String* string;
    String* variable_call;
} NodeValueData;

typedef struct NodeValueT {
    NodeValueType type;
    NodeValueData data;
} NodeValue;

typedef enum NodeVariableDefinitionTypeT {
    NODE_VARIABLE_VAR,
    NODE_VARIABLE_CONST
} NodeVariableDefinitionType;

typedef struct NodeVariableDefinitionT {
    NodeVariableDefinitionType var_type; // TODO: Change to positioned (later, currently having issue with pointers to int)
    String* name;
    String* type;
    Positioned* value;
} NodeVariableDefinition;

typedef struct NodeFunctionParameterT {
    String* name;
    String* type;
} NodeFunctionParameter;

NodeFunctionParameter* node_function_parameter_create(String* name, String* type);
void node_function_parameter_destroy(NodeFunctionParameter* self);

typedef struct NodeFunctionDefinitionT {
    String* name;
    List* parameters; // List[NodeFunctionParameter]
    String* type;
    List* body; // List[Positioned[Node]]
} NodeFunctionDefinition;

typedef struct NodeReturnT {
    Positioned* expr;
} NodeReturn;

typedef struct NodeFunctionCallT {
    String* name;
    List* parameters; // List[Positioned[Node]
} NodeFunctionCall;

typedef struct NodeIncludeT {
    String* path;
} NodeInclude;

typedef union NodeDataT {
    NodeBinaryOperation binary_operation;
    NodeValue node_value;
    NodeVariableDefinition variable_definition;
    NodeFunctionDefinition function_definition;
    NodeReturn node_return;
    NodeFunctionCall function_call;
    NodeInclude node_include;
    void* empty;
} NodeData;

typedef struct NodeT {
    NodeType type;
    NodeData data;
} Node;

Node* node_binary_operation(Positioned* lhs, NodeBinaryOperationType operation, Positioned* rhs);
Node* node_value_string(String* value);
Node* node_value_decimal(String* value);
Node* node_variable_call(String* value);
Node* node_variable_definition(NodeVariableDefinitionType var_type, String* name, String* type, Positioned* value);
Node* node_function_definition(String* name, List* parameters, String* type, List* body);
Node* node_return(Positioned* expr);
Node* node_function_call(String* name, List* parameters);
Node* node_include(String* path);

String* node_to_string(Node* self);

void node_destroy(Node* self);

#endif //APLA_COMPILER_NODE_H
