%{
#include <iostream>
#include <string>
#include <vector>
#include "ast.hpp"
#include "parser.hpp"

using namespace std;
extern int yylex();
extern int yylineno;
extern int charno;
extern int yyleng;
extern FILE *yyin;

void yyerror(const char *msg){
	cout<<yylineno<<":"<<(charno-yyleng)<<": error: "<<msg<<endl;
	if(yyin != NULL){
	fclose(yyin);
	}
	exit(1);
}

void setLocation(Node *node,YYLTYPE *loc,YYLTYPE *firstLoc,YYLTYPE *lastLoc){
	loc->first_line = firstLoc->first_line;
	loc->first_column = firstLoc->first_column;
	loc->last_line = lastLoc->last_line;
	loc->last_column = lastLoc->last_column;
	if(node != NULL){
	node->firstLine = loc->first_line;
	node->firstColumn = loc->first_column;
	node->lastLine = loc->last_line;
	node->lastColumn = loc->last_column;
	}
}

void setLocation(Node *node,YYLTYPE *loc){
	loc->first_line = yylineno;
	loc->first_column = charno;
	loc->last_line = yylineno;
	loc->last_column = charno-1;
	if(node != NULL){
	node->firstLine = loc->first_line;
	node->firstColumn = loc->first_line;
	node->lastLine = loc->last_line;
	node->lastColumn = loc->last_column;
	}
}

Program *program;

%}
%error-verbose
%debug

%union{
	int token;
	string *str;
	Program *program;
	Statement *stmt;
	Expression *expr;
	VarInit *varInit;
	SimpleVarDecl *spvarDecl;
	SimpleStmtList *spstmtList;
	CallExpr *callExpr;
	VarDecl *varDecl;
	GlobalStatement *globalStmt;
	
	vector<string*> *identList;
	vector<Statement*> *stmtList;
	vector<Expression*> *exprList;
	vector<VarInit*> *varInitList;
	vector<SimpleVarDecl*> *spvarDeclList;
	vector<GlobalStatement*> *globalStmtList;
}

%token <str> IDENT DOUBLE LONG ERROR
%token <token> RETURN FOR IF ELSE BREAK AND OR EQUAL NEQUAL TRUE FALSE LE GE CONTINUE

%type <program> program
%type <stmt> stmt simple_stmt var_assi return_stmt if_stmt for_stmt for_init for_loop
%type <expr> expr numeric bool for_cond
%type <varInit> var_init
%type <spvarDecl> simple_var_decl
%type <spstmtList> simple_stmt_list
%type <callExpr> call_expr
%type <varDecl> var_decl
%type <globalStmt> global_stmt func_decl

%type <stmtList> stmt_list stmt_block
%type <identList> ident_list ident_list_allow_null
%type <exprList> expr_list
%type <varInitList> var_init_list
%type <spvarDeclList> spvar_decl_list
%type <globalStmtList> global_stmt_list

%left OR
%left AND
%left EQUAL NEQUAL
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS
%nonassoc LOGICNOT

%start program

%%
program:
	global_stmt_list {program=new Program(*$1);$$=program;setLocation($$,&@$,&@1,&@1);}
	;
global_stmt_list:
	global_stmt {$$=new vector<GlobalStatement*>();$$->push_back($1);setLocation(NULL,&@$,&@1,&@1);}
	|global_stmt_list global_stmt {$1->push_back($2);$$=$1;setLocation(NULL,&@$,&@1,&@2);}
	;
global_stmt:
	var_decl ';' {$$=$1;setLocation($$,&@$,&@1,&@1);}
	|func_decl {$$=$1;setLocation($$,&@$,&@1,&@1);}
	;
var_decl:
	IDENT var_init_list {$$=new VarDecl(*$1,*$2);setLocation($$,&@$,&@1,&@2);}
	;
var_init:
	IDENT {$$=new VarInit(*$1,NULL);setLocation($$,&@$,&@1,&@1);}
	|IDENT '=' expr {$$=new VarInit(*$1,$3);setLocation($$,&@$,&@1,&@3);}
	;
var_init_list:
	var_init {$$=new vector<VarInit*>();$$->push_back($1);setLocation(NULL,&@$,&@1,&@1);}
	|var_init_list ',' var_init {$1->push_back($3);$$=$1;setLocation(NULL,&@$,&@1,&@3);}
	;
