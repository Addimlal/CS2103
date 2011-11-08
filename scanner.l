/* C Code der an Flex übergeben wird */
%{

/*
 * scanner.l -- SPL scanner specification
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "scanner.h"
#include "preliminary.h"

YYSTYPE yylval;
static int lineNumber = 1;

%}

/* Makros für die Regulären Ausdrücke */
IDENT		([a-zA-Z_][a-zA-Z0-9_]*)
NUM		([0-9]+)
HEXLIT		(0x[0-9a-fA-F]+)
ASCII		(\'[u00-u127]\')
COMMENT		(\/\/.*)
LF		(\n)
WHITESPACE	([ \t]+)

/* Regeln für gematchte Token */
%%

array				{ yylval.noVal.line = lineNumber; return ARRAY;	}
else 				{ yylval.noVal.line = lineNumber; return ELSE; }
if 				{ yylval.noVal.line = lineNumber; return IF; }
of  				{ yylval.noVal.line = lineNumber; return OF; }
proc  				{ yylval.noVal.line = lineNumber; return PROC; }
ref 				{ yylval.noVal.line = lineNumber; return REF; }
type  				{ yylval.noVal.line = lineNumber; return TYPE; }
var  				{ yylval.noVal.line = lineNumber; return VAR; }
while  				{ yylval.noVal.line = lineNumber; return WHILE; }
\(	  			{ yylval.noVal.line = lineNumber; return LPAREN; }
\)  				{ yylval.noVal.line = lineNumber; return RPAREN; }
\[  				{ yylval.noVal.line = lineNumber; return LBRACK; }
\] 	 			{ yylval.noVal.line = lineNumber; return RBRACK; }
\{  				{ yylval.noVal.line = lineNumber; return LCURL; }
\}  				{ yylval.noVal.line = lineNumber; return RCURL; }
\=  				{ yylval.noVal.line = lineNumber; return EQ; }
\!=  				{ yylval.noVal.line = lineNumber; return NE; }
\<  				{ yylval.noVal.line = lineNumber; return LT; }
\<=  				{ yylval.noVal.line = lineNumber; return LE; }
\>  				{ yylval.noVal.line = lineNumber; return GT; }
\>=  				{ yylval.noVal.line = lineNumber; return GE; }
\:=  				{ yylval.noVal.line = lineNumber; return ASGN; }
\:  				{ yylval.noVal.line = lineNumber; return COLON; }
\,  				{ yylval.noVal.line = lineNumber; return COMMA; }
\;  				{ yylval.noVal.line = lineNumber; return SEMIC; }
\+  				{ yylval.noVal.line = lineNumber; return PLUS; }
\-  				{ yylval.noVal.line = lineNumber; return MINUS; }
\*  				{ yylval.noVal.line = lineNumber; return STAR; }
\/  				{ yylval.noVal.line = lineNumber; return SLASH; }

{IDENT}  			{
				yylval.noVal.line = lineNumber;
				yylval.stringVal.val = allocate(strlen(yytext) + 1);
				strcpy(yylval.stringVal.val, yytext);
				return IDENT;
				}

{NUM}				{
				yylval.noVal.line = lineNumber;
				yylval.intVal.val = atoi(yytext);
				return INTLIT;
				}

{HEXLIT}  			{
				yylval.intVal.line = lineNumber;
				yylval.intVal.val = atoi(yytext);
				return INTLIT;
				}

{ASCII}  			{
				yylval.intVal.line = lineNumber;
				yylval.intVal.val = atoi(yytext);
				return INTLIT;
				}


{COMMENT}			/* Bei Einzeiligen Kommentaren nichts tun */

{LF}				{ lineNumber++; }

{WHITESPACE}          		/* Tabulatoren und Linefeeds aufessen */

.				{
				error("Illegal Symbol at %s \t in line %i.\n", yytext, lineNumber);
				}
%%


/* Hilfsfunktionen die für gematchte */
int yywrap(void) {
  return 1;
}


void showToken(int token) {
	switch(token) {
	case 0:
		printf("TOKEN = - - EOF --\n");
		break;
	case ARRAY:
		printf("TOKEN = %s in line %i\n", "ARRAY", lineNumber);
		break;
	case ELSE:
		printf("TOKEN = %s in line %i\n", "ELSE", lineNumber);
		break;
	case IF:
		printf("TOKEN = %s in line %i\n", "IF", lineNumber);
		break;
	case OF:
		printf("TOKEN = %s in line %i\n", "OF", lineNumber);
		break;
	case PROC:
		printf("TOKEN = %s in line %i\n", "PROC", lineNumber);
		break;
	case REF:
		printf("TOKEN = %s in line %i\n", "REF", lineNumber);
		break;
	case TYPE:
		printf("TOKEN = %s in line %i\n", "TYPE", lineNumber);
		break;
	case VAR:
		printf("TOKEN = %s in line %i\n", "VAR", lineNumber);
		break;
	case WHILE:
		printf("TOKEN = %s in line %i\n", "WHILE", lineNumber);
		break;
	case LPAREN:
		printf("TOKEN = %s in line %i\n", "LPAREN", lineNumber);
		break;
	case RPAREN:
		printf("TOKEN = %s in line %i\n", "RPAREN", lineNumber);
		break;
	case LBRACK:
		printf("TOKEN = %s in line %i\n", "LBRACK", lineNumber);
		break;
	case RBRACK:
		printf("TOKEN = %s in line %i\n", "RBRACK", lineNumber);
		break;
	case LCURL:
		printf("TOKEN = %s in line %i\n", "LCURL", lineNumber);
		break;
	case RCURL:
		printf("TOKEN = %s in line %i\n", "RCURL", lineNumber);
		break;
	case EQ:
		printf("TOKEN = %s in line %i\n", "EQ", lineNumber);
		break;
	case NE:
		printf("TOKEN = %s in line %i\n", "NE", lineNumber);
		break;
	case LT:
		printf("TOKEN = %s in line %i\n", "LT", lineNumber);
		break;
	case LE:
		printf("TOKEN = %s in line %i\n", "LE", lineNumber);
		break;
	case GT:
		printf("TOKEN = %s in line %i\n", "GT", lineNumber);
		break;
	case GE:
		printf("TOKEN = %s in line %i\n", "GE", lineNumber);
		break;
	case ASGN:
		printf("TOKEN = %s in line %i\n", "ASGN", lineNumber);
		break;
	case COLON:
		printf("TOKEN = %s in line %i\n", "COLON", lineNumber);
		break;
	case COMMA:
		printf("TOKEN = %s in line %i\n", "COMMA", lineNumber);
		break;
	case SEMIC:
		printf("TOKEN = %s in line %i\n", "SEMIC", lineNumber);
		break;
	case PLUS:
		printf("TOKEN = %s in line %i\n", "PLUS", lineNumber);
		break;
	case MINUS:
		printf("TOKEN = %s in line %i\n", "MINUS", lineNumber);
		break;
	case STAR:
		printf("TOKEN = %s in line %i\n", "STAR", lineNumber);
		break;
	case SLASH:
		printf("TOKEN = %s in line %i\n", "SLASH", lineNumber);
		break;
	case IDENT:
		printf("TOKEN = %s in line %i, value=\"%s\"\n", "IDENT", yylval.stringVal.line, yylval.stringVal.val);
		break;
	case INTLIT:
		printf("TOKEN = %s in line %i, value=\"%i\n\"", "INTLIT", yylval.intVal.line, yylval.intVal.val);
		break;
	default:
		error("No match found - undefined Token.");
		break;
	}
}
