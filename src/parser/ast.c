// src/parser/ast.c
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// implementations of AST constructors
ASTNode* create_print_node(ASTNode *expr) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_PRINT;
    node->control.body = expr;
    return node;
}

ASTNode* create_str_node(char *str) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_STR;
    node->str_value = strdup(str);
    return node;
}

ASTNode* create_ident_node(char *id) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_IDENT;
    node->str_value = strdup(id);
    return node;
}

ASTNode* create_num_node(int value) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_NUM;
    node->num_value = value;
    return node;
}

ASTNode* create_if_node(ASTNode* cond, ASTNode* body) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_IF;
    node->control.condition = cond;
    node->control.body = body;
    return node;
}

ASTNode* create_while_node(ASTNode* cond, ASTNode* body) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_WHILE;
    node->control.condition = cond;
    node->control.body = body;
    return node;
}

ASTNode* create_assign_node(char* id, ASTNode* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_ASSIGN;
    node->str_value = strdup(id);
    node->binop.left = value; // Reuse binop struct for assignment
    return node;
}

ASTNode* create_binop_node(char op, ASTNode* left, ASTNode* right) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_BINOP;
    node->binop.op = op;
    node->binop.left = left;
    node->binop.right = right;
    return node;
}

void print_ast(ASTNode *node, int indent) {
    if (!node) return;
    for (int i = 0; i < indent; i++) printf("  ");
    
    switch (node->type) {
        case NODE_PRINT:
            printf("PRINT\n");
            print_ast(node->control.body, indent + 1);
            break;
        case NODE_BINOP:
            printf("BINOP(%c)\n", node->binop.op);
            print_ast(node->binop.left, indent + 1);
            print_ast(node->binop.right, indent + 1);
            break;
        case NODE_NUM:
            printf("NUM(%d)\n", node->num_value);
            break;
        case NODE_STR:
            printf("STR(%s)\n", node->str_value);
            break;
        case NODE_IDENT:
            printf("IDENT(%s)\n", node->str_value);
            break;
        // [more cases...]
    }
}