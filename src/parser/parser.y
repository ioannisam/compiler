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
    ASTNode* node;
    int num;
    char* str;
}

%token PRINT IF ELSE WHILE 
%token NUMBER IDENTIFIER STRING
%token ASSIGN EQ LT GT PLUS MINUS MULT DIV
%token SEMICOLON NEWLINE LPAREN RPAREN

%left PLUS MINUS
%left MULT DIV

%type <node> program statements statement expression
%type <str> IDENTIFIER STRING
%type <num> NUMBER

%%

start:
    program { print_ast(root, 0); }
    ;

program:
    statements { root = $1; }
    ;

statements:
    statement { $$ = create_compound_node($1, NULL); }
    | statements NEWLINE statement { $$ = append_statement($1, $3); }
    | statements statement { $$ = append_statement($1, $2); }
    | /* Allow empty input */ { $$ = NULL; }
    ;

statement:
    PRINT expression SEMICOLON { $$ = create_print_node($2); }
    | IF LPAREN expression RPAREN statement { $$ = create_if_node($3, $5); }
    | WHILE LPAREN expression RPAREN statement { $$ = create_while_node($3, $5); }
    | IDENTIFIER ASSIGN expression SEMICOLON { $$ = create_assign_node($1, $3); }
    | error SEMICOLON { 
          fprintf(stderr, "Syntax error near line %d\n", yylineno); 
          yyerrok;
          $$ = NULL; 
      }
    ;

expression:
      expression EQ expression { $$ = create_binop_node('=', $1, $3); }
    | expression PLUS expression { $$ = create_binop_node('+', $1, $3); }
    | expression MINUS expression { $$ = create_binop_node('-', $1, $3); }
    | expression MULT expression { $$ = create_binop_node('*', $1, $3); }
    | expression DIV expression { $$ = create_binop_node('/', $1, $3); }
    | IDENTIFIER { $$ = create_ident_node($1); }
    | NUMBER { $$ = create_num_node($1); }
    | STRING { $$ = create_str_node($1); }
    ;

%%

int yyerror(char* msg) {
    fprintf(stderr, "Parser Error: %s at line %d\n", msg, yylineno);
    return 1;
}

int main(int argc, char *argv[]) {
    
    if (argc < 2) {
        fprintf(stderr, "No input file provided. Defaulting to /dev/stdin.\n");
        yyin = fopen("/dev/stdin", "r");
    } else {
        yyin = fopen(argv[1], "r");
    }

    if (!yyin) {
        perror("Error opening file");
        return 1;
    }

    yyparse();
    fclose(yyin);
    return 0;
}