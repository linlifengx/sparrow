%{
#include <string>
#include <iostream>
#include <stdint.h>
#include "common.h"
#include "parser.hpp"

#define STATUS_BACK if(status == STRINGSTATUS){BEGIN STRINGSTATUS;}else{BEGIN CHARSTATUS;}

using namespace std;

extern "C" int yywrap(){}
extern void yyerror(const char *msg);

int charno = 1;
int status;
wstring strBuf;

void saveLocation(){
	yylloc.first_line = yylineno;
	yylloc.first_column = charno;
	yylloc.last_line = yylineno;
	charno += yyleng;
	yylloc.last_column = charno-1;
}

int32_t getUnicode(){
	return strtol(yytext+1,NULL,16);
}

int32_t getWchar(){
	if(strBuf.size() == 0){
		yyerror("empty character constant");
	}else if(strBuf.size() > 1){
		yyerror("multi-character in char syntax");
	}
	return strBuf[0];
}

%}
%option yylineno

%x CMNT ESCAPE CHARSTATUS STRINGSTATUS
%%
[\t ]*	{saveLocation();}
\n	{charno = 1;}
\/\/.*\n	{charno = 1;}
"/*"	{saveLocation(); BEGIN CMNT;}
<CMNT>.	{saveLocation();}
<CMNT>\n	{charno = 1;}
<CMNT>"*/"	{saveLocation(); BEGIN INITIAL;}

'	{saveLocation(); BEGIN CHARSTATUS; strBuf.clear();}
<CHARSTATUS>\\	{saveLocation(); status = CHARSTATUS; BEGIN ESCAPE;}
<CHARSTATUS>\n	{yyerror("missing terminating ' character");}
<CHARSTATUS>'	{saveLocation(); BEGIN INITIAL; yylval.wchar = getWchar(); return CHAR;}
<CHARSTATUS>.	{saveLocation(); strBuf.push_back(yytext[0]);}

\"	{saveLocation(); BEGIN STRINGSTATUS; strBuf.clear();}
<STRINGSTATUS>\\	{saveLocation(); status = STRINGSTATUS; BEGIN ESCAPE;}
<STRINGSTATUS>\n	{yyerror("missing terminating \" character");}
<STRINGSTATUS>\"	{saveLocation(); BEGIN INITIAL; yylval.wstr = new wstring(strBuf); return STRING;}
<STRINGSTATUS>.	{saveLocation(); strBuf.push_back(yytext[0]);}

<ESCAPE>n	{saveLocation(); STATUS_BACK; strBuf.push_back('\n');}
<ESCAPE>t	{saveLocation(); STATUS_BACK; strBuf.push_back('\t');}
<ESCAPE>b	{saveLocation(); STATUS_BACK; strBuf.push_back('\b');}
<ESCAPE>r	{saveLocation(); STATUS_BACK; strBuf.push_back('\r');}
<ESCAPE>f	{saveLocation(); STATUS_BACK; strBuf.push_back('\f');}
<ESCAPE>'	{saveLocation(); STATUS_BACK; strBuf.push_back('\'');}
<ESCAPE>\"	{saveLocation(); STATUS_BACK; strBuf.push_back('"');}
<ESCAPE>\\	{saveLocation(); STATUS_BACK; strBuf.push_back('\\');}
<ESCAPE>u[0-9a-fA-F]{4}	{saveLocation(); STATUS_BACK; strBuf.push_back(getUnicode());}
<ESCAPE>.	{yyerror("unknown escape sequence");}

"void"	{saveLocation(); yylval.token = VOID; return VOID;}
"class"	{saveLocation(); yylval.token = CLASS; return CLASS;}
"new"	{saveLocation(); yylval.token = NEW; return NEW;}
"this"	{saveLocation(); yylval.token = THIS; return THIS;}
"super"	{saveLocation(); yylval.token = SUPER; return SUPER;}
"null"	{saveLocation(); yylval.token = null; return null;}
"for"	{saveLocation(); yylval.token = FOR; return FOR;}
"if"	{saveLocation(); yylval.token = IF; return IF;}
"else"	{saveLocation(); yylval.token = ELSE; return ELSE;}
"break"	{saveLocation(); yylval.token = BREAK; return BREAK;}
"continue"	{saveLocation(); yylval.token = CONTINUE; return CONTINUE;}
"return"	{saveLocation(); yylval.token = RETURN; return RETURN;}
"true"	{saveLocation(); yylval.token = TRUE; return TRUE;}
"false"	{saveLocation(); yylval.token = FALSE; return FALSE;}
"isa"	{saveLocation(); yylval.token = ISA; return ISA;}
"&&"	{saveLocation(); yylval.token = AND; return AND;}
"||"	{saveLocation(); yylval.token = OR; return OR;}
"!="	{saveLocation(); yylval.token = NEQUAL; return NEQUAL;}
"=="	{saveLocation(); yylval.token = EQUAL; return EQUAL;}
"<="	{saveLocation(); yylval.token = LE; return LE;}
">="	{saveLocation(); yylval.token = GE; return GE;}
\[[\t ]*\]	{saveLocation(); yylval.token = BRACKETS; return BRACKETS;}

[a-zA-Z_][a-zA-Z0-9_]*	{
	saveLocation();
	yylval.str = new string(yytext,yyleng);
	return IDENT;
}
([0-9]+\.[0-9]+)|([0-9]+(\.[0-9]+)?[Ee][+-]?[0-9]+)	{
	saveLocation();
	string tempStr(yytext,yyleng);
	yylval.doubleValue = atof(tempStr.c_str());
	return DOUBLE;
}
[0-9]+	{
	saveLocation();
	string tempStr(yytext,yyleng);
	yylval.longValue = atoll(tempStr.c_str());
	return LONG;
}
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
":" {saveLocation(); return ':';}
"@" {saveLocation(); return '@';}
"." {saveLocation(); return '.';}
"#" {saveLocation(); return '#';}
"%" {saveLocation(); return '%';}
"$" {saveLocation(); return '$';}
"?" {saveLocation(); return '?';}
"^" {saveLocation(); return '^';}
"~" {saveLocation(); return '~';}
.	{saveLocation(); return ERROR;}
%%
