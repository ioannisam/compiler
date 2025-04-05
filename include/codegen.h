// codegen.h
#ifndef CODEGEN_H
#define CODEGEN_H
#include "ast.h"
#include <stdio.h>

extern int msg_len;
void generate_code(ASTNode* node, FILE* output);
static char* find_print_message(ASTNode* node);
void generate_code_to_file(ASTNode* node);

#endif