func_decl:
	IDENT IDENT '(' spvar_decl_list ')' '{' stmt_list '}'
	{vector<string*> *types = new vector<string*>();types->push_back($1);
	$$=new FuncDecl(*types,*$2,*$4,*$7);setLocation($$,&@$,&@1,&@8);}
	|ident_list IDENT '(' spvar_decl_list ')' '{' stmt_list '}'
	{$$=new FuncDecl(*$1,*$2,*$4,*$7);setLocation($$,&@$,&@1,&@8);}
	|'[' spvar_decl_list ']' IDENT '(' spvar_decl_list ')' '{' stmt_list '}'
	{$$=new FuncDecl2(*$2,*$4,*$6,*$9);setLocation($$,&@$,&@1,&@10);}
	;
simple_var_decl:
	IDENT IDENT {$$=new SimpleVarDecl(*$1,*$2);setLocation($$,&@$,&@1,&@2);}
	;
spvar_decl_list:
	/*blank*/ {$$=new vector<SimpleVarDecl*>();setLocation(NULL,&@$);}
	|simple_var_decl {$$=new vector<SimpleVarDecl*>();$$->push_back($1);setLocation(NULL,&@$,&@1,&@1);}
	|spvar_decl_list ',' simple_var_decl {$1->push_back($3);$$=$1;setLocation(NULL,&@$,&@1,&@3);}
	;
ident_list:
	IDENT {$$=new vector<string*>();$$->push_back($1);setLocation(NULL,&@$,&@1,&@1);}
	|ident_list ',' IDENT {$1->push_back($3);$$=$1;setLocation(NULL,&@$,&@1,&@3);}
	;
stmt_list:
	/*blank*/ {$$=new vector<Statement*>();setLocation(NULL,&@$);}
	|stmt {$$=new vector<Statement*>();$$->push_back($1);setLocation(NULL,&@$,&@1,&@1);}
	|stmt_list stmt {$1->push_back($2);$$=$1;setLocation(NULL,&@$,&@1,&@2);}
	;
stmt:
	';' {$$=new NullStmt();setLocation($$,&@$,&@1,&@1);}
	|var_decl ';' {$$=$1;setLocation($$,&@$,&@1,&@2);}
	|simple_stmt_list ';' {$$=$1;setLocation($$,&@$,&@1,&@2);}
	|for_stmt {$$=$1;setLocation($$,&@$,&@1,&@1);}
	|if_stmt {$$=$1;setLocation($$,&@$,&@1,&@1);}
	|return_stmt ';' {$$=$1;setLocation($$,&@$,&@1,&@2);}
	|BREAK ';' {$$=new BreakStmt();setLocation($$,&@$,&@1,&@2);}
	|CONTINUE ';' {$$=new ContinueStmt();setLocation($$,&@$,&@1,&@2);}
	;
for_stmt:
	FOR '(' for_init ';' for_cond ';' for_loop ')' stmt_block
	{$$=new ForStmt(*$3,*$5,*$7,*$9);setLocation($$,&@$,&@1,&@9);}
	;
for_init:
	/*blank*/ {$$=new NullStmt();setLocation($$,&@$);}
	|var_decl {$$=$1;setLocation($$,&@$,&@1,&@1);}
	|simple_stmt_list {$$=$1;setLocation($$,&@$,&@1,&@1);}
	;
for_cond:
	/*blank*/ {$$=new Bool(true);setLocation($$,&@$);}
	|expr {$$=$1;setLocation($$,&@$,&@1,&@1);}
	;
for_loop:
	/*blank*/ {$$=new NullStmt();setLocation($$,&@$);}
	|simple_stmt_list {$$=$1;setLocation($$,&@$,&@1,&@1);}
	;
var_assi:
	IDENT '=' expr {$$=new VarAssi(*$1,*$3);setLocation($$,&@$,&@1,&@3);}
	|'[' ident_list_allow_null ']' '=' call_expr {$$=new MultiVarAssi(*$2,*$5);setLocation($$,&@$,&@1,&@5);}
	;
ident_list_allow_null:
	/*blank*/ {$$=new vector<string*>();$$->push_back(new string(""));setLocation(NULL,&@$);}
	|IDENT {$$=new vector<string*>();$$->push_back($1);setLocation(NULL,&@$,&@1,&@1);}
	|ident_list_allow_null ',' IDENT {$1->push_back($3);$$=$1;setLocation(NULL,&@$,&@1,&@3);}
	|ident_list_allow_null ',' {$1->push_back(new string(""));$$=$1;setLocation(NULL,&@$,&@1,&@2);}
	;
simple_stmt:
	var_assi {$$=$1;setLocation($$,&@$,&@1,&@1);}
	|expr {$$=new ExprStmt(*$1);setLocation($$,&@$,&@1,&@1);}
	;
