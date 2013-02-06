#ifndef AST_EXPRESSION_H
#define AST_EXPRESSION_H

#include <stddef.h>
#include "node.h"

class AValue;

using namespace std;


class Expression : public Node{
public:
	virtual AValue codeGen(AstContext *astContext)=0;
};

class BinaryOpExpr : public Expression{
public:
	Expression *leftExpr;
	Expression *rightExpr;
	int op;
	
	BinaryOpExpr(Expression *leftExpr,int op,Expression *rightExpr){
		this->leftExpr = leftExpr;
		this->op = op;
		this->rightExpr = rightExpr;
	}
	
	AValue codeGen(AstContext *astContext);
};

class PrefixOpExpr : public Expression{
public:
	int op;
	Expression *expr;
	
	PrefixOpExpr(int op,Expression *expr){
		this->op = op;
		this->expr = expr;
	}
	
	AValue codeGen(AstContext *astContext);
};

class IdentExpr : public Expression{
public:
	string *ident;
	
	IdentExpr(string *ident){
		this->ident = ident;
	}
	
	AValue codeGen(AstContext *astContext);
};

class FuncInvoke : public Expression{
public:
	string *funcName;
	vector<Expression*> *exprList;
	
	FuncInvoke(string *funcName,vector<Expression*> *exprList){
		this->funcName = funcName;
		this->exprList = exprList;
	}
	
	AValue codeGen(AstContext *astContext);
	vector<AValue> multiCodeGen(AstContext *astContext);
};

class MethodInvoke : public Expression{
public:
	Expression *expr;
	FuncInvoke *funcInvoke;
	
	MethodInvoke(Expression *expr,FuncInvoke *funcInvoke){
		this->expr = expr;
		this->funcInvoke = funcInvoke;
	}
	
	AValue codeGen(AstContext *astContext);
};

class Long : public Expression{
public:
	string *valStr;
	
	Long(string *valStr){
		this->valStr = valStr;
	}
	
	AValue codeGen(AstContext *astContext);
};

class Double : public Expression{
public:
	string *valStr;
	
	Double(string *valStr){
		this->valStr = valStr;
	}
	
	AValue codeGen(AstContext *astContext);
};

class Bool : public Expression{
public:
	bool value;
	
	Bool(bool value){
		this->value = value;
	}
	
	AValue codeGen(AstContext *astContext);
};

class NewObjExpr : public Expression{
public:
	FuncInvoke *funcInvoke;
	
	NewObjExpr(FuncInvoke *funcInvoke){
		this->funcInvoke = funcInvoke;
	}
	
	AValue codeGen(AstContext *astContext);
};

class ObjectField : public Expression{
public:
	Expression *expr;
	string *fieldName;
	
	ObjectField(Expression *expr,string *fieldName){
		this->expr = expr;
		this->fieldName = fieldName;
	}
	
	AValue codeGen(AstContext *astContext);
};

#endif