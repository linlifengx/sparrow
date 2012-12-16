#include "ast.hpp"
#include "parser.hpp"

void throwError(Node *node,string msg){
	cout<<node->firstLine<<":"<<node->firstColumn<<": error: "<<msg<<endl;
	exit(1);
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

Value* createCast(Value *value,Type *type) throw(string){
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
		string fromType = getTypeName(valType);
		string toType = getTypeName(type);
		throw string("no viable conversion from '"+fromType+"' to '"+toType+"'");
	}
}

Constant* getInitial(Type *type) throw(string){
	if(type->isDoubleTy()){
		return ConstantFP::get(builder.getDoubleTy(),0);
	}else if(type->isIntegerTy(64)){
		return builder.getInt64(0);
	}else if(type->isIntegerTy(1)){
		return builder.getInt1(false);
	}else{
		throw string("no initializer for '"+getTypeName(type)+"'");
	}
}

Type* AstContext::getType(string name) throw(string){
	Type *type = typeTable[name];
	if(type == NULL && parent != NULL){
		type = parent->getType(name);
	}
	if(type == NULL){
		if(name == "void"){
			throw string("variable has incomplete type 'void'");
		}else{
			throw string("undeclared type '"+name+"'");
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
		throw string("undeclared function '"+name+"'");
	}
	return function;
}

Value* AstContext::getVar(string name) throw(string){
	Value *var = varTable[name];
	if(var == NULL && parent != NULL){
		return parent->getVar(name);
	}
	if(var == NULL){
		throw string("undeclared identifier '"+name+"'");
	}
	return var;
}

void AstContext::addFunction(string name, MyFunction *function) throw(string){
	if(functionTable[name] != NULL){
		throw string("redefine function named '"+name+"'");
	}
	functionTable[name] = function;
}

void AstContext::addVar(string name, Value *value) throw(string){
	if(varTable[name] != NULL){
		throw string("redefine variable named '"+name+"'");
	}
	varTable[name] = value;
}

void AstContext::addType(string name, Type *type) throw(string){
	if(typeTable[name] != NULL){
		throw string("redefine type named '"+name+"'");
	}
	typeTable[name] = type;
}

void Program::codeGen(AstContext &astContext){
	for(unsigned i=0; i<stmts.size(); i++){
		GlobalStatement *stmt = stmts[i];
		stmt->globalDeclGen(astContext);
	}
	
	//create init func
	FunctionType *initFuncType = FunctionType::get(builder.getVoidTy(),false);
	Function *initFunc = Function::Create(initFuncType,Function::ExternalLinkage,"",&module);
	builder.SetInsertPoint(BasicBlock::Create(context,"entry",initFunc));
	for(unsigned i=0;i<stmts.size();i++){
		GlobalStatement *stmt = stmts[i];
		if(!stmt->isFuncDecl()){
			stmt->globalCodeGen(astContext);
		}
	}
	try{
		MyFunction *mainFunc = astContext.getFunction("main");
		builder.CreateCall(mainFunc->llvmFunction);
	}catch(string msg){
		cout<<msg<<endl;
	}
	builder.CreateRetVoid();

	startFunc = initFunc;

	for(unsigned i = 0; i < stmts.size(); i++){
		GlobalStatement *stmt = stmts[i];
		if(stmt->isFuncDecl()){
			stmt->globalCodeGen(astContext);
		}
	}
}
