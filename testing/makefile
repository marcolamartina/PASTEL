Flex_src = lexer.l
Bison_src = parser.y

parser.tab.c : $(Bison_src)
	bison -d $^

compiler-lex.c : $(Flex_src)
	flex $^;
	mv lex.yy.c $@

COMPILER : parser.tab.c compiler-lex.c
	gcc -ll $^ -o COMPILER;

clean:
	rm *.c;
	rm *.h;
	rm COMPILER;
