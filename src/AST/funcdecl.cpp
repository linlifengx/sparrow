#include "ast.hpp"
//#include "parser.hpp"

AstFunction* FuncDecl::declGen(AstContext &astContext,AstType *astType){
	vector<AstType*> returnTypes;
	vector<Type*> returnllvmTypes;
	
	if(retTypeNameList.size() > 1 || *retTypeNameList[0] != "void"){
		for(unsigned i = 0; i < retTypeNameList.size(); i++){
			string &typeName = *retTypeNameList[i];
			AstType *type = astContext.getType(typeName);
			if(type == NULL){
				throwError(this);
			}
			returnTypes.push_back(type);
			returnllvmTypes.push_back(type->llvmType);
		}
	}
	
	Type *returnType = NULL;
	if(returnllvmTypes.size() == 0){
		returnType = builder.getVoidTy();
	}else if(returnllvmTypes.size() == 1){
		returnType = returnllvmTypes[0];
	}else{
		ArrayRef<Type*> typesArray(returnllvmTypes);
		returnType = StructType::create(context,typesArray);
	}
	
	vector<AstType*> argTypes;
	vector<Type*> argllvmTypes;
	if(astType != NULL){
		argTypes.push_back(astType);
		argllvmTypes.push_back(astType->llvmType);
	}
	for(unsigned i = 0; i < argDeclList.size(); i++){
		SimpleVarDecl *argDecl = argDeclList[i];
		AstType *type = astContext.getType(argDecl->typeName);
		if(type == NULL){
			throwError(argDecl);
		}
		argTypes.push_back(type);
		argllvmTypes.push_back(type->llvmType);
	}
	
	FunctionType *functionType = NULL;
	if(argllvmTypes.size() == 0){
		functionType = FunctionType::get(returnType,false);
	}else{
		ArrayRef<Type*> argTypeArrayRef(argllvmTypes);
		functionType = FunctionType::get(returnType,argTypeArrayRef,false);
	}
	Function *function = Function::Create(functionType,Function::ExternalLinkage,funcName+"_sp",&module);
	AstFunction *astFunction = new AstFunction(funcName,function,returnTypes,argTypes,astType);
	if(astType != NULL ){
		if(!astType->addMethod(funcName,astFunction)){
			throwError(this);
		}
	}else if(!astContext.addFunction(funcName,astFunction)){
		throwError(this);
	}
	return astFunction;
}

void FuncDecl::codeGen(AstContext &astContext,AstType *astType){
	AstFunction *astFunction = NULL;
	if(astType != NULL){
		astFunction = astType->getMethod(funcName);
	}else{
		astFunction = astContext.getFunction(funcName);
	}
	Function *function = astFunction->llvmFunction;
	vector<AstType*> &argTypes = astFunction->argTypes;
	AstContext newContext(&astContext);
	builder.SetInsertPoint(BasicBlock::Create(context,"entry",function));
	unsigned i = 0;
	TypeContext *typeContext = NULL;
	for(Function::arg_iterator ai = function->arg_begin();ai != function->arg_end(); ai++,i++){
		SimpleVarDecl *argDecl = argDeclList[i];
		Value *alloc = builder.CreateAlloca(argTypes[i]->llvmType);
		builder.CreateStore(ai,alloc);
		if(i == 0 && astFunction->dominateType != NULL){
			typeContext = new TypeContext(new AstValue(alloc,argTypes[i]));
		}else if(!newContext.addVar(argDecl->varName,new AstValue(alloc,argTypes[i]))){
			throwError(argDecl);
		}
	}
	
	newContext.currentFunc = astFunction;
	newContext.typeContext = typeContext;
	for(i = 0; i < stmtList.size(); i++){
		stmtList[i]->codeGen(newContext);
	}
	if(astFunction->isReturnVoid){
		builder.CreateRetVoid();
	}else if(astFunction->isReturnSingle){
		Value *retVal = astFunction->returnTypes[0].getInitial();
		builder.CreateRet(retVal);
	}else{
		Value *alloc = builder.CreateAlloca(astFunction->returnType);
		vector<AstType*> returnTypes = astFunction->returnTypes;
		for(i = 0; i < returnTypes.size(); i++){
			Value *element = builder.CreateStructGEP(alloc,i);	
			builder.CreateStore(returnTypes[i]->getInitial(),element);
		}
		builder.CreateRet(builder.CreateLoad(alloc));
	}
}

