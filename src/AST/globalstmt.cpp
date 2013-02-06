#include "ast.hpp"
//#include "parser.hpp"

void FuncDecl::globalDeclGen(AstContext &astContext){
	vector<Type*> returnTypes;
	if(retTypeNameList.size() > 1 || *retTypeNameList[0] != "void"){
		for(unsigned i = 0; i < retTypeNameList.size(); i++){
			string &typeName = *retTypeNameList[i];
			Type *type = astContext.getType(typeName);
			if(type == NULL){
				throwError(this);
			}
			returnTypes.push_back(type);
		}
	}
	
	Type *returnType = NULL;
	if(returnTypes.size() == 0){
		returnType = builder.getVoidTy();
	}else if(returnTypes.size() == 1){
		returnType =  returnTypes[0];
	}else{
		ArrayRef<Type*> typesArray(returnTypes);
		returnType = StructType::create(context,typesArray);
	}
	
	vector<Type*> argTypes;
	for(unsigned i = 0; i < argDeclList.size(); i++){
		SimpleVarDecl *argDecl = argDeclList[i];
		Type *type = astContext.getType(argDecl->typeName);
		if(type == NULL){
			throwError(argDecl);
		}
		argTypes.push_back(type);
	}
	
	FunctionType *functionType = NULL;
	if(argTypes.size() == 0){
		functionType = FunctionType::get(returnType,false);
	}else{
		ArrayRef<Type*> argTypeArrayRef(argTypes);
		functionType = FunctionType::get(returnType,argTypeArrayRef,false);
	}
	Function *function = Function::Create(functionType,Function::ExternalLinkage,funcName+"_sp",&module);
	MyFunction *myFunction = new MyFunction(funcName,function,returnTypes,argTypes);
	if(!astContext.addFunction(funcName,myFunction)){
		throwError(this);
	}
}

void FuncDecl::globalCodeGen(AstContext &astContext){
	MyFunction *myFunction = astContext.getFunction(funcName);
	Function* function = myFunction->llvmFunction;
	vector<Type*> &argTypes = myFunction->argTypes;
	AstContext newContext(&astContext);
	builder.SetInsertPoint(BasicBlock::Create(context,"entry",function));
	unsigned i = 0;
	for(Function::arg_iterator ai = function->arg_begin();ai != function->arg_end(); ai++,i++){
		SimpleVarDecl *argDecl = argDeclList[i];
		Value *alloc = builder.CreateAlloca(argTypes[i]);
		builder.CreateStore(ai,alloc);
		if(!newContext.addVar(argDecl->varName,alloc)){
			throwError(argDecl);
		}
	}
	newContext.currentFunc = myFunction;
	for(i = 0; i < stmtList.size(); i++){
		stmtList[i]->codeGen(newContext);
	}
	if(myFunction->isReturnVoid){
		builder.CreateRetVoid();
	}else if(myFunction->isReturnSingle){
		Value *retVal = getInitial(myFunction->returnType);
		if(retVal == NULL){
			throwError(this);
		}
		builder.CreateRet(retVal);
	}else{
		Value *alloc = builder.CreateAlloca(myFunction->returnType);
		vector<Type*> returnTypes = myFunction->returnTypes;
		for(i = 0; i < returnTypes.size(); i++){
			Value *element = builder.CreateStructGEP(alloc,i);
			Value *elemVal = getInitial(returnTypes[i]);
			if(elemVal == NULL){
				throwError(this);
			}
			builder.CreateStore(elemVal,element);
		}
		builder.CreateRet(builder.CreateLoad(alloc));
	}
}

