%{
#include <stdio.h>
#include "statement.h"
#include "expression.h"
#include "parser.hpp"

using namespace std;
extern int yylex();
extern int yylineno;
extern int charno;
extern int yyleng;
extern FILE *yyin;
extern Program *program;

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

void setNodeLocation(Node *node,YYLTYPE *loc){
	node->firstLine = loc->first_line;
	node->firstColumn = loc->first_line;
	node->lastLine = loc->last_line;
	node->lastColumn = loc->last_column;
}

%}
%error-verbose
%debug

%union{
	int token;
	string *str;

	int64_t longValue;
	double doubleValue;
	int32_t wchar;
	wstring *wstr;

	Program *program;
	TypeDecl *typeDecl;
	SimpleVarDecl *simpleVarDecl;
	StmtBlock *stmtBlock;	

	Statement* stmt;
	VarDef *varDef;
	VarInit *varInit;
	FuncDef *funcDef;
	FuncDecl *funcDecl;
	SimpleStmtList *spstmtList;
	ClassDef *classDef;
	ClassBody *classBody;
	Constructor *constructor;

	Expression *expr;
	LeftValueExpr *leftValueExpr;
	FuncInvoke *funcInvoke;
	
	vector<TypeDecl*> *typeDeclList;
	vector<SimpleVarDecl*> *spvarDeclList;
	vector<Statement*> *stmtList;
	vector<VarInit*> *varInitList;
	vector<LeftValueExpr*> *leftExprList;
	vector<Expression*> *exprList;
}

%token <str> IDENT ERROR
%token <longValue> LONG
%token <doubleValue> DOUBLE
%token <wchar> CHAR
%token <wstr> STRING
%token <token> RETURN FOR IF ELSE BREAK AND OR EQUAL null VOID BRACKETS
%token <token> NEQUAL TRUE FALSE LE GE CONTINUE CLASS NEW SUPER THIS ISA

%type <program> program def_stmt_list
%type <typeDecl> type_decl
%type <typeDeclList> type_decl_list
%type <simpleVarDecl> simple_var_decl
%type <spvarDeclList> spvar_decl_list
%type <stmtList> stmt_list
%type <stmtBlock> stmt_block

%type <stmt> stmt simple_stmt var_assi return_stmt super_init array_assi
%type <stmt> if_stmt for_stmt for_init for_loop
%type <varDef> var_def
%type <varInit> var_init
%type <varInitList> var_init_list
%type <funcDef> func_def
%type <funcDecl> func_decl
%type <spstmtList> simple_stmt_list
%type <classDef> class_def
%type <classBody> class_body class_stmt_list
%type <constructor> constructor

%type <expr> for_cond expr numeric bool new_object
%type <expr> new_array array_init dynamic_cast instance_of
%type <exprList> expr_list expr_list_allow_null
%type <leftValueExpr> ident_expr array_element left_expr
%type <leftExprList> left_expr_list_allow_null
%type <funcInvoke> func_invoke

%left OR
%left AND
%left EQUAL NEQUAL
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/' '%'
%nonassoc UMINUS
%nonassoc LOGICNOT
%left ISA
%nonassoc DYNAMICCAST
%left '.'

%start program

%%

/************************** node ***************************/
program:
	def_stmt_list {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
		program = $1;
	};

def_stmt_list:
	/*blank*/ {
		$$ = new Program();
		setLocation(NULL,&@$);
	}
	|class_def {
		$$ = new Program();
		$$->addClassDef($1);
		setLocation(NULL,&@$,&@1,&@1);
	}
	|var_def ';'{
		$$ = new Program();
		$$->addVarDef($1);
		setLocation(NULL,&@$,&@1,&@2);
	}
	|func_def {
		$$ = new Program();
		$$->addFuncDef($1);
		setLocation(NULL,&@$,&@1,&@1);
	}
	|def_stmt_list class_def {
		$1->addClassDef($2);
		$$ = $1;
		setLocation(NULL,&@$,&@1,&@2);
	}
	|def_stmt_list var_def ';'{
		$1->addVarDef($2);
		$$ = $1;
		setLocation(NULL,&@$,&@1,&@3);
	}
	|def_stmt_list func_def {
		$1->addFuncDef($2);
		$$ = $1;
		setLocation(NULL,&@$,&@1,&@2);
	};
