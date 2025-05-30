%define api.header.include {"parser/parser.tab.h"}

%code requires {
    #include "parser/parser.tab.h"
    #include "parser/ast.h"
}

%{
#include "parser/ast.h"
#include "codegen/codegen.h"

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

// tokens
%token ERROR
%token MAIN TYPE_INT 
%token TRUE FALSE
%token PRINT IF ELSE WHILE BREAK RETURN
%token NUMBER IDENTIFIER STRING
%token ASSIGN 
%token EQ GE LE LT GT NEQ LAND LOR LNOT
%token BNOT BAND BOR BXOR BNAND BNOR BXNOR LSHIFT RSHIFT
%token PLUS MINUS MULT DIV MOD
%token SEMICOLON COMMA NEWLINE LPAREN RPAREN LBRACE RBRACE

// optional declaration conflict
%nonassoc DECL_PREC
%nonassoc FUNCTION_PREC

// dangling if conflict
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

// precedence
%right LNOT
%left LAND
%left LOR

%right BNOT
%left BAND BNAND
%left BOR BNOR BXOR BXNOR
%left LSHIFT RSHIFT

%left EQ NEQ GE LE GT LT
%left PLUS MINUS
%left MULT DIV MOD

%nonassoc UMINUS UPLUS

// types
%type <node> program statements statement expression block functions function_decl arg_list params param_list param main_block decl optional_init
%type <str> IDENTIFIER STRING
%type <num> NUMBER

%%

start:
    program { root = $1; }
    ;

program:
      functions main_block { $$ = create_program_node($1, $2); }
    ;

main_block:
    /* empty */ { $$ = NULL; }
    /* scripted mode: statements in a block */
    | block { $$ = $1; }
    /* structured mode: functions + optional MAIN { … } */
    | MAIN block { $$ = $2; }
    ;

functions:
    /* empty */ { $$ = NULL; }
    | functions function_decl { $$ = append_function($1, $2); }
    | functions NEWLINE { $$ = $1; }
    ;

block: 
    LBRACE statements RBRACE { 
        $$ = $2 ? $2 : create_compound_node(NULL, NULL); 
    }
    ;

function_decl:
    TYPE_INT IDENTIFIER LPAREN params RPAREN block %prec FUNCTION_PREC
        { $$ = create_func_node("int", $2, $4, $6); }
    | TYPE_INT MAIN LPAREN params RPAREN block %prec FUNCTION_PREC
        { $$ = create_func_node("int", strdup("main"), $4, $6); }
    ;

arg_list:
    /* empty */ { $$ = NULL; }
    | expression { $$ = create_compound_node($1, NULL); }
    | arg_list COMMA expression { $$ = append_arg($1, $3); }
    ;

params:
    /* empty */ { $$ = NULL; }
    | param_list { $$ = $1; }
    ;

param_list:
    param { $$ = create_compound_node($1, NULL); }
    | param_list COMMA param { $$ = append_param($1, $3); }
    ;

param:
    TYPE_INT IDENTIFIER
        { $$ = create_param_node("int", $2); }
    ;

decl: 
    TYPE_INT IDENTIFIER optional_init %prec DECL_PREC  
        { $$ = create_decl_node("int", $2, $3); }
    ;

optional_init:
      /* empty */            { $$ = NULL; }
    | ASSIGN expression      { $$ = $2; }
    ;

statements:
      /* empty */ { $$ = NULL; }
    | statements NEWLINE statement { $$ = append_statement($1, $3); }
    | statements NEWLINE { $$ = $1; }
    | statement { $$ = create_compound_node($1, NULL); }
    ;

statement:
      decl SEMICOLON
        { $$ = $1; }
    | IDENTIFIER ASSIGN expression SEMICOLON
        { $$ = create_assign_node($1, $3); }
    | block
    | PRINT expression SEMICOLON
        { $$ = create_print_node($2); }
    | IF LPAREN expression RPAREN statement %prec LOWER_THAN_ELSE
        { $$ = create_if_node($3, $5, NULL); }
    | IF LPAREN expression RPAREN statement ELSE statement
        { $$ = create_if_node($3, $5, $7); }
    | WHILE LPAREN expression RPAREN statement
        { $$ = create_while_node($3, $5); }
    | BREAK SEMICOLON
        { $$ = create_break_node(); }
    | RETURN expression SEMICOLON
       { $$ = create_return_node($2); }
    | error SEMICOLON
        { yyerrok; yyclearin; $$ = create_empty_node(); }
    ;

expression:
      LPAREN expression RPAREN { $$ = $2; }

    | TRUE      { $$ = create_num_node(1); }
    | FALSE     { $$ = create_num_node(0); }

    | IDENTIFIER { $$ = create_ident_node($1); }
    | NUMBER     { $$ = create_num_node($1); }
    | STRING     { $$ = create_str_node($1); }

    | MINUS expression %prec UMINUS { $$ = create_unop_node(OP_NEG, $2); }
    | PLUS  expression %prec UPLUS  { $$ = create_unop_node(OP_POS, $2); }

    | expression EQ     expression { $$ = create_binop_node(OP_EQ, $1, $3); }
    | expression NEQ    expression { $$ = create_binop_node(OP_NEQ, $1, $3); }
    | expression GE     expression { $$ = create_binop_node(OP_GE, $1, $3); }
    | expression LE     expression { $$ = create_binop_node(OP_LE, $1, $3); }
    | expression LT     expression { $$ = create_binop_node(OP_LT, $1, $3); }
    | expression GT     expression { $$ = create_binop_node(OP_GT, $1, $3); }

    | expression LAND   expression { $$ = create_binop_node(OP_LAND, $1, $3); }
    | expression LOR    expression { $$ = create_binop_node(OP_LOR, $1, $3); }
    |            LNOT   expression { $$ = create_unop_node(OP_LNOT, $2); }

    |            BNOT   expression { $$ = create_unop_node(OP_BNOT, $2); }
    | expression BAND   expression { $$ = create_binop_node(OP_BAND, $1, $3); }
    | expression BOR    expression { $$ = create_binop_node(OP_BOR, $1, $3); }
    | expression BXOR   expression { $$ = create_binop_node(OP_BXOR, $1, $3); }
    | expression BNAND  expression { $$ = create_binop_node(OP_BNAND, $1, $3); }
    | expression BNOR   expression { $$ = create_binop_node(OP_BNOR, $1, $3); }
    | expression BXNOR  expression { $$ = create_binop_node(OP_BXNOR, $1, $3); }

    | expression LSHIFT expression { $$ = create_binop_node(OP_LSHIFT, $1, $3); }
    | expression RSHIFT expression { $$ = create_binop_node(OP_RSHIFT, $1, $3); }

    | expression PLUS   expression { $$ = create_binop_node(OP_ADD, $1, $3); }
    | expression MINUS  expression { $$ = create_binop_node(OP_SUB, $1, $3); }
    | expression MULT   expression { $$ = create_binop_node(OP_MUL, $1, $3); }
    | expression DIV    expression { $$ = create_binop_node(OP_DIV, $1, $3); }
    | expression MOD    expression { $$ = create_binop_node(OP_MOD, $1, $3); }
    
    | IDENTIFIER LPAREN arg_list RPAREN { $$ = create_call_node($1, $3); }
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