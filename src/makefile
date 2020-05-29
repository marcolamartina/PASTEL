Flex_src = lexer.l
Bison_src = parser.y

parser.tab.c : $(Bison_src)
	bison -d $^

compiler-lex.c : $(Flex_src)
	flex $^;
	mv lex.yy.c $@

pcc : parser.tab.c compiler-lex.c utils.c
	gcc -ll $^ -o pcc;
	mv *.c ../bin/;
	mv *.h ../bin/;
	mv pcc ../bin/;

clean:
	rm ../bin/*.c;
	rm ../bin/*.h;
	rm ../bin/pcc;
