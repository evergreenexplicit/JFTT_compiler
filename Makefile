default:
	bison -d compiler.ypp 
	flex compiler.l
	g++ -std=c++11 -o kompilator compiler.tab.cpp lex.yy.c
