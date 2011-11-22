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
%type <node>			program declarations type_def typ procedure
				parameter add_parameter opt_parameters
				variable variable_decl opt_variables
				expression term factor_expr bool_expr
				add_expression opt_expressions statement
				opt_statements
//////////////////////////////////////////////////////////////////////////////


/*______________________________Startsymbol festlegen_______________________*/
%start			program
//////////////////////////////////////////////////////////////////////////////


%%

/*______________________________Hauptprogramm_______________________________*/
program		:	declarations
			{ progTree = $1; }
;
//////////////////////////////////////////////////////////////////////////////


/*______________________________Deklarationen_______________________________*/
declarations	: 	/*empty*/
			{ $$ = emptyDecList(); }
		| 	type_def declarations
			{ $$ = newDecList($1, $2); }
		| 	procedure declarations
			{ $$ = newDecList($1, $2); }
;
//////////////////////////////////////////////////////////////////////////////

/*______________________________Typen_______________________________________*/

type_def	:	TYPE IDENT EQ typ SEMIC
			{ $$ = newTypeDec($1.line, newSym($2.val), $4); }
;

typ		:	IDENT
			{ $$ = newNameTy($1.line, newSym($1.val)); }
		|	ARRAY LBRACK INTLIT RBRACK OF typ
			{ $$ = newArrayTy($1.line, $3.val, $6); }
;
//////////////////////////////////////////////////////////////////////////////


/*_____________________________Prozeduren___________________________________*/
procedure	:	PROC IDENT LPAREN opt_parameters RPAREN
			LCURL
				opt_variables opt_statements
			RCURL
			{ $$ = newProcDec($1.line, newSym($2.val), $4, $7, $8); }
;
//////////////////////////////////////////////////////////////////////////////


/*____________________________Parameter_____________________________________*/
parameter	:	IDENT COLON typ
			{ $$ = newParDec($1.line, newSym($1.val), $3, FALSE); }
		|	REF IDENT COLON typ
			{ $$ = newParDec($1.line, newSym($2.val), $4, TRUE); }
;

add_parameter	:	parameter
			{ $$ = newDecList($1, emptyDecList()); }
		|	parameter COMMA add_parameter
			{ $$ = newDecList($1, $3); }
;

opt_parameters	:	/*empty*/
			{ $$ = emptyDecList(); }
		|	add_parameter
			{ $$ = $1; }
;
//////////////////////////////////////////////////////////////////////////////


/*_____________________________Variablen____________________________________*/
variable	:	IDENT
			{ $$ = newSimpleVar($1.line, newSym($1.val)); }
		|	variable LBRACK expression RBRACK
			{ $$ = newArrayVar($1->line, $1, $3); }
;

variable_decl	:	VAR IDENT COLON typ SEMIC
			{ $$ = newVarDec($1.line, newSym($2.val), $4); }
;

opt_variables	:	/*empty*/
			{ $$ = emptyDecList(); }
		|	variable_decl opt_variables
			{ $$ = newDecList($1, $2); }
;
//////////////////////////////////////////////////////////////////////////////


/*_____________________________Ausdrücke____________________________________*/
expression	:	expression PLUS term
			{ $$ = newOpExp($2.line, ABSYN_OP_ADD, $1, $3); }
		|	expression MINUS term
			{ $$ = newOpExp($2.line, ABSYN_OP_SUB, $1, $3); }
		|	term
			{ $$ = $1; }
;

term		:	term STAR factor_expr
			{ $$ = newOpExp($2.line, ABSYN_OP_MUL, $1, $3); }
		|	term SLASH factor_expr
			{ $$ = newOpExp($2.line, ABSYN_OP_DIV, $1, $3); }
		|	factor_expr
			{ $$ = $1; }
;

factor_expr	:	MINUS factor_expr
			{ $$ = newOpExp($1.line, ABSYN_OP_SUB, newIntExp($1.line, 0), $2); }
		|	INTLIT
			{ $$ = newIntExp($1.line, $1.val); }
		|	variable
			{ $$ = newVarExp($1->line, $1); }
		|	LPAREN expression RPAREN
			{ $$ = $2 }
;

bool_expr	:	expression EQ expression		// =
			{ $$ = newOpExp($2.line, ABSYN_OP_EQU, $1, $3); }
		|	expression NE expression		// !=
			{ $$ = newOpExp($2.line, ABSYN_OP_NEQ, $1, $3); }
		|	expression LT expression		// <
			{ $$ = newOpExp($2.line, ABSYN_OP_LST, $1, $3); }
		|	expression LE expression		// <=
			{ $$ = newOpExp($2.line, ABSYN_OP_LSE, $1, $3); }
		|	expression GT expression		// >
			{ $$ = newOpExp($2.line, ABSYN_OP_GRT, $1, $3); }
		|	expression GE expression		// >=
			{ $$ = newOpExp($2.line, ABSYN_OP_GRE, $1, $3); }
;

add_expression	:	expression
			{ $$ = newExpList($1, emptyExpList()); }
		|	expression COMMA add_expression
			{ $$ = newExpList($1, $3); }
;

opt_expressions	:	/* empty */
			{ $$ = emptyExpList(); }
		|	add_expression
			{ $$ = $1; }
;
//////////////////////////////////////////////////////////////////////////////


/*_____________________________Statements___________________________________*/
statement	:	SEMIC
			{ $$ = newEmptyStm($1.line); }
		|	variable ASGN expression
			{ $$ = newAssignStm($1->line, $1, $3); }
		|	IF LPAREN bool_expr RPAREN statement
			{ $$ = newIfStm($1.line, $3, $5, newEmptyStm($1.line)); }
		|	IF LPAREN bool_expr RPAREN statement ELSE statement
			{ $$ = newIfStm($1.line, $3, $5, $7); }
		|	WHILE LPAREN bool_expr RPAREN statement
			{ $$ = newWhileStm($1.line, $3, $5); }
		|	LCURL opt_statements RCURL
			{ $$ = newCompStm($1.line, $2); }
		|	IDENT LPAREN opt_expressions RPAREN
			{ $$ = newCallStm($1.line, newSym($1.val), $3); }
;

opt_statements	:	/*empty*/
			{ $$ = emptyStmList(); }
		|	statement opt_statements
			{ $$ = newStmList($1, $2); }
;
//////////////////////////////////////////////////////////////////////////////


%%


void yyerror(char *msg) {
	error("%s in line %d", msg, yylval.noVal.line);
}
