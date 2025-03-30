%{
#include "ast.h"

#include <stdio.h>
#include <stddef.h>

extern FILE* yyin;
extern int yyerror(char* msg);
extern int yylex(void);
extern int yylineno; 

ASTNode* root;
%}

%union {
    ASTNode *node;
    int num;
    char *str;
}

%token PRINT IF ELSE WHILE NUMBER IDENTIFIER ASSIGN EQ LT GT PLUS MINUS MULT DIV SEMICOLON STRING NEWLINE

%left PLUS MINUS
%left MULT DIV

%type <node> program statement expression
%type <str> IDENTIFIER STRING
%type <num> NUMBER

%%

start:
    program { print_ast(root, 0); }  // Explicit start symbol
    ;

program:
    /* Empty */ { $$ = NULL; root = NULL; }
    | program statement NEWLINE { root = $2; }
    | program statement { root = $2; }
    | program NEWLINE { /* Ignore empty lines */ }
    ;

statement:
    PRINT STRING SEMICOLON { $$ = create_print_node(create_str_node($2)); }
    | IF '(' expression ')' statement { $$ = create_if_node($3, $5); }
    | WHILE '(' expression ')' statement { $$ = create_while_node($3, $5); }
    | IDENTIFIER ASSIGN expression SEMICOLON { $$ = create_assign_node($1, $3); }
    | error SEMICOLON { 
        fprintf(stderr, "Syntax error near line %d\n", yylineno); 
        yyerrok;
        $$ = NULL; 
      }
    ;

expression:
    expression PLUS expression { $$ = create_binop_node('+', $1, $3); }
    | IDENTIFIER { $$ = create_ident_node($1); }
    | NUMBER { $$ = create_num_node($1); }
    ;

%%

int yyerror(char* msg) {
    fprintf(stderr, "Parser Error: %s at line %d\n", msg, yylineno);
    return 1;
}

int main(int argc, char *argv[]) {
    
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input-file>\n", argv[0]);
        return 1;
    }

    yyin = fopen(argv[1], "r");
    if (!yyin) {
        perror("Error opening file");
        return 1;
    }
    yyparse();
    fclose(yyin);
    return 0;
}