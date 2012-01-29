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
#include "varalloc.h"
#include "codegen.h"

int temp, nextReg, backupReg;
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
 * @brief Mark ABSYN node for debugging. To enable set verbose to true
 *
 * @param outFile assembly
 * @param comment abstract syntax leaf
 * @return void
 **/
void fComment(FILE * outFile, char *comment)
{
	if (verbose) {
		fprintf(outFile, "\t;%s\n", comment);
	}
}


/**
 * @brief Create code so that node displays the value of "target"
 *
 * @param node abstract syntax
 * @param dst target value
 * @return void
 **/
void genCodeOpExp(Absyn * node, Table * symTab, FILE * outFile, int dst, int label, boolean arithmetic)
{
	absynTreeWalker(node->u.opExp.left, symTab, outFile, dst);
	nextReg = dst + 1;

	if (nextReg > MAX_REGISTER) {
		error("expression too complicated, running out of registers.");
	}
	absynTreeWalker(node->u.opExp.right, symTab, outFile, nextReg);

	/* Arithmethic Expressions */
	if (arithmetic) {
		switch (node->u.opExp.op) {
		case ABSYN_OP_ADD:
			{
				fComment(outFile, "add");
				fprintf(outFile, "\tadd\t$%i,$%i,$%i\n",
					dst, dst, nextReg);
				break;
			}
		case ABSYN_OP_SUB:
			{
				fComment(outFile, "sub");
				fprintf(outFile, "\tsub\t$%i,$%i,$%i\n",
					dst, dst, nextReg);
				break;
			}
		case ABSYN_OP_MUL:
			{
				fComment(outFile, "mul");
				fprintf(outFile, "\tmul\t$%i,$%i,$%i\n",
					dst, dst, nextReg);
				break;
			}
		case ABSYN_OP_DIV:
			{
				fComment(outFile, "div");
				fprintf(outFile, "\tdiv\t$%i,$%i,$%i\n",
					dst, dst, nextReg);
				break;
			}
		}
	}
	/* Boolean Comparison Expressions */
	else {
		switch (node->u.opExp.op) {
		case ABSYN_OP_EQU:
			fprintf(outFile, "\tbne\t$%i,$%i,L%i\n", dst, dst + 1, label);
			break;
		case ABSYN_OP_NEQ:
			fprintf(outFile, "\tbeq\t$%i,$%i,L%i\n", dst, dst + 1, label);
			break;
		case ABSYN_OP_LST:
			fprintf(outFile, "\tbge\t$%i,$%i,L%i\n", dst, dst + 1, label);
			break;
		case ABSYN_OP_LSE:
			fprintf(outFile, "\tbgt\t$%i,$%i,L%i\n", dst, dst + 1, label);
			break;
		case ABSYN_OP_GRT:
			fprintf(outFile, "\tble\t$%i,$%i,L%i\n", dst, dst + 1, label);
			break;
		case ABSYN_OP_GRE:
			fprintf(outFile, "\tblt\t$%i,$%i,L%i\n", dst, dst + 1, label);
			break;
		}
	}

}


// global vars for absynTreeWalker
int oldTarget;
ParamTypes *params = NULL;


