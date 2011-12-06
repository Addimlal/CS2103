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


void genCode(Absyn *program, Table *globalTable, FILE *outFile) {
  assemblerProlog(outFile);
}
