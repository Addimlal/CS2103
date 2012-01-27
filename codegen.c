/*
 * codegen.c -- ECO32 code generator
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
#include "codegen.h"

int	temp, nextReg, backupReg;
boolean verbose = FALSE;

/**
 * @brief Write assembler header impor instructions and default code alignment
 *
 * @param outFile assembly
 * @return void
 **/
void assemblerProlog(FILE * outFile)
{
	fprintf(outFile, "\t.import\tprinti\n");
	fprintf(outFile, "\t.import\tprintc\n");
	fprintf(outFile, "\t.import\treadi\n");
	fprintf(outFile, "\t.import\treadc\n");
	fprintf(outFile, "\t.import\texit\n");
	fprintf(outFile, "\t.import\ttime\n");
	fprintf(outFile, "\t.import\tclearAll\n");
	fprintf(outFile, "\t.import\tsetPixel\n");
	fprintf(outFile, "\t.import\tdrawLine\n");
	fprintf(outFile, "\t.import\tdrawCircle\n");
	fprintf(outFile, "\t.import\t_indexError\n");
	fprintf(outFile, "\n");
	fprintf(outFile, "\t.code\n");
	fprintf(outFile, "\t.align\t4\n");
}

/**
 * @brief Label chunks in assembly code with incrementing labels
 *
 * @param  void
 * @return int
 **/
int getLabelNum(void)
{
	static int labelnum = 0;
	return (labelnum++);
}

/**
 * @brief Create assembly file by walking through abstract syntax
 *
 * @param program abstract syntax
 * @param globalTable symbol table
 * @param outFile assembly
 * @return void
 **/
void genCode(Absyn * program, Table * globalTable, FILE * outFile)
{
	Absyn *node;
	assemblerProlog(outFile);
	node = program;
	absynTreeWalker(node, globalTable, outFile, MIN_REGISTER);

}

/**
 * @brief Create code so that node displays the value of "target"
 *
 * @param node abstract syntax
 * @param dst target value
 * @return void
 **/

int genCodeOp(Absyn * node, Table * symTab, FILE * outFile, int dst)
{
	absynTreeWalker(node->u.opExp.left, symTab, outFile, dst);
	nextReg = dst + 1;

	if (nextReg > MAX_REGISTER) {
		error("expression too complicated, running out of registers.");
	}

	    absynTreeWalker(node->u.opExp.right, symTab, outFile, nextReg);
	    fComment(outFile, "opExp");

	    return nextReg;
}

void fComment(FILE * outFile, char * comment)
{
	if(verbose) {
		fprintf(outFile, "\t;%s\n", comment);
	}
}

void genCodeCompare(Absyn * node, Table * symTab, int label, FILE * outFile, int dst)
{

	absynTreeWalker(node->u.opExp.left, symTab, outFile, dst);

	nextReg = dst + 1;
	if (nextReg > MAX_REGISTER) {
		error("expression too complicated, running out of registers.");
	}
	absynTreeWalker(node->u.opExp.right, symTab, outFile, nextReg);

	switch (node->u.opExp.op) {
	case ABSYN_OP_EQU:
		fprintf(outFile, "\tbne\t$%d,$%d,L%d\n", dst, dst + 1,
			label);
		break;
	case ABSYN_OP_NEQ:
		fprintf(outFile, "\tbeq\t$%d,$%d,L%d\n", dst, dst + 1,
			label);
		break;
	case ABSYN_OP_LST:
		fprintf(outFile, "\tbge\t$%d,$%d,L%d\n", dst, dst + 1,
			label);
		break;
	case ABSYN_OP_LSE:
		fprintf(outFile, "\tbgt\t$%d,$%d,L%d\n", dst, dst + 1,
			label);
		break;
	case ABSYN_OP_GRT:
		fprintf(outFile, "\tble\t$%d,$%d,L%d\n", dst, dst + 1,
			label);
		break;
	case ABSYN_OP_GRE:
		fprintf(outFile, "\tblt\t$%d,$%d,L%d\n", dst, dst + 1,
			label);
		break;
	}

}

// global vars for absynTreeWalker
int oldTarget;

ParamTypes *params = NULL;

Entry *tabEntry = NULL;