AstFunction* FuncDecl2::declGen(AstContext &astContext,AstType *astType){
	vector<AstType*> returnTypes;
	vector<Type*> returnllvmTypes;
	for(unsigned i = 0; i < retDeclList.size(); i++){
		SimpleVarDecl *retDecl = retDeclList[i];
		AstType *type = astContext.getType(retDecl->typeName);
		if(type == NULL){
			throwError(retDecl);
		}
		returnTypes.push_back(type);
		returnllvmTypes.push_back(type->llvmType);
	}
	
	Type *returnType = NULL;
	if(returnllvmTypes.size() == 0){
		returnType = builder.getVoidTy();
	}else if(returnllvmTypes.size() == 1){
		returnType =  returnllvmTypes[0];
	}else{
		ArrayRef<Type*> typesArray(returnllvmTypes);
		returnType = StructType::create(context,typesArray);
	}
	
	vector<Type*> argTypes;
	vector<Type*> argllvmTypes;
	if(astType != NULL){
		argTypes.push_back(astType);
		argllvmTypes.push_back(astType->llvmType);
	}
	for(unsigned i = 0; i < argDeclList.size(); i++){
		SimpleVarDecl *argDecl = argDeclList[i];
		AstType *type = astContext.getType(argDecl->typeName);
		if(type == NULL){
			throwError(argDecl);
		}
		argTypes.push_back(type);
		argllvmTypes.push_back(type->llvmType);
	}
	
	FunctionType *functionType = NULL;
	if(argllvmTypes.size() == 0){
		functionType = FunctionType::get(returnType,false);
	}else{
		ArrayRef<Type*> argTypeArrayRef(argllvmTypes);
		functionType = FunctionType::get(returnType,argTypeArrayRef,false);
	}
	Function *function = Function::Create(functionType,Function::ExternalLinkage,funcName+"_sp",&module);
	AstFunction *astFunction = new AstFunction(funcName,function,returnTypes,argTypes,astType,2);
	if(astType != NULL ){
		if(!astType->addMethod(funcName,astFunction)){
			throwError(this);
		}
	}else if(!astContext.addFunction(funcName,astFunction)){
		throwError(this);
	}
	return astFunction;
}

void FuncDecl2::codeGen(AstContext &astContext,AstType *astType){
	AstFunction *astFunction = NULL;
	if(astType != NULL){
		astFunction = astType->getMethod(funcName);
	}else{
		astFunction = astContext.getFunction(funcName);
	}
	Function* function = astFunction->llvmFunction;
	vector<AstType*> &argTypes = astFunction->argTypes;
	vector<AstType*> &retTypes = astFunction->returnTypes;
	AstContext newContext(&astContext);
	builder.SetInsertPoint(BasicBlock::Create(context,"entry",function));
	unsigned i = 0;
	TypeContext typeContext = NULL;
	for(Function::arg_iterator ai = function->arg_begin();ai != function->arg_end(); ai++,i++){
		if(i == 0 && astFunction->dominateType != NULL){
			typeContext = new TypeContext(new AstValue(ai,argTypes[i]));
			continue;
		}
		SimpleVarDecl *argDecl = argDeclList[i];
		Value *alloc = builder.CreateAlloca(argTypes[i]->llvmType);
		builder.CreateStore(ai,alloc);
		if(!newContext.addVar(argDecl->varName,new AstValue(alloc,argTypes[i]))){
			throwError(argDecl);
		}
	}
	vector<AstValue*> retVarList;
	for(i = 0; i < retDeclList.size(); i++){
		SimpleVarDecl *retDecl = retDeclList[i];
		Value *alloc = builder.CreateAlloca(retTypes[i]->llvmType);
		builder.CreateStore(retTypes[i]->getInitial(),alloc);
		AstValue *retVar = new AstValue(alloc,retTypes[i]);
		if(!newContext.addVar(retDecl->varName,retVar)){
			throwError(retDecl);
		}
		retVarList.push_back(retVar);
	}
	astFunction->returnVars = retVarList;
	newContext.currentFunc = astFunction;
	newContext.typeContext = typeContext;
	for(i = 0; i < stmts.size(); i++){
		stmts[i]->codeGen(newContext);
	}
	if(astFunction->isReturnVoid){
		builder.CreateRetVoid();
	}else if(astFunction->isReturnSingle){
		builder.CreateRet(builder.CreateLoad(retVarList[0]->llvmValue));
	}else{
		Value *alloc = builder.CreateAlloca(astFunction->returnType);
		for(i = 0; i < retVarList.size(); i++){
			Value *element = builder.CreateStructGEP(alloc,i);
			builder.CreateStore(builder.CreateLoad(retVarList[i]->llvmValue),element);
		}
		builder.CreateRet(builder.CreateLoad(alloc));
	}
}

