#ifndef AST_NODE_H
#define AST_NODE_H

#include <vector>
#include <stddef.h>

class ClassDef;
class VarDef;
class FuncDef;
class AstContext;

class Expression;

using namespace std;

class Node{
public:
	int firstLine;
	int firstColumn;
	int lastLine;
	int lastColumn;
};

class Program : public Node{
public:
	vector<ClassDef*> classDefs;
	vector<VarDef*> varDefs;
	vector<FuncDef*> funcDefs;
	
	void addClass(ClassDef *classDef){
		classDefs.push_back(classDef);
	}
	
	void addVar(VarDef *varDef){
		varDefs.push_back(varDef);
	}
	
	void addFunction(FuncDef *funcDef){
		funcDefs.push_back(funcDef);
	}
	
	void codeGen(AstContext *astContext);
};

class VarInit : public Node{
public:
	string *varName;
	Expression *expr;
	
	VarInit(string *varName,Expression *expr=NULL){
		this->varName = varName;
		this->expr = expr;
	}
};

class SimpleVarDecl : public Node{
public:
	string *typeName;
	string *varName;
	
	SimpleVarDecl(string *typeName,string *varName){
		this->typeName = typeName;
		this->varName = varName;
	}
};

#endif