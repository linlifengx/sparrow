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
class Constructor;
class IdentExpr;

using namespace std;

class StmtBlock: public Node {
public:
	vector<Statement*> statements;

	StmtBlock(vector<Statement*> &statements) {
		this->statements = statements;
	}

	StmtBlock(Statement *statement) {
		statements.push_back(statement);
	}

	void codeGen(AstContext &astContext);
};

class ClassDef: public Node {
public:
	string className;
	string superName;
	ClassBody *body;

	ClassInfo *classInfo;

	ClassDef(string &className, string &superName, ClassBody *body) {
		this->className = className;
		this->superName = superName;
		this->body = body;
		this->classInfo = NULL;
	}

	void declGen();
	void codeGen(AstContext &astContext);
};

class ClassBody: public Node {
public:
	vector<VarDef*> fieldDefs;
	vector<FuncDef*> methodDefs;
	vector<Constructor*> constructors;

	void addField(VarDef *fieldDef) {
		fieldDefs.push_back(fieldDef);
	}

	void addMethod(FuncDef *methodDef) {
		methodDefs.push_back(methodDef);
	}

	void addConstructor(Constructor *constructor) {
		constructors.push_back(constructor);
	}
};

class FuncDef: public Node {
public:
	FuncDecl *funcDecl;
	StmtBlock *stmtBlock;

	FunctionInfo *functionInfo;

	FuncDef(FuncDecl *funcDecl, StmtBlock *stmtBlock) {
		this->funcDecl = funcDecl;
		this->stmtBlock = stmtBlock;
		this->functionInfo = NULL;
	}

	Function* declGen(ClassInfo *classInfo = NULL);
	void codeGen(AstContext &astContext);
};

class FuncDecl: public Node {
public:
	vector<string> returnClasses;
	vector<SimpleVarDecl*> returnDecls;
	string funcName;
	vector<SimpleVarDecl*> argDecls;
	int style; //0 normal 1 retdecl

	FunctionInfo *functionInfo;

	FuncDecl(vector<string> &returnClasses, string &funcName,
			vector<SimpleVarDecl*> &argDecls) {
		this->returnClasses = returnClasses;
		this->funcName = funcName;
		this->argDecls = argDecls;
		this->style = 0;
		this->functionInfo = NULL;
	}

	FuncDecl(vector<SimpleVarDecl*> &returnDecls, string &funcName,
			vector<SimpleVarDecl*> &argDecls) {
		this->returnDecls = returnDecls;
		this->funcName = funcName;
		this->argDecls = argDecls;
		this->style = 1;
		this->functionInfo = NULL;
	}

	FunctionInfo* codeGen(ClassInfo *classInfo = NULL);
};

class Constructor: public Node {
public:
	string name;
	vector<SimpleVarDecl*> argDeclList;
	StmtBlock *stmtBlock;

	FunctionInfo *functionInfo;

	Constructor(string &name, vector<SimpleVarDecl*> &argDeclList,
			StmtBlock *stmtBlock) {
		this->name = name;
		this->argDeclList = argDeclList;
		this->stmtBlock = stmtBlock;
		this->functionInfo = NULL;
	}
	void declGen(ClassInfo &classInfo);
	void codeGen(AstContext &astContext);
};

enum StmtType {
	NORMAL, VAR_DEF, SUPER_INIT
};

class Statement: public Node {
public:
	StmtType type;
	virtual void codeGen(AstContext &astContext)=0;
	virtual ~Statement() {
	}
	;
};

class VarDef: public Statement {
public:
	string typeName;
	vector<VarInit*> varInitList;

	VarDef(string &typeName, vector<VarInit*> &varInitList) {
		this->typeName = typeName;
		this->varInitList = varInitList;
		type = VAR_DEF;
	}

	void globalGen(AstContext &astContext);
	void fieldGen(AstContext &astContext);
	void codeGen(AstContext &astContext);
};

class VarAssi: public Statement {
public:
	IdentExpr *identExpr;
	Expression *expr;

	VarAssi(IdentExpr *identExpr, Expression *expr) {
		this->identExpr = identExpr;
		this->expr = expr;
		type = NORMAL;
	}

	void codeGen(AstContext &astContext);
};

class MultiVarAssi: public Statement {
public:
	vector<IdentExpr*> identList;
	FuncInvoke *funcInvoke;

	MultiVarAssi(vector<IdentExpr*> identList, FuncInvoke *funcInvoke) {
		this->identList = identList;
		this->funcInvoke = funcInvoke;
		type = NORMAL;
	}

	void codeGen(AstContext &astContext);
};

class SimpleStmtList: public Statement {
public:
	vector<Statement*> stmtList;

	SimpleStmtList() {
		type = NORMAL;
	}

	void codeGen(AstContext &astContext);
	void add(Statement* stmt) {
		stmtList.push_back(stmt);
	}
};

class ExprStmt: public Statement {
public:
	Expression *expr;

	ExprStmt(Expression *expr) {
		this->expr = expr;
		type = NORMAL;
	}

	void codeGen(AstContext &astContext);
};

class IfElseStmt: public Statement {
public:
	Expression *condExpr;
	StmtBlock *thenBlock;
	StmtBlock *elseBlock;

	IfElseStmt(Expression *condExpr, StmtBlock *thenBlock,
			StmtBlock *elseBlock) {
		this->condExpr = condExpr;
		this->thenBlock = thenBlock;
		this->elseBlock = elseBlock;
		type = NORMAL;
	}

	void codeGen(AstContext &astContext);
};

class ForStmt: public Statement {
public:
	Statement *initStmt;
	Expression *condExpr;
	Statement *loopStmt;
	StmtBlock *stmtList;

	ForStmt(Statement *initStmt, Expression *condExpr, Statement *loopStmt,
			StmtBlock *stmtList) {
		this->initStmt = initStmt;
		this->condExpr = condExpr;
		this->loopStmt = loopStmt;
		this->stmtList = stmtList;
		type = NORMAL;
	}

	void codeGen(AstContext &astContext);
};

class NullStmt: public Statement {
public:
	NullStmt() {
		type = NORMAL;
	}

	void codeGen(AstContext &astContext) {
	}
};

class ReturnStmt: public Statement {
public:
	vector<Expression*> exprList;

	ReturnStmt(vector<Expression*> &exprList) {
		this->exprList = exprList;
		type = NORMAL;
	}

	void codeGen(AstContext &astContext);
};

class BreakStmt: public Statement {
public:
	BreakStmt() {
		type = NORMAL;
	}

	void codeGen(AstContext &astContext);
};

class ContinueStmt: public Statement {
public:
	ContinueStmt() {
		type = NORMAL;
	}

	void codeGen(AstContext &astContext);
};

class SuperInit: public Statement {
public:
	vector<Expression*> exprList;

	SuperInit(vector<Expression*> &exprList) {
		this->exprList = exprList;
		type = SUPER_INIT;
	}

	void codeGen(AstContext &astContext);
};

#endif
