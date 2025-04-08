#ifndef HANDLERS_H
#define HANDLERS_H

#include "codegen/codegen.h"

void handle_print(ASTNode* node, FILE* output);
void handle_assign(ASTNode* node, FILE* output);

void handle_if(ASTNode* node, FILE* output);
void handle_while(ASTNode* node, FILE* output);

void handle_num(ASTNode* node, FILE* output);
void handle_ident(ASTNode* node, FILE* output);
void handle_binop(ASTNode* node, FILE* output);
void handle_unop(ASTNode* node, FILE* output);
void handle_compound(ASTNode* node, FILE* output);

#endif