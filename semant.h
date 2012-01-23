/*
 * semant.h -- semantic analysis
 */

#ifndef _SEMANT_H_
#define _SEMANT_H_

Table *check(Absyn * program, boolean tables);

Type *checkNode(Absyn * node, Table * symTab);
Type *checkNameTy(Absyn * node, Table * symTab);
Type *checkArrayTy(Absyn * node, Table * symTab);
Type *checkTypeDec(Absyn * node, Table * symTab);
Type *checkProcDec(Absyn * node, Table * symTab);
Type *checkParDec(Absyn * node, Table * symTab);
Type *checkVarDec(Absyn * node, Table * symTab);
Type *checkEmptyStm(Absyn * node, Table * symTab);
Type *checkCompStm(Absyn * node, Table * symTab);
Type *checkAssignStm(Absyn * node, Table * symTab);
Type *checkIfStm(Absyn * node, Table * symTab);
Type *checkWhileStm(Absyn * node, Table * symTab);
Type *checkCallStm(Absyn * node, Table * symTab);
Type *checkOpExp(Absyn * node, Table * symTab);
Type *checkVarExp(Absyn * node, Table * symTab);
Type *checkIntExp(Absyn * node, Table * symTab);
Type *checkSimpleVar(Absyn * node, Table * symTab);
Type *checkArrayVar(Absyn * node, Table * symTab);
Type *checkDecList(Absyn * node, Table * symTab);
Type *checkStmList(Absyn * node, Table * symTab);
Type *checkExpList(Absyn * node, Table * symTab);

ParamTypes *checkParamTypes(Absyn * params, Table * symTab);

#endif				/* _SEMANT_H_ */
