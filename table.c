/*
 * table.c -- the symbol table
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "table.h"


Tabelle *aendereTabelle(Tabelle *tabelle, char *name, int wert) {
	Tabelle *temp = allocate(sizeof(Tabelle));
	temp->name = name;
	temp->wert = wert;
	temp->next = tabelle;
	return temp;
}

int sucheWert(Tabelle *tabelle, char *name) {
	if(tabelle == NULL){
		error("Eintrag %s nicht vorhanden", name);
	}
	if(strcmp(tabelle->name, name) == 0){
		return tabelle->wert;
	}
	else{
		return sucheWert(tabelle->next, name);
	}
}
