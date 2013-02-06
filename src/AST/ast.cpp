#include "ast.hpp"
//#include "parser.hpp"

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

AstValue* AstType::getInitialValue(){
	return new AstValue(getInitial(),this);
}

Constant* AstType::getInitial(){
	if(llvmType == int64Type){
		return int64_0;
	}else if(llvmType == doubleType){
		return double_0;
	}else if(llvmType == ptrType){
		return ptr_null;
	}else if(llvmType == boolType){
		return bool_false;
	}else{
		cout<<"impossible........"<<endl;
		exit(0);
	}
}

bool AstType::addField(string fieldName, AstType* type){
	if(fieldTable[fieldName] != NULL){
		errorMsg = "redefine field named '"+fieldName+"'";
		return false;
	}else{
		if(superClass != NULL){
			AstType *superFieldType = superClass->getFieldType(fieldName);
			if(superFieldType != NULL && !type->isSubOf(superFieldType)){
				errorMsg = "field named '"+fieldName+"' of class '"+name+"' can't override the field of super";
				return false;
			}
		}
		fieldTable[fieldName] = type;
		return true;
	}
}

bool AstType::addMethod(string methodName, AstFunction* method){
	if(methodTable[methodName] != NULL){
		errorMsg = "redefine method named '"+methodName+"'";
		return false;
	}else{
		if(superClass != NULL){
			AstFunction *superMethod = superClass->getMethod(methodName);
			if(superMethod != NULL){
				if(method->returnTypes.size() != superMethod->returnTypes.size() ||
					method->argTypes.size() != superMethod->argTypes.size()){
					errorMsg = "method '"+methodName+"' of class '"+name+"' can't override the method of super";
					return false;
				}else{
					for(unsigned i = 0; i < method->returnTypes.size(); i++){
						if(method->returnTypes[i] != superMethod->returnTypes[i]){
							errorMsg = "method '"+methodName+"' of class '"+name+"' can't override the method of super";
							return false;
						}
					}
					for(unsigned i = 1; i < method->argTypes.size(); i++){
						if(method->argTypes[i] != superMethod->argTypes[i]){
							errorMsg = "method '"+methodName+"' of class '"+name+"' can't override the method of super";
							return false;
						}
					}
				}
			}
		}
		methodTable[methodName] = method;
		return true;
	}
}

AstType* AstType::getFieldType(string fieldName){
	AstType *type = fieldTable[fieldName];
	if(type == NULL && superClass != NULL){
		type = superClass->getFieldType(fieldName);
	}
	if(type == NULL){
		errorMsg = "undeclared field '"+fieldName+"' in class '"+name+"'";
	}
	return type;
}

AstFunction* AstType::getMethod(string methodName){
	AstFunction *method = fieldTable[methodName];
	if(method == NULL && superClass != NULL){
		method = superClass->getMethod(methodName);
	}
	if(method == NULL){
		errorMsg = "undeclared method '"+methodName+"'";
	}
	return method;
}

bool AstType::isSubOf(AstType* superType){
	if(superClass == NULL || superType == NULL){
		return false;
	}else if(superClass == superType){
		return true;
	}else{
		return superClass->isSubOf(superType);
	}
}

bool AstValue::castTo(AstType* destType){
	if(destType == type){
		return true;
	}else if(type->isBoolType() && destType->isBoolType()){
		return true;
	}else if(type->isDoubleType() && destType->isDoubleType()){
		return true;
	}else if(type->isLongType() && destType->isLongType()){
		return true;
	}else if(type->isDoubleType() && destType->isLongType()){
		llvmValue = builder.CreateFPToSI(llvmValue,int64Type);
		type = destType;
		return true;
	}else if(type->isLongType() && destType->isDoubleType()){
		llvmValue = builder.CreateSIToFP(llvmValue,doubleType);
		type = destType;
		return true;
	}else if(type->isObjectType() && destType->isObjectType() && type->isSubOf(destType)){
		return true;
	}else{
		errorMsg = "no viable conversion from '"+type->name
					  +"' to '"+destType->name+"'";
		return false;
	}
}


AstValue* TypeContext::getField(string name){
	AstValue *var = fieldTable[name];
	if(var == NULL) {
		AstType *fieldType = thisObject->type->getFieldType(name);
		if(fieldType != NULL) {
			Value *fieldName = builder.CreateGlobalStringPtr(name);
			Value *field = builder.CreateCall2(sysObjectField, thisObject->llvmValue, fieldName);
			field = builder.CreateBitCast(field, PointerType::get(fieldType->llvmType));
			var = new AstValue(field, fieldType);
			fieldTable[name] = var;
		}
	}
	if(var == NULL){
		errorMsg = "undeclared field '"+name+"'";
	}
	return var;
}

AstFunction* TypeContext::getMethod(string name){
	return thisObject->type->getMethod(string);
}

