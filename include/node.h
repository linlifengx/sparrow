#ifndef AST_NODE_H_
#define AST_NODE_H_

#include "common.h"

using namespace std;
using namespace llvm;

class Node {
public:
	int firstLine;
	int firstColumn;
	int lastLine;
	int lastColumn;
};

class Program: public Node {
public:
	vector<ClassDef*> classDefs;
	vector<VarDef*> varDefs;
	vector<FuncDef*> funcDefs;

	void addClassDef(ClassDef *classDef) {
		classDefs.push_back(classDef);
	}

	void addVarDef(VarDef *varDef) {
		varDefs.push_back(varDef);
	}

	void addFuncDef(FuncDef *funcDef) {
		funcDefs.push_back(funcDef);
	}

	void codeGen();
};

class VarInit: public Node {
public:
	string varName;
	Expression *expr;

	VarInit(string &varName, Expression *expr = NULL) {
		this->varName = varName;
		this->expr = expr;
	}
};

class SimpleVarDecl: public Node {
public:
	TypeDecl *typeDecl;
	string varName;

	SimpleVarDecl(TypeDecl *typeDecl, string &varName) {
		this->typeDecl = typeDecl;
		this->varName = varName;
	}
};

class TypeDecl: public Node {
public:
	string typeName;
	unsigned dimension;

	TypeDecl(string &typeName, unsigned dimension = 0) {
		this->typeName = typeName;
		this->dimension = dimension;
	}

	ClassInfo* getClassInfo();
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
	void codeGen();
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
	void codeGen();
};

class FuncDecl: public Node {
public:
	vector<TypeDecl*> returnTypes;
	vector<SimpleVarDecl*> returnDecls;
	string funcName;
	vector<SimpleVarDecl*> argDecls;
	int style; //0 normal 1 retdecl

	FunctionInfo *functionInfo;

	FuncDecl(vector<TypeDecl*> &returnTypes, string &funcName,
			vector<SimpleVarDecl*> &argDecls) {
		this->returnTypes = returnTypes;
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
	void codeGen();
};

#endif
