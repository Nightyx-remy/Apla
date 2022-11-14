//
// Created by remy on 11/7/22.
//

#include "position.h"

Position position_create(uintptr_t index, uintptr_t line, uintptr_t column) {
    Position self;
    self.index = index;
    self.line = line;
    self.column = column;
    return self;
}

Position position_default() {
    Position self;
    self.index = 0;
    self.line = 1;
    self.column = 0;
    return self;
}

Position position_clone(Position other) {
    Position self;
    self.index = other.index;
    self.line = other.line;
    self.column = other.column;
    return self;
}

void position_advance(Position* self, char chr) {
    if (chr == '\n') {
        self->line += 1;
        self->column = 0;
    } else {
        self->column += 1;
    }
    self->index += 1;
}

Positioned* positioned_create(void* data, Position start, Position end) {
    Positioned* self = (Positioned*) malloc(sizeof(Positioned));
    self->start = start;
    self->end = end;
    self->data = data;
    return self;
}

Positioned* positioned_convert(Positioned* positioned, void* new_data) {
    Positioned* self = (Positioned*) malloc(sizeof(Positioned));
    self->start = position_clone(positioned->start);
    self->end = position_clone(positioned->end);
    self->data = new_data;
    return self;
}

String* positioned_arrow_message(Positioned* self, String* src) {
    String* buf = string_empty();
    List* lines = string_lines(src);

    uintptr_t line_index = self->start.line;
    while (line_index <= self->end.line) {
        String* line = lines->ptr[line_index - 1];
        uintptr_t start = line_index == self->start.line ? self->start.column : 0;
        uintptr_t end = line_index == self->end.line ? self->end.column - start : line->length - start;

        if (line_index != self->start.line) {
            string_push(buf, '\n');
        }

        string_push_string(buf, line);
        string_push(buf, '\n');
        uintptr_t i = 0;
        while (i < start) {
            string_push(buf, ' ');
            i += 1;
        }

        i = 0;
        while (i < end) {
            string_push(buf, '^');
            i += 1;
        }

        line_index += 1;
    }

    return buf;
}

void positioned_destroy(Positioned* self) {
    free(self);
}