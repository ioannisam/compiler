%{
#include "../parser/ast.h"
#include "../parser/parser.tab.h"

#include <stdio.h>
extern int yylineno;  // line tracking
%}

%option yylineno

%%
[ \t\r]+   ; // ignore whitespace
"//".*     ; // ignore comments

"print"    { printf("PRINT "); return PRINT; }
"if"       { printf("IF "); return IF; }
"else"     { printf("ELSE "); return ELSE; }
"while"    { printf("WHILE "); return WHILE; }

[0-9]+ { yylval.num = atoi(yytext); printf("NUMBER(%s) ", yytext); return NUMBER; }
\"([^\"]*)\" {
    yylval.str = strndup(yytext + 1, strlen(yytext) - 2);
    printf("STRING(%s) ", yylval.str);
    return STRING;
}
[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.str = strdup(yytext);
    printf("IDENTIFIER(%s) ", yylval.str);
    return IDENTIFIER;
}

"="        { printf("ASSIGN "); return ASSIGN; }
"=="       { printf("EQ "); return EQ; }
"<"        { printf("LT "); return LT; }
">"        { printf("GT "); return GT; }
"&"        { printf("AND "); return AND; }
"|"        { printf("OR "); return OR; }
"^"        { printf("XOR "); return XOR; }
"!"        { printf("NOT "); return NOT; }
"<<"       { printf("LSHIFT "); return LSHIFT; }
">>"       { printf("RSHIFT "); return RSHIFT; }
"+"        { printf("PLUS "); return PLUS; }
"-"        { printf("MINUS "); return MINUS; }
"*"        { printf("MULT "); return MULT; }
"/"        { printf("DIV "); return DIV; }

"("        { printf("LPAREN "); return LPAREN; }
")"        { printf("RPAREN "); return RPAREN; }
"{"        { printf("LBRACE "); return LBRACE; }
"}"        { printf("RBRACE "); return RBRACE; }

";"        { printf("SEMICOLON "); return SEMICOLON; }

\n         { 
    printf("NEWLINE\n"); 
    return NEWLINE; 
}

.          { printf("UNKNOWN(%s) ", yytext); return yytext[0]; }
%% // end of rules

int yywrap() { return 1; } // or include -lfl flag at compilation