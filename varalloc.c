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

	Absyn *node;
	Entry *entry;
	unsigned int i;

	/* compute access information for arguments of predefined procs */
	char *builtinProcs[] = {
		"clearAll"	,	"drawCircle"	,
		"drawLine"	, 	"exit" 		,
		"printc"	,	"printc"	,
		"printi"	,	"readc"		,
		"setPixel"	,	"time"
	};

	i = (sizeof(builtinProcs) / sizeof(builtinProcs[0]));
	while (i-- > 0) {
		entry = lookup(globalTable, newSym(builtinProcs[i]));
		entry->u.procEntry.paramSize =
		    setParamOffsets(entry->u.procEntry.paramTypes, TRUE);
	}

	/* compute access information for arguments, parameters and local vars */
	node = program;
	while (!node->u.decList.isEmpty) {
		if (node->u.decList.head->type == ABSYN_PROCDEC) {
			entry = lookup(globalTable, node->u.decList.head->u.procDec.name);

			/* set incoming arguments offsets */
			entry->u.procEntry.paramSize =
			setParamOffsets(entry->u.procEntry.paramTypes, FALSE);

			/* set outgoing arguments offsets */
			entry->u.procEntry.paramSize =
			setArgOffsets(node->u.decList.head->u.procDec.params,
				      entry->u.procEntry.localTable, entry);

			/* set local variable offsets */
			entry->u.procEntry.localVarSize=
			setVarOffsets(node->u.decList.head->u.procDec.decls,
				      entry->u.procEntry.localTable, entry);
		}

		node = node->u.decList.tail;
	}

	/* compute outgoing area sizes */
	node = program;
	while (!node->u.decList.isEmpty) {
		if (node->u.decList.head->type == ABSYN_PROCDEC) {
			entry =
			lookup(globalTable, node->u.decList.head->u.procDec.name);

			entry->u.procEntry.argSize =
			checkLocalOffsets(node->u.decList.head->u.procDec.body, globalTable);
		}

		node = node->u.decList.tail;
	}

	/* show variable allocation if requeintsted */
	if (showVarAlloc) {
		showVars(program, globalTable);
	}
}


int setParamOffsets(ParamTypes * params, boolean builtinProcs)
{
	int paramOffset = 0;

	while (!params->isEmpty) {
		params->offset = paramOffset;
		if (params->isRef && !builtinProcs) {
			paramOffset += REF_BYTE_SIZE;
		} else {
			paramOffset += params->type->byte_size;
		}
		params = params->next;
	}

	return paramOffset;
}


int setVarOffsets(Absyn * node, Table * symTab, Entry * entry)
{
	int varOffset = 0;

	/* set variable offsets */
	while (!node->u.decList.isEmpty) {
		if (node->u.decList.head->type == ABSYN_VARDEC) {
			entry = lookup(symTab, node->u.decList.head->u.varDec.name);
			varOffset += entry->u.varEntry.type->byte_size;
			entry->u.varEntry.offset = -(varOffset);
		}

		node = node->u.decList.tail;
	}

	return varOffset;
}

int setArgOffsets(Absyn * node, Table * symTab, Entry * entry)
{
	Absyn *procDec = node;
	int argOffset = 0;

	while (!procDec->u.decList.isEmpty) {
		if (procDec->u.decList.head->type == ABSYN_PARDEC) {
			entry = lookup(symTab, procDec->u.decList.head->u.parDec.name);
			entry->u.varEntry.offset = argOffset;
			if (entry->u.varEntry.isRef) {
				argOffset += REF_BYTE_SIZE;
			} else {
				argOffset += INT_BYTE_SIZE;
			}
		}
		procDec = procDec->u.decList.tail;
	}

	return argOffset;
}

