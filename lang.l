%{
#include <stdio.h>
#include "parser.tab.h"
%}

%%
[ \t]+     ; // Ignore whitespace
"print"    { printf("PRINT "); return PRINT; }
"if"       { printf("IF "); return IF; }
"else"     { printf("ELSE "); return ELSE; }
"while"    { printf("WHILE "); return WHILE; }
[0-9]+     { printf("NUMBER(%s) ", yytext); return NUMBER; }
[a-zA-Z_][a-zA-Z0-9_]*   { printf("IDENTIFIER(%s) ", yytext); return IDENTIFIER; }
"//".*     ; // Ignore comments
"="        { printf("ASSIGN "); return ASSIGN; }
"=="       { printf("EQ "); return EQ; }
"<"        { printf("LT "); return LT; }
">"        { printf("GT "); return GT; }
"+"        { printf("PLUS "); return PLUS; }
"-"        { printf("MINUS "); return MINUS; }
"*"        { printf("MULT "); return MULT; }
"/"        { printf("DIV "); return DIV; }
";"        { printf("SEMICOLON "); return SEMICOLON; }
\"[^\"]*\" { printf("STRING(%s) ", yytext); return STRING; }
\n         { printf("NEWLINE\n"); return NEWLINE; }
.          { printf("UNKNOWN(%s) ", yytext); return yytext[0]; } // Return other characters as they are
%%

int yywrap() { return 1; }