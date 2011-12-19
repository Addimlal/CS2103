/*
 * semant.c -- semantic analysis
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
#include "semant.h"

static Type *intType;
static Type *booleanType;

Table *check(Absyn * program, boolean showSymbolTables)
{
	Table *globalTable;
	Entry *entry;

	/* generate built-in types */
	intType = newPrimitiveType("int");
	booleanType = newPrimitiveType("boolean");

	/* setup global symbol table */
	globalTable = newTable(NULL);

	/* do semantic checks */
	checkNode(program, globalTable);

	/* check if "main()" is present */
	entry = lookup(globalTable, newSym("main"));
	if (entry->kind != ENTRY_KIND_PROC) {
		error("'main' is not a procedure");
	}

	if (entry->u.procEntry.paramTypes->isEmpty == FALSE) {
		error("procedure 'main' must not have any parameters");
	}

	/* return global symbol table */
	if (showSymbolTables == TRUE) {
		showTable(globalTable);
	}

	return globalTable;
}

// TODO: Remove dummy function
Type *dummy(Absyn * node, Table * symTab) {
	error("check not implemented yet.");
	return NULL;
}

Type *checkNode(Absyn * node, Table * symTab)
{
	switch (node->type) {
	case ABSYN_NAMETY:		return checkNameTy(node, symTab);
	case ABSYN_ARRAYTY:		return dummy(node, symTab);
	case ABSYN_TYPEDEC:		return dummy(node, symTab);
	case ABSYN_PROCDEC:		return dummy(node, symTab);
	case ABSYN_PARDEC:		return dummy(node, symTab);
	case ABSYN_VARDEC:		return dummy(node, symTab);
	case ABSYN_EMPTYSTM:		return dummy(node, symTab);
	case ABSYN_COMPSTM:		return dummy(node, symTab);
	case ABSYN_ASSIGNSTM:		return dummy(node, symTab);
	case ABSYN_IFSTM:		return dummy(node, symTab);
	case ABSYN_WHILESTM:		return dummy(node, symTab);
	case ABSYN_CALLSTM:		return dummy(node, symTab);
	case ABSYN_OPEXP:		return checkOp(node, symTab);
	case ABSYN_VAREXP:		return dummy(node, symTab);
	case ABSYN_INTEXP:		return dummy(node, symTab);
	case ABSYN_SIMPLEVAR:		return checkSimpleVar(node, symTab);
	case ABSYN_ARRAYVAR:		return dummy(node, symTab);
	case ABSYN_DECLIST:		return dummy(node, symTab);
	case ABSYN_STMLIST:		return dummy(node, symTab);
	}
	return NULL;
}

Type *checkOp(Absyn * node, Table * symTab)
{
	Type 	*leftType,
		*rightType,
		*type;

	leftType = checkNode(node->u.opExp.left, symTab);
	rightType = checkNode(node->u.opExp.right, symTab);

	if (leftType != rightType) {
		error("expression combines different types in line %i", node->line);
	}

	switch (node->u.opExp.op) {
	case ABSYN_OP_EQU:
		if (leftType != intType) {
			error("comparison requires integer operands in line %i", node->line);
		}
		type = booleanType;
		break;
	case ABSYN_OP_NEQ:
		if (leftType != intType) {
			error("comparison requires integer operands in line %i", node->line);
		}
		type = booleanType;
		break;
	case ABSYN_OP_LST:
		if (leftType != intType) {
			error("comparison requires integer operands in line %i", node->line);
		}
		type = booleanType;
		break;
	case ABSYN_OP_LSE:
		if (leftType != intType) {
			error("comparison requires integer operands in line %i", node->line);
		}
		type = booleanType;
		break;
	case ABSYN_OP_GRT:
		if (leftType != intType) {
			error("comparison requires integer operands in line %i", node->line);
		}
		type = booleanType;
		break;
	case ABSYN_OP_GRE:
		if (leftType != intType) {
			error("comparison requires integer operands in line %i", node->line);
		}
		type = booleanType;
		break;
	case ABSYN_OP_ADD:
		if (leftType != intType) {
			error("arithmetic operation requires integer operands in line %i", node->line);
		}
		type = intType;
		break;
	case ABSYN_OP_SUB:
		if (leftType != intType) {
			error("arithmetic operation requires integer operands in line %i", node->line);
		}
		type = intType;
		break;
	case ABSYN_OP_MUL:
		if (leftType != intType) {
			error("arithmetic operation requires integer operands in line %i", node->line);
		}
		type = intType;
		break;
	case ABSYN_OP_DIV:
		if (leftType != intType) {
			error("arithmetic operation requires integer operands in line %i", node->line);
		}
		type = intType;
		break;
	}

	return type;
}

/** Checking named types for conformity
 * @param Absyn *node
 * @param Table *symTab
 * @return entry->u.typeEntry.type - Typgraph of named type
 * */
Type *checkNameTy(Absyn * node, Table * symTab)
{
	Entry *entry;

	entry = lookup(symTab, node->u.nameTy.name);

	if (entry == NULL) {
		error("undefined type '%s' in line %i",
		      symToString(node->u.nameTy.name), node->line);
	}
	if (entry->kind != ENTRY_KIND_TYPE) {
		error("'%s' is not a type in line %i",
		      symToString(node->u.nameTy.name), node->line);
	}

	return entry->u.typeEntry.type;
}

Type *checkSimpleVar(Absyn * node, Table * symTab)
{
	Entry *entry;

	entry = lookup(symTab, node->u.simpleVar.name);

	if (entry == NULL) {
		error("undefined variable '%s' in line %i",
		      symToString(node->u.simpleVar.name), node->line);
	}
	if (entry->kind != ENTRY_KIND_VAR) {
		error("'%s' is not a variable in line %i",
		      symToString(node->u.simpleVar.name), node->line);
	}

	return entry->u.varEntry.type;
}
