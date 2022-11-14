//
// Created by remy on 11/6/22.
//

#include "list.h"

List* list_create() {
    List* self = (List*) malloc(sizeof(List));
    self->length = 0;
    self->capacity = LIST_MIN_LENGTH;
    self->ptr = (void**) malloc(sizeof(void*) * self->capacity);
    return self;
}

void list_push(List* self, void* data) {
    if (self->length >= self->capacity) {
        self->capacity *= 2;
        self->ptr = (void**) realloc(self->ptr, sizeof(void*) * self->capacity);
    }

    self->ptr[self->length] = data;
    self->length += 1;
}

void list_destroy(List* self) {
    free(self->ptr);
    free(self);
}
