#ifndef AST_EXPRESSION_H
#define AST_EXPRESSION_H

#include <stddef.h>
#include "node.h"

class AValue;

using namespace std;

class Expression: public Node {
public:
	virtual AValue codeGen(AstContext &astContext)=0;
	virtual ~Expression(){};
};

class BinaryOpExpr: public Expression {
public:
	Expression *leftExpr;
	Expression *rightExpr;
	int op;

	BinaryOpExpr(Expression *leftExpr, int op, Expression *rightExpr) {
		this->leftExpr = leftExpr;
		this->op = op;
		this->rightExpr = rightExpr;
	}

	AValue codeGen(AstContext &astContext);
};

class BinaryLogicExpr: public Expression {
public:
	Expression *leftExpr;
	Expression *rightExpr;
	int op;

	BinaryLogicExpr(Expression *leftExpr, int op, Expression *rightExpr) {
		this->leftExpr = leftExpr;
		this->op = op;
		this->rightExpr = rightExpr;
	}

	AValue codeGen(AstContext &astContext);
};

class PrefixOpExpr: public Expression {
public:
	int op;
	Expression *expr;

	PrefixOpExpr(int op, Expression *expr) {
		this->op = op;
		this->expr = expr;
	}

	AValue codeGen(AstContext &astContext);
};

class IdentExpr: public Expression {
public:
	Expression *expr;
	string ident;

	IdentExpr(Expression *expr, string &ident) {
		this->expr = expr;
		this->ident = ident;
	}

	AValue codeGen(AstContext &astContext);
	AValue lvalueGen(AstContext &astContext);
};

class FuncInvoke: public Expression {
public:
	Expression *expr;
	string funcName;
	vector<Expression*> exprList;
	bool isConstructor;

	FuncInvoke(Expression *expr, string &funcName,
			vector<Expression*> &exprList, bool isConstructor = false) {
		this->funcName = funcName;
		this->exprList = exprList;
		this->expr = expr;
		this->isConstructor = isConstructor;
	}

	vector<AValue> multiCodeGen(AstContext &astContext);
	AValue codeGen(AstContext &astContext);

};

class Long: public Expression {
public:
	long long value;

	Long(long long value) {
		this->value = value;
	}

	AValue codeGen(AstContext &astContext);
};

class Double: public Expression {
public:
	double value;

	Double(double value) {
		this->value = value;
	}

	AValue codeGen(AstContext &astContext);
};

class Bool: public Expression {
public:
	bool value;

	Bool(bool value) {
		this->value = value;
	}

	AValue codeGen(AstContext &astContext);
};

class Nil: public Expression {
public:
	AValue codeGen(AstContext &astContext);
};

class ThisExpr: public Expression {
public:
	AValue codeGen(AstContext &astContext);
};

class SuperExpr: public Expression {
public:
	AValue codeGen(AstContext &astContext);
};

#endif
