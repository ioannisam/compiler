#ifndef AST_H
#define AST_H

typedef struct ASTNode ASTNode;
typedef enum {
    NODE_PROGRAM,
    NODE_FUNC,
    NODE_PARAM,
    NODE_PRINT,
    NODE_IF,
    NODE_WHILE,
    NODE_BREAK,
    NODE_RETURN,
    NODE_DECL,
    NODE_ASSIGN,
    NODE_BINOP,
    NODE_IDENT,
    NODE_NUM,
    NODE_STR,
    NODE_COMPOUND,
    NODE_UNOP,
    NODE_EMPTY
} NodeType;

typedef enum {
    OP_POS, OP_NEG, 
    OP_EQ, OP_NEQ, OP_GE, OP_LE, OP_LT, OP_GT,
    OP_LAND, OP_LOR, OP_LNOT,
    OP_BNOT, OP_BAND, OP_BOR, OP_BXOR, OP_BNAND, OP_BNOR, OP_BXNOR,
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
    OP_LSHIFT, OP_RSHIFT
} Operator;

typedef struct ASTNode {
    NodeType type;
    union {
        int num_value;
        char* str_value;
        struct {
            ASTNode* functions;
            ASTNode* main_block;
        } program;
        struct {
            char* return_type;
            char* name;
            ASTNode* params;
            ASTNode* body;
        } func;
        struct {
            char* type;
            char* name;
        } param;
        struct {
            struct ASTNode* left;
            struct ASTNode* right;
            Operator op;
        } binop;
        struct {
            struct ASTNode* condition;
            struct ASTNode* loop_body;
            struct ASTNode* if_body;
            struct ASTNode* else_body;
        } control;
        struct {
            Operator op;
            struct ASTNode* operand;
        } unop;
        struct {
            struct ASTNode* expr;
        } print_expr;
        struct {
            struct ASTNode* expr;
        } return_stmt;
        struct {
            char* type;
            char* name;
            ASTNode* init_expr;
        } decl;
        struct {
            ASTNode* target;
            ASTNode* value;
        } assign;
    };
} ASTNode;

ASTNode* create_program_node(ASTNode* functions, ASTNode* main_block);
ASTNode* create_func_node(char* return_type, char* name, ASTNode* params, ASTNode* body);
ASTNode* append_function(ASTNode* func_list, ASTNode* func);
ASTNode* create_param_node(char* type, char* name);
ASTNode* append_param(ASTNode* param_list, ASTNode* param);

ASTNode* create_print_node(ASTNode* expr);
ASTNode* create_if_node(ASTNode* cond, ASTNode* body, ASTNode* else_body);
ASTNode* create_while_node(ASTNode* cond, ASTNode* body);
ASTNode* create_break_node(void);
ASTNode* create_return_node(ASTNode* expr);
ASTNode* create_decl_node(char* type, char* name, ASTNode* init_expr);
ASTNode* create_assign_node(char* id, ASTNode* value);
ASTNode* create_binop_node(Operator op, ASTNode* left, ASTNode* right);
ASTNode* create_ident_node(char* id);
ASTNode* create_num_node(int value);
ASTNode* create_str_node(char* str);
ASTNode* create_compound_node(ASTNode* stmt, ASTNode* next);
ASTNode* append_statement(ASTNode* compound, ASTNode* stmt);
ASTNode* create_unop_node(Operator op, ASTNode* operand);
ASTNode* create_if_else_node(ASTNode* cond, ASTNode* if_body, ASTNode* else_body);
ASTNode* create_empty_node(void);

const char* operator_to_string(Operator op);

void print_ast(ASTNode* node, int indent);
void free_ast(ASTNode* node);

#endif