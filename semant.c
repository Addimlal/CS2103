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
#include "varalloc.h"

static Type *intType;
static Type *booleanType;
static boolean showSymbolTable;

/** (root) Initiating semantic analysis phase
 * @param Absyn *program
 * @param boolean *showSymbolTables
 * @return globalTable - global table of parsed symbols
 * */
Table *check(Absyn * program, boolean tables)
{
	Table *globalTable;
	Entry *entry;
	Type *arrayType;

	showSymbolTable = tables;

	/* generate built-in types */
	intType = newPrimitiveType("int", INT_BYTE_SIZE);
	booleanType = newPrimitiveType("boolean", BOOL_BYTE_SIZE);

	/* setup global symbol table */
	globalTable = newTable(NULL);

	/* do semantic checks */
	checkNode(program, globalTable);

	/* check if "main()" is present */
	entry = lookup(globalTable, newSym("main"));

	if (entry == NULL) {
		error("procedure 'main' is missing");
	}

	if (entry->kind != ENTRY_KIND_PROC) {
		error("'main' is not a procedure");
	}

	if (!entry->u.procEntry.paramTypes->isEmpty) {
		error("procedure 'main' must not have any parameters");
	}

	/* return global symbol table */
	if (showSymbolTable) {
		showTable(globalTable);
	}

	return globalTable;
}

/** (0) Do recursive node-traversal to check for semantic errors
 * @param Absyn *node
 * @param Table *symTab
 * @return nodeType or NULL
 * */
Type *checkNode(Absyn * node, Table * symTab)
{
	switch (node->type) {
	case ABSYN_NAMETY:		return checkNameTy(node, symTab);
	case ABSYN_ARRAYTY:		return checkArrayTy(node, symTab);
	case ABSYN_TYPEDEC:		return checkTypeDec(node, symTab);
	case ABSYN_PROCDEC:		return checkProcDec(node, symTab);
	case ABSYN_PARDEC:		return checkParDec(node, symTab);
	case ABSYN_VARDEC:		return checkVarDec(node, symTab);
	case ABSYN_EMPTYSTM:		return checkEmptyStm(node, symTab);
	case ABSYN_COMPSTM:		return checkCompStm(node, symTab);
	case ABSYN_ASSIGNSTM:		return checkAssignStm(node, symTab);
	case ABSYN_IFSTM:		return checkIfStm(node, symTab);
	case ABSYN_WHILESTM:		return checkWhileStm(node, symTab);
	case ABSYN_CALLSTM:		return checkCallStm(node, symTab);
	case ABSYN_OPEXP:		return checkOpExp(node, symTab);
	case ABSYN_VAREXP:		return checkVarExp(node, symTab);
	case ABSYN_INTEXP:		return checkIntExp(node, symTab);
	case ABSYN_SIMPLEVAR:		return checkSimpleVar(node, symTab);
	case ABSYN_ARRAYVAR:		return checkArrayVar(node, symTab);
	case ABSYN_DECLIST:		return checkDecList(node, symTab);
	case ABSYN_STMLIST:		return checkStmList(node, symTab);
	case ABSYN_EXPLIST:		return checkExpList(node, symTab);
	}
	return NULL;
}

/** (1) Checking named types for conformity
 * @param Absyn *node
 * @param Table *symTab
 * @return entry->u.typeEntry.type - Typegraph of named type
 * */
Type *checkNameTy(Absyn * node, Table * symTab)
{
	Entry *nameEntry;

	nameEntry = lookup(symTab, node->u.nameTy.name);

	if (nameEntry == NULL) {
		error("undefined type '%s' in line %i",
		      symToString(node->u.nameTy.name), node->line);
	}
	if (nameEntry->kind != ENTRY_KIND_TYPE) {
		error("'%s' is not a type in line %i",
		      symToString(node->u.nameTy.name), node->line);
	}

	return nameEntry->u.typeEntry.type;
}

