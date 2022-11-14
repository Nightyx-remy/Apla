//
// Created by remy on 11/6/22.
//

#ifndef APLA_COMPILER_LEXER_H
#define APLA_COMPILER_LEXER_H

#include "str.h"
#include "list.h"
#include "token.h"
#include "position.h"
#include <stdint.h>
#include <ctype.h>

typedef enum LexerErrorTypeT {
    LEXER_UNEXPECTED_EOF,
    LEXER_UNEXPECTED_CHAR
} LexerErrorType;

typedef struct LexerErrorT {
    LexerErrorType type;
    String* message;
} LexerError;

LexerError lexer_error_create(LexerErrorType type, String* message);

void lexer_error_destroy(LexerError* self);

typedef enum LexerResultTypeT {
    LEXER_ERROR,
    LEXER_OK
} LexerResultType;

typedef union LexerResultDataT {
    LexerError error;
    List* tokens;
} LexerResultData;

typedef struct LexerResultT {
    LexerResultType type;
    LexerResultData data;
} LexerResult;

LexerResult lexer_result_ok(List* tokens);
LexerResult lexer_result_err(LexerError error);

void lexer_result_destroy(LexerResult* self);

typedef union LexerSingleResultDataT {
    LexerError error;
    Positioned* pos;
} LexerSingleResultData;

typedef struct LexerSingleResultT {
    LexerResultType type;
    LexerSingleResultData data;
} LexerSingleResult;

LexerSingleResult lexer_single_result_ok(Positioned* pos);
LexerSingleResult lexer_single_result_err(LexerError error);

LexerResult lexer_single_result_to_result(LexerSingleResult self);

typedef struct LexerT {
    String* src;
    Position pos;
} Lexer;

Lexer lexer_create(String* src);

LexerResult lexer_tokenize(Lexer* self);

#endif //APLA_COMPILER_LEXER_H
