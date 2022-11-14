//
// Created by remy on 11/13/22.
//

#ifndef APLA_COMPILER_GENERATOR_H
#define APLA_COMPILER_GENERATOR_H

#include "str.h"
#include "list.h"
#include "position.h"
#include "node.h"

typedef enum GeneratedFileTypeT {
    GENERATED_FILE_HEADER,
    GENERATED_FILE_C,
} GeneratedFileType;

typedef struct GeneratedFileT {
    GeneratedFileType type;
    String* name;
    String* src;
} GeneratedFile;

typedef struct GeneratedProjectT {
    List* files;
} GeneratedProject;

GeneratedProject* generated_project_create();
GeneratedFile* generated_project_get_file(GeneratedProject* self, GeneratedFileType type, const char* name);
void generated_project_destroy(GeneratedProject* self);

typedef enum GeneratorErrorTypeT {
    GENERATOR_UNEXPECTED_EOF,
    GENERATOR_UNEXPECTED_NODE
} GeneratorErrorType;

typedef struct GeneratorErrorT {
    GeneratorErrorType type;
    String* message;
} GeneratorError;

GeneratorError generator_error_create(GeneratorErrorType type, String* message);
void generator_error_destroy(GeneratorError* self);

typedef enum GeneratorResultTypeT {
    GENERATOR_ERROR,
    GENERATOR_OK
} GeneratorResultType;

typedef union GeneratorResultDataT {
    GeneratorError error;
    GeneratedProject* project;
} GeneratorResultData;

typedef struct GeneratorResultT {
    GeneratorResultType type;
    GeneratorResultData data;
} GeneratorResult;

GeneratorResult generator_result_ok(GeneratedProject* project);
GeneratorResult generator_result_err(GeneratorError error);

void generator_result_destroy(GeneratorResult* self);

typedef union GeneratorStringResultDataT {
    GeneratorError error;
    String* str;
} GeneratorStringResultData;

typedef struct GeneratorStringResultT {
    GeneratorResultType type;
    GeneratorStringResultData data;
} GeneratorStringResult;

GeneratorStringResult generator_string_result_ok(String* str);
GeneratorStringResult generator_string_result_err(GeneratorError error);

GeneratorResult generator_string_result_to_result(GeneratorStringResult* self);

void generator_string_result_destroy(GeneratorStringResult* self);

typedef struct GeneratorT {
    String* src;
    List* ast;
    uintptr_t index;
    GeneratedProject* project;
} Generator;

Generator generator_create(String* src, List* ast);
GeneratorResult generator_generate(Generator* self);

#endif //APLA_COMPILER_GENERATOR_H