/** (2) Create typegraph for arrayType
 * @param Absyn *node
 * @param Table *symTab
 * @return type - Typegraph of array type
 * */
Type *checkArrayTy(Absyn * node, Table * symTab)
{
	Type *arrayType;

	arrayType = checkNode(node->u.arrayTy.ty, symTab);

	return newArrayType(node->u.arrayTy.size, arrayType);
}

/** (3) Checking type declarations for conformity
 * @param Absyn *node
 * @param Table *symTab
 * @return NULL - Typechecks of statements always return NULL
 * */
Type *checkTypeDec(Absyn * node, Table * symTab) {
	Type *type;
	Entry *typeEntry;

	type = checkNode(node->u.typeDec.ty, symTab);
	typeEntry = newTypeEntry(type);

	if (enter(symTab, node->u.typeDec.name, typeEntry)  == NULL) {
		error("redeclaration of %s as type in line %i",
		      symToString(node->u.typeDec.name), node->line);
	}

	return NULL;
}


/** (4) Checking type declarations for conformity
 * @param Absyn *node
 * @param Table *symTab
 * @return NULL - Typechecks of statements always return NULL
 * */
Type *checkProcDec(Absyn * node, Table * symTab) {
	ParamTypes *parTypes;
	Entry *procEntry;
	Table *localTable;

	localTable = newTable(NULL);

	parTypes = checkParamTypes(node->u.procDec.params, localTable);
	procEntry = newProcEntry(parTypes, localTable);

	if (enter(symTab, node->u.procDec.name, procEntry)  == NULL) {
		error("redeclaration of %s as procedure in line %i",
		      symToString(node->u.procDec.name), node->line);
	}

	checkNode(node->u.procDec.decls, localTable);
	checkNode(node->u.procDec.body, localTable);

	if (showSymbolTable) {
		showTable(localTable);
	}

	return NULL;
}

/** (5) Checking type declarations for conformity
 * @param Absyn *node
 * @param Table *symTab
 * @return NULL - Typechecks of statements always return NULL
 * */
Type *checkParDec(Absyn * node, Table * symTab) {
	Type *paramType;
	Entry *paramEntry;

	paramType = checkNode(node->u.parDec.ty, symTab);
	paramEntry = newVarEntry(paramType, node->u.parDec.isRef);

	if (enter(symTab, node->u.parDec.name, paramEntry)  == NULL) {
		error("redeclaration of %s as parameter in line %i",
		      symToString(node->u.parDec.name), node->line);
	}

	return NULL;
}

/** (6) Checking variable declarations for conformity
 * @param Absyn *node
 * @param Table *symTab
 * @return NULL - Typechecks of variable return NULL
 * */
Type *checkVarDec(Absyn * node, Table * symTab) {
	Type *varType;
	Entry *varEntry;

	varType = checkNode(node->u.varDec.ty, symTab);
	varEntry = newVarEntry(varType, FALSE);

	if (enter(symTab, node->u.varDec.name, varEntry)  == NULL) {
		error("redeclaration of %s as variable in line %i",
		      symToString(node->u.varDec.name), node->line);
	}

	return NULL;
}

/** (7) Checking empty statemens for conformity
 * @param Absyn *node
 * @param Table *symTab
 * @return NULL - Typechecks of empty-statements always return NULL
 * */
Type *checkEmptyStm(Absyn * node, Table * symTab) {
	return NULL;
}

/** (8) Checking compound-statemens for conformity
 * @param Absyn *node
 * @param Table *symTab
 * @return NULL - Typechecks of compound-statements always return NULL
 * */
Type *checkCompStm(Absyn * node, Table * symTab) {

	checkNode(node->u.compStm.stms, symTab);

	return NULL;
}

/** (9) Checking assign statemens for conformity
 * @param Absyn *node
 * @param Table *symTab
 * @return NULL - Typechecks of assign-statements always return NULL
 * */
