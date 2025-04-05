%code requires {
    #include "ast.h"
}

%{
#include "ast.h"
#include "codegen.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

extern FILE* yyin;
extern int yyerror(char* msg);
extern int yylex(void);
extern char* yytext;
extern int yylineno; 

ASTNode* root = NULL;
int parse_errors = 0;
%}

%union {
    ASTNode* node;
    int num;
    char* str;
}

%token ERROR
%token PRINT IF ELSE WHILE 
%token NUMBER IDENTIFIER STRING
%token ASSIGN EQ LT GT AND OR XOR NOT LSHIFT RSHIFT PLUS MINUS MULT DIV 
%token SEMICOLON NEWLINE LPAREN RPAREN LBRACE RBRACE

%right NOT
%left AND
%left OR XOR
%left LSHIFT RSHIFT
%left EQ LT GT
%left PLUS MINUS
%left MULT DIV

%type <node> program statements statement expression block
%type <str> IDENTIFIER STRING
%type <num> NUMBER

%%

start:
    program { root = $1; }
    ;

program:
    statements { root = $1; }
    ;

statements:
      /* empty */ { $$ = NULL; }
    | statements statement { $$ = append_statement($1, $2); }
    ;

block: 
    LBRACE statements RBRACE { 
        $$ = $2 ? $2 : create_compound_node(NULL, NULL); 
    }
    ;

statement:
      PRINT expression SEMICOLON { $$ = create_print_node($2); }
    | IF LPAREN expression RPAREN statement { $$ = create_if_node($3, $5); }
    | WHILE LPAREN expression RPAREN statement { $$ = create_while_node($3, $5); }
    | IDENTIFIER ASSIGN expression SEMICOLON { $$ = create_assign_node($1, $3); }
    | IF LPAREN expression RPAREN block { $$ = create_if_node($3, $5); }
    | IF LPAREN expression RPAREN block ELSE block { $$ = create_if_else_node($3, $5, $7); }
    | WHILE LPAREN expression RPAREN block { $$ = create_while_node($3, $5); }
    | error SEMICOLON { 
          yyerrok; 
          yyclearin; 
          $$ = create_empty_node();
      }
    ;

expression:
      expression EQ    expression { $$ = create_binop_node(OP_EQ, $1, $3); }
    | expression AND   expression { $$ = create_binop_node(OP_AND, $1, $3); }
    | expression OR    expression { $$ = create_binop_node(OP_OR, $1, $3); }
    | expression XOR   expression { $$ = create_binop_node(OP_XOR, $1, $3); }
    |            NOT   expression { $$ = create_unop_node(OP_NOT, $2); }
    | expression LSHIFT expression { $$ = create_binop_node(OP_LSHIFT, $1, $3); }
    | expression RSHIFT expression { $$ = create_binop_node(OP_RSHIFT, $1, $3); }
    | expression PLUS  expression { $$ = create_binop_node(OP_ADD, $1, $3); }
    | expression MINUS expression { $$ = create_binop_node(OP_SUB, $1, $3); }
    | expression MULT  expression { $$ = create_binop_node(OP_MUL, $1, $3); }
    | expression DIV   expression { $$ = create_binop_node(OP_DIV, $1, $3); }
    | IDENTIFIER { $$ = create_ident_node($1); }
    | NUMBER { $$ = create_num_node($1); }
    | STRING { $$ = create_str_node($1); }
    ;
%%

int yyerror(char* msg) {

    const char* token = (yytext && yytext[0]) ? yytext : "end of input";
    fprintf(stderr, "Syntax error at line %d: %s (near '%s')\n", yylineno, msg, token);
    parse_errors++;
    return 1;
}

int main(int argc, char* argv[]) {

    if (argc > 2) {
        fprintf(stderr, "Usage: %s [input_file]\n", argv[0]);
        return 1;
    } else if (argc < 2) {
        fprintf(stderr, "No input file provided. Defaulting to /dev/stdin.\nEnter input (press Ctrl+D when done): ");
        yyin = fopen("/dev/stdin", "r");
    } else {
        yyin = fopen(argv[1], "r");
    }

    if (!yyin) {
        perror("Error opening file");
        return 1;
    }

    int parse_result = yyparse();
    fclose(yyin);

    if (parse_result != 0) {
        fprintf(stderr, "Parsing failed with %d errors.\n", parse_errors);
        return 1;
    }

    generate_code_to_file(root);

    free_ast(root);

    return 0;
}