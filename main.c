#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "str.h"
#include "lexer.h"
#include "parser.h"
#include "generator.h"

char* read_file(const char* path) {
    // Open the file
    FILE* f = fopen(path, "r");
    if (f == NULL) {
        printf("Failed to read file %s", path);
        return NULL;
    }

    // Get the length of the file
    fseek(f, 0, SEEK_END);
    uintptr_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Read the file
    char* content = malloc((size + 1) * sizeof(char));
    fread(content, size, size, f);
    content[size] = '\0';

    // close the file
    fclose(f);

    return content;
}

int main() {
    char* c_src = read_file("/home/remy/CLionProjects/apla_compiler/res/main.apla");
    if (c_src != NULL) {
        String* src = string_from_const(c_src);
        printf("%s\n\n", src->ptr);

        Lexer lexer = lexer_create(src);

        LexerResult lexer_result = lexer_tokenize(&lexer);
        switch (lexer_result.type) {
            case LEXER_ERROR:
                printf("[Lexer]: %s\n", lexer_result.data.error.message->ptr);
                break;
            case LEXER_OK: {
                List *tokens = lexer_result.data.tokens;

                printf("--- Tokens ---\n");
                uintptr_t i = 0;
                while (i < tokens->length) {
                    Positioned* positioned_token = (Positioned*) tokens->ptr[i];

                    String* str = token_to_string((Token*) positioned_token->data);
                    printf("%s\n", str->ptr);
                    string_destroy(str);

                    //str = positioned_arrow_message(positioned_token, src);
                    //printf("%s\n", str->ptr);
                    //string_destroy(str);

                    i += 1;
                }
                printf("\n");

                // Parse
                Parser parser = parser_create(src, tokens);

                ParserResult parser_res = parser_parse(&parser);
                if (parser_res.type == PARSER_ERROR) {
                    printf("[Parser]: %s\n", parser_res.data.error.message->ptr);
                    return -1;
                } else {
                    List* ast = parser_res.data.ast;

                    printf("--- AST ---\n");
                    i = 0;
                    while (i < ast->length) {
                        Positioned* positioned_node = (Positioned*) ast->ptr[i];

                        String* str = node_to_string((Node*) positioned_node->data);
                        printf("%s\n", str->ptr);
                        string_destroy(str);

                        //str = positioned_arrow_message(positioned_token, src);
                        //printf("%s\n", str->ptr);
                        //string_destroy(str);

                        i += 1;
                    }
                    printf("\n");

                    Generator generator = generator_create(src, ast);
                    GeneratorResult generator_res = generator_generate(&generator);
                    if (generator_res.type == GENERATOR_ERROR) {
                        printf("[Generator]: %s\n", generator_res.data.error.message->ptr);
                        return -1;
                    } else {
                        GeneratedProject* project = generator_res.data.project;
                        printf("--- Generated ---\n");
                        i = 0;
                        while (i < project->files->length) {
                            GeneratedFile* file = project->files->ptr[i];
                            printf("\n%s:\n", file->name->ptr);
                            printf("%s\n", file->src->ptr);
                            i += 1;
                        }
                    }

                    generator_result_destroy(&generator_res);
                }

                parser_result_destroy(&parser_res);
            } break;
        }
        lexer_result_destroy(&lexer_result);

        string_destroy(src);
    }
    return 0;
}
