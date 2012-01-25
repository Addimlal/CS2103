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

void allocVars(Absyn * program, Table * globalTable, boolean showVarAlloc)
{
	Absyn *prog_copy;
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
		entry = lookup(globalTable, newSym(builtinProcs[i]));
		entry->u.procEntry.paramSize =
		    sendParamOffsets(entry->u.procEntry.paramTypes);
	}

	/* compute access information for arguments, parameters and local vars */
	prog_copy = program;
	while (!prog_copy->u.decList.isEmpty) {
		if (prog_copy->u.decList.head->type == ABSYN_PROCDEC) {
			checkOffsets(prog_copy, globalTable);
		}

		prog_copy = prog_copy->u.decList.tail;
	}

	/* show original variable allocation if requested */
	if (showVarAlloc) {
		showAllocs(program, globalTable);
	}
}

void checkOffsets(Absyn * node, Table * symTab)
{
	Entry *entry;
	ParamTypes *params;
	switch (node->type) {
	case ABSYN_PROCDEC:
		entry = lookup(symTab, node->u.procDec.name);
		params = entry->u.procEntry.paramTypes;

		entry->u.procEntry.paramSize = sendParamOffsets(params);
		entry->u.procEntry.localVarSize = sendVarOffsets(node->u.procDec.decls, symTab);
		setCallOffsets(node->u.procDec.body, symTab);
		break;
	case ABSYN_PARDEC:
		entry = lookup(symTab, node->u.procDec.name);
		params = entry->u.procEntry.paramTypes;
		entry->u.procEntry.paramSize = sendParamOffsets(params);
		break;
	case ABSYN_VARDEC:
		entry = lookup(symTab, node->u.varDec.name);
		sendVarOffsets(entry->u.varEntry.type, symTab);
		return;
	case ABSYN_DECLIST:
		if (!node->u.decList.isEmpty) {
			checkOffsets(node->u.decList.head, symTab);
			checkOffsets(node->u.decList.tail, symTab);
		}

		return;
	}
}

int sendParamOffsets(ParamTypes * params)
{

	int offset = 0;

	while (!params->isEmpty) {
		params->offset = offset;
		if (params->isRef) {
			offset = offset + REF_BYTE_SIZE;
		} else {
			offset = offset + params->type->byte_size;
		}
		params = params->next;
	}

	return offset;
}

int sendVarOffsets(Absyn * decList, Table * symTab)
{
	int offset = 0;
	Entry *varEntry;

	while (!decList->u.decList.isEmpty) {
		if (decList->type == ABSYN_VARDEC) {
			varEntry = lookup(symTab, decList->u.varDec.name);
			offset = offset + varEntry->u.varEntry.type->byte_size;
			varEntry->u.varEntry.offset = -offset;
		}

		decList = decList->u.decList.tail;
	}

	return varEntry->u.varEntry.offset;
}

int countCallArgs(Absyn * node, Table * symTab)
{
	Entry *entry;
	ParamTypes *params;
	int argCounter = 0;

	entry = lookup(symTab, node->u.callStm.name);
	params = entry->u.procEntry.paramTypes;

	if (params->isEmpty) {
		return 0;
	}

	while (!params->isEmpty) {

		argCounter = +4;

		params = params->next;
	}

	return argCounter;
}

Entry *entry = NULL;

void setCallOffsets(Absyn * node, Table * symTab)
{

	int max;

	switch (node->type) {
	case ABSYN_PROCDEC:
		entry = lookup(symTab, node->u.procDec.name);
		entry->u.procEntry.argSize = -1;
		setCallOffsets(node->u.procDec.body, symTab);
		return;

	case ABSYN_CALLSTM:
		if (entry != NULL) {

			max = countCallArgs(node, symTab);

			if (max > entry->u.procEntry.argSize)
				entry->u.procEntry.argSize = max;
		}
		return;

	case ABSYN_DECLIST:
		if (!node->u.decList.isEmpty) {
			setCallOffsets(node->u.decList.head, symTab);
			setCallOffsets(node->u.decList.tail, symTab);
		}

	case ABSYN_STMLIST:
		if (!node->u.decList.isEmpty) {
			setCallOffsets(node->u.stmList.head, symTab);
			setCallOffsets(node->u.stmList.tail, symTab);
		}
		return;

	case ABSYN_IFSTM:
		setCallOffsets(node->u.ifStm.thenPart, symTab);
		setCallOffsets(node->u.ifStm.elsePart, symTab);
		return;

	case ABSYN_WHILESTM:
		setCallOffsets(node->u.whileStm.body, symTab);
		return;

	case ABSYN_COMPSTM:
		setCallOffsets(node->u.compStm.stms, symTab);
		return;

	}
}

void showAllocs(Absyn * node, Table * symTab)
{
	Entry *entry;
	ParamTypes *params;
	int i = 0;

	switch (node->type) {

	case ABSYN_PROCDEC:

		printf("\nVariable allocation for procedure '%s'\n",
		       symToString(node->u.procDec.name));

		entry = lookup(symTab, node->u.procDec.name);

		params = entry->u.procEntry.paramTypes;

		if (params->isEmpty) {

		} else {

			while (!params->isEmpty) {
				i++;

				printf("arg %d: sp + %d\n", i, params->offset);

				params = params->next;
			}

		}

		printf("size of argument area = %d\n",
		       entry->u.procEntry.paramSize);

		showAllocs(node->u.procDec.params,
			   entry->u.procEntry.localTable);
		showAllocs(node->u.procDec.decls,
			   entry->u.procEntry.localTable);

		printf("size of localvar area = %d\n",
		       entry->u.procEntry.localVarSize);
		printf("size of outgoing area = %d\n",
		       entry->u.procEntry.argSize);

		break;
	case ABSYN_PARDEC:
		entry = lookup(symTab, node->u.parDec.name);

		printf("param '%s': fp + %d\n",
		       symToString(node->u.parDec.name),
		       entry->u.varEntry.offset);

		break;
	case ABSYN_VARDEC:
		entry = lookup(symTab, node->u.varDec.name);
		printf("var '%s': fp - %d\n", symToString(node->u.varDec.name),
		       entry->u.varEntry.offset);

		return;
	case ABSYN_DECLIST:
		if (!node->u.decList.isEmpty) {
			showAllocs(node->u.decList.head, symTab);
			showAllocs(node->u.decList.tail, symTab);
		}

		return;

	}
}