AstType* AstContext::getType(string name){
	AstType *type = typeTable[name];
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

AstFunction* AstContext::getFunction(string name){
	AstFunction *function = functionTable[name];
	if(function == NULL && typeContext != NULL){
		function = typeContext->getMethod(name);
	}
	if(function == NULL && parent != NULL){
		return parent->getFunction(name);
	}
	if(function == NULL){
		errorMsg = "undeclared function '"+name+"'";
	}
	return function;
}

AstValue* AstContext::getVar(string name){
	AstValue *var = varTable[name];
	if(var == NULL && typeContext != NULL){
		var = typeContext->getField(name);
	}
	if(var == NULL && parent != NULL){
		return parent->getVar(name);
	}
	if(var == NULL){
		errorMsg = "undeclared identifier '"+name+"'";
	}
	return var;
}

bool AstContext::addFunction(string name, AstFunction *function){
	if(functionTable[name] != NULL){
		errorMsg = "redefine function named '"+name+"'";
		return false;
	}
	functionTable[name] = function;
	return true;
}

bool AstContext::addVar(string name, AstValue *value){
	if(varTable[name] != NULL){
		errorMsg = "redefine variable named '"+name+"'";
		return false;
	}
	varTable[name] = value;
	return true;
}

bool AstContext::addType(string name, AstType *type){
	if(typeTable[name] != NULL){
		errorMsg =  "redefine type named '"+name+"'";
		return false;
	}
	typeTable[name] = type;
	return true;
}

void Program::codeGen(AstContext &astContext){
	vector<ClassDecl*> classDecls;
	vector<FuncDeclStmt*> funcDecls;
	vector<VarDecl*> varDecls;
	for(unsigned i = 0; i < stmts.size(); i++){
		StmtType stmtType = stmts[i]->stmtType();
		switch(stmtType){
		case CLASS_DECL:
			classDecls.push_back(static_cast<ClassDecl*>(stmts[i]));
			break;
		case FUNC_DECL:
			funcDecls.push_back(static_cast<FuncDeclStmt*>(stmts[i]));
			break;
		case VAR_DECL:
			varDecls.push_back(static_cast<VarDecl*>(stmts[i]));
			break;
		default:
			;
		}
	}
	
	//1 class check
	for(unsigned i = 0; i < classDecls.size(); i++){
		ClassDecl *classDecl = classDecls[i];
		if(!astContext.addType(classDecl->className,new AstType(classDecl->className,ptrType,classDecl))){
			throwError(classDecl);
		}
	}
	for(unsigned i = 0; i < classDecls.size(); i++){
		ClassDecl *classDecl = classDecls[i];
		AstType *astType = astContext.getType(classDecl->className);
		if(classDecl->superName != ""){
			AstType *superClass = astContext.getType(classDecl->superName);
			if(superClass != NULL){
				astType->superClass = superClass;
			}else{
				throwError(classDecl);
			}
		}
	}
	
	//2 class decl gen
	for(unsigned i = 0; i < classDecls.size(); i++){
		AstType *astType = astContext.getType(classDecls[i]->className);
		extern void classDeclGen(AstContext &astContext,AstType *astType);
		classDeclGen(astContext,astType);
	}
	
	//3 func decl gen
	for(unsigned  i = 0; i < funcDecls.size(); i++){
		funcDecls[i]->declGen(astContext);
	}
	
	//4 create init func and global var gen
	FunctionType *initFuncType = FunctionType::get(builder.getVoidTy(),false);
	Function *initFunc = Function::Create(initFuncType,Function::ExternalLinkage,"main",&module);
	builder.SetInsertPoint(BasicBlock::Create(context,"entry",initFunc));
	
	for(unsigned  i = 0; i < varDecls.size(); i++){
		varDecls[i]->globalCodeGen(astContext);
	}
	
	AstFunction *mainFunc = astContext.getFunction("main");
	if(mainFunc == NULL){
		cout<<errorMsg<<endl;
	}else{
		builder.CreateCall(mainFunc->llvmFunction);
		builder.CreateRetVoid();
	}
	startFunc = initFunc;
	
	//5 class and func gen
	for(unsigned i = 0; i < classDecls.size(); i++){
		classDecls[i]->codeGen(astContext);
	}
	for(unsigned  i = 0; i < funcDecls.size(); i++){
		funcDecls[i]->codeGen(astContext);
	}
	
	// class default constructor gen
	
	
	
	
	return;
}


void classDeclGen(AstContext &astContext,AstType *astType){
	if(astType->status == 3){
		return;
	}
	astType->status = 2;
	AstType *superClass = astType.superClass;
	if(superClass != NULL){
		if(superClass->status == 2){
			errorMsg = "cyclic inheritance involving class '"+superClass->name+"'";
			throwError(superClass->classDecl);
		}else if(superClass->status == 1){
			classDeclGen(astContext,superClass);
		}
	}
	astType->classDecl->declGen(astContext);
	astType->status = 3;
}