/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IDENTIFIER = 258,
     PLUS = 259,
     MINUS = 260,
     TIMES = 261,
     DIV = 262,
     NOT = 263,
     OR = 264,
     AND = 265,
     XOR = 266,
     GT = 267,
     LT = 268,
     GEQ = 269,
     LEQ = 270,
     EQ = 271,
     NEQ = 272,
     ASN = 273,
     BOOLEAN = 274,
     CONST = 275,
     VOID = 276,
     STRING = 277,
     INTEGER = 278,
     FLOAT_NUM = 279,
     IF = 280,
     ELSE = 281,
     WHILE = 282,
     COMMA = 283,
     SEMICOL = 284,
     LPAREN = 285,
     RPAREN = 286,
     LBRACE = 287,
     RBRACE = 288,
     LBRACKET = 289,
     RBRACKET = 290,
     INT = 291,
     BOOL = 292,
     FLOAT = 293,
     VEC2 = 294,
     VEC3 = 295,
     VEC4 = 296,
     BVEC2 = 297,
     BVEC3 = 298,
     BVEC4 = 299,
     IVEC2 = 300,
     IVEC3 = 301,
     IVEC4 = 302,
     DP3 = 303,
     LIT = 304,
     RSQ = 305
   };
#endif
/* Tokens.  */
#define IDENTIFIER 258
#define PLUS 259
#define MINUS 260
#define TIMES 261
#define DIV 262
#define NOT 263
#define OR 264
#define AND 265
#define XOR 266
#define GT 267
#define LT 268
#define GEQ 269
#define LEQ 270
#define EQ 271
#define NEQ 272
#define ASN 273
#define BOOLEAN 274
#define CONST 275
#define VOID 276
#define STRING 277
#define INTEGER 278
#define FLOAT_NUM 279
#define IF 280
#define ELSE 281
#define WHILE 282
#define COMMA 283
#define SEMICOL 284
#define LPAREN 285
#define RPAREN 286
#define LBRACE 287
#define RBRACE 288
#define LBRACKET 289
#define RBRACKET 290
#define INT 291
#define BOOL 292
#define FLOAT 293
#define VEC2 294
#define VEC3 295
#define VEC4 296
#define BVEC2 297
#define BVEC3 298
#define BVEC4 299
#define IVEC2 300
#define IVEC3 301
#define IVEC4 302
#define DP3 303
#define LIT 304
#define RSQ 305




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 2068 of yacc.c  */
#line 57 "parser.y"

  int   token;
  bool  boolean;
  char *str;
  int   num;
  float fnum;



/* Line 2068 of yacc.c  */
#line 160 "y.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


