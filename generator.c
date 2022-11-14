//
// Created by remy on 11/13/22.
//

#include "generator.h"
#include <stdio.h>

GeneratedProject* generated_project_create() {
    GeneratedProject* self = (GeneratedProject*) malloc(sizeof(GeneratedProject));
    self->files = list_create();
    return self;
}

GeneratedFile* generated_project_get_file(GeneratedProject* self, GeneratedFileType type, const char* name) {
    uintptr_t i = 0;
    while (i < self->files->length) {
        GeneratedFile* file = self->files->ptr[i];
        if (file->type == type && string_equal_const(file->name,name)) {
            return file;
        }
        i += 1;
    }
    GeneratedFile* file = (GeneratedFile*) malloc(sizeof(GeneratedFile));
    file->type = type;
    file->name = string_from_const(name);
    file->src = string_empty();
    list_push(self->files, file);
    return file;
}

void generated_project_destroy(GeneratedProject* self) {
    uintptr_t i = 0;
    while (i < self->files->length) {
        GeneratedFile* file = self->files->ptr[i];
        string_destroy(file->name);
        string_destroy(file->src);
        free(file);
        i += 1;
    }
    list_destroy(self->files);
    free(self);
}

GeneratorError generator_error_create(GeneratorErrorType type, String* message) {
    GeneratorError self;
    self.type = type;
    self.message = message;
    return self;
}

void generator_error_destroy(GeneratorError* self) {
    string_destroy(self->message);
}

GeneratorResult generator_result_ok(GeneratedProject* project) {
    GeneratorResult self;
    self.type = GENERATOR_OK;
    self.data.project = project;
    return self;
}

GeneratorResult generator_result_err(GeneratorError error) {
    GeneratorResult self;
    self.type = GENERATOR_ERROR;
    self.data.error = error;
    return self;
}

void generator_result_destroy(GeneratorResult* self) {
    if (self->type == GENERATOR_ERROR) {
        generator_error_destroy(&self->data.error);
    } else {
        generated_project_destroy(self->data.project);
    }
}

GeneratorStringResult generator_string_result_ok(String* str) {
    GeneratorStringResult self;
    self.type = GENERATOR_OK;
    self.data.str = str;
    return self;
}
GeneratorStringResult generator_string_result_err(GeneratorError error) {
    GeneratorStringResult self;
    self.type = GENERATOR_ERROR;
    self.data.error = error;
    return self;
}

void generator_string_result_destroy(GeneratorStringResult* self) {
    if (self->type == GENERATOR_ERROR) generator_error_destroy(&self->data.error);
    string_destroy(self->data.str);
}

GeneratorResult generator_string_result_to_result(GeneratorStringResult* self) {
    if (self->type == GENERATOR_ERROR) generator_result_err(self->data.error);
    else generator_result_ok(generated_project_create());
}

GeneratorStringResult generator_generate_current(Generator* self, Positioned* current);
bool generator_should_have_semicolon(Node* node);

Generator generator_create(String* src, List* ast) {
    Generator self;
    self.src = src;
    self.ast = ast;
    self.index = 0;
    self.project = generated_project_create();
    return self;
}

Positioned* generator_get_current(Generator* self) {
    if (self->index >= self->ast->length) return NULL;
    return self->ast->ptr[self->index];
}

void generator_advance(Generator* self) {
    self->index += 1;
}

GeneratorStringResult generator_generate_type(Generator* self, String* type) {
    if (type == NULL) return generator_string_result_ok(string_from_const("void"));
    else return generator_string_result_ok(string_clone(type));
}

GeneratorStringResult generator_generate_function_definition(Generator* self, Positioned* current) {
    Node* node = current->data;

    String* buf = string_empty();

    // Type
    GeneratorStringResult res = generator_generate_type(self, node->data.function_definition.type);
    if (res.type == GENERATOR_ERROR) return res;
    string_push_string_free(buf, res.data.str);

    // Name
    string_push(buf, ' ');
    string_push_string(buf, node->data.function_definition.name);

    // Parameters
    string_push(buf, '(');
    uintptr_t i = 0;
    while (i < node->data.function_definition.parameters->length) {
        NodeFunctionParameter* param = node->data.function_definition.parameters->ptr[i];
        if (i != 0) string_push_const(buf, ", ");
        res = generator_generate_type(self, param->type);
        if (res.type == GENERATOR_ERROR) return res;
        string_push_string_free(buf, res.data.str);
        string_push(buf, ' ');
        string_push_string(buf, param->name);
        i += 1;
    }
    string_push(buf, ')');

    // Body
    string_push_const(buf, " {");
    i = 0;
    printf("e\n");
    while (i < node->data.function_definition.body->length) {
        printf("a\n");
        Positioned* pos = node->data.function_definition.body->ptr[i];
        if (i == 0) string_push(buf, '\n');
        string_push(buf, '\t');
        res = generator_generate_current(self, pos);
        if (res.type == GENERATOR_ERROR) return res;
        string_push_string_free(buf, res.data.str);
        if (generator_should_have_semicolon(pos->data)) {
            string_push(buf, ';');
        }
        string_push(buf, '\n');
        i += 1;
        printf("b\n");
    }
    printf("c\n");

    string_push(buf, '}');

    string_push(buf, '\n');

    return generator_string_result_ok(buf);
}

