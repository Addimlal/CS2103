#
# Makefile for SPL compiler
#
# See: https://github.com/X4/CS2103-Tools
# Author: Fernandos

NO_COLOR="\033[0m"
OK_COLOR="\033[30;1m"
FILE_COLOR="\033[32;1m"

CC = /usr/bin/gcc
CFLAGS = -Wall -Wno-unused -g
LDLIBS = -lm

LDFLAGS = -g
SRCS = main.c utils.c parser.tab.c lex.yy.c absyn.c sym.c semant.c table.c types.c varalloc.c codegen.c
OBJS = $(patsubst %.c,%.o,$(SRCS))
BIN = spl

.PHONY:		all codegen ast run fast verify scannerTest scannerTest2 scannerRef parserTest parserTest2 parserRef astTest astTest2 astRef tests depend clean dist-clean

all:		$(BIN)

$(BIN):		$(OBJS)
		$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o:		%.c
		$(CC) $(CFLAGS) -o $@ -c $<

parser.tab.c:	parser.y
		bison -d -v -t -g parser.y

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
		@$(MAKE) CC="tcc" CFLAGS="-Wall -Werror -Wunusupported -c -g" LDLIBS="-L/usr/lib/x86_64-linux-gnu/"| sed -e '/Entering\|Leaving/d' -e '/Betrete\|Verlasse/d'

-include depend.mak

fast64:
		@$(MAKE) CC="tcc" CFLAGS="-Wall -Werror -Wunusupported -c -g" LDLIBS="-L/usr/lib64/gcc/x86_64-slackware-linux/"| sed -e '/Entering\|Leaving/d' -e '/Betrete\|Verlasse/d'

-include depend.mak


codegen:	all
		@./spl Tests/sort.spl a.out && ./splRef Tests/sort.spl ref.out && diff -s --suppress-common-lines -w a.out ref.out | less

graph:
		@$(MAKE) all | sed '/make/d'
		@echo -e $(FILE_COLOR)Erstelle vektorbasierten Parser-Graph ... $(NO_COLOR)
		@echo -e $(OK_COLOR)Ausgabe: parser.svg $(NO_COLOR)
		@dot -Tsvg  parser.dot -o parser.svg
		@echo

run:		all
		@for i in Tests/*.spl ; \
		do echo -e $(OK_COLOR)File: $(FILE_COLOR)$$i $(NO_COLOR); ./$(BIN) $$i; \
		done
		@echo

runRef:
		@for i in Tests/*.spl ; \
		do echo -e $(OK_COLOR)File: $(FILE_COLOR)$$i $(NO_COLOR); ./splRef $$i /dev/null; \
		done
		@echo


ast:		all
		@for i in Tests/??_test_*.spl ; \
		do echo -e$(OK_COLOR)File: $(FILE_COLOR)$$i $(NO_COLOR); ./$(BIN) --absyn $$i; \
		done
		@echo


verify:		all
		@./verify
		@echo

saturn:		all
		@./verifyRemote
		@echo

-include depend.mak


depend:		parser.tab.c lex.yy.c
		$(CC) $(CFLAGS) -MM $(SRCS) > depend.mak

clean:
		rm -f *~ *.o *.swp
		rm -f Tests/*~
		rm -f Tests/*.absyn
		rm -f parser_*.txt
		rm -f parser.dot

dist-clean:	clean
		rm -f $(BIN) parser.tab.c parser.tab.h parser.output parser.svg lex.yy.c depend.mak


%splRef :

scannerTest:	all
		@echo -e $(FILE_COLOR)TESTE SCANNER'\n'$(OK_COLOR)CTRL + D signalisiert -- EOF --'\n'$(NO_COLOR)
		@./spl --tokens /dev/stdin
scannerTest2:	all
		@echo -e $(FILE_COLOR)TESTE SCANNER'\n'$(OK_COLOR)CTRL + D signalisiert -- EOF --'\n'$(NO_COLOR)
		@./spl --tokens /dev/stdin /dev/null
scannerRef:	splRef
		@echo -e $(FILE_COLOR)TESTE REFERENZ SCANNER'\n'$(OK_COLOR)CTRL + D signalisiert -- EOF --'\n'$(NO_COLOR)
		@./splRef --tokens /dev/stdin /dev/null


parserTest:	all
		@echo -e $(FILE_COLOR)INTERAKTIVER PARSER'\n'$(OK_COLOR)CTRL + D signalisiert -- EOF --'\n'$(NO_COLOR)
		@./spl /dev/stdin
parserTest2:	all
		@echo -e $(FILE_COLOR)INTERAKTIVER PARSER'\n'$(OK_COLOR)CTRL + D signalisiert -- EOF --'\n'$(NO_COLOR)
		@./spl /dev/stdin /dev/null
parserRef:	splRef
		@echo -e $(FILE_COLOR)INTERAKTIVER REFERENZ PARSER'\n'$(OK_COLOR)CTRL + D signalisiert -- EOF --'\n'$(NO_COLOR)
		@./splRef /dev/stdin /dev/null


astTest:	all
		@echo -e $(FILE_COLOR)ABSTRAKTER SYNTAXBAUM'\n'$(OK_COLOR)CTRL + D signalisiert -- EOF --'\n'$(NO_COLOR)
		@./spl --absyn /dev/stdin
astTest2:	all
		@echo -e $(FILE_COLOR)ABSTRAKTER SYNTAXBAUM'\n'$(OK_COLOR)CTRL + D signalisiert -- EOF --'\n'$(NO_COLOR)
		@./spl --absyn /dev/stdin /dev/null
astRef:		splRef
		@echo -e $(FILE_COLOR)ABSTRAKTER SYNTAXBAUM DER REFERENZ'\n'$(OK_COLOR)CTRL + D signalisiert -- EOF --'\n'$(NO_COLOR)
		@./splRef --absyn  /dev/stdin /dev/null


tableTest:	all
		@echo -e $(FILE_COLOR)SYMBOLTABELLEN'\n'$(OK_COLOR)CTRL + D signalisiert -- EOF --'\n'$(NO_COLOR)
		@./spl --tables /dev/stdin
tableTest2:	all
		@echo -e $(FILE_COLOR)SYMBOLTABELLEN'\n'$(OK_COLOR)CTRL + D signalisiert -- EOF --'\n'$(NO_COLOR)
		@./spl --tables /dev/stdin /dev/null
tableRef:	splRef
		@echo -e $(FILE_COLOR)SYMBOLTABELLEN DER REFERENZ'\n'$(OK_COLOR)CTRL + D signalisiert -- EOF --'\n'$(NO_COLOR)
		@./splRef --tables  /dev/stdin /dev/null