stmt_list:
	/*blank*/ {
		$$ = new vector<Statement*>();
		setLocation(NULL,&@$);
	}
	|stmt {
		$$ = new vector<Statement*>();
		$$->push_back($1);
		setLocation(NULL,&@$,&@1,&@1);
	}
	|stmt_list stmt {
		$1->push_back($2);
		$$ = $1;
		setLocation(NULL,&@$,&@1,&@2);
	};

simple_var_decl:
	type_decl IDENT {
		$$ = new SimpleVarDecl($1,*$2);
		setLocation($$,&@$,&@1,&@2);
		delete $2;
	};
spvar_decl_list:
	/*blank*/ {
		$$ = new vector<SimpleVarDecl*>();
		setLocation(NULL,&@$);
	}
	|simple_var_decl {
		$$ = new vector<SimpleVarDecl*>();
		$$->push_back($1);
		setLocation(NULL,&@$,&@1,&@1);}
	|spvar_decl_list ',' simple_var_decl {
		$1->push_back($3);
		$$ = $1; 
		setLocation(NULL,&@$,&@1,&@3);
	};

type_decl:
	IDENT {
		$$ = new TypeDecl(*$1);
		setLocation($$,&@$,&@1,&@1);
		delete $1;
	}
	|type_decl BRACKETS {
		$$ = $1;
		$$->dimension++;
		setLocation($$,&@$,&@1,&@2);
	};
type_decl_list:
	type_decl {
		$$ = new vector<TypeDecl*>();
		$$->push_back($1);
		setLocation(NULL,&@$,&@1,&@1);
	}
	|type_decl_list ',' type_decl {
		$$ = $1;
		$$->push_back($3);
		setLocation(NULL,&@$,&@1,&@3);
	};

func_def:
	func_decl stmt_block {
		$$ = new FuncDef($1,$2);
		setLocation(NULL,&@$,&@1,&@2);
	};
func_decl:
	type_decl IDENT '(' spvar_decl_list ')' {
		vector<TypeDecl*> retTypes;
		retTypes.push_back($1);
		$$ = new FuncDecl(retTypes,*$2,*$4);
		setLocation($$,&@$,&@1,&@5);
		delete $2;
		delete $4;
	}
	|VOID IDENT '(' spvar_decl_list ')' {
		vector<TypeDecl*> retTypes;
		$$ = new FuncDecl(retTypes,*$2,*$4);
		setLocation($$,&@$,&@1,&@5);
		delete $2;
		delete $4;
	}
	|type_decl_list IDENT '(' spvar_decl_list ')' {
		$$ = new FuncDecl(*$1,*$2,*$4);
		setLocation($$,&@$,&@1,&@5);
		delete $1;
		delete $2;
		delete $4;
	}
	|'[' spvar_decl_list ']' IDENT '(' spvar_decl_list ')' {
		$$ = new FuncDecl(*$2,*$4,*$6);
		setLocation($$,&@$,&@1,&@7);
		delete $2;
		delete $4;
		delete $6;
	};

class_def:
	CLASS IDENT class_body {
		string emptyStr = "";
		$$ = new ClassDef(*$2,emptyStr,$3);
		setLocation($$,&@$,&@1,&@3);
		delete $2;
	}
	|CLASS IDENT ':' IDENT class_body {
		$$ = new ClassDef(*$2,*$4,$5);
		setLocation($$,&@$,&@1,&@5);
		delete $2;
		delete $4;
	};
class_body:
	'{' class_stmt_list '}' {
		$$ = $2;
		setLocation($$,&@$,&@1,&@3);
	};
class_stmt_list:
	/*blank*/ {
		$$ = new ClassBody();
		setLocation($$,&@$);
	}
	|var_def ';'{
		$$ = new ClassBody();
		$$->addField($1);
		setLocation($$,&@$,&@1,&@1);
	}
	|func_def {
		$$ = new ClassBody();
		$$->addMethod($1);
		setLocation($$,&@$,&@1,&@1);
	}
	|constructor {
		$$ = new ClassBody();
		$$->addConstructor($1);
		setLocation($$,&@$,&@1,&@1);
	}
	|class_stmt_list var_def ';'{
		$1->addField($2);
		$$ = $1;
		setLocation($$,&@$,&@1,&@2);
	}
	|class_stmt_list func_def {
		$1->addMethod($2);
		$$ = $1;
		setLocation($$,&@$,&@1,&@2);
	}
	|class_stmt_list constructor {
		$1->addConstructor($2);
		$$ = $1;
		setLocation($$,&@$,&@1,&@2);
	};
