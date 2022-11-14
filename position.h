//
// Created by remy on 11/7/22.
//

#ifndef APLA_COMPILER_POSITION_H
#define APLA_COMPILER_POSITION_H

#include <stdint.h>
#include "str.h"

typedef struct PositionT {
    uintptr_t index;
    uintptr_t line;
    uintptr_t column;
} Position;

Position position_create(uintptr_t index, uintptr_t line, uintptr_t column);
Position position_default();
Position position_clone(Position other);

void position_advance(Position* self, char chr);

typedef struct PositionedT {
    Position start;
    Position end;
    void* data;
} Positioned;

Positioned* positioned_create(void* data, Position start, Position end);
Positioned* positioned_convert(Positioned* positioned, void* new_data);

String* positioned_arrow_message(Positioned* self, String* src);

void positioned_destroy(Positioned* self);

#endif //APLA_COMPILER_POSITION_H
