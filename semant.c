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

	/* generate built-in types */
	intType = newPrimitiveType("int");
	booleanType = newPrimitiveType("boolean");

	/* setup global symbol table */
	globalTable = newTable(NULL);

	/* do semantic checks */
	checkNode(program, globalTable);

	/* check if "main()" is present */
	if (showSymbolTables == TRUE) {

	}

	/* return global symbol table */
	return globalTable;
}

Type *checkNode(Absyn * node, Table * symTab)
{
	switch (node->type) {
	case ABSYN_NAMETY:		return checknode(node, symtab);
	case ABSYN_ARRAYTY:		return checknode(node, symtab);
	case ABSYN_TYPEDEC:		return checknode(node, symtab);
	case ABSYN_PROCDEC:		return checknode(node, symtab);
	case ABSYN_PARDEC:		return checknode(node, symtab);
	case ABSYN_VARDEC:		return checknode(node, symtab);
	case ABSYN_EMPTYSTM:		return checknode(node, symtab);
	case ABSYN_COMPSTM:		return checknode(node, symtab);
	case ABSYN_ASSIGNSTM:		return checknode(node, symtab);
	case ABSYN_IFSTM:		return checknode(node, symtab);
	case ABSYN_WHILESTM:		return checknode(node, symtab);
	case ABSYN_CALLSTM:		return checknode(node, symtab);
	case ABSYN_OPEXP:		return checkOp(node, symTab);
	case ABSYN_VAREXP:		return checknode(node, symtab);
	case ABSYN_INTEXP:		return checknode(node, symtab);
	case ABSYN_SIMPLEVAR:		return checkSimpleVar(node, symTab);
	case ABSYN_ARRAYVAR:		return checknode(node, symtab);
	case ABSYN_DECLIST:		return checknode(node, symtab);
	case ABSYN_STMLIST:		return checknode(node, symtab);
	}
	return NULL;
}

Type *checkOp(Absyn * node, Tabelle * symTab)
{
	Type 	*leftType,
		*rightType,
		*type;

	leftType = checkNode(node->u.opExp.left, symTab);
	rightType = checkNode(node->u.opNode.right, symTab);

	if (leftType != rightType) {
		error("expression combines different types in line %i", node->line);
	}

	switch (node->u.opNode.op) {
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

	node->expType = type;
	return type;
}

Type *checkSimpleVar(Absyn * node, Table * symTab)
{
	Entry *entry;

	entry = lookup(symTab, node->u.simplevarNode.name);

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
