/*
 * varalloc.c -- variable allocation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "sym.h"
#include "types.h"
#include "absyn.h"
#include "table.h"
#include "varalloc.h"

/**
 * @brief ...
 *
 * @param program ...
 * @param globalTable ...
 * @param showVarAlloc ...
 * @return void
 **/
void allocVars(Absyn * program, Table * globalTable, boolean showVarAlloc)
{
	Absyn decList = program;
	Entry *procEntry;
	unsigned int i = 10;

	/* compute access information for arguments of predefined procs */
	char *builtinProcs[10] = {
		"clearAll"	,	"drawCircle"	,
		"drawLine"	,	"exit"		,
		"printc"	,	"printc"	,
		"printi"	,	"readc"		,
		"setPixel"	,	"time"
	};

	while (i--) {
		setProcOffsets(builtinProcs[i], globalTable, TRUE);
	}

	/* compute access information for arguments, parameters and local vars */
	while (!decList->u.decList.isEmpty) {
		if (decList->u.decList.head->type == ABSYN_PROCDEC) {
			procEntry = lookup(globalTable, decList->u.decList.head->u.procDec.name);
			/** TODO: param-, arg-, variable-, and localVariable offsets are not implemented yet. */
		}

		/* continue with tail */
		decList = decList->u.decList.tail;
	}

	/* compute outgoing area sizes */

	/* show original variable allocation if requested */
	if (showVarAlloc) {
		showProcVars(program, globalTable);
	}
}

/**
 * @brief ...
 *
 * @param procs ...
 * @param symTab ...
 * @param builtin ...
 * @return void
 **/
void setProcOffsets(char *procs, Table * symTab, boolean * builtin)
{
	if (!builtin) {

	} else {

	}
}

/**
 * @brief Show variable allocations for a program's procedures in the symbol table
 *
 * @param node declarations lists of a program
 * @param symTab global symbol table for lookups
 * @return void
 **/
void showProcVars(Absyn * node, Table * symTab)
{

}