Type *checkAssignStm(Absyn * node, Table * symTab) {
	Type 	*leftType,
		*rightType;

	leftType = checkNode(node->u.assignStm.var, symTab);
	rightType = checkNode(node->u.assignStm.exp, symTab);

	if (leftType != rightType) {
		error("assignment has different types in line %i", node->line);
	}

	if (leftType->kind == TYPE_KIND_ARRAY) {
		error("assignment requires integer variable in line %i", node->line);
	}

	return NULL;
}

/** (10) Checking if statemens for conformity
 * @param Absyn *node
 * @param Table *symTab
 * @return NULL - Typechecks of if-statements always return NULL
 * */
Type *checkIfStm(Absyn * node, Table * symTab) {
	Type *ifType;

	ifType = checkNode(node->u.ifStm.test, symTab);

	if (ifType != booleanType) {
		error("'if' test expression must be of type boolean in line %i", node->line);
	}

	checkNode(node->u.ifStm.thenPart, symTab);
	checkNode(node->u.ifStm.elsePart, symTab);

	return NULL;
}

/** (11) Checking while statemens for conformity
 * @param Absyn *node
 * @param Table *symTab
 * @return NULL - Typechecks of while-statements always return NULL
 * */
Type *checkWhileStm(Absyn * node, Table * symTab) {
	Type *whileType;

	whileType = checkNode(node->u.whileStm.test, symTab);

	if (whileType != booleanType) {
		error("'while' test expression must be of type boolean in line %i", node->line);
	}

	checkNode(node->u.whileStm.body, symTab);

	return NULL;
}

/** (12) Checking procedure declarations for conformity
 * @param Absyn *node
 * @param Table *symTab
 * @return NULL - Typechecks of statements always return NULL
 * */
Type *checkCallStm(Absyn * node, Table * symTab) {
	int argNr = 1;
	Absyn *callArgs;
	Type *callType;
	ParamTypes *paramTypes;
	Entry *entryParam;

	entryParam = lookup(symTab, node->u.callStm.name);

	if (entryParam == NULL) {
		error("undefined procedure '%s' in line %i",
		      symToString(node->u.callStm.name), node->line);
	}

	if (entryParam->kind != ENTRY_KIND_PROC ) {
		error("call of non-procedure %s in line %i",
		      symToString(node->u.callStm.name), node->line);
	}

	paramTypes = entryParam->u.procEntry.paramTypes;
	callArgs = node->u.callStm.args;

	while(!(paramTypes->isEmpty) && !(callArgs->u.expList.isEmpty)) {
		callType = checkNode(callArgs->u.expList.head, symTab);
		if(paramTypes->type != callType) {
		      error("procedure %s argument %i type mismatch in line %i",
			    symToString(node->u.callStm.name), argNr, node->line);
		}

		if(paramTypes->isRef && callArgs->u.expList.head->type != ABSYN_VAREXP) {
		      error("procedure %s argument %i must be a variable in line %i",
			    symToString(node->u.callStm.name), argNr, node->line);
		}

		paramTypes = paramTypes->next;
		callArgs = callArgs->u.expList.tail;
		argNr++;
	}

	if(!paramTypes->isEmpty) {
		error("procedure %s called with too few arguments in line %i",
		      symToString(node->u.callStm.name), node->line);
	}

	if(!callArgs->u.expList.isEmpty) {
		error("procedure %s called with too many arguments in line %i",
		      symToString(node->u.callStm.name), node->line);
	}

	return NULL;
}

/** (13) Checking assignments and operator expressions for conformity
 * @param Absyn *node
 * @param Table *symTab
 * @return type - Typegraph of operator assigment
 * */
Type *checkOpExp(Absyn * node, Table * symTab)
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

/** (14) Do recursive node-traversal of variable expressions
 * @param Absyn *node
 * @param Table *symTab
 * @return type - Returns true type by recursing with checkNode
 * */
Type *checkVarExp(Absyn * node, Table * symTab)
{
	Type *varType;
	varType = checkNode(node->u.varExp.var, symTab);

	return varType;
}

