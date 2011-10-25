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
				yylval.intVal.val = allocate(strlen(yytext)+1);
				strcpy(yylval.intVal.val, yytext);
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
	case ARRAY :
	case ELSE :
	case IF :
	case OF :
	case PROC :
	case REF :
	case TYPE :
	case VAR :
	case WHILE :
	case LPAREN :
	case RPAREN :
	case LBRACK :
	case RBRACK :
	case LCURL :
	case RCURL :
	case EQ :
	case NE :
	case LT :
	case LE :
	case GT :
	case GE :
	case ASGN :
	case COLON :
	case COMMA :
	case SEMIC :
	case PLUS :
	case MINUS :
	case STAR :
	case SLASH :
	case IDENT :
	case INTLIT :
	default:
	}
}
