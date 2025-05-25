%{
#include "parser/parser.tab.h"
#include "parser/ast.h"

#include <stdio.h>
extern int yylineno;  // line tracking
%}

%option yylineno

%%
[ \t\r]+   ; // ignore whitespace
"//".*     ; // ignore comments
\n         { printf("NEWLINE\n"); }

"int"      { printf("TYPE_INT "); return TYPE_INT; }
"main"     { printf("MAIN "); return MAIN; }

"true"     { printf("TRUE "); return TRUE; }
"false"    { printf("FALSE "); return FALSE; }

"print"    { printf("PRINT "); return PRINT; }
"if"       { printf("IF "); return IF; }
"else"     { printf("ELSE "); return ELSE; }
"while"    { printf("WHILE "); return WHILE; }
"break"    { printf("BREAK "); return BREAK; }
"return"   { printf("RETURN "); return RETURN; }

[0-9]+ { yylval.num = atoi(yytext); printf("NUMBER(%s) ", yytext); return NUMBER; }
\"([^\"]*)\" {
    yylval.str = strndup(yytext+1, strlen(yytext)-2);
    printf("STRING(%s) ", yylval.str);
    return STRING;
}
[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.str = strdup(yytext);
    printf("IDENTIFIER(%s) ", yylval.str);
    return IDENTIFIER;
}

"<<"       { printf("LSHIFT "); return LSHIFT; }
">>"       { printf("RSHIFT "); return RSHIFT; }

"=="       { printf("EQ "); return EQ; }
"!="       { printf("NEQ "); return NEQ; }
">="       { printf("GE "); return GE; }
"<="       { printf("LE "); return LE; }
"<"        { printf("LT "); return LT; }
">"        { printf("GT "); return GT; }
"!"        { printf("LNOT "); return LNOT; }
"&&"       { printf("LAND "); yylval.num = 0; return LAND; }
"||"       { printf("LOR ");  yylval.num = 0; return LOR; }

"="        { printf("ASSIGN "); return ASSIGN; }

"&"        { printf("BAND "); return BAND; }
"|"        { printf("BOR ");  return BOR; }
"^"        { printf("BXOR "); return BXOR; }
"~&"       { printf("BNAND "); return BNAND; }
"~|"       { printf("BNOR ");  return BNOR; }
"~^"       { printf("BXNOR "); return BXNOR; }
"~"        { printf("BNOT ");  return BNOT; }

"+"        { printf("PLUS "); return PLUS; }
"-"        { printf("MINUS "); return MINUS; }
"*"        { printf("MULT "); return MULT; }
"/"        { printf("DIV "); return DIV; }
"%"        { printf("MOD "); return MOD; }

"("        { printf("LPAREN "); return LPAREN; }
")"        { printf("RPAREN "); return RPAREN; }
"{"        { printf("LBRACE "); return LBRACE; }
"}"        { printf("RBRACE "); return RBRACE; }

";"        { printf("SEMICOLON "); return SEMICOLON; }

.          { 
    fprintf(stderr, "Error: Invalid character '%s' at line %d\n", yytext, yylineno);
    return ERROR; 
}
%% // end of rules

int yywrap() { return 1; } // or include -lfl flag at compilation