%{
    #include <stdio.h>
    extern int yylex(void);
    int yyerror(char* msg);
%}

%token PRINT IF ELSE WHILE NUMBER IDENTIFIER ASSIGN EQ LT GT PLUS MINUS MULT DIV SEMICOLON STRING NEWLINE

%%
program:
    | program statement NEWLINE
    | program statement
    ;

statement:
    PRINT STRING SEMICOLON
    | IF '(' expression ')' statement
    | WHILE '(' expression ')' statement
    | expression SEMICOLON
    ;

expression:
    IDENTIFIER ASSIGN NUMBER
    | IDENTIFIER EQ NUMBER
    | IDENTIFIER LT NUMBER
    | IDENTIFIER GT NUMBER
    ;

%%
int main() {
    yyparse();
    return 0;
}

int yyerror(char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
    return 1;
}