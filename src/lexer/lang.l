%{
#include "../parser/ast.h"
#include "../parser/parser.tab.h"

#include <stdio.h>
extern int yylineno;  // line tracking
%}

%option yylineno

%%
[ \t]+     ; // Ignore whitespace

"print"    { printf("PRINT "); return PRINT; }
"if"       { printf("IF "); return IF; }
"else"     { printf("ELSE "); return ELSE; }
"while"    { printf("WHILE "); return WHILE; }

[0-9]+     { printf("NUMBER(%s) ", yytext); return NUMBER; }
\"([^\"]*)\" {
    yylval.str = strdup(yytext+1);
    yylval.str[strlen(yylval.str)-1] = '\0';  // remove closing quote
    printf("STRING(%s) ", yylval.str);
    return STRING;
}
[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.str = strdup(yytext);
    printf("IDENTIFIER(%s) ", yylval.str);
    return IDENTIFIER;
}

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

\n         { 
    printf("NEWLINE\n"); 
    yylineno++;  // Ensure line numbers increment
    return NEWLINE; 
}

.          { printf("UNKNOWN(%s) ", yytext); return yytext[0]; } // Return other characters as they are
%%

int yywrap() { return 1; } // or include -lfl flag in compilation