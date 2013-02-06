#ifndef AST_H
#define AST_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <set>

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

class AstFunction;
class AstType;
class AstValue;
class AstContext;
class Node;
class Program;
class SimpleVarDecl;
class VarInit;

class Statement;
class FuncDeclStmt;
class FuncDecl;
class FuncDecl2;
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

class ClassDecl;
class Constructor;
class NewObject;
class ObjectField;
class ObjectMethod;

extern int AND,OR,EQUAL,NEQUAL,LE,GE;

extern LLVMContext &context;
extern IRBuilder<> builder;
extern Module module;
extern Function *startFunc;
extern string errorMsg;

extern Value* createCast(Value *value,Type *type);
extern Constant* getInitial(Type *type);
extern void throwError(Node *node);
extern void throwWarning(Node *node,string msg);
extern string getOperatorName(int op);
extern string getTypeName(Type *type);

extern Type *ptrType;
extern Type *ptrptrType;
extern Type *int64Type;
extern Type *doubleType;
extern Type *boolType;
extern Type *voidType;

extern Constant *int64_0;
extern Constant *double_0;
extern Constant *bool_true;
extern Constant *bool_false;
extern Constant *ptr_null;

extern Function *sysObjectField;
extern Function *sysObjectAlloca;
extern Function *sysObjectMethod;

extern vector<AstFunction*> defaultContructors;

class AstFunction{
public:
	string name;
	Function *llvmFunction;
	Type *returnType;
	bool isReturnSingle;
	bool isReturnVoid;
	vector<AstType*> returnTypes;
	vector<AstType*> argTypes;
	int style;
	vector<AstValue*> returnVars;
	AstType *dominateType;
	AstFunction(string name,Function *llvmFunction,vector<AstType*> &returnTypes,
	vector<AstType*> &argTypes,AstType *dominateType=NULL,int style=1)
		:name(name),llvmFunction(llvmFunction),returnTypes(returnTypes),
		argTypes(argTypes),style(style),dominateType(dominateType){
		isReturnSingle = (returnTypes.size() ==  1);
		isReturnVoid = (returnTypes.size() == 0);
		returnType = llvmFunction->getReturnType();
	}
};

class AstType{
public:
	int status; //1 primitive 2 preparing 3 complete
	ClassDecl *classDecl;
	
	string name;
	AstType *superClass;
	map<string,AstFunction*> methodTable;
	map<string,AstType*> fieldTable;
	Function *initFunc;

	Type *llvmType;
	GlobalVariable *info;
	AstType(string name,Type *llvmType = ptrType,ClassDecl *classDecl=NULL)
		:name(name),llvmType(llvmType),classDecl(classDecl){
		status = 1;
		superClass = NULL;
		initFunc = NULL;
	}
	AstValue* getInitialValue();
	Constant* getInitial();
	
	bool addField(string fieldName,AstType *type);
	bool addMethod(string methodName,AstFunction *method);
	AstType* getFieldType(string fieldName);
	AstFunction* getMethod(string methodName);
	
	bool isSubOf(AstType *destType);
	
	bool isDoubleType(){return llvmType == doubleType;}
	bool isLongType(){return llvmType == int64Type;}
	bool isBoolType(){return llvmType == boolType;}
	bool isObjectType(){return llvmType == ptrType;}
};

class AstValue{
public:
	Value *llvmValue;
	AstType *type;
	AstValue(Value *llvmValue,AstType *type)
		:llvmValue(llvmValue),type(type){}
	bool castTo(AstType *destType);
};

class TypeContext{
	map<string,AstValue*> fieldTable;
public:
	AstValue *thisObject;
	AstValue *superObject;
	TypeContext(AstValue *thisObject=NULL,AstValue *superObject=NULL)
		:thisObject(thisObject),superObject(superObject){}
	AstValue* getField(string name);
	AstFunction* getMethod(string name);
};

class AstContext{
public:
	AstContext *parent;
	map<string,Type*> typeTable;
	map<string,AstFunction*> functionTable;
	map<string,AstValue*> varTable;
	map<string,AstType*> classTable;
	AstFunction *currentFunc;
	BasicBlock *breakOutBB;
	BasicBlock *continueBB;
	TypeContext *typeContext;

	AstContext(AstContext *parent=NULL):parent(parent){
		if(parent != NULL){
			currentFunc = parent->currentFunc;
			breakOutBB = parent->breakOutBB;
			continueBB = parent->continueBB;
			typeContext = parent->typeContext;
		}else{
			currentFunc = NULL;
			breakOutBB = NULL;
			continueBB = NULL;
			typeContext = NULL;
		}
	}

	AstType* getType(string name);
	AstFunction* getFunction(string name);
	AstValue* getVar(string name);

