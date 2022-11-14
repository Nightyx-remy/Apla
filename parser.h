//
// Created by remy on 11/9/22.
//

#ifndef APLA_COMPILER_PARSER_H
#define APLA_COMPILER_PARSER_H

#include "list.h"
#include "str.h"
#include "node.h"
#include "token.h"
#include "stdio.h"
#include "position.h"

typedef enum ParserErrorTypeT {
    PARSER_UNEXPECTED_EOF,
    PARSER_UNEXPECTED_TOKEN,
} ParserErrorType;

typedef struct ParserErrorT {
    ParserErrorType type;
    String* message;
} ParserError;

ParserError parser_error_create(ParserErrorType type, String* message);

void parser_error_destroy(ParserError* self);

typedef enum ParserResultTypeT {
    PARSER_ERROR,
    PARSER_OK,
} ParserResultType;

typedef union ParserResultDataT {
    ParserError error;
    List* ast;
} ParserResultData;

typedef struct ParserResultT {
    ParserResultType type;
    ParserResultData data;
} ParserResult;

ParserResult parser_result_ok(List* ast);
ParserResult parser_result_err(ParserError error);

void parser_result_destroy(ParserResult* self);

typedef union ParserSingleResultDataT {
    ParserError error;
    Positioned* node;
} ParserSingleResultData;

typedef struct ParserSingleResultT {
    ParserResultType type;
    ParserSingleResultData data;
} ParserSingleResult;

ParserSingleResult parser_single_result_ok(Positioned* node);
ParserSingleResult parser_single_result_err(ParserError error);

ParserResult parser_single_result_to_result(ParserSingleResult self);

typedef struct ParserT {
    String* src;
    List* tokens;
    uintptr_t index;
} Parser;

Parser parser_create(String* src, List* tokens);
ParserResult parser_parse(Parser* self);

#endif //APLA_COMPILER_PARSER_H
