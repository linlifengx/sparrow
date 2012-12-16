#ifndef AST_H
#define AST_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <typeinfo>

#include <llvm/Value.h>
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/Type.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Function.h>
#include <llvm/BasicBlock.h>
#include <llvm/Argument.h>
#include <llvm/Instructions.h>
#include <llvm/IRBuilder.h>

#include <stdio.h>

using namespace std;
using namespace llvm;

class MyFunction;
class AstContext;
class MyFunction;
class Node;
class Program;
class SimpleVarDecl;
class VarInit;

class GlobalStatement;
class FuncDecl;
class FuncDecl2;

class Statement;
class VarDecl;
class VarAssi;
class MultiVarAssi;
class ExprStmt;
class SimpleStmtList;
class IfElseStmt;
class ForStmt;
class NullStmt;
class ReturnStmt;
class BreakStmt;
class ContinueStmt;

class Expression;
class BinaryExpr;
class LogicExpr;
class PrefixExpr;
class IdentExpr;
class CallExpr;
class Long;
class Double;
class Bool;

extern LLVMContext &context;
extern IRBuilder<> builder;
extern Module module;
extern Function *startFunc;

extern Value* createCast(Value *value,Type *type) throw(string);
extern Constant* getInitial(Type *type) throw(string);
extern void throwError(Node *node,string msg);
extern string getOperatorName(int op);
extern string getTypeName(Type *type);

class MyFunction{
public:
	string name;
	Function *llvmFunction;
	Type *returnType;
	bool isReturnSingle;
	bool isReturnVoid;
	vector<Type*> returnTypes;
	vector<Type*> argTypes;
	int style;
	vector<Value*> returnVars;
	MyFunction(string name,Function *llvmFunction,vector<Type*> &returnTypes,vector<Type*> &argTypes,int style=1)
		:name(name),llvmFunction(llvmFunction),returnTypes(returnTypes),argTypes(argTypes),style(style){
		isReturnSingle = (returnTypes.size() ==  1);
		isReturnVoid = (returnTypes.size() == 0);
		returnType = llvmFunction->getReturnType();
	}
};

class AstContext{
	AstContext *parent;
	map<string,Type*> typeTable;
	map<string,MyFunction*> functionTable;
	map<string,Value*> varTable;
public:
	MyFunction *currentFunc;
	BasicBlock *breakOutBB;
	BasicBlock *continueBB;

	AstContext(AstContext *parent=NULL):parent(parent){
		if(parent != NULL){
			currentFunc = parent->currentFunc;
			breakOutBB = parent->breakOutBB;
			continueBB = parent->continueBB;
		}else{
			currentFunc = NULL;
			breakOutBB = NULL;
			continueBB = NULL;
		}
	}

	Type* getType(string name) throw(string);
	MyFunction* getFunction(string name) throw(string);
	Value* getVar(string name) throw(string);
	void addFunction(string name,MyFunction *MyFunction) throw(string);
	void addVar(string name,Value *var) throw(string);
	void addType(string name,Type *type) throw(string);
};

class Node{
public:
	int firstLine;
	int firstColumn;
	int lastLine;
	int lastColumn;
};

class Program : public Node{
public:
	vector<GlobalStatement*> &stmts;
	Program(vector<GlobalStatement*> &stmts):stmts(stmts){}
	void codeGen(AstContext &astContext);
};

class VarInit : public Node{
public:
	string &varName;
	Expression *expr;
	VarInit(string &varName,Expression *expr):varName(varName),expr(expr){}
};

class SimpleVarDecl : public Node{
public:
	string &typeName;
	string &varName;
	SimpleVarDecl(string &typeName,string &varName)
		:typeName(typeName),varName(varName){}
};

/* Statement declare */
class Statement : public Node{
public:
	virtual void codeGen(AstContext &astContext)=0;
};

class GlobalStatement : public Statement{
public:
	virtual void globalDeclGen(AstContext &astContext)=0;
	virtual void globalCodeGen(AstContext &astContext)=0;
	virtual bool isFuncDecl()=0;
	virtual void codeGen(AstContext &astContext)=0;
};

class FuncDecl : public GlobalStatement{
public:
	vector<string*> &retTypeNameList;
	string &funcName;
	vector<SimpleVarDecl*> &argDeclList;
	vector<Statement*> &stmtList;
	FuncDecl(vector<string*> &retTypeNameList,string &funcName,
			 vector<SimpleVarDecl*> &argDeclList,vector<Statement*> &stmtList)
				 :retTypeNameList(retTypeNameList),funcName(funcName),
				 argDeclList(argDeclList),stmtList(stmtList){}
	void globalDeclGen(AstContext &astContext);
	void globalCodeGen(AstContext &astContext);
	void codeGen(AstContext &astContext){};
	bool isFuncDecl(){return true;};
};

