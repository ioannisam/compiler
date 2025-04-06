#ifndef HELPERS_H
#define HELPERS_H

#include "codegen/codegen.h"
#include "parser/ast.h"

void collect_print_messages(ASTNode* node, FILE* output);
void emit_data_section(ASTNode* node, FILE* output);

void collect_variables(ASTNode* node);
void emit_bss_section(FILE* output);

void emit_text_section(ASTNode* node, FILE* output);
void emit_itoa(FILE* output);

#endif