GeneratorStringResult generator_generate_include(Generator* self, Positioned* current) {
    String* buf = string_empty();
    Node* node = current->data;

    string_push_const(buf, "#include ");
    if (string_start_with_const(node->data.node_include.path, "std-")) {
        string_push(buf, '<');
        string_push_string_free(buf, string_slice(node->data.node_include.path, 4, -1));
        string_push_const(buf, ".h");
        string_push(buf, '>');
    } else {
        string_push(buf, '"');
        string_push_string(buf, node->data.node_include.path);
        string_push_const(buf, ".h");
        string_push(buf, '"');
    }

    return generator_string_result_ok(buf);
}

GeneratorStringResult generator_generate_function_call(Generator* self, Positioned* current) {
    String* buf = string_empty();
    Node* node = current->data;

    string_push_string(buf, node->data.function_call.name);

    // Parameters
    string_push(buf, '(');
    uintptr_t i = 0;
    while (i < node->data.function_call.parameters->length) {
        if (i != 0) string_push_const(buf, ", ");
        Positioned* pos = node->data.function_call.parameters->ptr[i];
        GeneratorStringResult res = generator_generate_current(self, pos);
        if (res.type == GENERATOR_ERROR) return res;
        string_push_string_free(buf, res.data.str);
        i += 1;
    }
    string_push(buf, ')');

    return generator_string_result_ok(buf);
}

GeneratorStringResult generator_generate_value(Generator* self, Positioned* current) {
    String* buf = string_empty();
    Node* node = current->data;

    switch (node->data.node_value.type) {
        case NODE_VALUE_DECIMAL:
            string_push_string(buf, node->data.node_value.data.decimal);
            break;
        case NODE_VALUE_STRING:
            string_push(buf, '"');
            string_push_string(buf, node->data.node_value.data.string);
            string_push(buf, '"');
            break;
        case NODE_VARIABLE_CALL:
            string_push_string(buf, node->data.node_value.data.variable_call);
            break;
    }

    return generator_string_result_ok(buf);
}

GeneratorStringResult generator_generate_current(Generator* self, Positioned* current) {
    if (current == NULL) {
        return generator_string_result_err(generator_error_create(GENERATOR_UNEXPECTED_EOF, string_from_const("Unexpected EOF!")));
    }

    Node* node = current->data;
    switch (node->type) {
        case NODE_BINARY_OPERATION:
            break;
        case NODE_VALUE: return generator_generate_value(self, current);
        case NODE_VARIABLE_DEFINITION:
            break;
        case NODE_FUNCTION_DEFINITION: return generator_generate_function_definition(self, current);
        case NODE_RETURN:
            break;
        case NODE_FUNCTION_CALL: return generator_generate_function_call(self, current);
        case NODE_INCLUDE: return generator_generate_include(self, current);
    }
}

bool generator_should_have_semicolon(Node* node) {
    switch (node->type) {
        case NODE_BINARY_OPERATION:
        case NODE_VALUE:
        case NODE_VARIABLE_DEFINITION:
        case NODE_RETURN:
        case NODE_FUNCTION_CALL: return true;
        case NODE_FUNCTION_DEFINITION:
        case NODE_INCLUDE: return false;
    }
}

GeneratorResult generator_generate(Generator* self) {
    Positioned* current = generator_get_current(self);
    while (current != NULL) {
        GeneratorStringResult res = generator_generate_current(self, current);
        if (res.type == GENERATOR_ERROR) return generator_string_result_to_result(&res);

        GeneratedFile* file = generated_project_get_file(self->project, GENERATED_FILE_C, "main");
        string_push_string_free(file->src, res.data.str);
        if (generator_should_have_semicolon(current->data)) {
            string_push(file->src, ';');
        }
        string_push(file->src, '\n');

        generator_advance(self);

        current = generator_get_current(self);
    }

    return generator_result_ok(self->project);
}