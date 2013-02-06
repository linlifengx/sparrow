#ifndef AST_SUPPORT_H
#define AST_SUPPORT_H

#include <stddef.h>

class ClassInfo;
class FunctionInfo;
class ClassContext;
class ClassDef;

using namespace std;
using namespace llvm;

extern LLVMContext &context;
extern IRBuilder<> builder;
extern Module module;

extern map<string,ClassInfo*> classTable;
extern map<string,FunctionInfo*> functionTable;
extern vector<AstFunction*> defaultContructors;
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
extern Function *mainFunc;

extern string errorMsg;

extern void throwError(Node *node);
extern bool addClass(ClassInfo *clazz);
extern bool addFunction(FunctionInfo *func);
extern ClassInfo* getClass(string &name);
extern FunctionInfo* getFunction(string &name);

class ClassInfo{
public:
	string name;
	int status; //0 primitive 1 preparing 2 complete
	ClassDef *classDef;
	
	ClassInfo *superClassInfo;
	Type *llvmType;
	GlobalVariable *info; //ptr of first element
	Function *initor; // void (i8*) *
	FunctionInfo *constructor;
	map<string,ClassInfo*> fieldTable;
	map<string,FunctionInfo*> methodTable;
	
	ClassInfo(string name,ClassDef *classDef=NULL,Type *llvmType=ptrType){
		this->name = name;
		this->llvmType = llvmType;
		this->classDef = classDef;
		
		status = 0;
		superClassInfo = NULL;
		initor = NULL;
		constructor = NULL;
	}
	
	bool isSubClassOf(ClassInfo *superClass);
	bool addField(string &name,ClassInfo *fieldClass);
	bool addMethod(FunctionInfo *method);
	ClassInfo* getFieldClass(string &name);
	FunctionInfo* getMethod(string &name);
	Constant* getInitial();
};

class FunctionInfo{
public:
	string name;
	Function *llvmFunction;
	
	Type *returnType;
	int returnNum;
	vector<ClassInfo*> returnClasses;
	vector<ClassInfo*> argClasses;
	ClassInfo *dominateClass;
	int style;
	
	FunctionInfo(string &name,Function *llvmFunction,vector<ClassInfo*> returnClasses,
		vector<ClassInfo*> argClasses,int style=0,ClassInfo *dominateClass=NULL){
		this->name = name;
		this->llvmFunction = llvmFunction;
		this->returnClasses = returnClasses;
		this->argClasses = argClasses;
		this->dominateClass = dominateClass;
		this->style = style;
		
		returnNum = returnClasses.size();
		returnType = llvmFunction->getReturnType();
	}
	
	FunctionInfo(){
		this->llvmFunction = NULL;
	}
};

class AValue{
public:
	Value *llvmValue;
	ClassInfo *clazz;
	
	AValue(Value *llvmValue=NULL,ClassInfo *clazz=NULL){
		this->llvmValue = llvmValue;
		this->clazz = clazz;
	}
	
	bool castTo(ClassInfo *destClazz);
};

class AFunction{
public:
	Value *llvmFunc;
	FunctionInfo *funcInfo;
	
	AFunction(Value *llvmFunc=NULL,FunctionInfo *funcInfo=NULL){
		this->llvmFunc = llvmFunc;
		this->funcInfo = funcInfo;
	}
	
};

class AstContext{
public:
	AstContext *superior;
	map<string,AValue> varTable;
	FunctionInfo *currentFunc;
	BasicBlock *breakOutBB;
	BasicBlock *continueBB;
	ClassContext *classContext;
	vector<Value*> returnVars;
	
	AstContext(AstContext *superior=NULL){
		this->superior = superior;
	}
	
	bool addVar(string &name,AValue avalue);
	bool addReturnVar(string &name,AValue avalue);
	AValue getVar(string &name);
	AFunction getFunc(string &name);
};

class ClassContext{
public:
	ClassInfo *currentClass;
	Value *thisObject;
	Value *superObject;
	map<string,AValue> fieldTable;
	map<string,FunctionInfo> methodTable;
	
	ClassContext(ClassInfo *currentClass,Value *thisObject=NULL,Value *superObject=NULL){
		this->currentClass = currentClass;
		this->thisObject = thisObject;
		this->superObject = superObject;
	}
	
	AValue getField(string &name);
	AFunction getMethod(string &name);
};

#endif