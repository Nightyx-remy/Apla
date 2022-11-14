#include "str.h"

String* string_empty() {
    // Create the String object
    String* str = (String*) malloc(sizeof(String));
    str->length = 0;
    str->capacity = STRING_MIN_LENGTH;
    str->ptr = (char*) malloc(sizeof(char) * str->capacity);
    str->ptr[0] = '\0';

    // Return the created object
    return str;
};

String* string_from_const(const char* str) {
    // Create the String object
    String *self = (String *) malloc(sizeof(String));
    self->length = (uintptr_t) strlen(str);
    self->capacity = MAX(STRING_MIN_LENGTH, next_pow2(self->length));
    self->ptr = (char *) malloc(sizeof(char) * self->capacity);

    // Copy the content of str
    uintptr_t i = 0;
    while (i < self->length) {
        self->ptr[i] = str[i];
        i += 1;
    }
    self->ptr[self->length] = '\0';

    // Return the created object
    return self;
}

String* string_clone(String* other) {
    // Create the String object
    String *self = (String *) malloc(sizeof(String));
    self->length = other->length;
    self->capacity = other->capacity;
    self->ptr = (char *) malloc(sizeof(char) * self->capacity);

    // Copy the content of str
    uintptr_t i = 0;
    while (i < self->length) {
        self->ptr[i] = other->ptr[i];
        i += 1;
    }
    self->ptr[self->length] = '\0';

    // Return the created object
    return self;
}

String* string_format_(const char* format, va_list args) {
    enum FormatState {
        NO_FORMAT,
        FORMAT_START,
        FORMAT_DECIMAL,             // {d}
        FORMAT_FLOAT,               // {f}
        FORMAT_CHAR,                // {c}
        FORMAT_CONST_STRING,        // {cs}
        FORMAT_STRING,              // {s}
        FORMAT_STRING_FREE,         // {sf}
        FORMAT_BOOLEAN              // {b}
    };

    String* self = string_empty();

    // Calculate the number of arguments
    uintptr_t arguments = 0;
    uintptr_t format_length = strlen(format);
    uintptr_t i = 0;
    enum FormatState f_state = NO_FORMAT;
    while (i < format_length) {
        char current = format[i];

        switch (f_state) {
            case NO_FORMAT:
                if (current == '{') f_state = FORMAT_START;
                else string_push(self, current);
                break;
            case FORMAT_START:
                switch (current) {
                    case '{':
                        string_push(self, '{');
                        f_state = NO_FORMAT; // {{ => '{'
                        break;
                    case 'd':
                        f_state = FORMAT_DECIMAL;
                        break;
                    case 'f':
                        f_state = FORMAT_FLOAT;
                        break;
                    case 'c':
                        f_state = FORMAT_CHAR;
                        break;
                    case 's':
                        f_state = FORMAT_STRING;
                        break;
                    case 'b':
                        f_state = FORMAT_BOOLEAN;
                        break;
                    default:
                        f_state = NO_FORMAT;
                }

                break;
            case FORMAT_DECIMAL:
                if (current == '}') {
                    f_state = NO_FORMAT;
                    arguments += 1;
                    string_push_int(self, va_arg(args, int));
                } else {
                    f_state = NO_FORMAT;
                }
                break;
            case FORMAT_FLOAT:
                if (current == '}') {
                    f_state = NO_FORMAT;
                    arguments += 1;
                    //TODO: string_push_float(self, va_arg(args, float*)[0]);
                } else {
                    f_state = NO_FORMAT;
                }
                break;
            case FORMAT_CONST_STRING:
                if (current == '}') {
                    f_state = NO_FORMAT;
                    arguments += 1;
                    string_push_const(self, va_arg(args, const char*));
                } else {
                    f_state = NO_FORMAT;
                }
                break;
            case FORMAT_STRING_FREE:
                if (current == '}') {
                    f_state = NO_FORMAT;
                    arguments += 1;
                    String* s = va_arg(args, String*);
                    string_push_string(self, s);
                    string_destroy(s);
                } else {
                    f_state = NO_FORMAT;
                }
                break;
            case FORMAT_BOOLEAN:
                if (current == '}') {
                    f_state = NO_FORMAT;
                    arguments += 1;
                    //string_push_bool(self, va_arg(args, bool*)[0]);
                } else {
                    f_state = NO_FORMAT;
                }
                break;
            case FORMAT_CHAR:
                if (current == 's') {
                    f_state = FORMAT_CONST_STRING;
                } else if (current == '}') {
                    f_state = NO_FORMAT;
                    arguments += 1;
                    string_push(self, va_arg(args, const char*)[0]);
                } else {
                    f_state = NO_FORMAT;
                }
                break;
            case FORMAT_STRING:
                if (current == 'f') {
                    f_state = FORMAT_STRING_FREE;
                } else if (current == '}') {
                    f_state = NO_FORMAT;
                    arguments += 1;
                    string_push_string(self, va_arg(args, String*));
                } else {
                    f_state = NO_FORMAT;
                }
                break;
        }

        i += 1;
    }

    return self;
}

