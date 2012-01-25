/*
 * varalloc.h -- variable allocation
 */

#ifndef _VARALLOC_H_
#define _VARALLOC_H_

#define INT_BYTE_SIZE	4	/* size of an int in bytes */
#define BOOL_BYTE_SIZE	4	/* size of a bool in bytes */
#define REF_BYTE_SIZE	4	/* size of an address in bytes */

void allocVars(Absyn * program, Table * globalTable, boolean showVarAlloc);
void checkOffsets(Absyn * node, Table * symTab);
int sendParamOffsets(ParamTypes * params);
int sendVarOffsets(Absyn * node, Table * symTab);
int countCallArgs(Absyn * node, Table * symTab);
void setCallOffsets(Absyn * node, Table * symTab);
void showAllocs(Absyn * node, Table * symTab);

#endif /* _VARALLOC_H_ */
