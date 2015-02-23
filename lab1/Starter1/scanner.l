%{
/**********************************************************************
 *
 * ** Rashad Sookram 998438935**
 * ** Geoffrey Li 998267432 **
 *  Interface to the scanner module for CSC467 course project
 **********************************************************************/


#include "common.h"
#include "parser.tab.h"
#include "string.h"


#define YY_USER_INIT { yyin = inputFile; }
#define	yyinput      input
#define yTRACE(x)    { if (traceScanner) fprintf(traceFile, "TOKEN %3d : %s\n", x, yytext); }
#define yERROR(x)    { fprintf(errorFile, "\nLEXICAL ERROR, LINE %d: %s\n", yyline, x); errorOccurred = TRUE; yyterminate(); }

int CheckInt(void);
int yyline = 1;

%}
%option noyywrap

WS              [ \t\n]
DIGIT           [0-9]
LETTER          [a-zA-Z] 

%%
"/*"        {
              int c1      = 0;
              int c2      = yyinput();
              int curline = yyline;
              for(;;) {
                if (c2 == EOF) {
                  fprintf(errorFile, "\nLEXICAL ERROR, LINE %d: Unmatched /*\n", curline);
                  errorOccurred = TRUE;
                  yyterminate();
                }
                if (c1 == '*' && c2 == '/')
                  break;
                c1 = c2;
                c2 = yyinput();
                if (c1 == '\n' && c2 != EOF)
                  yyline++;
              }
            }

{WS}+ {}

"!"  { yTRACE(NOT);   yylval.token = NOT;   return NOT;   }
"^"  { yTRACE(XOR);   yylval.token = XOR;   return XOR;   }
"+"  { yTRACE(PLUS);  yylval.token = PLUS;  return PLUS ; }
"-"  { yTRACE(MINUS); yylval.token = MINUS; return MINUS; }
"*"  { yTRACE(TIMES); yylval.token = TIMES; return TIMES; }
"/"  { yTRACE(DIV);   yylval.token = DIV;   return DIV;   }
"&&" { yTRACE(AND);   yylval.token = AND;   return AND;   }
"||" { yTRACE(OR);    yylval.token = OR;    return OR;    }
"==" { yTRACE(EQ);    yylval.token = EQ;    return EQ;    }
"!=" { yTRACE(NEQ);   yylval.token = NEQ;   return NEQ;   }
">=" { yTRACE(GEQ);   yylval.token = GEQ;   return GEQ;   }
"<=" { yTRACE(LEQ);   yylval.token = LEQ;   return LEQ;   }
"="  { yTRACE(ASN);   yylval.token = ASN;   return ASN;   }
">"  { yTRACE(GT);    yylval.token = GT;    return GT;    }
"<"  { yTRACE(LT);    yylval.token = LT;    return LT;    }

"true"  { yTRACE(BOOLEAN); yylval.boolean = true;  return BOOLEAN; }
"false" { yTRACE(BOOLEAN); yylval.boolean = false; return BOOLEAN; }

"const" { yTRACE(CONST); yylval.token = CONST; return CONST; }
"void"  { yTRACE(VOID);  yylval.token = VOID;  return VOID;  }

"int"   { yTRACE(INT);   yylval.token = INT;   return INT;   }
"bool"  { yTRACE(BOOL);  yylval.token = BOOL;  return BOOL;  }
"float" { yTRACE(FLOAT); yylval.token = FLOAT; return FLOAT; }
"vec2"  { yTRACE(VEC2);  yylval.token = VEC2;  return VEC2;  }
"vec3"  { yTRACE(VEC3);  yylval.token = VEC3;  return VEC3;  }
"vec4"  { yTRACE(VEC4);  yylval.token = VEC4;  return VEC4;  }
"bvec2" { yTRACE(BVEC2); yylval.token = BVEC2; return BVEC2; }
"bvec3" { yTRACE(BVEC3); yylval.token = BVEC3; return BVEC3; }
"bvec4" { yTRACE(BVEC4); yylval.token = BVEC4; return BVEC4; }
"ivec2" { yTRACE(IVEC2); yylval.token = IVEC2; return IVEC2; }
"ivec3" { yTRACE(IVEC3); yylval.token = IVEC3; return IVEC3; }
"ivec4" { yTRACE(IVEC4); yylval.token = IVEC4; return IVEC4; }

"if"    { yTRACE(IF);    yylval.token = IF;    return IF;    }
"else"  { yTRACE(ELSE);  yylval.token = ELSE;  return ELSE;  }
"while" { yTRACE(WHILE); yylval.token = WHILE; return WHILE; }

"dp3" { yTRACE(DP3); yylval.token = DP3; return DP3; }
"lit" { yTRACE(LIT); yylval.token = LIT; return LIT; }
"rsq" { yTRACE(RSQ); yylval.token = RSQ; return RSQ; }

"(" { yTRACE(LPAREN);    yylval.token = LPAREN;   return LPAREN;   }
")" { yTRACE(RPAREN);    yylval.token = RPAREN;   return RPAREN;   }
"{" { yTRACE(LBRACE);    yylval.token = LBRACE;   return LBRACE;   }
"}" { yTRACE(RBRACE);    yylval.token = RBRACE;   return RBRACE;   }
"[" { yTRACE(LBRACKET);  yylval.token = LBRACKET; return LBRACKET; }
"]" { yTRACE(RBRACKET);  yylval.token = RBRACKET; return RBRACKET; }
"," { yTRACE(COMMA);     yylval.token = COMMA;    return COMMA;    }
";" { yTRACE(SEMICOL);   yylval.token = SEMICOL;  return SEMICOL;  }

\"[^\"]*\" {
	if (strlen(yytext) > MAX_TEXT) {
		yERROR("Text exceeds maximum length");
	}
	yTRACE(STRING);
	return STRING;
}

(-?{DIGIT}+"."{DIGIT}*)/{LETTER} {
    yERROR("Numbers must be separated from identifiers/keywords by whitespace");
}

(-?{DIGIT}*"."{DIGIT}+)/{LETTER} {
    yERROR("Numbers must be separated from identifiers/keywords by whitespace");
}

(-?{DIGIT}+"."{DIGIT}*)|(-?{DIGIT}*"."{DIGIT}+) {
	double val = strtod(yytext, NULL);
	if (errno == ERANGE || val > MAX_INTEGER || val < -MAX_INTEGER) {
		yERROR("Float exceeds maximum length");
	}

	yTRACE(FLOAT_NUM);
	yylval.fnum = (float) val;
	return FLOAT_NUM;
}

"0"{DIGIT}+ {
    yERROR("Octal numbers are not supported");
}

-?{DIGIT}+/{LETTER} {
    yERROR("Numbers must be separated from identifiers/keywords by whitespace");
}

-?{DIGIT}+ {
	double val = strtod(yytext, NULL);
	if (errno == ERANGE || val > MAX_INTEGER || val < -MAX_INTEGER) {
		yERROR("Integer exceeds maximum length");
	}

	yTRACE(INTEGER);
	yylval.num = (int) val;
	return INTEGER;
}

({LETTER}|_)({LETTER}|{DIGIT}|_)* { 
	if (strlen(yytext) > MAX_IDENTIFIER) {
		yERROR("Identifier exceeds maximum length");
	}
	yTRACE(IDENTIFIER);
	yylval.str = (char*) malloc(sizeof(char) * strlen(yytext));
	strcpy(yylval.str, yytext);
	return IDENTIFIER;
}

. {
	yERROR("Unknown token");
}

%%
