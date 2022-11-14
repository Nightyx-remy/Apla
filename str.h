#ifndef APLA_COMPILER_STR_H
#define APLA_COMPILER_STR_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include "utils.h"
#include "list.h"

#define STRING_MIN_LENGTH 8

typedef struct StringT {
    char* ptr;
    uintptr_t length;
    uintptr_t capacity;
} String;

String* string_empty();
String* string_from_const(const char* str);
String* string_clone(String* other);

String* string_format(const char* format, ...);

void string_push(String* self, char chr);
void string_push_const(String* self, const char* str);
void string_push_string(String* self, String* other);
void string_push_string_free(String* self, String* other);
void string_push_int(String* self, int value);
void string_push_format(String* self, const char* format, ...);

bool string_equal_const(String* self, const char* str);
bool string_equal_string(String* self, String* other);

bool string_start_with_const(String* self, const char* str);

String* string_slice(String* self, uintptr_t start, intptr_t end);

bool string_is_empty(String* self);

List* string_lines(String* self);

void string_destroy(String* self);

#endif //APLA_COMPILER_STR_H
