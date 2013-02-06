#ifndef AST_SUPPORT_H
#define AST_SUPPORT_H

#include <stddef.h>
#include <map>
#include <vector>
#include <string>
#include <iostream>

#include <llvm/IRBuilder.h>
#include <llvm/Function.h>
#include <llvm/Module.h>
#include <llvm/Constants.h>

class Node;
class ClassInfo;
class FunctionInfo;
class ClassContext;
class ClassDef;
class AFunction;
class AValue;

using namespace std;
using namespace llvm;

extern LLVMContext &context;
extern Module module;
extern IRBuilder<> builder;

extern map<string, ClassInfo*> classTable;
extern map<string, FunctionInfo*> functionTable;
extern Type *ptrType;
extern Type *ptrptrType;
extern Type *int64Type;
extern Type *doubleType;
extern Type *boolType;
extern Type *voidType;
extern ClassInfo *longClass;
extern ClassInfo *doubleClass;
extern ClassInfo *boolClass;
extern ClassInfo *nilClass;
extern ClassInfo *voidClass;

extern Constant *int64_0;
extern Constant *double_0;
extern Constant *bool_true;
extern Constant *bool_false;
extern Constant *ptr_null;

extern Constant *sysGCinit;
extern Constant *sysObjectField;
extern Constant *sysObjectMethod;
extern Constant *sysObjectAlloca;
extern Function *mainFunc;

extern string errorMsg;

extern void throwError(Node *node);
extern bool addClass(ClassInfo *clazz);
extern bool addFunction(FunctionInfo *func);
extern ClassInfo* getClass(string &name);
extern string getOperatorName(int op);
extern AFunction getFunctionV(string &name);
extern AFunction getMethodV(AValue object, string &methodName);
extern AValue getFieldV(AValue object, string &fieldName);
extern Value* createAlloca(Type *type, BasicBlock *bb);

class ClassInfo {
public:
	string name;
	int status; //0 primitive 1 preparing 2 complete
	ClassDef *classDef;

	ClassInfo *superClassInfo;
	Type *llvmType;
	Value *info; //ptr of first element
	Function *initor; // void (i8*) *
	FunctionInfo *constructor;
	map<string, ClassInfo*> fieldTable;
	map<string, FunctionInfo*> methodTable;

	ClassInfo(string name, ClassDef *classDef = NULL, Type *llvmType = ptrType);

	bool isSubClassOf(ClassInfo *superClass);
	bool addField(string &name, ClassInfo *fieldClass);
	bool addMethod(FunctionInfo *method);
	ClassInfo* getFieldClass(string &name);
	FunctionInfo* getMethod(string &name);
	Constant* getInitial();
};

class FunctionInfo {
public:
	string name;
	Function *llvmFunction;

	Type *returnType;
	int returnNum;
	vector<ClassInfo*> returnClasses;
	vector<ClassInfo*> argClasses;
	ClassInfo *dominateClass;
	int style;  //0 normal 1 retdecl 2 constructor

	FunctionInfo(string name, Function *llvmFunction,
			vector<ClassInfo*> returnClasses, vector<ClassInfo*> argClasses,
			int style = 0, ClassInfo *dominateClass = NULL) {
		this->name = name;
		this->llvmFunction = llvmFunction;
		this->returnClasses = returnClasses;
		this->argClasses = argClasses;
		this->dominateClass = dominateClass;
		this->style = style;

		returnNum = returnClasses.size();
		returnType = llvmFunction->getReturnType();
	}

	FunctionInfo() {
		this->llvmFunction = NULL;
		this->returnType = NULL;
		this->dominateClass = NULL;
		this->style = 0;
		this->returnNum = 0;
	}
};

class AValue {
public:
	Value *llvmValue;
	ClassInfo *clazz;

	AValue(Value *llvmValue = NULL, ClassInfo *clazz = NULL) {
		this->llvmValue = llvmValue;
		this->clazz = clazz;
	}

	bool castTo(ClassInfo *destClazz);
	bool isBool();
	bool isLong();
	bool isDouble();
	bool isObject();
};

class AFunction {
public:
	Value *llvmFunc;
	FunctionInfo *funcInfo;

	AFunction(Value *llvmFunc = NULL, FunctionInfo *funcInfo = NULL) {
		this->llvmFunc = llvmFunc;
		this->funcInfo = funcInfo;
	}

};

class AstContext {
public:
	AstContext *superior;
	map<string, AValue> varTable;
	FunctionInfo *currentFunc;
	BasicBlock *allocBB;
	BasicBlock *breakOutBB;
	BasicBlock *continueBB;
	ClassContext *classContext;
	vector<Value*> returnVars;
	Value *returnAlloc;

	explicit AstContext(AstContext *superior = NULL) {
		this->superior = superior;
		if (superior != NULL) {
			this->currentFunc = superior->currentFunc;
			this->allocBB = superior->allocBB;
			this->breakOutBB = superior->breakOutBB;
			this->continueBB = superior->continueBB;
			this->classContext = superior->classContext;
			this->returnAlloc = superior->returnAlloc;
		} else {
			this->currentFunc = NULL;
			this->allocBB = NULL;
			this->breakOutBB = NULL;
			this->continueBB = NULL;
			this->classContext = NULL;
			this->returnAlloc = NULL;
		}
	}

	bool addVar(string &name, AValue avalue);
	AValue getVar(string &name);
	AFunction getFunc(string &name);
	vector<Value*>& getReturnVars();
};

class ClassContext {
public:
	BasicBlock *allocBB;
	ClassInfo *currentClass;
	Value *thisObject;
	Value *superObject;
	map<string, AValue> fieldTable;
	map<string, AFunction> methodTable;
	map<string, AValue> superFieldTable;
	map<string, AFunction> superMethodTable;

	ClassContext(ClassInfo *currentClass, BasicBlock *allocBB,
			Value *thisObject = NULL, Value *superObject = NULL) {
		this->currentClass = currentClass;
		this->thisObject = thisObject;
		this->superObject = superObject;
		this->allocBB = allocBB;
	}

	AValue getField(string &name);
	AFunction getMethod(string &name);
	AValue getSuperField(string &name);
	AFunction getSuperMethod(string &name);
};

#endif
