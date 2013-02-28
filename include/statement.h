#ifndef AST_STATEMENT_H_
#define AST_STATEMENT_H_

#include "node.h"

using namespace std;

enum StmtType {
	NORMAL,
	STMT_BLOCK,
	VAR_DEF,
	VAR_ASSI,
	SIMPLE_LIST,
	EXPR_STMT,
	IFELSE_STMT,
	FOR_STMT,
	NULL_STMT,
	RETRUN_STMT,
	BREAK_STMT,
	CONTINUE_STMT,
	SUPER_INIT,
	ARRAY_ASSI
};

class Statement: public Node {
public:
	StmtType type;
	virtual void codeGen(AstContext &astContext)=0;
	virtual ~Statement() {
	}
};

class StmtBlock: public Statement {
public:
	vector<Statement*> statements;

	StmtBlock(vector<Statement*> &statements) {
		this->statements = statements;
		this->type = STMT_BLOCK;
	}

	StmtBlock(Statement *statement) {
		statements.push_back(statement);
		this->type = STMT_BLOCK;
	}

	void codeGen(AstContext &astContext);
};

class VarDef: public Statement {
public:
	TypeDecl *typeDecl;
	vector<VarInit*> varInitList;

	VarDef(TypeDecl *typeDecl, vector<VarInit*> &varInitList) {
		this->typeDecl = typeDecl;
		this->varInitList = varInitList;
		this->type = VAR_DEF;
	}

	void globalGen();
	void fieldGen(AstContext &astContext);
	void codeGen(AstContext &astContext);
};

class VarAssi: public Statement {
public:
	LeftValueExpr *leftExpr;
	Expression *expr;

	VarAssi(LeftValueExpr *leftExpr, Expression *expr) {
		this->leftExpr = leftExpr;
		this->expr = expr;
		this->type = VAR_ASSI;
	}

	void codeGen(AstContext &astContext);
};

class MultiVarAssi: public Statement {
public:
	vector<LeftValueExpr*> leftExprList;
	FuncInvoke *funcInvoke;

	MultiVarAssi(vector<LeftValueExpr*> leftExprList, FuncInvoke *funcInvoke) {
		this->leftExprList = leftExprList;
		this->funcInvoke = funcInvoke;
		this->type = VAR_ASSI;
	}

	void codeGen(AstContext &astContext);
};

class SimpleStmtList: public Statement {
public:
	vector<Statement*> stmtList;

	SimpleStmtList() {
		this->type = SIMPLE_LIST;
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
		this->type = EXPR_STMT;
	}

	void codeGen(AstContext &astContext);
};

class IncStmt: public Statement {
public:
	LeftValueExpr *expr;
	bool inc;

	IncStmt(LeftValueExpr *expr, bool inc = true) {
		this->expr = expr;
		this->inc = inc;
		this->type = EXPR_STMT;
	}

	void codeGen(AstContext &astContext);
};

class IfElseStmt: public Statement {
public:
	Expression *condExpr;
	Statement *thenBlock;
	Statement *elseBlock;

	IfElseStmt(Expression *condExpr, Statement *thenBlock,
			Statement *elseBlock) {
		this->condExpr = condExpr;
		this->thenBlock = thenBlock;
		this->elseBlock = elseBlock;
		this->type = IFELSE_STMT;
	}

	void codeGen(AstContext &astContext);
};

class ForStmt: public Statement {
public:
	Statement *initStmt;
	Expression *condExpr;
	Statement *loopStmt;
	Statement *block;

	ForStmt(Statement *initStmt, Expression *condExpr, Statement *loopStmt,
			Statement *block) {
		this->initStmt = initStmt;
		this->condExpr = condExpr;
		this->loopStmt = loopStmt;
		this->block = block;
		this->type = FOR_STMT;
	}

	void codeGen(AstContext &astContext);
};

class NullStmt: public Statement {
public:
	NullStmt() {
		this->type = NULL_STMT;
	}

	void codeGen(AstContext &astContext) {
	}
};

class ReturnStmt: public Statement {
public:
	vector<Expression*> exprList;

	ReturnStmt(vector<Expression*> &exprList) {
		this->exprList = exprList;
		this->type = RETRUN_STMT;
	}

	void codeGen(AstContext &astContext);
};

class BreakStmt: public Statement {
public:
	BreakStmt() {
		this->type = BREAK_STMT;
	}

	void codeGen(AstContext &astContext);
};

class ContinueStmt: public Statement {
public:
	ContinueStmt() {
		this->type = CONTINUE_STMT;
	}

	void codeGen(AstContext &astContext);
};

class SuperInit: public Statement {
public:
	vector<Expression*> exprList;
	ClassContext *classContext;

	SuperInit(vector<Expression*> &exprList) {
		this->exprList = exprList;
		this->type = SUPER_INIT;
		this->classContext = NULL;
	}

	void codeGen(AstContext &astContext);
};

class ArrayAssi: public Statement {
public:
	LeftValueExpr *leftExpr;
	vector<Expression*> exprList;

	ArrayAssi(LeftValueExpr *leftExpr, vector<Expression*> exprList) {
		this->leftExpr = leftExpr;
		this->exprList = exprList;
		this->type = ARRAY_ASSI;
	}

	void codeGen(AstContext &astContext);
};

#endif