String* string_format(const char* format, ...) {
    va_list args;
    va_start(args, format);
    String* str = string_format_(format, args);
    va_end(args);
    return str;
}

void string_push(String* self, char chr) {
    if (self->length >= self->capacity) {
        self->capacity *= 2;
        self->ptr = (char*) realloc(self->ptr, sizeof(char) * self->capacity);
    }

    self->ptr[self->length] = chr;
    self->length += 1;

    self->ptr[self->length] = '\0';
}

void string_push_const(String* self, const char* str) {
    uintptr_t str_length = (uintptr_t) strlen(str);
    while(self->length + str_length >= self->capacity) {
        self->capacity *= 2;
        self->ptr = (char*) realloc(self->ptr, sizeof(char) * self->capacity);
    }

    uintptr_t i = 0;
    while (i < str_length) {
        self->ptr[self->length] = str[i];
        self->length += 1;
        i += 1;
    }
    self->ptr[self->length] = '\0';
}

void string_push_string(String* self, String* other) {
    while(self->length + other->length >= self->capacity) {
        self->capacity *= 2;
        self->ptr = (char*) realloc(self->ptr, sizeof(char) * self->capacity);
    }

    uintptr_t i = 0;
    while (i < other->length) {
        self->ptr[self->length] = other->ptr[i];
        self->length += 1;
        i += 1;
    }
    self->ptr[self->length] = '\0';
}

void string_push_string_free(String* self, String* other) {
    string_push_string(self, other);
    string_destroy(other);
}

void string_push_digit(String* self, int value) {
    switch (value) {
        case 0:
            string_push(self, '0');
            break;
        case 1:
            string_push(self, '1');
            break;
        case 2:
            string_push(self, '2');
            break;
        case 3:
            string_push(self, '3');
            break;
        case 4:
            string_push(self, '4');
            break;
        case 5:
            string_push(self, '5');
            break;
        case 6:
            string_push(self, '6');
            break;
        case 7:
            string_push(self, '7');
            break;
        case 8:
            string_push(self, '8');
            break;
        case 9:
            string_push(self, '9');
            break;
        default: break;
    }
}

void string_push_int(String* self, int value) {
    if (value >= 10) {
        int right = value % 10;
        int left = value / 10;
        string_push_int(self, left);
        string_push_digit(self, right);
    } else {
        string_push_digit(self, value);
    }
}

void string_push_format(String* self, const char* format, ...) {
    va_list args;
    va_start(args, format);
    String* str = string_format_(format, args);
    string_push_string(self, str);
    string_destroy(str);
    va_end(args);
}

bool string_equal_const(String* self, const char* str) {
    if (self->length != strlen(str)) return false;

    uintptr_t i = 0;
    while (i <  self->length) {
        if (self->ptr[i] != str[i]) return false;
        i += 1;
    }

    return true;
}

bool string_equal_string(String* self, String* other) {
    if (self->length != other->length) return false;

    uintptr_t i = 0;
    while (i <  other->length) {
        if (self->ptr[i] != other->ptr[i]) return false;
        i += 1;
    }

    return true;
}

bool string_start_with_const(String* self, const char* str) {
    uintptr_t len = strlen(str);
    uintptr_t i = 0;
    while (i < len) {
        if (self->ptr[i] != str[i]) return false;
        i += 1;
    }
    return true;
}

String* string_slice(String* self, uintptr_t start, intptr_t end) {
    String* new = string_empty();

    if (end == -1) {
        end = (intptr_t) self->length;
    }

    uintptr_t i = start;
    while (i < end) {
        string_push(new, self->ptr[i]);
        i += 1;
    }

    return new;
}

bool string_is_empty(String* self) {
    return self->length == 0;
}

List* string_lines(String* self) {
    List* lines = list_create();
    String* line = string_empty();

    uintptr_t index = 0;
    while (index < self->length) {
        char current = self->ptr[index];
        if (current == '\n') {
            list_push(lines, line);
            line = string_empty();
        } else {
            string_push(line, current);
        }
        index += 1;
    }

    if (!string_is_empty(line)) {
        list_push(lines, line);
    }

    return lines;
}

void string_destroy(String* self) {
    free(self->ptr);
    free(self);
}
