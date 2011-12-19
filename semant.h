/*
 * semant.h -- semantic analysis
 */

#ifndef _SEMANT_H_
#define _SEMANT_H_

Table *check(Absyn * program, boolean showSymbolTables);

Type *dummy(Absyn * node, Table * symTab);

Type *checkNode(Absyn * node, Table * symTab);
Type *checkOp(Absyn * node, Table * symTab);
Type *checkNameTy(Absyn * node, Table * symTab);
Type *checkSimpleVar(Absyn * node, Table * symTab);

#endif				/* _SEMANT_H_ */
