%{

/*
 * parser.y -- SPL parser specification
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "scanner.h"
#include "sym.h"
#include "absyn.h"
#include "parser.h"

#define YYDEBUG 1

Absyn *progTree;

%}

%union {
	NoVal noVal;
	IntVal intVal;
	StringVal stringVal;
	Absyn *node;
}

/*______________________________Tokendefinitionen___________________________*/
%token	<noVal>			ARRAY ELSE IF OF PROC REF TYPE VAR WHILE
				LPAREN RPAREN LBRACK RBRACK LCURL  RCURL
				EQ NE LT LE GT GE ASGN COLON COMMA SEMIC
				PLUS  MINUS  STAR  SLASH

%token <stringVal>		IDENT

%token <intVal>			INTLIT
//////////////////////////////////////////////////////////////////////////////


/*______________________________Rückgabetypen der Non-Terminale_____________*/
%type <node>			program declarations type_list typ procedure
				parameter add_parameter opt_parameters
				variable variable_decl opt_variables
				expression term factor_expr bool_expr
				add_expression opt_expressions statement
				opt_statements
//////////////////////////////////////////////////////////////////////////////


/*______________________________Startsymbol festlegen_______________________*/
%start				program
//////////////////////////////////////////////////////////////////////////////


%%

/*______________________________Hauptprogramm_______________________________*/
program		:	declarations
;
//////////////////////////////////////////////////////////////////////////////


/*______________________________Deklarationen_______________________________*/
declarations	: 	/*empty*/
		| 	type_list declarations
		| 	procedure declarations
;
//////////////////////////////////////////////////////////////////////////////

/*______________________________Typen_______________________________________*/

type_list	:	TYPE IDENT EQ typ SEMIC
;

typ		:	IDENT
		|	ARRAY LBRACK INTLIT RBRACK OF typ
;
//////////////////////////////////////////////////////////////////////////////


/*_____________________________Prozeduren___________________________________*/
procedure	:	PROC IDENT LPAREN opt_parameters RPAREN
			LCURL
				opt_variables opt_statements
			RCURL
;
//////////////////////////////////////////////////////////////////////////////


/*____________________________Parameter_____________________________________*/
parameter	:	IDENT COLON typ
		|	REF IDENT COLON typ
;

add_parameter	:	parameter
		|	parameter COMMA add_parameter
;

opt_parameters	:	/*empty*/
		|	add_parameter
;
//////////////////////////////////////////////////////////////////////////////


/*_____________________________Variablen____________________________________*/
variable	:	IDENT
		|	variable LBRACK expression RBRACK
;

variable_decl	:	VAR IDENT COLON typ SEMIC
;

opt_variables	:	/*empty*/
		|	variable_decl opt_variables
;
//////////////////////////////////////////////////////////////////////////////


/*_____________________________Ausdrücke____________________________________*/
expression	:	expression PLUS term
		|	expression MINUS term
		|	term
;

term		:	term STAR factor_expr
		|	term SLASH factor_expr
		|	factor_expr
;

factor_expr	:	MINUS factor_expr
		|	INTLIT
		|	variable
		|	LPAREN expression RPAREN
;

bool_expr	:	expression EQ expression		// =
		|	expression NE expression		// !=
		|	expression LT expression		// <
		|	expression LE expression		// <=
		|	expression GT expression		// >
		|	expression GE expression		// >=
;

add_expression	:	expression
		|	expression COMMA add_expression
;

opt_expressions	:	/* empty */
		|	add_expression
;
//////////////////////////////////////////////////////////////////////////////


/*_____________________________Statements___________________________________*/
statement	:	SEMIC
		|	variable ASGN expression
		|	IF LPAREN bool_expr RPAREN statement
		|	IF LPAREN bool_expr RPAREN statement ELSE statement
		|	WHILE LPAREN bool_expr RPAREN statement
		|	LCURL opt_statements RCURL
		|	IDENT LPAREN opt_expressions RPAREN
;

opt_statements	:	/*empty*/
		|	statement opt_statements
;
//////////////////////////////////////////////////////////////////////////////


%%


void yyerror(char *msg) {
	error("%s in line %d", msg, yylval.noVal.line);
}
