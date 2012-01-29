/*
 * codegen.h -- ECO32 code generator
 */

#ifndef _CODEGEN_H_
#define _CODEGEN_H_

typedef enum REGISTER {		/* Defines temporary variable integer registers */
	MIN_REGISTER = 8,	/* Minimum temporary variable register */
	MAX_REGISTER = 16	/* Maximum temporary variable register */
} reg_t;

void genCode(Absyn * program, Table * globalTable, FILE * outFile);
void fComment(FILE * outFile, char *comment);
int getLabelNum(void);
void genCodeOpExp(Absyn * node, Table * symTab, FILE * outFile, int dst, int label, boolean arithmetic);
void absynTreeWalker(Absyn * node, Table * symTab, FILE * outFile, int dst);
#endif				/* _CODEGEN_H_ */
