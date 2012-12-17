#include "ast.hpp"
#include "parser.hpp"

void throwError(Node *node){
	cout<<node->firstLine<<":"<<node->firstColumn<<": error: "<<errorMsg<<endl;
	exit(1);
}

void throwWarning(Node *node,string msg){
	cout<<node->firstLine<<":"<<node->firstColumn<<": warning: "<<msg<<endl;
}

string getOperatorName(int op){
	switch(op){
	case '+':
		return "+";
	case '-':
		return "-";
	case '*':
		return "*";
	case '/':
		return "/";
	case AND:
		return "&&";
	case OR:
		return "||";
	case EQUAL:
		return "==";
	case NEQUAL:
		return "!=";
	case LE:
		return "<=";
	case GE:
		return ">=";
	case '<':
		return "<";
	case '>':
		return ">";
	case '=':
		return "=";
	case '!':
		return "!";
	defalut:
		return "unknow";
	}
}

string getTypeName(Type *type){
	if(type->isDoubleTy()){
		return "double";
	}else if(type->isIntegerTy(64)){
		return "long";
	}else if(type->isIntegerTy(1)){
		return "bool";
	}else if(type->isVoidTy()){
		return "void";
	}else{
		return "unknow";
	}
}

Value* createCast(Value *value,Type *type){
	Type *valType = value->getType();
	if(valType == type){
		return value;
	}else if(type->isDoubleTy() && valType->isDoubleTy()){
		return value;
	}else if(type->isIntegerTy(64) && valType->isIntegerTy(64)){
		return value;
	}else if(type->isDoubleTy() && valType->isIntegerTy(64)){
		return builder.CreateSIToFP(value,type);
	}else if(type->isIntegerTy(64) && valType->isDoubleTy()){
		return builder.CreateFPToSI(value,type);
	}else if(type->isIntegerTy(1) && valType->isIntegerTy(1)){
		return value;
	}else{
		errorMsg = "no viable conversion from '"+getTypeName(valType)
					  +"' to '"+getTypeName(type)+"'";
		return NULL;
	}
}

Constant* getInitial(Type *type){
	if(type->isDoubleTy()){
		return ConstantFP::get(builder.getDoubleTy(),0);
	}else if(type->isIntegerTy(64)){
		return builder.getInt64(0);
	}else if(type->isIntegerTy(1)){
		return builder.getInt1(false);
	}else{
		errorMsg = "no initializer for '"+getTypeName(type)+"'";
		return NULL;
	}
}

Type* AstContext::getType(string name){
	Type *type = typeTable[name];
	if(type == NULL && parent != NULL){
		type = parent->getType(name);
	}
	if(type == NULL){
		if(name == "void"){
			errorMsg = "variable has incomplete type 'void'";
		}else{
			errorMsg = "undeclared type '"+name+"'";
		}
	}
	return type;
}

MyFunction* AstContext::getFunction(string name) throw(string){
	MyFunction *function = functionTable[name];
	if(function == NULL && parent != NULL){
		return parent->getFunction(name);
	}
	if(function == NULL){
		errorMsg = "undeclared function '"+name+"'";
	}
	return function;
}

Value* AstContext::getVar(string name){
	Value *var = varTable[name];
	if(var == NULL && parent != NULL){
		return parent->getVar(name);
	}
	if(var == NULL){
		errorMsg = "undeclared identifier '"+name+"'";
	}
	return var;
}

bool AstContext::addFunction(string name, MyFunction *function){
	if(functionTable[name] != NULL){
		errorMsg = "redefine function named '"+name+"'";
		return false;
	}
	functionTable[name] = function;
	return true;
}

bool AstContext::addVar(string name, Value *value){
	if(varTable[name] != NULL){
		errorMsg = "redefine variable named '"+name+"'";
		return false;
	}
	varTable[name] = value;
	return true;
}

bool AstContext::addType(string name, Type *type){
	if(typeTable[name] != NULL){
		errorMsg =  "redefine type named '"+name+"'";
		return false;
	}
	typeTable[name] = type;
	return true;
}

void Program::codeGen(AstContext &astContext){
	for(unsigned i=0; i<stmts.size(); i++){
		GlobalStatement *stmt = stmts[i];
		stmt->globalDeclGen(astContext);
	}
	
	//create init func
	FunctionType *initFuncType = FunctionType::get(builder.getVoidTy(),false);
	Function *initFunc = Function::Create(initFuncType,Function::ExternalLinkage,"main",&module);
	builder.SetInsertPoint(BasicBlock::Create(context,"entry",initFunc));
	for(unsigned i=0;i<stmts.size();i++){
		GlobalStatement *stmt = stmts[i];
		if(!stmt->isFuncDecl()){
			stmt->globalCodeGen(astContext);
		}
	}

	MyFunction *mainFunc = astContext.getFunction("main");
	if(mainFunc == NULL){
		cout<<errorMsg<<endl;
	}else{
		builder.CreateCall(mainFunc->llvmFunction);
		builder.CreateRetVoid();
	}

	startFunc = initFunc;

	for(unsigned i = 0; i < stmts.size(); i++){
		GlobalStatement *stmt = stmts[i];
		if(stmt->isFuncDecl()){
			stmt->globalCodeGen(astContext);
		}
	}
}
