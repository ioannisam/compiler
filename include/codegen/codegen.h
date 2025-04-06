#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser/ast.h"
#include <stdio.h>

extern int data_label_counter;
extern int code_label_counter;

void generate_code(ASTNode* node, FILE* output);
void generate_code_to_file(ASTNode* node);

#endif