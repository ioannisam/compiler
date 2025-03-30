// src/parser/ast.h
#ifndef AST_H
#define AST_H

typedef enum {
    NODE_PRINT,
    NODE_IF,
    NODE_WHILE,
    NODE_ASSIGN,
    NODE_BINOP,
    NODE_IDENT,
    NODE_NUM,
    NODE_STR
} NodeType;

typedef struct ASTNode {
    NodeType type;
    union {
        int num_value;
        char* str_value;
        struct {
            struct ASTNode* left;
            struct ASTNode* right;
            char op;  // '+', '-', etc.
        } binop;
        struct {
            struct ASTNode* condition;
            struct ASTNode* body;
        } control;
    };
} ASTNode;

// Constructor functions
ASTNode* create_print_node(ASTNode* expr);
ASTNode* create_if_node(ASTNode* cond, ASTNode* body);
ASTNode* create_while_node(ASTNode* cond, ASTNode* body);
ASTNode* create_assign_node(char* id, ASTNode* value);
ASTNode* create_binop_node(char op, ASTNode* left, ASTNode* right);
ASTNode* create_ident_node(char* id);
ASTNode* create_num_node(int value);
ASTNode* create_str_node(char* str);

void print_ast(ASTNode* node, int indent);
void free_ast(ASTNode* node);

#endif