void absynTreeWalker(Absyn * node, Table * symTab, FILE * outFile, int dst)
{

	Absyn *simpleVar;

	// hold variable for assignment
	int var;

	int oldFp;
	int frameSize;
	int setLabelA = getLabelNum();
	int setLabelB = getLabelNum();


	static int args = 0;

	switch (node->type) {
		case ABSYN_STMLIST:
		{
			if(!node->u.stmList.isEmpty) {
				fComment(outFile, "stmList");
				absynTreeWalker(node->u.stmList.head, symTab, outFile, dst);
				absynTreeWalker(node->u.stmList.tail, symTab, outFile, dst);
			}
		        break;
		}

		case ABSYN_INTEXP:
		{
			fComment(outFile, "intExp");
			fprintf(outFile, "\tadd\t$%d,$0,%d\n", dst, node->u.intExp.val);
			break;
		}
		case ABSYN_OPEXP:
		{
			fComment(outFile, "opExp");

			/* Arithmethic Operators */
			switch(node->u.opExp.op) {
				case ABSYN_OP_ADD:
				{
					fComment(outFile, "add");
					nextReg = genCodeOp(node, symTab, outFile, dst);
					fprintf(outFile, "\tadd\t$%d,$%d,$%d\n", dst, dst, nextReg);
					break;
				}
				case ABSYN_OP_SUB:
				{
					fComment(outFile, "sub");
					nextReg = genCodeOp(node, symTab, outFile, dst);
					fprintf(outFile, "\tsub\t$%d,$%d,$%d\n", dst, dst, nextReg);
					break;
				}
				case ABSYN_OP_MUL:
				{
					fComment(outFile, "mul");
					nextReg = genCodeOp(node, symTab, outFile, dst);
					fprintf(outFile, "\tmul\t$%d,$%d,$%d\n", dst, dst, nextReg);
					break;
				}
				case ABSYN_OP_DIV:
				{
					fComment(outFile, "div");
					nextReg = genCodeOp(node, symTab, outFile, dst);
					fprintf(outFile, "\tdiv\t$%d,$%d,$%d\n", dst, dst, nextReg);
					break;
				}
			}
			break;
		}

	case ABSYN_PROCDEC:
		tabEntry = lookup(symTab, node->u.procDec.name);

		if (tabEntry->u.procEntry.argSize == -1) {
			frameSize = tabEntry->u.procEntry.localVarSize + 4;
			oldFp = 0;
		} else {
			frameSize =
			    tabEntry->u.procEntry.localVarSize + 8 +
			    tabEntry->u.procEntry.argSize;
			oldFp = tabEntry->u.procEntry.argSize + 4;
		}

		fprintf(outFile, "\n\t.export %s\n",
			symToString(node->u.procDec.name));

		fprintf(outFile, "%s:\n", symToString(node->u.procDec.name));

		fprintf(outFile, "\tsub\t$29,$29,%d\t\t; allocate frame\n", frameSize);

		fprintf(outFile, "\tstw\t$25,$29,%d\t\t; save old frame pointer\n", oldFp);

		fprintf(outFile, "\tadd\t$25,$29,%d\t\t; setup new frame pointer\n", frameSize);

		if (tabEntry->u.procEntry.argSize != -1) {
			fprintf(outFile,
				"\tstw\t$31,$25,%d\t\t; save return register\n",
				(tabEntry->u.procEntry.localVarSize + 8) * -1);
		}

		absynTreeWalker(node->u.procDec.body,
				tabEntry->u.procEntry.localTable, outFile, dst);

		if (tabEntry->u.procEntry.argSize != -1) {
			fprintf(outFile,
				"\tldw\t$31,$25,%d\t\t; restore return register\n",
				(tabEntry->u.procEntry.localVarSize + 8) * -1);
		}
		fprintf(outFile, "\tldw\t$25,$29,%d\t\t; restore old frame pointer\n", oldFp);
		fprintf(outFile, "\tadd\t$29,$29,%d\t\t; release frame\n", frameSize);
		fprintf(outFile, "\tjr\t$31\t\t\t; return\n");
		break;
	case ABSYN_ASSIGNSTM:

		// hold dst
		var = dst;

		absynTreeWalker(node->u.assignStm.var, symTab, outFile, dst);

		dst++;

		absynTreeWalker(node->u.assignStm.exp, symTab, outFile, dst);

		fprintf(outFile, "\tstw\t$%d,$%d,0\n", dst, var);
		dst = 8;

		break;

	case ABSYN_WHILESTM:
	{
		fComment(outFile, "whileStm");
		fprintf(outFile, "L%i:\n", setLabelA);

		genCodeCompare(node->u.whileStm.test, symTab, setLabelB, outFile, dst);
		absynTreeWalker(node->u.whileStm.body, symTab, outFile, dst);

		fprintf(outFile, "\tj\tL%i\n", setLabelA);
		fprintf(outFile, "L%i:\n", setLabelB);
		break;
	}
	case ABSYN_IFSTM:
	{
		fComment(outFile, "ifStm");
		setLabelA = getLabelNum();
		setLabelB = getLabelNum();

		if (node->u.ifStm.elsePart->type == ABSYN_EMPTYSTM) {

			genCodeCompare(node->u.ifStm.test, symTab, setLabelA, outFile, dst);
			absynTreeWalker(node->u.ifStm.thenPart, symTab, outFile, dst);
			fprintf(outFile, "L%d:\n", setLabelA);

		} else {
			genCodeCompare(node->u.ifStm.test, symTab, setLabelA, outFile, dst);
			absynTreeWalker(node->u.ifStm.thenPart, symTab, outFile, dst);
			fprintf(outFile, "\tj\tL%d\n",setLabelB);

			fprintf(outFile,"L%d:\n",setLabelA);
			absynTreeWalker(node->u.ifStm.elsePart, symTab, outFile, dst);
			fprintf(outFile,"L%d:\n",setLabelB);
		}
	        break;
	}

	case ABSYN_CALLSTM:

		tabEntry = lookup(symTab, node->u.callStm.name);

		oldFp = tabEntry->u.procEntry.localVarSize + 8;

		params = tabEntry->u.procEntry.paramTypes;

		absynTreeWalker(node->u.callStm.args, symTab, outFile, dst);

		args = 0;
		fprintf(outFile, "\tjal\t%s\n",
			symToString(node->u.callStm.name));

		break;

	case ABSYN_VAREXP:
	{
		fComment(outFile, "varExp");
		absynTreeWalker(node->u.varExp.var, symTab, outFile, dst);
		fprintf(outFile, "\tldw\t$%d,$%d,0\n", dst, dst);
		break;
	}
	case ABSYN_SIMPLEVAR:
	{
		fComment(outFile, "simpleVar");
		tabEntry = lookup(symTab, node->u.simpleVar.name);
		fprintf(outFile, "\tadd\t$%d,$25,%d\n", dst, tabEntry->u.varEntry.offset);

		if (tabEntry->u.varEntry.isRef) {
			fprintf(outFile, "\tldw\t$%d,$%d,0\n", dst, dst);
		}
	        break;
	}

	case ABSYN_ARRAYVAR:
		absynTreeWalker(node->u.arrayVar.var, symTab, outFile, dst);

		nextReg = (dst + 1);
		if (nextReg > MAX_REGISTER) {
			error("expression too complicated, running out of registers.");
		}

		        absynTreeWalker(node->u.arrayVar.var, symTab, outFile, nextReg);

			backupReg = (nextReg + 1);
			if (backupReg > MAX_REGISTER) {
				error("expression too complicated, running out of registers.");
			}

			        fprintf(outFile, "\tadd\t$%d,$0,%d\n", backupReg, node->typeGraph->u.arrayType.size);
				fprintf(outFile, "\tbgeu\t$%d,$%d,_indexError\n", nextReg, backupReg);
				fprintf(outFile, "\tmul\t$%d,$%d,%d\n", nextReg, nextReg, node->typeGraph->u.arrayType.baseType->byte_size);
				fprintf(outFile, "\tadd\t$%d,$%d,$%d\n", dst, dst, nextReg);


        /**
         *			fprintf(outFile,"\tadd\t$%d,$0,%d\n", backupReg, node->typeGraph->u.arrayType.size);
         *			fprintf(outFile,"\tbgeu\t$%d,$%d,_indexError\n", (backupReg-1), backupReg);
         *			fprintf(outFile,"\tmul\t$%d,$%d,%d\n", (backupReg-1), (backupReg-1), node->typeGraph->u.arrayType.baseType->byte_size);
         *			fprintf(outFile,"\tadd\t$%d,$%d,$%d\n", dst, dst, (backupReg-1));
         **/
		break;
	case ABSYN_DECLIST:
	{
		if(!node->u.decList.isEmpty) {
			fComment(outFile, "decList");

			absynTreeWalker(node->u.decList.head, symTab, outFile, dst);
			absynTreeWalker(node->u.decList.tail, symTab, outFile, dst);
		}
		        break;
	}
	case ABSYN_COMPSTM:
	{
		fComment(outFile, "ABSYN_COMPSTM");
		absynTreeWalker(node->u.compStm.stms, symTab, outFile, dst);
		break;
	}
	case ABSYN_EXPLIST:
	{
		if (!node->u.expList.isEmpty) {

			if (params->isRef) {

				simpleVar = node->u.expList.head->u.varExp.var;

				absynTreeWalker(simpleVar, symTab,
						outFile, dst);
			} else {
				absynTreeWalker(node->u.expList.head,
						symTab, outFile, dst);

			}

			fprintf(outFile,
				"\tstw\t$%d,$29,%d\t\t; store arg #%d\n",
				dst, params->offset, params->offset);

			params = params->next;

			absynTreeWalker(node->u.expList.tail, symTab,
					outFile, dst);

		}
	}

	}

}