/** (15) Set primitive integer type
 * @param Absyn *node
 * @param Table *symTab
 * @return intType
 * */

Type *checkIntExp(Absyn * node, Table * symTab)
{
	Type *type;
	type = intType;

	return type;
}

/** (16) Checking simple variable types for conformity
 * @param Absyn *node
 * @param Table *symTab
 * @return entry->u.varEntry.type - Typegraph of simple variable type
 * */
Type *checkSimpleVar(Absyn * node, Table * symTab)
{
	Entry *simpleEntry;

	simpleEntry = lookup(symTab, node->u.simpleVar.name);

	if (simpleEntry == NULL) {
		error("undefined variable '%s' in line %i",
		      symToString(node->u.simpleVar.name), node->line);
	}

	if (simpleEntry->kind != ENTRY_KIND_VAR) {
		error("'%s' is not a variable in line %i",
		      symToString(node->u.simpleVar.name), node->line);
	}

	return simpleEntry->u.varEntry.type;
}

/** (17) Checking array variable types for conformity
 * @param Absyn *node
 * @param Table *symTab
 * @return arrEntry->u.varEntry.type - Typegraph of array variable type
 * */
Type *checkArrayVar(Absyn * node, Table * symTab)
{
	Type *indexType,
	     *arrayType;

	indexType = checkNode(node->u.arrayVar.index, symTab);

	if (indexType != intType) {
		error("illegal indexing a non-array in line %i", node->line);
	}

	arrayType = checkNode(node->u.arrayVar.var, symTab);

	if (arrayType->kind != TYPE_KIND_ARRAY) {
		error("illegal indexing with a non-integer in line %i", node->line);
	}

	return arrayType->u.arrayType.baseType;
}

/** (18) Do recursive node-traversal of declaration lists
 * @param Absyn *node
 * @param Table *symTab
 * @return NULL - Resolve to typegraph of declaration lists
 * */
Type *checkDecList(Absyn * node, Table * symTab)
{
	if (node->u.decList.isEmpty == FALSE) {
		checkNode(node->u.decList.head, symTab);
		checkNode(node->u.decList.tail, symTab);
	}

	return NULL;
}

/** (19) Do recursive node-traversal of statement lists
 * @param Absyn *node
 * @param Table *symTab
 * @return NULL - Resolve to typegraph of statement lists
 * */
Type *checkStmList(Absyn * node, Table * symTab)
{
	if (node->u.stmList.isEmpty == FALSE) {
		checkNode(node->u.stmList.head, symTab);
		checkNode(node->u.stmList.tail, symTab);
	}
	return NULL;
}

/** (20) Do recursive node-traversal of expression lists
 * @param Absyn *node
 * @param Table *symTab
 * @return NULL - Resolve to typegraph of expression lists
 * */
Type *checkExpList(Absyn * node, Table * symTab)
{
	if (node->u.expList.isEmpty == FALSE) {
		checkNode(node->u.expList.head, symTab);
		checkNode(node->u.expList.tail, symTab);
	}

	return NULL;
}


ParamTypes *checkParamTypes(Absyn * params, Table * symTab)
{
  Type *parType;
  Entry *parEntry;

  if(params->u.decList.isEmpty){
	  return emptyParamTypes();
  }

  parType = checkNode(params->u.decList.head->u.parDec.ty, symTab);

  if (parType->kind == TYPE_KIND_ARRAY && !params->u.decList.head->u.parDec.isRef) {
	  error("parameter %s must be a reference parameter in line %i",
		symToString(params->u.decList.head->u.parDec.name), params->line);
  }

  parEntry = newVarEntry(parType, params->u.decList.head->u.parDec.isRef);

  enter(symTab, params->u.decList.head->u.parDec.name, parEntry);

  return newParamTypes(parType, params->u.decList.head->u.parDec.isRef,
		       checkParamTypes(params->u.decList.tail, symTab));
}
