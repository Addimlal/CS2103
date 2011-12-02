#
# Makefile for SPL compiler
#

NO_COLOR="\033[0m"
OK_COLOR="\033[30;1m"
FILE_COLOR="\033[32;1m"

CC = gcc
CFLAGS = -Wall -Wno-unused -g
LDLIBS = -lm

LDFLAGS = -g
SRCS = main.c utils.c parser.tab.c lex.yy.c absyn.c sym.c
OBJS = $(patsubst %.c,%.o,$(SRCS))
BIN = spl

.PHONY:		all ast run fast verify tests depend clean dist-clean

all:		$(BIN)

$(BIN):		$(OBJS)
		$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o:		%.c
		$(CC) $(CFLAGS) -o $@ -c $<

parser.tab.c:	parser.y
		bison -v -d -t --warnings=all parser.y

lex.yy.c:	scanner.l
		flex scanner.l

tests:		all
		@for i in Tests/test??.spl ; do \
		  echo ; \
		  ./$(BIN) $$i ; \
		done
		@echo

-include depend.mak

fast:
		@make CC="tcc" CFLAGS="-Wall -Wimplicit-function-declaration -c -g" LDLIBS="-L/usr/lib/x86_64-linux-gnu/"| sed -e '/Entering\|Leaving/d' -e '/Betrete\|Verlasse/d'

-include depend.mak


run:		all
		@for i in Tests/*.spl ; \
		do echo $(OK_COLOR)File: $(FILE_COLOR)$$i $(NO_COLOR); ./$(BIN) $$i; \
		done | column -c 80
		@echo


ast:		all
		@for i in Tests/??_test_*.spl ; \
		do echo $(OK_COLOR)File: $(FILE_COLOR)$$i $(NO_COLOR); ./$(BIN) --absyn $$i; \
		done | column -c 80
		@echo


verify:		all
		@./verify

-include depend.mak


depend:		parser.tab.c lex.yy.c
		$(CC) $(CFLAGS) -MM $(SRCS) > depend.mak

clean:
		rm -f *~ *.o *.swp
		rm -f Tests/*~
		rm -f Tests/*.absyn
		rm -f parser_referenz.txt parser_test.txt parser_unterschiede.txt

dist-clean:	clean
		rm -f $(BIN) parser.tab.c parser.tab.h parser.output lex.yy.c depend.mak