void absynTreeWalker(Absyn * node, Table * symTab, FILE * outFile, int dst)
{
	Entry *entry = NULL;
	Absyn *simpleVar;
	Type *nodeLeaf;

	// hold variable for assignment
	int var;

	int oldFp;
	int frameSize;
	int setLabelA = getLabelNum();
	int setLabelB = getLabelNum();

	static int args = 0;

	switch (node->type) {
	case ABSYN_PROCDEC:
		{
			/* Framegroesse berechnen */
			entry = lookup(symTab, node->u.procDec.name);
			if (entry->u.procEntry.argSize == -1) {
				frameSize = entry->u.procEntry.localVarSize + INT_BYTE_SIZE;
				oldFp = 0;
			} else {
				frameSize =
				    entry->u.procEntry.localVarSize + 8 +
				    entry->u.procEntry.argSize;
				    oldFp = entry->u.procEntry.argSize + INT_BYTE_SIZE;
			}
			/* Prozedur-Prolog ausgeben */
			fprintf(outFile, "\n\t.export %s\n%s:\n",
				symToString(node->u.procDec.name),
				symToString(node->u.procDec.name));
			/* Code fuer Prozedurkoerper erzeugen */
			fprintf(outFile, "\tsub\t$29,$29,%i\t\t; allocate frame\n"
					 "\tstw\t$25,$29,%i\t\t; save old frame pointer\n"
					 "\tadd\t$25,$29,%i\t\t; setup new frame pointer\n",
					frameSize, oldFp, frameSize);

			if (entry->u.procEntry.argSize != -1) {
				fprintf(outFile,
					"\tstw\t$31,$25,%i\t\t; save return register\n",
					-(entry->u.procEntry.localVarSize + 8) );
			}
			/* Prozedur-Epilog ausgeben */
			absynTreeWalker(node->u.procDec.body,
					entry->u.procEntry.localTable, outFile, dst);

			if (entry->u.procEntry.argSize != -1) {
				fprintf(outFile,
					"\tldw\t$31,$25,%i\t\t; restore return register\n",
					-(entry->u.procEntry.localVarSize + 8) );
			}
			fprintf(outFile, "\tldw\t$25,$29,%i\t\t; restore old frame pointer\n",
				oldFp);
			fprintf(outFile, "\tadd\t$29,$29,%i\t\t; release frame\n", frameSize);
			fprintf(outFile, "\tjr\t$31\t\t\t; return\n");
			break;
		}

	case ABSYN_STMLIST:
		{
			if (!node->u.stmList.isEmpty) {
				fComment(outFile, "stmList");
				absynTreeWalker(node->u.stmList.head,
						symTab, outFile, dst);
				absynTreeWalker(node->u.stmList.tail,
						symTab, outFile, dst);
			}
			break;
		}

	case ABSYN_INTEXP:
		{
			fComment(outFile, "intExp");
			fprintf(outFile, "\tadd\t$%i,$0,%i\n", dst, node->u.intExp.val);
			break;
		}

	case ABSYN_OPEXP:
		{
			fComment(outFile, "opExp");
			genCodeOpExp(node, symTab, outFile, dst, 0, TRUE);
			break;
		}

	case ABSYN_SIMPLEVAR:
		{
			fComment(outFile, "simpleVar");
			entry = lookup(symTab, node->u.simpleVar.name);
			fprintf(outFile, "\tadd\t$%i,$25,%i\n", dst,
				entry->u.varEntry.offset);

			if (entry->u.varEntry.isRef) {
				fprintf(outFile, "\tldw\t$%i,$%i,0\n", dst, dst);
			}
			break;
		}

	case ABSYN_VAREXP:
		{
			fComment(outFile, "varExp");
			absynTreeWalker(node->u.varExp.var, symTab, outFile, dst);
			fprintf(outFile, "\tldw\t$%i,$%i,0\n", dst, dst);
			break;
		}

	case ABSYN_ASSIGNSTM:
		{
			var = dst;
			absynTreeWalker(node->u.assignStm.var, symTab, outFile, dst);
			dst++;
			absynTreeWalker(node->u.assignStm.exp, symTab, outFile, dst);
			fprintf(outFile, "\tstw\t$%i,$%i,0\n", dst, var);
			dst = 8;
			break;
		}

	case ABSYN_ARRAYVAR:
		{
			/*absynTreeWalker(node->u.arrayVar.var, symTab, outFile, dst);

			nextReg = (dst + 1);
			if (nextReg > MAX_REGISTER) {
				error
				    ("expression too complicated, running out of registers.");
			}

			absynTreeWalker(node->u.arrayVar.var, symTab, outFile, nextReg);

			backupReg = (nextReg + 1);
			if (backupReg > MAX_REGISTER) {
				error
				    ("expression too complicated, running out of registers.");
			}
			nodeLeaf = node->typeGraph;
			fprintf(outFile, "\tadd\t$%i,$0,%i\n", backupReg, nodeLeaf->u.arrayType.size);
			fprintf(outFile, "\tbgeu\t$%i,$%i,_indexError\n",
				nextReg, backupReg);
			fprintf(outFile, "\tmul\t$%i,$%i,%i\n",
				nextReg, nextReg, nodeLeaf->u.arrayType.baseType->byte_size);
			fprintf(outFile, "\tadd\t$%i,$%i,$%i\n", dst, dst, nextReg);*/


			backupReg = dst;

			absynTreeWalker(node->u.arrayVar.var, symTab, outFile, dst);
			dst++;
			absynTreeWalker(node->u.arrayVar.var, symTab, outFile, dst);
			dst++;


			nodeLeaf = node->typeGraph;
			fprintf(outFile,"\tadd\t$%d,$0,%d \n",dst,nodeLeaf->u.arrayType.size);
			fprintf(outFile,"\tbgeu\t$%d,$%d,_indexError\n",dst-1,dst);
			fprintf(outFile,"\tmul\t$%d,$%d,%d\n",dst-1,dst-1,nodeLeaf->u.arrayType.baseType->byte_size);
			fprintf(outFile,"\tadd\t$%d,$%d,$%d\n",oldTarget,oldTarget,dst-1);

			break;
		}

	case ABSYN_WHILESTM:
		{
			fComment(outFile, "whileStm");
			setLabelA = getLabelNum();
			setLabelB = getLabelNum();

			fprintf(outFile, "L%i:\n", setLabelA);

			genCodeOpExp(node->u.whileStm.test, symTab, outFile, dst, setLabelB, FALSE);
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
				genCodeOpExp(node->u.ifStm.test, symTab, outFile, dst, setLabelA, FALSE);
				absynTreeWalker(node->u.ifStm.thenPart, symTab, outFile, dst);
				fprintf(outFile, "L%i:\n", setLabelA);

			} else {
				genCodeOpExp(node->u.ifStm.test, symTab, outFile, dst, setLabelA, FALSE);
				absynTreeWalker(node->u.ifStm.thenPart, symTab, outFile, dst);
				fprintf(outFile, "\tj\tL%i\n", setLabelB);

				fprintf(outFile, "L%i:\n", setLabelA);
				absynTreeWalker(node->u.ifStm.elsePart, symTab, outFile, dst);

				fprintf(outFile, "L%i:\n", setLabelB);
			}
			break;
		}

	case ABSYN_CALLSTM:
		{
			entry = lookup(symTab, node->u.callStm.name);

			oldFp = entry->u.procEntry.localVarSize + 8;

			params = entry->u.procEntry.paramTypes;

			absynTreeWalker(node->u.callStm.args, symTab, outFile, dst);

			args = 0;
			fprintf(outFile, "\tjal\t%s\n", symToString(node->u.callStm.name));

			break;
		}

	case ABSYN_DECLIST:
		{
			if (!node->u.decList.isEmpty) {
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

					absynTreeWalker(simpleVar, symTab, outFile, dst);
				} else {
					absynTreeWalker(node->u.expList.head,
							symTab, outFile, dst);

				}

				fprintf(outFile,
					"\tstw\t$%i,$29,%i\t\t; store arg #%i\n",
					dst, params->offset,
					(params->offset != 0)? (params->offset/INT_BYTE_SIZE) : params->offset);

				params = params->next;

				absynTreeWalker(node->u.expList.tail, symTab,
						outFile, dst);

			}
		}

	}

}
