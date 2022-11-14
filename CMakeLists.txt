cmake_minimum_required(VERSION 3.21)
project(apla_compiler C)

set(CMAKE_C_STANDARD 99)

add_executable(apla_compiler main.c res/main.apla token.c token.h str.c str.h utils.c utils.h lexer.c lexer.h list.c list.h position.c position.h node.c node.h parser.c parser.h generator.c generator.h)
target_link_libraries(apla_compiler m)
