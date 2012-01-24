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
 * @param program abstract syntax
 * @param globalTable symbol table
 * @param showVarAlloc flag to enable showProcVars
 * @return void
 **/
void allocVars(Absyn * program, Table * globalTable, boolean showVarAlloc)
{
	Absyn decList;
	Entry *entry;
	unsigned int i;

	/* compute access information for arguments of predefined procs */
	char *builtinProcs[] = {
		"clearAll"	,	"drawCircle"	,
		"drawLine"	,	"exit"		,
		"printc"	,	"printc"	,
		"printi"	,	"readc"		,
		"setPixel"	,	"time"
	};

	i = (sizeof(builtinProcs) / sizeof(builtinProcs[0]));
	while (i-- > 0) {
		/* incoming arguments and offsets */
		entry = lookup(globalTable, newSym(builtinProcs[i]));
	}

	/* compute access information for arguments, parameters and local vars */
	decList = program;
	while (!decList->u.decList.isEmpty) {
		if (decList->u.decList.head->type == ABSYN_PROCDEC) {
			entry = lookup(globalTable, decList->u.decList.head->u.procDec.name);

			entry->u.procEntry.paramSize = setParamOffsets(entry->u.procEntry.paramTypes);
			entry->u.procEntry.localVarSize = setVarOffsets(decList->u.decList.head->u.procDec.decls, globalTable);
			entry->u.procEntry.argSize = setCallOffsets(decList->u.decList.head->u.procDec.body, globalTable);
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
 * @brief Set procedure stackframe offset for (library) functions
 *
 * @param params parameter types of procedure
 * @return int - Offset
 **/
int setParamOffsets(ParamTypes * params)
{
	int offset = 0;

	while (!params->isEmpty) {
		params->offset = offset;
		if (params->isRef){
			offset = offset + REF_BYTE_SIZE;
		} else{
			offset = offset + params->type->byte_size;
		}
		params = params->next;
	}

	return offset;
}

/**
 * @brief Set local variable offsets
 *
 * @return int
 **/
int setVarOffsets(Absyn * decList, Table * symTab)
{
	int offset = 0;
	Entry varEntry;

	while (!decList->decList.isEmpty) {
		if (decList->type == ABSYN_VARDEC) {
			varEntry = lookup(symTab, decList->u.decList.head->u.varDec.name);
			offset = offset + varEntry->u.varEntry.type->byte_size;
			varEntry->u.varEntry.offset = -offset;
		}

		decList = decList->u.decList.tail;
	}

	return varEntry->u.varEntry.offset;
}

/**
 * @brief Calculate offsets of callees
 *
 * @param node ...
 * @param symTab ...
 * @return int
 **/
int setCallOffsets(Absyn * nody, Table * symTab)
{
	switch (node->procDec.decls->callStm) {
		case ABSYN_NAMETY:		;
		case ABSYN_ARRAYTY:		;
		case ABSYN_TYPEDEC:		;
		case ABSYN_PROCDEC:		;
		case ABSYN_PARDEC:		;
		case ABSYN_VARDEC:		;
		case ABSYN_EMPTYSTM:		;
		case ABSYN_COMPSTM:		;
		case ABSYN_ASSIGNSTM:		;
		case ABSYN_IFSTM:		setCallOffsets(node, symTab);
		case ABSYN_WHILESTM:		setCallOffsets(node, symTab);
		case ABSYN_CALLSTM:		setCallOffsets(node, symTab);
		case ABSYN_OPEXP:		;
		case ABSYN_VAREXP:		;
		case ABSYN_INTEXP:		;
		case ABSYN_SIMPLEVAR:		;
		case ABSYN_ARRAYVAR:		;
		case ABSYN_DECLIST:		setCallOffsets(node, symTab);
		case ABSYN_STMLIST:		setCallOffsets(node, symTab);
		case ABSYN_EXPLIST:		;
	}
	return setCallOffsets();
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
