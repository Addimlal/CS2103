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
#include "parser.h"

#define YYDEBUG 1

%}

%union {
	NoVal noVal;
	IntVal intVal;
	StringVal stringVal;
}

%token				ARRAY ELSE IF OF PROC REF TYPE VAR WHILE
				LPAREN RPAREN LBRACK RBRACK LCURL  RCURL
				EQ NE LT LE GT GE ASGN COLON COMMA SEMIC
				PLUS  MINUS  STAR  SLASH   IDENT  INTLIT

%start				program


%%

/*__________________________________________________________________________*/
program		:	declarations
;

/*__________________________________________________________________________*/
declarations	: 	/*empty*/
		| 	type_list declarations
		| 	procedure declarations
;
/*__________________________________________________________________________*/

type_list	:	TYPE IDENT EQ typ SEMIC
;

typ		:	IDENT
		|	ARRAY LCURL INTLIT RCURL OF typ
;

/*__________________________________________________________________________*/
procedure	:	PROC IDENT LPAREN opt_parameters RPAREN
			LCURL
				opt_variables opt_statements
			RCURL
;

/*__________________________________________________________________________*/
parameter	:	IDENT COLON typ
		|	REF IDENT COLON typ
;

add_parameter	:	parameter
		|	parameter COMMA add_parameter
;

opt_parameters	:	/*empty*/
		|	add_parameter
;

/*__________________________________________________________________________*/
expression	:
;

add_expression	:	expression
		|	expression COMMA add_expression
;

opt_expressions	:	/*empty*/
		|	add_expression
;

/*__________________________________________________________________________*/
variable	:	VAR IDENT COLON typ SEMIC
;

add_variables	:	/*empty*/
		|	variable add_variables
;

/*__________________________________________________________________________*/
statement	:	SEMIC
		|	variable ASGN expression
		|	IF LPAREN expression RPAREN statement
		|	IF LPAREN expression RPAREN statement ELSE statement
		|	WHILE LPAREN expression RPAREN statement
		|	LCURL add_statement RCURL
		|	IDENT LPAREN opt_expressions RPAREN
;

add_statement	:	/*empty*/
		|	statement add_statement
;
/*__________________________________________________________________________*/

%%


void yyerror(char *msg) {
	error("%s in line %d", msg, yylval.noVal.line);
}