	bool addFunction(string name,AstFunction *function);
	bool addVar(string name,AstValue *var);
	bool addType(string name,AstType *type);
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
	vector<Statement*> &stmts;
	Program(vector<Statement*> &stmts):stmts(stmts){}
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

enum StmtType{
	FUNC_DECL,VAR_DECL,CLASS_DECL,INTERFACE_DECL,CONSTRUCTOR,SUPER_CALL,NORMAL
};

/* Statement declare */
class Statement : public Node{
public:
	virtual void codeGen(AstContext &astContext)=0;
	virtual StmtType stmtType(){return NORMAL;}
};

class FuncDeclStmt : public Statement{
public:
	string &funcName;
	virtual AstFunction* declGen(AstContext &astContext,AstType *astType=NULL)=0;
	virtual void codeGen(AstContext &astContext,AstType *astType=NULL)=0;
	virtual StmtType stmtType(){return FUNC_DECL;}
};

class FuncDecl : public FuncDeclStmt{
public:
	vector<string*> &retTypeNameList;
	vector<SimpleVarDecl*> &argDeclList;
	vector<Statement*> &stmtList;
	FuncDecl(vector<string*> &retTypeNameList,string &funcName,
			 vector<SimpleVarDecl*> &argDeclList,vector<Statement*> &stmtList)
				 :retTypeNameList(retTypeNameList),funcName(funcName),
				 argDeclList(argDeclList),stmtList(stmtList){}
	AstFunction* declGen(AstContext &astContext,AstType *astType=NULL);
	void codeGen(AstContext &astContext,AstType *astType=NULL);
	
};

class FuncDecl2 : public FuncDeclStmt{
public:
	vector<SimpleVarDecl*> &retDeclList;
	vector<SimpleVarDecl*> &argDeclList;
	vector<Statement*> &stmts;
	FuncDecl2(vector<SimpleVarDecl*> &retDeclList,string &funcName,
			 vector<SimpleVarDecl*> &argDeclList,vector<Statement*> &stmts)
				 :retDeclList(retDeclList),funcName(funcName),
				 argDeclList(argDeclList),stmts(stmts){}
	AstFunction* declGen(AstContext &astContext,AstType *astType=NULL);
	void codeGen(AstContext &astContext,AstType *astType=NULL);
};

class Constructor : public FuncDeclStmt{
public:
	vector<SimpleVarDecl*> &argDeclList;
	vector<Statement*> &stmts;
	Constructor(string &funcName,vector<SimpleVarDecl*> &argDeclList,vector<Statement*> &stmts)
				 :funcName(funcName),argDeclList(argDeclList),stmts(stmts){}
	AstFunction* declGen(AstContext &astContext,AstType *astType=NULL);
	void codeGen(AstContext &astContext,AstType *astType=NULL);
};

class VarDecl : public Statement{
public:
	string &typeName;
	vector<VarInit*> &varInitList;
	
	VarDecl(string &typeName,vector<VarInit*> &varInitList)
		:typeName(typeName),varInitList(varInitList){}
	void codeGen(AstContext &astContext);
	void globalCodeGen(AstContext &astContext);
	StmtType stmtType(){return VAR_DECL;}
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
	virtual AstValue* codeGen(AstContext &astContext)=0;
};

class BinaryExpr : public Expression{
public:
	Expression &lexpr;
	Expression &rexpr;
	int op;
	BinaryExpr(Expression &lexpr,int op,Expression &rexpr)
		:lexpr(lexpr),rexpr(rexpr),op(op){}
	AstValue* codeGen(AstContext &astContext);
};

class LogicExpr : public Expression{
public:
	Expression &lexpr;
	Expression &rexpr;
	int op;
	LogicExpr(Expression &lexpr,int op,Expression &rexpr)
		:lexpr(lexpr),rexpr(rexpr),op(op){}
	AstValue* codeGen(AstContext &astContext);
};

class PrefixExpr : public Expression{
public:
	int op;
	Expression &expr;
	PrefixExpr(int op,Expression &expr):op(op),expr(expr){}
	AstValue* codeGen(AstContext &astContext);
};

class IdentExpr : public Expression{
public:
	string &ident;
	IdentExpr(string &ident):ident(ident){}
	AstValue* codeGen(AstContext &astContext);
};

class CallExpr : public Expression{
public:
	string &funcName;
	vector<Expression*> &exprList;
	AstFunction *function;
	CallExpr(string &funcName,vector<Expression*> &exprList)
		:funcName(funcName),exprList(exprList){function=NULL;}
	AstValue* codeGen(AstContext &astContext);
	vector<AstValue*> multiCodeGen(AstContext &astContext);
};

class Long : public Expression{
public:
	string &valStr;
	Long(string &valStr):valStr(valStr){}
	AstValue* codeGen(AstContext &astContext);
};

class Double : public Expression{
public:
	string &valStr;
	Double(string &valStr):valStr(valStr){}
	AstValue* codeGen(AstContext &astContext);
};

class Bool : public Expression{
public:
	bool value;
	Bool(bool value):value(value){}
	AstValue* codeGen(AstContext &astContext);
};



class ClassDecl : public Statement{
public:
	string &className;
	string &superName;
	vector<Statement*> &stmtList;
	
	ClassDecl(string &className,string &superName,vector<Statement*> &stmtList):
		className(className),superName(superName),stmtList(stmtList){}
	void declGen(AstContext &astContext);
	void codeGen(AstContext &astContext);
	StmtType stmtType(){return CLASS_DECL;}
};

class NewObject : public Expression{
public:
	CallExpr &callExpr;
	NewObject(CallExpr &callExpr):callExpr(callExpr){}
	AstValue* codeGen(AstContext &astContext);
};

class ObjectField : public Expression{
public:
	Expression &expr;
	string &fieldName;
	ObjectField(Expression &expr,string &fieldName):expr(expr),fieldName(fieldName){}
	AstValue* codeGen(AstContext &astContext);
};

class ObjectMethod : public Expression{
public:
	Expression &expr;
	CallExpr &callExpr;
	ObjectMethod(Expression &expr,CallExpr &callExpr):expr(expr),callExpr(callExpr){}
	AstValue* codeGen(AstContext &astContext);
	vector<AstValue*> mulitCodeGen(AstContext &astContext);
};

#endif // AST_H
