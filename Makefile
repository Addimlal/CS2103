#
# Makefile for SPL compiler
#

CC = gcc
CFLAGS = -Wall -Wno-unused -g
LDFLAGS = -g
LDLIBS = -lm

SRCS = main.c utils.c parser.tab.c lex.yy.c absyn.c sym.c
OBJS = $(patsubst %.c,%.o,$(SRCS))
BIN = spl

.PHONY:		all tests verify depend clean dist-clean

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
		  ./$(BIN) $$i; \
		done
		@echo

-include depend.mak

verify:	all
		@for i in Tests/test??.spl ; do \
		  echo ; \
		  ./$(BIN) $$i; \
		done
		./testIt
		@echo

-include depend.mak

depend:		parser.tab.c lex.yy.c
		$(CC) $(CFLAGS) -MM $(SRCS) > depend.mak

clean:
		rm -f *~ *.o
		rm -f Tests/*~
		rm -f SomethingIsWrong.txt referenz.txt test.txt
		rm -f *.swp

dist-clean:	clean
		rm -f $(BIN) parser.tab.c parser.tab.h parser.output lex.yy.c depend.mak
