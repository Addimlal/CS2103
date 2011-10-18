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

int interpretAusdruck(Knoten *baum, Tabelle *tabelle){
	switch(baum->typ){
		case KNOTEN_LITERAL:
			return(baum->u.literal.wert);
			break;
		case KNOTEN_VARIABLE:
			return(sucheWert(tabelle, baum->u.variable.name));
			break;
		case KNOTEN_SUMME:
			return
				interpretAusdruck(baum->u.summe.ausdruck1, tabelle)
				+
				interpretAusdruck(baum->u.summe.ausdruck2, tabelle);
			break;
		case KNOTEN_DIFFERENZ:
			return
				interpretAusdruck(baum->u.differenz.ausdruck1, tabelle)
				-
				interpretAusdruck(baum->u.differenz.ausdruck2, tabelle);
			break;
		case KNOTEN_PRODUKT:
			return
				interpretAusdruck(baum->u.produkt.ausdruck1, tabelle)
				*
				interpretAusdruck(baum->u.produkt.ausdruck2, tabelle);
			break;
		case KNOTEN_QUOTIENT:
			if(baum->u.quotient.ausdruck1 == 0) {
				error("Dividend ist %i", baum->u.quotient.ausdruck1);
				break;
			}
			else {
				return
					interpretAusdruck(baum->u.quotient.ausdruck1, tabelle)
					/
					interpretAusdruck(baum->u.quotient.ausdruck2, tabelle);
			}
			break;
	}
	return 0;
}

Tabelle *interpretAnweisung(Knoten *baum, Tabelle *tabelle) {
	switch(baum->typ){
		case KNOTEN_FOLGE:
			tabelle = interpretAnweisung(baum->u.folge.anweisung1, tabelle);
			return interpretAnweisung(baum->u.folge.anweisung2, tabelle);
			break;
		case KNOTEN_ZUWEISUNG:
			return aendereTabelle(tabelle, baum->u.variable.name, interpretAusdruck(baum->u.zuweisung.ausdruck, tabelle));
			break;
		case KNOTEN_AUSGABE:
			printf("%i", interpretAusdruck(baum->u.ausgabe.ausdruck, tabelle));
			break;
	}
	return NULL;
}

void zeigeBaum(Knoten *baum) {
	if(baum == NULL){
		error("NULL-Zeiger in zeigeBaum");
	}

	switch(baum->typ){

	case KNOTEN_FOLGE:
		zeigeBaum(baum->u.folge.anweisung1);
		printf(" ; ");
		zeigeBaum(baum->u.folge.anweisung2);
		break;
	case KNOTEN_ZUWEISUNG:
		printf("%s := ", baum->u.zuweisung.name);
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
	if(baum == NULL){
		error("NULL-Zeiger in interpretiere");
	}

	Tabelle *temp = NULL;
	/*
	 if(baum->type == KNOTEN_FOLGE)
	 */
	switch(baum->typ){

		case KNOTEN_FOLGE:
			interpretAnweisung(baum->u.folge.anweisung1, temp);
			interpretAnweisung(baum->u.folge.anweisung2, temp);
			break;
		case KNOTEN_ZUWEISUNG:
			interpretAnweisung(baum->u.zuweisung.ausdruck, temp);
			break;
		case KNOTEN_AUSGABE:
			interpretAnweisung(baum->u.ausgabe.ausdruck, temp);
			break;
		case KNOTEN_LITERAL:
			interpretAusdruck(baum, temp);
			break;
		case KNOTEN_VARIABLE:
			interpretAusdruck(baum, temp);
			break;
		case KNOTEN_SUMME:
			interpretAusdruck(baum, temp);
			break;
		case KNOTEN_DIFFERENZ:
			interpretAusdruck(baum, temp);
			break;
		case KNOTEN_PRODUKT:
			interpretAusdruck(baum, temp);
			break;
		case KNOTEN_QUOTIENT:
			interpretAusdruck(baum, temp);
			break;
		default:
			error("Eingabe konnte nicht interpretiert werden.");
			break;
	}
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
