%{
/***********************************************************************
 * --YOUR GROUP INFO SHOULD GO HERE--
 * 
 *   Interface to the parser module for CSC467 course project.
 * 
 *   Phase 2: Implement context free grammar for source language, and
 *            parse tracing functionality.
 *   Phase 3: Construct the AST for the source language program.
 ***********************************************************************/

/***********************************************************************
 *  C Definitions and external declarations for this module.
 *
 *  Phase 3: Include ast.h if needed, and declarations for other global or
 *           external vars, functions etc. as needed.
 ***********************************************************************/

#include <string.h>
#include "common.h"
//#include "ast.h"
//#include "symbol.h"
//#include "semantic.h"
#define YYERROR_VERBOSE
#define yTRACE(x)    { if (traceParser) fprintf(traceFile, "%s\n", x); }

void yyerror(char* s);    /* what to do in case of error            */
int yylex();              /* procedure for calling lexical analyzer */
extern int yyline;        /* variable holding current line number   */

%}

/***********************************************************************
 *  Yacc/Bison declarations.
 *  Phase 2:
 *    1. Add precedence declarations for operators (after %start declaration)
 *    2. If necessary, add %type declarations for some nonterminals
 *  Phase 3:
 *    1. Add fields to the union below to facilitate the construction of the
 *       AST (the two existing fields allow the lexical analyzer to pass back
 *       semantic info, so they shouldn't be touched).
 *    2. Add <type> modifiers to appropriate %token declarations (using the
 *       fields of the union) so that semantic information can by passed back
 *       by the scanner.
 *    3. Make the %type declarations for the language non-terminals, utilizing
 *       the fields of the union as well.
 ***********************************************************************/

%{
#define YYDEBUG 1
%}


// TODO:Modify me to add more data types
// Can access me from flex useing yyval

%union {
  int   token;
  bool  boolean;
  char *str;
  int   num;
  float fnum;
}

%token <str>	 IDENTIFIER
%token <token>   PLUS MINUS TIMES DIV
%token <token>   NOT OR AND XOR GT LT GEQ LEQ EQ NEQ
%token <token>   ASN
%token <boolean> BOOLEAN
%token <token>   CONST
%token <token>   VOID
%token <str>     STRING
%token <num>     INTEGER
%token <fnum>    FLOAT_NUM  
%token <token>	 IF  
%token <token>	 ELSE  
%token <token>	 WHILE  
%token <token>	 COMMA  
%token <token>	 SEMICOL  
%token <token>	 LPAREN  
%token <token>	 RPAREN  
%token <token>	 LBRACE  
%token <token>	 RBRACE  
%token <token>	 LBRACKET  
%token <token>	 RBRACKET
%token <token>   INT
%token <token>   BOOL
%token <token>   FLOAT
%token <token>   VEC2
%token <token>   VEC3
%token <token>   VEC4
%token <token>   BVEC2
%token <token>   BVEC3
%token <token>   BVEC4
%token <token>   IVEC2
%token <token>   IVEC3
%token <token>   IVEC4
%token <token>   DP3
%token <token>   LIT
%token <token>   RSQ


%start    program

%%

/***********************************************************************
 *  Yacc/Bison rules
 *  Phase 2:
 *    1. Replace grammar found here with something reflecting the source
 *       language grammar
 *    2. Implement the trace parser option of the compiler
 *  Phase 3:
 *    1. Add code to rules for construction of AST.
 ***********************************************************************/
program
  :   tokens       
  ;
tokens
  :  tokens token  
  |      
  ;
// TODO: replace myToken with the token the you defined.
token
  :     IDENTIFIER 
  |     PLUS 
  |     MINUS 
  |     TIMES 
  |     DIV
  |     NOT                     
  |     OR                     
  |     AND                     
  |     XOR                     
  |     GT                     
  |     LT                     
  |     GEQ                     
  |     LEQ                     
  |     EQ                     
  |     NEQ                     
  |     ASN
  |     BOOLEAN                     
  |     CONST                     
  |     VOID                     
  |     STRING                     
  |     INTEGER                     
  |     FLOAT_NUM                     
  |     INT
  |     BOOL
  |     FLOAT
  |     VEC2
  |     VEC3
  |     VEC4
  |     BVEC2
  |     BVEC3
  |     BVEC4
  |     IVEC2
  |     IVEC3
  |     IVEC4
  |     IF                     
  |     ELSE                     
  |     WHILE                     
  |     COMMA                     
  |     SEMICOL                     
  |     LPAREN                     
  |     RPAREN                     
  |     LBRACE                     
  |     RBRACE                     
  |     LBRACKET                     
  |     RBRACKET                     
  |     DP3                     
  |     LIT                     
  |     RSQ                     
  ;


%%

/***********************************************************************ol
 * Extra C code.
 *
 * The given yyerror function should not be touched. You may add helper
 * functions as necessary in subsequent phases.
 ***********************************************************************/
void yyerror(char* s) {
  if (errorOccurred)
    return;    /* Error has already been reported by scanner */
  else
    errorOccurred = 1;
        
  fprintf(errorFile, "\nPARSER ERROR, LINE %d",yyline);
  if (strcmp(s, "parse error")) {
    if (strncmp(s, "parse error, ", 13))
      fprintf(errorFile, ": %s\n", s);
    else
      fprintf(errorFile, ": %s\n", s+13);
  } else
    fprintf(errorFile, ": Reading token %s\n", yytname[YYTRANSLATE(yychar)]);
}