void FuncDecl2::globalDeclGen(AstContext &astContext){
	vector<Type*> returnTypes;
	for(unsigned i = 0; i < retDeclList.size(); i++){
		SimpleVarDecl *retDecl = retDeclList[i];
		Type *type = astContext.getType(retDecl->typeName);
		if(type == NULL){
			throwError(retDecl);
		}
		returnTypes.push_back(type);
	}
	
	Type *returnType = NULL;
	if(returnTypes.size() == 0){
		returnType = builder.getVoidTy();
	}else if(returnTypes.size() == 1){
		returnType =  returnTypes[0];
	}else{
		ArrayRef<Type*> typesArray(returnTypes);
		returnType = StructType::create(context,typesArray);
	}
	
	vector<Type*> argTypes;
	for(unsigned i = 0; i < argDeclList.size(); i++){
		SimpleVarDecl *argDecl = argDeclList[i];
		Type *type = astContext.getType(argDecl->typeName);
		if(type == NULL){
			throwError(argDecl);
		}
		argTypes.push_back(type);
	}
	
	FunctionType *functionType = NULL;
	if(argTypes.size() == 0){
		functionType = FunctionType::get(returnType,false);
	}else{
		ArrayRef<Type*> argTypeArrayRef(argTypes);
		functionType = FunctionType::get(returnType,argTypeArrayRef,false);
	}
	Function *function = Function::Create(functionType,Function::ExternalLinkage,funcName+"_sp",&module);
	MyFunction *myFunction = new MyFunction(funcName,function,returnTypes,argTypes,2);
	if(!astContext.addFunction(funcName,myFunction)){
		throwError(this);
	}
}

void FuncDecl2::globalCodeGen(AstContext &astContext){
	MyFunction *myFunction = astContext.getFunction(funcName);
	Function* function = myFunction->llvmFunction;
	vector<Type*> &argTypes = myFunction->argTypes;
	vector<Type*> &retTypes = myFunction->returnTypes;
	AstContext newContext(&astContext);
	builder.SetInsertPoint(BasicBlock::Create(context,"entry",function));
	unsigned i = 0;
	for(Function::arg_iterator ai = function->arg_begin();ai != function->arg_end(); ai++,i++){
		SimpleVarDecl *argDecl = argDeclList[i];
		Value *alloc = builder.CreateAlloca(argTypes[i]);
		builder.CreateStore(ai,alloc);
		if(!newContext.addVar(argDecl->varName,alloc)){
			throwError(argDecl);
		}
	}
	vector<Value*> retVarList;
	for(i = 0; i < retDeclList.size(); i++){
		SimpleVarDecl *retDecl = retDeclList[i];
		Value *alloc = builder.CreateAlloca(retTypes[i]);
		Value *initial = getInitial(retTypes[i]);
		if(initial == NULL){
			throwError(retDecl);
		}
		if(!newContext.addVar(retDecl->varName,alloc)){
			throwError(retDecl);
		}
		retVarList.push_back(alloc);
	}
	myFunction->returnVars = retVarList;
	newContext.currentFunc = myFunction;
	for(i = 0; i < stmts.size(); i++){
		stmts[i]->codeGen(newContext);
	}
	if(myFunction->isReturnVoid){
		builder.CreateRetVoid();
	}else if(myFunction->isReturnSingle){
		builder.CreateRet(builder.CreateLoad(retVarList[0]));
	}else{
		Value *alloc = builder.CreateAlloca(myFunction->returnType);
		for(i = 0; i < retVarList.size(); i++){
			Value *element = builder.CreateStructGEP(alloc,i);
			builder.CreateStore(builder.CreateLoad(retVarList[i]),element);
		}
		builder.CreateRet(builder.CreateLoad(alloc));
	}
}

void VarDecl::globalDeclGen(AstContext &astContext){
	Type *type = astContext.getType(typeName);
	if(type == NULL){
		throwError(this);
	}
	Constant *initial = getInitial(type);
	if(initial == NULL){
		throwError(this);
	}
	for(unsigned i = 0; i < varInitList.size(); i++){
		VarInit *varInit = varInitList[i];
		Value *var = new GlobalVariable(module,type,false,GlobalValue::ExternalLinkage,initial);
		astContext.addVar(varInit->varName,var);
	}
}

void VarDecl::globalCodeGen(AstContext &astContext){
	Type *type = astContext.getType(typeName);
	if(type == NULL){
		throwError(this);
	}
	for(unsigned i = 0; i < varInitList.size(); i++){
		VarInit *varInit = varInitList[i];
		if(varInit->expr != NULL){
			Value *var = astContext.getVar(varInit->varName);
			Value *v = varInit->expr->codeGen(astContext);
			v = createCast(v,type);
			if(v == NULL){
				throwError(varInit->expr);
			}
			builder.CreateStore(v,var);
		}
	}
}