AstFunction* Constructor::declGen(AstContext &astContext,AstType *astType){
	if(funcName != astType->name){
		errorMsg = "method '"+funcName + "' require return decl";
		throwError(this);
	}
	vector<AstType*> returnTypes;
	vector<Type*> returnllvmTypes;
	returnTypes.push_back(astType);
	returnllvmTypes.push_back(astType->llvmType);
	
	Type *returnType = astType->llvmType;
	
	vector<AstType*> argTypes;
	vector<Type*> argllvmTypes;
	for(unsigned i = 0; i < argDeclList.size(); i++){
		SimpleVarDecl *argDecl = argDeclList[i];
		AstType *type = astContext.getType(argDecl->typeName);
		if(type == NULL){
			throwError(argDecl);
		}
		argTypes.push_back(type);
		argllvmTypes.push_back(type->llvmType);
	}
	
	ArrayRef<Type*> argTypeArrayRef(argllvmTypes);
	FunctionType *functionType = FunctionType::get(returnType,argTypeArrayRef,false);
	Function *function = Function::Create(functionType,Function::ExternalLinkage,"new$"+funcName,&module);
	AstFunction *astFunction = new AstFunction("new$"+funcName,function,returnTypes,argTypes,astType,3);
	if(!astContext.addFunction("new$"+funcName,astFunction)){
		throwError(this);
	}
	return astFunction;
}

void Constructor::codeGen(AstContext &astContext,AstType *astType){
	AstFunction *astFunction = astContext.getFunction("new$"+funcName);
	Function* function = astFunction->llvmFunction;
	vector<AstType*> &argTypes = astFunction->argTypes;
	AstContext newContext(&astContext);
	builder.SetInsertPoint(BasicBlock::Create(context,"entry",function));
	unsigned i = 0;
	for(Function::arg_iterator ai = function->arg_begin();ai != function->arg_end(); ai++,i++){
		SimpleVarDecl *argDecl = argDeclList[i];
		Value *alloc = builder.CreateAlloca(argTypes[i]->llvmType);
		builder.CreateStore(ai,alloc);
		if(!newContext.addVar(argDecl->varName,new AstValue(alloc,argTypes[i]))){
			throwError(argDecl);
		}
	}

	newContext.currentFunc = astFunction;

	if(astType->superClass != NULL && (stmts.size() == 0 || stmts[0]->stmtType() != SUPER_CALL)){
		AstFunction *superConstructor = astContext.getFunction("new$"+astType->superClass.name);
		if(superConstructor->argTypes.size() > 0){
			errorMsg = "constructor for '"+astType->superClass->name+"' require some args";
			throwError(this);
		}else{
			Value *superObject = builder.CreateCall(superConstructor->llvmFunction);
			Value *thisObject = builder.CreateCall(sysObjectAlloca,astType->info);
			Value *superElementPtr = builder.CreateGEP(thisObject,1);
			superElementPtr = builder.CreateBitCast(superElementPtr,ptrptrType);
			builder.CreateStore(superObject,superElementPtr);
			builder.CreateCall2(astType->initFunc,thisObject,superObject);
			newContext.typeContext = new TypeContext(new AstValue(thisObject,astType));
			newContext.typeContext->superObject = new AstValue(superObject,astType->superClass);
		}
	}else if(astType->superClass == NULL){
		Value *thisObject = builder.CreateCall(sysObjectAlloca,astType->info);
		builder.CreateCall(astType->initFunc,thisObject);
		newContext.typeContext = new TypeContext(new AstValue(thisObject,astType));
	}
	
	for(i = 0; i < stmts.size(); i++){
		stmts[i]->codeGen(newContext);
	}
	builder.CreateRet(newContext.typeContext->thisObject->llvmValue);
}