simple_stmt_list:
	simple_stmt {$$=new SimpleStmtList();$$->add($1);setLocation($$,&@$,&@1,&@1);}
	|simple_stmt_list ',' simple_stmt {$1->add($3);$$=$1;setLocation($$,&@$,&@1,&@3);}
	;
if_stmt:
	IF '(' expr ')' stmt_block ELSE stmt_block {$$=new IfElseStmt(*$3,*$5,*$7);setLocation($$,&@$,&@1,&@7);}
	|IF '(' expr ')' stmt_block {$$=new IfElseStmt(*$3,*$5,*(new vector<Statement*>()));setLocation($$,&@$,&@1,&@5);}
	;
stmt_block:
	stmt {$$=new vector<Statement*>();$$->push_back($1);setLocation(NULL,&@$,&@1,&@1);}
	| '{' stmt_list '}' {$$=$2;setLocation(NULL,&@$,&@1,&@3);}
	;
return_stmt:
	RETURN expr_list {$$=new ReturnStmt(*$2);setLocation($$,&@$,&@1,&@2);}
	;
expr:
	expr '+' expr {$$=new BinaryExpr(*$1,'+',*$3);setLocation($$,&@$,&@1,&@3);}
	|expr '-' expr {$$=new BinaryExpr(*$1,'-',*$3);setLocation($$,&@$,&@1,&@3);}
	|expr '*' expr {$$=new BinaryExpr(*$1,'*',*$3);setLocation($$,&@$,&@1,&@3);}
	|expr '/' expr {$$=new BinaryExpr(*$1,'/',*$3);setLocation($$,&@$,&@1,&@3);}
	|expr AND expr {$$=new LogicExpr(*$1,AND,*$3);setLocation($$,&@$,&@1,&@3);}
	|expr OR expr {$$=new LogicExpr(*$1,OR,*$3);setLocation($$,&@$,&@1,&@3);}
	|expr EQUAL expr {$$=new BinaryExpr(*$1,EQUAL,*$3);setLocation($$,&@$,&@1,&@3);}
	|expr NEQUAL expr {$$=new BinaryExpr(*$1,NEQUAL,*$3);setLocation($$,&@$,&@1,&@3);}
	|expr LE expr {$$=new BinaryExpr(*$1,LE,*$3);setLocation($$,&@$,&@1,&@3);}
	|expr GE expr {$$=new BinaryExpr(*$1,GE,*$3);setLocation($$,&@$,&@1,&@3);}
	|expr '<' expr {$$=new BinaryExpr(*$1,'<',*$3);setLocation($$,&@$,&@1,&@3);}
	|expr '>' expr {$$=new BinaryExpr(*$1,'>',*$3);setLocation($$,&@$,&@1,&@3);}
	|'(' expr ')' {$$=$2;setLocation($$,&@$,&@1,&@3);}
	|'-' expr %prec UMINUS {$$=new PrefixExpr('-',*$2);setLocation($$,&@$,&@1,&@2);}
	|'!' expr %prec LOGICNOT {$$=new PrefixExpr('!',*$2);setLocation($$,&@$,&@1,&@2);}
	|IDENT {$$=new IdentExpr(*$1);setLocation($$,&@$,&@1,&@1);}
	|numeric {$$=$1;setLocation($$,&@$,&@1,&@1);}
	|bool {$$=$1;setLocation($$,&@$,&@1,&@1);}
	|call_expr {$$=$1;setLocation($$,&@$,&@1,&@1);}
	;
numeric:
	LONG {$$=new Long(*$1);setLocation($$,&@$,&@1,&@1);}
	|DOUBLE {$$=new Double(*$1);setLocation($$,&@$,&@1,&@1);}
	;
bool:
	TRUE {$$=new Bool(true);setLocation($$,&@$,&@1,&@1);}
	|FALSE {$$=new Bool(false);setLocation($$,&@$,&@1,&@1);}
	;
expr_list:
	/*blank*/ {$$=new vector<Expression*>();setLocation(NULL,&@$);}
	|expr {$$=new vector<Expression*>();$$->push_back($1);setLocation(NULL,&@$,&@1,&@1);}
	|expr_list ',' expr {$1->push_back($3);$$=$1;setLocation(NULL,&@$,&@1,&@3);}
	;
call_expr:
	IDENT '(' expr_list ')' {$$=new CallExpr(*$1,*$3);setLocation($$,&@$,&@1,&@4);}
	;
%%
