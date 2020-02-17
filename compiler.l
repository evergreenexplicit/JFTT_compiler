

%{
#include <iostream>
#include <string>
#include <vector>
# include "compiler.tab.hpp"

%}
%x COMMENT
%option noyywrap
%option yylineno
%%
"["             { BEGIN(COMMENT); }
<COMMENT>"]"       { BEGIN(INITIAL); }
<COMMENT>(\n|.)
<COMMENT><<EOF>>   { printf("Error: infinite comment.\n"); yyterminate(); }


"DECLARE" {return DECLARE;}
"ASSIGN" {return ASSIGN;}
"BEGIN" {return _BEGIN;}
"END"   {return END;}

"IF"    {return IF;}
"THEN"  {return THEN; }
"ELSE"  {return ELSE; }
"ENDIF" {return ENDIF; }

"WHILE"  {return WHILE; }
"DO"     {return DO; }
"ENDWHILE" {return ENDWHILE; }
"ENDDO"  {return ENDDO; }

"FOR"    {return FOR; }
"FROM"   {return FROM; }
"TO"     {return TO; }
"DOWNTO" {return DOWNTO; }
"ENDFOR" {return ENDFOR; }

"READ"   {return READ;}
"WRITE"  {return WRITE;}

"EQ"     { return EQ; }
"NEQ"    { return NEQ; }
"LE"     { return LE; }
"GE"     { return GE; }
"LEQ"    {return LEQ;}
"GEQ"    {return GEQ;}

"PLUS"	 { return PLUS; }
"MINUS"	 { return MINUS; }
"TIMES"	 { return TIMES; }
"DIV"	 { return DIV; }
"MOD"    {return MOD; }
[_a-z]+  {yylval.str = new std::string(yytext); return pidentifier; }
\-?[0-9]+	 {yylval.str = new std::string(yytext); return num; }
[ \n\r\t]   {}
[\(\)\;\:\,] {return yytext[0];}
.	{return ERROR;}
%%