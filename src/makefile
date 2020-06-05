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

pcc : parser.tab.c compiler-lex.c utils.c
	if [ "$(OS_NAME)" = "linux" ]; then gcc -lfl $^ -o pcc -lm; else gcc -g -ll $^ -o pcc ; fi
	mv parser.tab.c ../bin/;
	mv compiler-lex.c ../bin/;
	mv parser.tab.h ../bin/;
	mv pcc ../bin/;

clean:
	rm ../bin/*.c;
	rm ../bin/*.h;
	rm ../bin/pcc;
