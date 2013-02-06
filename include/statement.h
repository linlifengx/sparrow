#ifndef AST_STATEMENT_H
#define AST_STATEMENT_H

#include <stddef.h>
#include "node.h"
#include "support.h"

class FuncDecl;
class ClassDecl;
class ClassBody;
class Statement;
class Expression;
class FuncDef;
class VarDef;
class FuncInvoke;
class MethodInvoke;
class Constructor;

using namespace std;

class StmtBlock : public Node{
public:
	vector<Statement*> *statements;
	
	StmtBlock(vector<Statement*> *statements){
		this->statements = statements;
	}
	
	void codeGen(AstContext *astContext);
};

class ClassDef : public Node{
public:
	string *className;
	string *superName;
	ClassBody *body;
	
	ClassDef(string *className,string *superName,ClassBody *body){
		this->className = className;
		this->superName = superName;
		this->body = body;
	}
	
	void declGen(AstContext *astContext);
	void codeGen(AstContext *astContext);
};

class ClassBody : public Node{
public:
	vector<VarDef*> fieldDefs;
	vector<FuncDef*> methodDefs;
	vector<Constructor*> constructors; 
	
	void addField(VarDef *fieldDef){
		fieldDefs.push_back(fieldDef);
	}
	
	void addMethod(FuncDef *methodDef){
		methodDefs.push_back(methodDef);
	}
	
	void addConstructor(Constructor *constructor){
		constructors.push_back(constructor);
	}
};

class FuncDef : public Node{
public:
	FuncDecl *funcDecl;
	StmtBlock *stmtBlock;
	
	FuncDef(FuncDecl *funcDecl,StmtBlock *stmtBlock){
		this->funcDecl = FuncDecl;
		this->stmtBlock = stmtBlock;
	}
	
	Function* declGen(AstContext *astContext,ClassInfo *classInfo=NULL);
	void codeGen(AstContext *astContext,ClassInfo *classInfo);
};

class FuncDecl : public Node{
public:
	vector<string> returnClasses;
	vector<SimpleVarDecl*> returnDecls;
	string *funcName;
	vector<SimpleVarDecl*> argDecls;
	int style; //0 normal 1 retdecl
	
	FuncDecl(vector<string*> returnClasses,string *funcName,vector<SimpleVarDecl*> argDecls){
		this->returnClasses = returnClasses;
		this->funcName = funcName;
		this->argDecls = argDecls;
		style = 0;
	}
	
	FuncDecl(vector<SimpleVarDecl*> *returnDecls,string *funcName,vector<SimpleVarDecl*> *argDecls){
		this->returnDecls = returnDecls;
		this->funcName = funcName;
		this->argDecls = argDecls;
		returnClasses = NULL;
		style = 1;
	}
	
	Function* codeGen(AstContext *astContext,ClassInfo *classInfo=NULL);
};

class Constructor{
public:
	string name;
	vector<SimpleVarDecl*> argDeclList;
	StmtBlock *stmtBlock;
	Constructor(string &name,vector<SimpleVarDecl*> &argDeclList,StmtBlock *stmtBlock){
		this->name = name;
		this->argDeclList = argDeclList;
		this->stmtBlock = stmtBlock;
	}
	void declGen(AstContext *astContext,ClassInfo *classInfo);
	void codeGen(AstContext *astContext);
};

class Statement : public Node{
public:
	virtual void codeGen(AstContext *astContext)=0;
};

class VarDef : public Statement{
public:
	string *typeName;
	vector<VarInit*> *varInitList;
	
	VarDef(string *typeName,vector<VarInit*> *varInitList){
		this->typeName = typeName;
		this->varInitList = varInitList;
	}
	
	void globalGen(AstContext *astContext);
	void fieldGen(AstContext *astContext);
	void codeGen(AstContext *astContext);
};

class VarAssi : public Statement{
public:
	string *varName;
	Expression *expr;
	
	VarAssi(string *varName,Expression *expr){
		this->varName = varName;
		this->expr = expr;
	}
	
	void codeGen(AstContext *astContext);
};

class MultiVarAssi : public Statement{
public:
	vector<string*> *varNameList;
	FuncInvoke *funcInvoke;
	MethodInvoke *methodInvoke;
	
	MultiVarAssi(vector<string*> *varNameList,FuncInvoke *funcInvoke){
		this->varNameList = varNameList;
		this->funcInvoke = funcInvoke;
		methodInvoke = NULL;
	}
	
	MultiVarAssi(vector<string*> *varNameList,MethodInvoke *methodInvoke){
		this->varNameList = varNameList;
		this->methodInvoke = methodInvoke;
		funcInvoke = NULL;
	}
	
	void codeGen(AstContext *astContext);
};

class SimpleStmtList : public Statement{
public:
	vector<Statement*> stmtList;

	void codeGen(AstContext *astContext);
};

class ExprStmt : public Statement{
public:
	Expression *expr;
	
	ExprStmt(Expression *expr){
		this->expr = expr;
	}
	
	void codeGen(AstContext *astContext);
};

class IfElseStmt : public Statement{
public:
	Expression *condExpr;
	StmtBlock *thenBlock;
	StmtBlock *elseBlock;
	
	IfElseStmt(Expression *condExpr,StmtBlock *thenBlock,StmtBlock *elseBlock){
		this->condExpr = condExpr;
		this->thenBlock = thenBlock;
		this->elseBlock = elseBlock;
	}
	
	void codeGen(AstContext *astContext);
};

class ForStmt : public Statement{
public:
	Statement *initStmt;
	Expression *condExpr;
	Statement *loopStmt;
	StmtBlock *stmtList;
	
	ForStmt(Statement *initStmt,Expression *condExpr,Statement *loopStmt,StmtBlock *stmtList){
		this->initStmt = initStmt;
		this->condExpr = condExpr;
		this->loopStmt = loopStmt;
		this->stmtList = stmtList;
	}
	
	void codeGen(AstContext *astContext);
};

class NullStmt : public Statement{
public:
	void codeGen(AstContext *astContext);
};

class ReturnStmt : public Statement{
public:
	vector<Expression*> *exprList;
	
	ReturnStmt(vector<Expression*> *exprList){
		this->exprList = exprList;
	}
	
	void codeGen(AstContext *astContext);
};

class BreakStmt : public Statement{
public:
	void codeGen(AstContext *astContext);
};

class ContinueStmt : public Statement{
public:
	void codeGen(AstContext *astContext);
};

#endif