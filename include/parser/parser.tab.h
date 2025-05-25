/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_INCLUDE_PARSER_PARSER_TAB_H_INCLUDED
# define YY_YY_INCLUDE_PARSER_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 3 "src/parser/parser.y"

    #include "parser/parser.tab.h"
    #include "parser/ast.h"

#line 54 "include/parser/parser.tab.h"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    ERROR = 258,                   /* ERROR  */
    PRINT = 259,                   /* PRINT  */
    IF = 260,                      /* IF  */
    ELSE = 261,                    /* ELSE  */
    WHILE = 262,                   /* WHILE  */
    BREAK = 263,                   /* BREAK  */
    RETURN = 264,                  /* RETURN  */
    NUMBER = 265,                  /* NUMBER  */
    IDENTIFIER = 266,              /* IDENTIFIER  */
    STRING = 267,                  /* STRING  */
    ASSIGN = 268,                  /* ASSIGN  */
    EQ = 269,                      /* EQ  */
    GE = 270,                      /* GE  */
    LE = 271,                      /* LE  */
    LT = 272,                      /* LT  */
    GT = 273,                      /* GT  */
    NEQ = 274,                     /* NEQ  */
    LAND = 275,                    /* LAND  */
    LOR = 276,                     /* LOR  */
    LNOT = 277,                    /* LNOT  */
    BNOT = 278,                    /* BNOT  */
    BAND = 279,                    /* BAND  */
    BOR = 280,                     /* BOR  */
    BXOR = 281,                    /* BXOR  */
    BNAND = 282,                   /* BNAND  */
    BNOR = 283,                    /* BNOR  */
    BXNOR = 284,                   /* BXNOR  */
    LSHIFT = 285,                  /* LSHIFT  */
    RSHIFT = 286,                  /* RSHIFT  */
    PLUS = 287,                    /* PLUS  */
    MINUS = 288,                   /* MINUS  */
    MULT = 289,                    /* MULT  */
    DIV = 290,                     /* DIV  */
    MOD = 291,                     /* MOD  */
    SEMICOLON = 292,               /* SEMICOLON  */
    NEWLINE = 293,                 /* NEWLINE  */
    LPAREN = 294,                  /* LPAREN  */
    RPAREN = 295,                  /* RPAREN  */
    LBRACE = 296,                  /* LBRACE  */
    RBRACE = 297,                  /* RBRACE  */
    LOWER_THAN_ELSE = 298,         /* LOWER_THAN_ELSE  */
    UMINUS = 299,                  /* UMINUS  */
    UPLUS = 300                    /* UPLUS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 27 "src/parser/parser.y"

    ASTNode* node;
    int num;
    char* str;

#line 122 "include/parser/parser.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_INCLUDE_PARSER_PARSER_TAB_H_INCLUDED  */