constructor:
	IDENT '(' spvar_decl_list ')' stmt_block {
		$$ = new Constructor(*$1,*$3,$5);
		setLocation($$,&@$,&@1,&@5);
		delete $1;
		delete $3;
	};

/*********************** statement ************************/
var_def:
	type_decl var_init_list {
		$$ = new VarDef($1,*$2);
		setLocation($$,&@$,&@1,&@2);
		delete $2;
	};
var_init:
	IDENT {
		$$ = new VarInit(*$1,NULL);
		setLocation($$,&@$,&@1,&@1);
		delete $1;
	}
	|IDENT '=' expr {
		$$ = new VarInit(*$1,$3);
		setLocation($$,&@$,&@1,&@3);
		delete $1;
	};
var_init_list:
	var_init {
		$$ = new vector<VarInit*>();
		$$->push_back($1);
		setLocation(NULL,&@$,&@1,&@1);
	}
	|var_init_list ',' var_init {
		$1->push_back($3);
		$$ = $1;
		setLocation(NULL,&@$,&@1,&@3);
	};

stmt:
	';' {
		$$ = new NullStmt();
		setLocation($$,&@$,&@1,&@1);
	}
	|var_def ';' {
		$$ = $1;
		setLocation($$,&@$,&@1,&@2);
	}
	|simple_stmt_list ';' {
		$$ = $1;
		setLocation($$,&@$,&@1,&@2);
	}
	|for_stmt {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	}
	|if_stmt {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	}
	|return_stmt ';' {
		$$ = $1;
		setLocation($$,&@$,&@1,&@2);
	}
	|BREAK ';' {
		$$ = new BreakStmt();
		setLocation($$,&@$,&@1,&@2);
	}
	|CONTINUE ';' {
		$$ = new ContinueStmt();
		setLocation($$,&@$,&@1,&@2);
	}
	|super_init ';' {
		$$ = $1;
		setLocation($$,&@$,&@1,&@2);
	}
	|stmt_block {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	};
stmt_block:
	'{' stmt_list '}' {
		$$ = new StmtBlock(*$2);
		setLocation(NULL,&@$,&@1,&@3);
		delete $2;
	};

