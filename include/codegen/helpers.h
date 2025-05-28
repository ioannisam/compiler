#ifndef HELPERS_H
#define HELPERS_H

#include <stdbool.h>

#include "codegen/codegen.h"
#include "parser/ast.h"

void push_loop(int label);
int pop_loop(void);
int current_loop(void);

bool has_main_function(ASTNode* functions);
void collect_and_define_strings(ASTNode* node, FILE* output);
void collect_print_messages(ASTNode* node, FILE* output);
void emit_data_section(ASTNode* node, FILE* output);

void collect_variables(ASTNode* node);
void emit_string_data(ASTNode* node, FILE* output);

#endif