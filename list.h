//
// Created by remy on 11/6/22.
//

#ifndef APLA_COMPILER_LIST_H
#define APLA_COMPILER_LIST_H

#include <stdint.h>
#include <stdlib.h>
#include "utils.h"

#define LIST_MIN_LENGTH 8

typedef struct ListT {
    void** ptr;
    uintptr_t length;
    uintptr_t capacity;
} List;

List* list_create();

void list_push(List* self, void* data);

void list_destroy(List* self);

#endif //APLA_COMPILER_LIST_H