class FuncDecl2 : public GlobalStatement{
public:
	vector<SimpleVarDecl*> &retDeclList;
	string &funcName;
	vector<SimpleVarDecl*> &argDeclList;
	vector<Statement*> &stmts;
	FuncDecl2(vector<SimpleVarDecl*> &retDeclList,string &funcName,
			 vector<SimpleVarDecl*> &argDeclList,vector<Statement*> &stmts)
				 :retDeclList(retDeclList),funcName(funcName),
				 argDeclList(argDeclList),stmts(stmts){}
	void globalDeclGen(AstContext &astContext);
	void globalCodeGen(AstContext &astContext);
	void codeGen(AstContext &astContext){};
	bool isFuncDecl(){return true;};
};

class VarDecl : public GlobalStatement{
public:
	string &typeName;
	vector<VarInit*> &varInitList;
	
	VarDecl(string &typeName,vector<VarInit*> &varInitList)
		:typeName(typeName),varInitList(varInitList){}
	void codeGen(AstContext &astContext);
	void globalDeclGen(AstContext &astContext);
	void globalCodeGen(AstContext &astContext);
	bool isFuncDecl(){return false;};
};

class VarAssi : public Statement{
public:
	string &varName;
	Expression &expr;
	VarAssi(string &varName,Expression &expr):varName(varName),expr(expr){}
	void codeGen(AstContext &astContext);
};

class MultiVarAssi : public Statement{
public:
	vector<string*> &varNameList;
	CallExpr &callExpr;
	MultiVarAssi(vector<string*> &varNameList,CallExpr &callExpr)
		:varNameList(varNameList),callExpr(callExpr){}
	void codeGen(AstContext &astContext);
};

class SimpleStmtList : public Statement{
public:
	vector<Statement*> stmtList;
	SimpleStmtList(){};
	void add(Statement *stmt);
	void codeGen(AstContext &astContext);
};

class ExprStmt : public Statement{
public:
	Expression &expr;
	ExprStmt(Expression &expr):expr(expr){};
	void codeGen(AstContext &astContext);
};

class IfElseStmt : public Statement{
public:
	Expression &condExpr;
	vector<Statement*> &thenStmts;
	vector<Statement*> &elseStmts;
	IfElseStmt(Expression &condExpr,vector<Statement*> &thenStmts,vector<Statement*> &elseStmts)
		:condExpr(condExpr),thenStmts(thenStmts),elseStmts(elseStmts){}
	void codeGen(AstContext &astContext);
};

class ForStmt : public Statement{
public:
	Statement &initStmt;
	Expression &condExpr;
	Statement &loopStmt;
	vector<Statement*> &stmtList;
	ForStmt(Statement &initStmt,Expression &condExpr,Statement &loopStmt,vector<Statement*> &stmtList)
		:initStmt(initStmt),condExpr(condExpr),loopStmt(loopStmt),stmtList(stmtList){}
	void codeGen(AstContext &astContext);
};

class NullStmt : public Statement{
public:
	NullStmt(){}
	void codeGen(AstContext &astContext){}
};

class ReturnStmt : public Statement{
public:
	vector<Expression*> &exprList;
	ReturnStmt(vector<Expression*> &exprList):exprList(exprList){}
	void codeGen(AstContext &astContext);
};

class BreakStmt : public Statement{
public:
	BreakStmt(){}
	void codeGen(AstContext &astContext);
};

class ContinueStmt : public Statement{
public:
	ContinueStmt(){}
	void codeGen(AstContext &astContext);
};

/* Expression declare */
class Expression : public Node{
public:
	virtual Value* codeGen(AstContext &astContext)=0;
};

class BinaryExpr : public Expression{
public:
	Expression &lexpr;
	Expression &rexpr;
	int op;
	BinaryExpr(Expression &lexpr,int op,Expression &rexpr)
		:lexpr(lexpr),rexpr(rexpr),op(op){}
	Value* codeGen(AstContext &astContext);
};

class LogicExpr : public Expression{
public:
	Expression &lexpr;
	Expression &rexpr;
	int op;
	LogicExpr(Expression &lexpr,int op,Expression &rexpr)
		:lexpr(lexpr),rexpr(rexpr),op(op){}
	Value* codeGen(AstContext &astContext);
};

class PrefixExpr : public Expression{
public:
	int op;
	Expression &expr;
	PrefixExpr(int op,Expression &expr):op(op),expr(expr){}
	Value* codeGen(AstContext &astContext);
};

class IdentExpr : public Expression{
public:
	string &ident;
	IdentExpr(string &ident):ident(ident){}
	Value* codeGen(AstContext &astContext);
};

class CallExpr : public Expression{
public:
	string &funcName;
	vector<Expression*> &exprList;
	CallExpr(string &funcName,vector<Expression*> &exprList)
		:funcName(funcName),exprList(exprList){}
	Value* codeGen(AstContext &astContext);
	vector<Value*> multiCodeGen(AstContext &astContext);
};

class Long : public Expression{
public:
	string &valStr;
	Long(string &valStr):valStr(valStr){}
	Value* codeGen(AstContext &astContext);
};

class Double : public Expression{
public:
	string &valStr;
	Double(string &valStr):valStr(valStr){}
	Value* codeGen(AstContext &astContext);
};

class Bool : public Expression{
public:
	bool value;
	Bool(bool value):value(value){}
	Value* codeGen(AstContext &astContext);
};

#endif // AST_H
