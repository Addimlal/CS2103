/*
 * main.c -- an interpreter for SLPL
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "slpl.h"
#include "table.h"
#include "prog.h"


void zeigeBaum(Knoten *baum) {
  switch( baum->typ )
  {
	  case KNOTEN_FOLGE:
	      zeigeBaum(baum->u.folge.anweisung1);
	      printf(" ; ");
	      zeigeBaum(baum->u.folge.anweisung2);
	      break;
	  case KNOTEN_ZUWEISUNG:
	      printf("%s :=  ", baum->u.zuweisung.name);
	      zeigeBaum(baum->u.zuweisung.ausdruck);
	      break;
	  case KNOTEN_AUSGABE:
	      printf("Zeige(");
	      zeigeBaum(baum->u.ausgabe.ausdruck);
	      printf(")");
	      break;
	  case KNOTEN_LITERAL:
	      printf("%i", baum->u.literal.wert);
	      break;
	  case KNOTEN_VARIABLE:
	      printf("%s", baum->u.variable.name);
	      break;
	  case KNOTEN_SUMME:
	      zeigeBaum(baum->u.summe.ausdruck1);
	      printf(" + ");
	      zeigeBaum(baum->u.summe.ausdruck2);
	      break;
	  case KNOTEN_DIFFERENZ:
	      zeigeBaum(baum->u.differenz.ausdruck1);
	      printf(" - ");
	      zeigeBaum(baum->u.differenz.ausdruck2);
	      break;
	  case KNOTEN_PRODUKT:
	      zeigeBaum(baum->u.produkt.ausdruck1);
	      printf(" * ");
	      zeigeBaum(baum->u.produkt.ausdruck2);
	      break;
	  case KNOTEN_QUOTIENT:
	      zeigeBaum(baum->u.quotient.ausdruck1);
	      printf(" / ");
	      zeigeBaum(baum->u.quotient.ausdruck2);
	      break;
  }
}


void interpretiere(Knoten *baum) {
  /******************************/
  /* Das muessen Sie schreiben! */
  /******************************/
  printf("Hier sollte die Ausgabe des von Ihnen ");
  printf("programmierten Interpreters stehen!");
}


int main(void) {
  Knoten *programm;

  programm = konstruiereProgramm();
  zeigeBaum(programm);
  printf("\n");
  interpretiere(programm);
  printf("\n");
  return 0;
}