int setLocalAreaOffset(Absyn * node, Table * globalTable)
{
	Entry *callEntry;
	int area = -1,
	    newarea,
	    ifarea,
	    elsearea;

	while (!node->u.stmList.isEmpty) {
		newarea = -1;

		switch (node->u.stmList.head->type) {
		case ABSYN_CALLSTM:
			{
				callEntry =
				lookup(globalTable, node->u.stmList.head->u.callStm.name);

				newarea = callEntry->u.procEntry.paramSize;
				break;
			}
		case ABSYN_COMPSTM:
			{
				newarea =
				checkStmOffsets(node->u.stmList.head->u.compStm.stms, globalTable);
				break;
			}
		case ABSYN_IFSTM:
			{
				ifarea =
				checkStmOffsets(node->u.stmList.head->u.ifStm.thenPart, globalTable);

				elsearea =
				checkStmOffsets(node->u.stmList.head->u.ifStm.elsePart, globalTable);

				if (ifarea > elsearea) {
					newarea = ifarea;
				}
				else {
					newarea = elsearea;
				}

				break;
			}
		case ABSYN_WHILESTM:
			{
				newarea =
				checkStmOffsets(node->u.stmList.head->u.whileStm.body, globalTable);
				break;
			}
		}

		if (newarea > area) {
			area = newarea;
		}

		node = node->u.stmList.tail;
	}

	return area;
}

int checkStms(Absyn * node, Table * symTab)
{
	Entry *callEntry;
	int ifArea,
	    elseArea;

	switch (node->type) {
	case ABSYN_CALLSTM:
		{
			callEntry = lookup(symTab, node->u.callStm.name);
			showEntry(callEntry);
			return callEntry->u.procEntry.paramSize;
		}
	case ABSYN_COMPSTM:
		{
			return checkLocalOffsets(node->u.compStm.stms, symTab);
		}
	case ABSYN_IFSTM:
	{
			ifArea = checkStmOffsets(node->u.ifStm.thenPart, symTab);
			elseArea = checkStmOffsets(node->u.ifStm.elsePart, symTab);
			if (ifArea > elseArea){
				return ifArea;
			} else {
				return elseArea;
			}
		}
	case ABSYN_WHILESTM:
		{
			return checkStmOffsets(node->u.whileStm.body, symTab);
		}
	}

	return EXIT_FAILURE;
}

int checkStmOffsets(Absyn * node, Table * symTab)
{
	Absyn *program = node;
	if (program->typeGraph){
	}
		return checkStms(program, symTab);

	return program->type;
}

int checkLocalOffsets(Absyn * node, Table * symTab)
{
	Absyn *program = node;
	return setLocalAreaOffset(program, symTab);

	return EXIT_FAILURE;
}

void showVars(Absyn * program, Table * globalTable)
{
	Absyn *node, *vars;
	Entry *entry, *varEntry;
	ParamTypes *params;
	int arg;

	node = program;
	while (!node->u.decList.isEmpty) {
		if (node->u.decList.head->type == ABSYN_PROCDEC) {
			entry =
			lookup(globalTable, node->u.decList.head->u.procDec.name);

			printf("\nVariable allocation for procedure '%s'\n",
			       symToString(node->u.decList.head->u.procDec.name));

			params = entry->u.procEntry.paramTypes;
			arg = 1;
			while (!params->isEmpty) {
				printf("arg %i: sp + %i\n", arg, params->offset);

				params = params->next;
				arg++;
			}

			printf("size of argument area = %i\n",
			       entry->u.procEntry.paramSize);

			vars = node->u.decList.head->u.procDec.params;
			while (!vars->u.decList.isEmpty) {
				varEntry =
				    lookup(entry->u.procEntry.localTable,
					   vars->u.decList.head->u.parDec.name);

				printf("param '%s': fp + %i\n",
				       symToString(vars->u.decList.head->u. parDec.name),
				       varEntry->u.varEntry.offset);

				vars = vars->u.decList.tail;
			}

			vars = node->u.decList.head->u.procDec.decls;
			while (!vars->u.decList.isEmpty) {
				varEntry =
				    lookup(entry->u.procEntry.localTable,
					   vars->u.decList.head->u.varDec.name);

				printf("var '%s': fp - %i\n",
				       symToString(vars->u.decList.head->u. varDec.name),
				       -(varEntry->u.varEntry.offset));

				vars = vars->u.decList.tail;
			}

			printf("size of localvar area = %i\n",
			       entry->u.procEntry.localVarSize);

			printf("size of outgoing area = %i\n",
			       entry->u.procEntry.argSize);
		}

		node = node->u.decList.tail;
	}
}