var_assi:
	left_expr '=' expr {
		$$ = new VarAssi($1,$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|'[' left_expr_list_allow_null ']' '=' func_invoke {
		$$ = new MultiVarAssi(*$2,$5);
		setLocation($$,&@$,&@1,&@5);
		delete $2;
	};
array_assi:
	left_expr '=' '[' expr_list_allow_null ']' {
		$$ = new ArrayAssi($1,*$4);
		setLocation($$,&@$,&@1,&@5);
		delete $4;
	};

simple_stmt:
	var_assi {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	}
	|array_assi {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	}
	|expr {
		$$ = new ExprStmt($1);
		setLocation($$,&@$,&@1,&@1);
	}
	|left_expr '+' '+' {
		$$ = new IncStmt($1);
		setLocation($$,&@$,&@1,&@3);
	}
	|left_expr '-' '-' {
		$$ = new IncStmt($1,false);
		setLocation($$,&@$,&@1,&@3);
	};
simple_stmt_list:
	simple_stmt {
		$$ = new SimpleStmtList();
		$$->add($1);
		setLocation($$,&@$,&@1,&@1);
	}
	|simple_stmt_list ',' simple_stmt {
		$1->add($3);
		$$ = $1;
		setLocation($$,&@$,&@1,&@3);
	};

if_stmt:
	IF '(' expr ')' stmt ELSE stmt {
		$$ = new IfElseStmt($3,$5,$7);
		setLocation($$,&@$,&@1,&@7);
	}
	|IF '(' expr ')' stmt {
		$$ = new IfElseStmt($3,$5,NULL);
		setLocation($$,&@$,&@1,&@5);
	};

for_stmt:
	FOR '(' for_init ';' for_cond ';' for_loop ')' stmt {
		$$ = new ForStmt($3,$5,$7,$9);
		setLocation($$,&@$,&@1,&@9);
	};
for_init:
	/*blank*/ {
		$$ = new NullStmt();
		setLocation($$,&@$);
	}
	|var_def {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	}
	|simple_stmt_list {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	};
for_cond:
	/*blank*/ {
		$$ = new Bool(true);
		setLocation($$,&@$);
	}
	|expr {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	};
for_loop:
	/*blank*/ {
		$$ = new NullStmt();
		setLocation($$,&@$);
	}
	|simple_stmt_list {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	};

return_stmt:
	RETURN expr_list {
		$$ = new ReturnStmt(*$2);
		setLocation($$,&@$,&@1,&@2);
		delete $2;
	};

super_init:
	SUPER '(' expr_list ')' {
		$$ = new SuperInit(*$3);
		setLocation($$,&@$,&@1,&@4);
		delete $3;
	};

/*********************** expression ************************/
left_expr_list_allow_null:
	/*blank*/ {
		$$ = new vector<LeftValueExpr*>();
		setLocation(NULL,&@$);
	}
	|left_expr {
		$$ = new vector<LeftValueExpr*>();
		$$->push_back($1);
		setLocation(NULL,&@$,&@1,&@1);
	}
	|left_expr_list_allow_null ',' left_expr {
		if($1->size() == 0){
			$1->push_back(NULL);
		}
		$1->push_back($3);
		$$ = $1;
		setLocation(NULL,&@$,&@1,&@3);
	}
	|left_expr_list_allow_null ',' {
		if($1->size() == 0){
			$1->push_back(NULL);
		}
		$1->push_back(NULL);
		$$ = $1;
		setLocation(NULL,&@$,&@1,&@2);
	};

expr:
	expr '+' expr {
		$$ = new BinaryOpExpr($1,'+',$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|expr '-' expr {
		$$ = new BinaryOpExpr($1,'-',$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|left_expr '+' expr {
		$$ = new BinaryOpExpr($1,'+',$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|left_expr '-' expr {
		$$ = new BinaryOpExpr($1,'-',$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|expr '*' expr {
		$$ = new BinaryOpExpr($1,'*',$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|expr '/' expr {
		$$ = new BinaryOpExpr($1,'/',$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|expr '%' expr {
		$$ = new BinaryOpExpr($1,'%',$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|expr EQUAL expr {
		$$ = new BinaryOpExpr($1,EQUAL,$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|expr NEQUAL expr {
		$$ = new BinaryOpExpr($1,NEQUAL,$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|expr LE expr {
		$$ = new BinaryOpExpr($1,LE,$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|expr GE expr {
		$$ = new BinaryOpExpr($1,GE,$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|expr '<' expr {
		$$ = new BinaryOpExpr($1,'<',$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|expr '>' expr {
		$$ = new BinaryOpExpr($1,'>',$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|expr AND expr {
		$$ = new BinaryLogicExpr($1,AND,$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|expr OR expr {
		$$ = new BinaryLogicExpr($1,OR,$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|'(' expr ')' {
		$$ = $2;
		setLocation($$,&@$,&@1,&@3);
	}
	|'(' IDENT ')' {
		$$ = new IdentExpr(NULL,*$2);
		setLocation($$,&@$,&@1,&@3);
		delete $2;
	}
	|'-' expr %prec UMINUS {
		$$ = new PrefixOpExpr('-',$2);
		setLocation($$,&@$,&@1,&@2);
	}
	|'!' expr %prec LOGICNOT {
		$$ = new PrefixOpExpr('!',$2);
		setLocation($$,&@$,&@1,&@2);
	}
	|left_expr {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	}
	|numeric {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	}
	|bool {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	}
	|func_invoke {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	}
	|new_object {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	}
	|THIS {
		$$ = new ThisExpr();
		setLocation($$,&@$,&@1,&@1);
	}
	|SUPER {
		$$ = new SuperExpr();
		setLocation($$,&@$,&@1,&@1);
	}
	|null {
		$$ = new Nil();
		setLocation($$,&@$,&@1,&@1);
	}
	|new_array {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	}
	|STRING {
		$$ = new String($1);
		setLocation($$,&@$,&@1,&@1);
	}
	|array_init {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	}
	|dynamic_cast {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	}
	|instance_of {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	};

left_expr:
	ident_expr {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	}
	|array_element {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	};

ident_expr:
	IDENT {
		$$ = new IdentExpr(NULL,*$1);
		setLocation($$,&@$,&@1,&@1);
		delete $1;
	}
	|expr '.' IDENT {
		$$ = new IdentExpr($1,*$3);
		setLocation($$,&@$,&@1,&@3);
		delete $3;
	};

array_element:
	expr '[' expr ']' {
		$$ = new ArrayElement($1,$3);
		setLocation($$,&@$,&@1,&@4);
	};
	
numeric:
	LONG {
		$$ = new Long($1);
		setLocation($$,&@$,&@1,&@1);
	}
	|DOUBLE {
		$$ = new Double($1);
		setLocation($$,&@$,&@1,&@1);
	}
	|CHAR {
		$$ = new Char($1);
		setLocation($$,&@$,&@1,&@1);
	};

bool:
	TRUE {
		$$ = new Bool(true);
		setLocation($$,&@$,&@1,&@1);
	}
	|FALSE {
		$$ = new Bool(false);
		setLocation($$,&@$,&@1,&@1);
	};

expr_list:
	/*blank*/ {
		$$ = new vector<Expression*>();
		setLocation(NULL,&@$);
	}
	|expr {
		$$ = new vector<Expression*>();
		$$->push_back($1);
		setLocation(NULL,&@$,&@1,&@1);
	}
	|expr_list ',' expr {
		$1->push_back($3);
		$$ = $1;
		setLocation(NULL,&@$,&@1,&@3);
	};
expr_list_allow_null:
	/*blank*/ {
		$$ = new vector<Expression*>();
		setLocation(NULL,&@$);
	}
	|expr {
		$$ = new vector<Expression*>();
		$$->push_back($1);
		setLocation(NULL,&@$,&@1,&@1);
	}
	|expr_list_allow_null ',' expr {
		if($1->size() == 0){
			$1->push_back(NULL);
		}
		$1->push_back($3);
		$$ = $1;
		setLocation(NULL,&@$,&@1,&@3);
	}
	|expr_list_allow_null ',' {
		if($1->size() == 0){
			$1->push_back(NULL);
		}
		$1->push_back(NULL);
		$$ =$1;
		setLocation(NULL,&@$,&@1,&@2);
	};

func_invoke:
	IDENT '(' expr_list ')' {
		$$ = new FuncInvoke(NULL,*$1,*$3);
		setLocation($$,&@$,&@1,&@4);
		delete $1;
		delete $3;
	}
	|expr '.' IDENT '(' expr_list ')' {
		$$ = new FuncInvoke($1,*$3,*$5);
		setLocation($$,&@$,&@1,&@6);
		delete $3;
		delete $5;
	};

new_object:
	NEW IDENT '(' expr_list ')' {
		$$ = new FuncInvoke(NULL,*$2,*$4,true);
		setLocation($$,&@$,&@1,&@5);
		delete $2;
		delete $4;
	};

new_array:
	NEW type_decl '[' expr ']' {
		$$ = new NewArray($2,$4);
		setLocation($$,&@$,&@1,&@5);
	};

array_init:
	'{' expr_list '}' {
		$$ = new ArrayInit(*$2);
		setLocation($$,&@$,&@1,&@3);
		delete $2;
	};

dynamic_cast:
	'(' IDENT ')' expr  %prec DYNAMICCAST {
		TypeDecl *typeDecl = new TypeDecl(*$2);
		setNodeLocation(typeDecl,&@2);
		$$ = new DynamicCast(typeDecl,$4);
		setLocation($$,&@$,&@1,&@4);
		delete $2;
	}
	|'(' type_decl ')' expr %prec DYNAMICCAST {
		$$ = new DynamicCast($2,$4);
		setLocation($$,&@$,&@1,&@4);
	};

instance_of:
	expr ISA type_decl %prec INSTANCEOF {
		$$ = new InstanceOf($1,$3);
		setLocation($$,&@$,&@1,&@3);
	};
%%
