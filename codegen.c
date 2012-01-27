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

int maxreg = 16;
int temp;

void assemblerProlog(FILE *outFile) {
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

int newLabel() {
    static int numlabels = 0;
    return numlabels++;
}

void genCode(Absyn *program, Table *globalTable, FILE *outFile) {
    assemblerProlog(outFile);

    int target = 8;
    TraverseAbsyn(program,target,globalTable,outFile);

}

void genCodeOp(Absyn *node, int target,Table *symTab,FILE *outFile) {

    TraverseAbsyn(node->u.opExp.left,target,symTab,outFile);

    temp = target + 1;
    if(temp > maxreg) error("Ausdruck (Arithmetik) zu kompliziert: line : %d\n",node->line);

    TraverseAbsyn(node->u.opExp.right,temp,symTab,outFile);

    switch(node->u.opExp.op) {
    case ABSYN_OP_ADD:
        fprintf(outFile,"\tadd\t$%d,$%d,$%d\n",target,target,temp);
        break;

    case ABSYN_OP_SUB:
        fprintf(outFile,"\tsub\t$%d,$%d,$%d\n",target,target,temp);
        break;

    case ABSYN_OP_MUL:
        fprintf(outFile,"\tmul\t$%d,$%d,$%d\n",target,target,temp);
        break;

    case ABSYN_OP_DIV:
        fprintf(outFile,"\tdiv\t$%d,$%d,$%d\n",target,target,temp);
        break;

    }
}

void genCodeComp(Absyn *node, int target,Table *symTab,int label,FILE *outFile) {

    TraverseAbsyn(node->u.opExp.left,target,symTab,outFile);

    temp = target + 1;
    if(temp > maxreg) error("Ausdruck (Vergleich) zu kompliziert! line: %d \n",node->line);

    TraverseAbsyn(node->u.opExp.right,temp,symTab,outFile);

    switch(node->u.opExp.op) {
    case ABSYN_OP_EQU:
        fprintf(outFile,"\tbne\t$%d,$%d,L%d\n",target,target+1,label);
        break;
    case ABSYN_OP_NEQ:
        fprintf(outFile,"\tbeq\t$%d,$%d,L%d\n",target,target+1,label);
        break;
    case ABSYN_OP_LST:
        fprintf(outFile,"\tbge\t$%d,$%d,L%d\n",target,target+1,label);
        break;
    case ABSYN_OP_LSE:
        fprintf(outFile,"\tbgt\t$%d,$%d,L%d\n",target,target+1,label);
        break;
    case ABSYN_OP_GRT:
        fprintf(outFile,"\tble\t$%d,$%d,L%d\n",target,target+1,label);
        break;
    case ABSYN_OP_GRE:
        fprintf(outFile,"\tblt\t$%d,$%d,L%d\n",target,target+1,label);
        break;
    }



}

// global vars for TraverseAbsyn
int oldTarget;

ParamTypes* params = NULL;

Entry *tabEntry = NULL;

void TraverseAbsyn(Absyn* node,int target,Table *symTab,FILE *outFile) {

    Absyn *simpleVar;

    // hold variable for assignment
    int var;

    int oldFp;
    int frameSize;

    int label1;
    int label2;

    static int args = 0;

    switch(node->type) {
    case ABSYN_NAMETY :

        break;
    case ABSYN_ARRAYTY		:

        break;
    case ABSYN_TYPEDEC		:

        break;
    case ABSYN_PROCDEC		:
        tabEntry = lookup(symTab,node->u.procDec.name);

        if(tabEntry->u.procEntry.argSize == -1) {
		frameSize = tabEntry->u.procEntry.localVarSize + 4;
            oldFp = 0;
        } else {
		frameSize = tabEntry->u.procEntry.localVarSize + 8 + tabEntry->u.procEntry.argSize;
            oldFp = tabEntry->u.procEntry.argSize + 4;
        }

        //export
        fprintf(outFile,"\n\t.export %s\n",symToString(node->u.procDec.name));

        fprintf(outFile,"%s:\n",symToString(node->u.procDec.name));

        fprintf(outFile,"\tsub\t$29,$29,%d\t;allocate frame\n",frameSize);

        fprintf(outFile,"\tstw\t$25,$29,%d\t;save old frame pointer\n",oldFp);

        fprintf(outFile,"\tadd\t$25,$29,%d\t;setup new frame pointer\n",frameSize);

	if(tabEntry->u.procEntry.argSize != -1) {
		fprintf(outFile,"\tstw\t$31,$25,%d\t;save return register\n",(tabEntry->u.procEntry.localVarSize+8)*-1);
        }

        TraverseAbsyn(node->u.procDec.body,target,tabEntry->u.procEntry.localTable,outFile);

        if(tabEntry->u.procEntry.argSize != -1) {
		fprintf(outFile,"\tldw\t$31,$25,%d\t;restore return register\n",(tabEntry->u.procEntry.localVarSize+8)*-1);
        }
        fprintf(outFile,"\tldw\t$25,$29,%d\t;restore old frame pointer\n",oldFp);
        fprintf(outFile,"\tadd\t$29,$29,%d\t;release frame\n",frameSize);
        fprintf(outFile,"\tjr\t$31\t;return\n");
        break;
    case ABSYN_PARDEC		:

        break;
    case ABSYN_VARDEC		:

        break;
    case ABSYN_EMPTYSTM		:

        break;
    case ABSYN_COMPSTM		:

        TraverseAbsyn(node->u.compStm.stms,target,symTab,outFile);

        break;
    case ABSYN_ASSIGNSTM	:

        // hold target
        var=target;

        TraverseAbsyn(node->u.assignStm.var,target,symTab,outFile);

        target++;

        TraverseAbsyn(node->u.assignStm.exp,target,symTab,outFile);

        fprintf(outFile,"\tstw\t$%d,$%d,0\n",target,var);
        target = 8;

        break;

    case ABSYN_IFSTM		:
        label1 = newLabel();
        label2 = newLabel();

        if(node->u.ifStm.elsePart->type == ABSYN_EMPTYSTM) {

            genCodeComp(node->u.ifStm.test,target,symTab,label1,outFile);

            TraverseAbsyn(node->u.ifStm.thenPart,target,symTab,outFile);

            fprintf(outFile,"L%d:\n",label1);

        } else {

            genCodeComp(node->u.ifStm.test,target,symTab,label1,outFile);

            TraverseAbsyn(node->u.ifStm.thenPart,target,symTab,outFile);


            fprintf(outFile,"\tj\tL%d\n",label2);

            fprintf(outFile,"L%d:\n",label1);

            TraverseAbsyn(node->u.ifStm.elsePart,target,symTab,outFile);

            fprintf(outFile,"L%d:\n",label2);
        }

        break;

    case ABSYN_WHILESTM		:
        label1 = newLabel();
        label2 = newLabel();

        fprintf(outFile,"L%d:\n",label1);

        genCodeComp(node->u.whileStm.test,target,symTab,label2,outFile);

        TraverseAbsyn(node->u.whileStm.body,target,symTab,outFile);

        fprintf(outFile,"\tj\tL%d\n",label1);

        fprintf(outFile,"L%d:\n",label2);

        break;

    case ABSYN_CALLSTM		:

        tabEntry = lookup(symTab,node->u.callStm.name);

        oldFp = tabEntry->u.procEntry.localVarSize + 8;

        params = tabEntry->u.procEntry.paramTypes;

        TraverseAbsyn(node->u.callStm.args,target,symTab,outFile);

        args = 0;
        fprintf(outFile,"\tjal\t%s\n",symToString(node->u.callStm.name));

        break;

    case ABSYN_OPEXP		:

        genCodeOp(node,target,symTab,outFile);
        break;

    case ABSYN_VAREXP		:

        TraverseAbsyn(node->u.varExp.var,target,symTab,outFile);


        fprintf(outFile,"\tldw\t$%d,$%d,0\n",target,target);

        break;

    case ABSYN_INTEXP		:
        fprintf(outFile,"\tadd\t$%d,$0,%d\n",target,node->u.intExp.val);


        break;
    case ABSYN_SIMPLEVAR	:


        tabEntry = lookup(symTab,node->u.simpleVar.name);

        fprintf(outFile,"\tadd\t$%d,$25,%d\n",target,tabEntry->u.varEntry.offset);

        if(tabEntry->u.varEntry.isRef) {
            fprintf(outFile,"\tldw\t$%d,$%d,0\n",target,target);
        }


        break;
    case ABSYN_ARRAYVAR		:
        oldTarget = target;

        TraverseAbsyn(node->u.arrayVar.var,target,symTab,outFile);

        target++;

        TraverseAbsyn(node->u.arrayVar.index,target,symTab,outFile);

        target++;


        fprintf(outFile,"\tadd\t$%d,$0,%d \n",target,node->typeGraph->u.arrayType.size);

        fprintf(outFile,"\tbgeu\t$%d,$%d,_indexError\n",target-1,target);

        fprintf(outFile,"\tmul\t$%d,$%d,%d\n",target-1,target-1, node->typeGraph->u.arrayType.baseType->byte_size);

        fprintf(outFile,"\tadd\t$%d,$%d,$%d\n",oldTarget,oldTarget,target-1);

        break;
    case ABSYN_DECLIST		:

        if(!node->u.decList.isEmpty) {

            TraverseAbsyn(node->u.decList.head,target,symTab,outFile);
            TraverseAbsyn(node->u.decList.tail,target,symTab,outFile);

        }
        break;
    case ABSYN_STMLIST		:

        if(!node->u.stmList.isEmpty) {

            TraverseAbsyn(node->u.stmList.head,target,symTab,outFile);
            TraverseAbsyn(node->u.stmList.tail,target,symTab,outFile);

        }
        break;
    case ABSYN_EXPLIST		:

        if(!node->u.expList.isEmpty) {

            if(params->isRef) {

                simpleVar = node->u.expList.head->u.varExp.var;

                TraverseAbsyn(simpleVar,target,symTab,outFile);
            } else {
                TraverseAbsyn(node->u.expList.head,target,symTab,outFile);

            }


            fprintf(outFile,"\tstw\t$%d,$29,%d\n",target,params->offset);

            params = params->next;

            TraverseAbsyn(node->u.expList.tail,target,symTab,outFile);

        }
    default:
        break;
    }

}






