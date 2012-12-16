%{
#include <string>
#include <iostream>
#include "ast.hpp"
#include "parser.hpp"

int charno = 1;
extern "C" int yywrap(){}
using namespace std;

void saveLocation(){
	yylloc.first_line = yylineno;
	yylloc.first_column = charno;
	yylloc.last_line = yylineno;
	charno += yyleng;
	yylloc.last_column = charno-1;
}

%}
%option yylineno

%x CMNT
%%
[\t ]*	{saveLocation();}
\n	{charno = 1;}
\/\/.*\n	{charno = 1;}
"/*"	{saveLocation(); BEGIN CMNT;}
<CMNT>.	{saveLocation();}
<CMNT>\n	{charno = 1;}
<CMNT>"*/"	{saveLocation(); BEGIN INITIAL;}
"for"	{saveLocation(); yylval.token = FOR; return FOR;}
"if"	{saveLocation(); yylval.token = IF; return IF;}
"else"	{saveLocation(); yylval.token = ELSE; return ELSE;}
"break"	{saveLocation(); yylval.token = BREAK; return BREAK;}
"continue"	{saveLocation(); yylval.token = CONTINUE; return CONTINUE;}
"return"	{saveLocation(); yylval.token = RETURN; return RETURN;}
"true"	{saveLocation(); yylval.token = TRUE; return TRUE;}
"false"	{saveLocation(); yylval.token = FALSE; return FALSE;}
"&&"	{saveLocation(); yylval.token = AND; return AND;}
"||"	{saveLocation(); yylval.token = OR; return OR;}
"!="	{saveLocation(); yylval.token = NEQUAL; return NEQUAL;}
"=="	{saveLocation(); yylval.token = EQUAL; return EQUAL;}
"<="	{saveLocation(); yylval.token = LE; return LE;}
">="	{saveLocation(); yylval.token = GE; return GE;}
[a-zA-Z_][a-zA-Z0-9_]*	{saveLocation(); yylval.str = new string(yytext,yyleng);return IDENT;}
[0-9]+\.[0-9]+	{saveLocation(); yylval.str = new string(yytext,yyleng);return DOUBLE;}
[0-9]+	{saveLocation(); yylval.str = new string(yytext,yyleng);return LONG;}
"="	{saveLocation(); return '=';}
"("	{saveLocation(); return '(';}
")"	{saveLocation(); return ')';}
"+"	{saveLocation(); return '+';}
"-"	{saveLocation(); return '-';}
"*"	{saveLocation(); return '*';}
"/"	{saveLocation(); return '/';}
";"	{saveLocation(); return ';';}
"}"	{saveLocation(); return '}';}
"{"	{saveLocation(); return '{';}
","	{saveLocation(); return ',';}
"<"	{saveLocation(); return '<';}
">"	{saveLocation(); return '>';}
"!"	{saveLocation(); return '!';}
"["	{saveLocation(); return '[';}
"]"	{saveLocation(); return ']';}
.	{saveLocation(); return ERROR;}
%%
