Flex_src = lexer.l
Bison_src = parser.y
OS_NAME := $(shell uname -s | tr A-Z a-z)

os:
	@echo $(OS_NAME)

parser.tab.c : $(Bison_src)
	bison -d $^

compiler-lex.c : $(Flex_src)
	flex $^;
	mv lex.yy.c $@

pastel : parser.tab.c compiler-lex.c *.c
	if [ ! -d ../bin ]; then mkdir ../bin; fi
	if [ "$(OS_NAME)" = "linux" ]; then gcc -g -lfl $^ -o pastel -lm; else gcc -g -ll $^ -o pastel ; fi
	mv parser.tab.c ../bin/;
	mv compiler-lex.c ../bin/;
	mv parser.tab.h ../bin/;
	mv pastel ../bin/;

clean:
	rm ../bin/*.c;
	rm ../bin/*.h;
	rm ../bin/pastel;
