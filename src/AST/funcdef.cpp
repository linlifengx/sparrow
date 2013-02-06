#include "statement.h"
#include "support.h"

Function* FuncDecl::codeGen(AstContext* astContext,ClassInfo *classInfo){
	vector<ClassInfo*> returnInfos;
	vector<Type*> returnllvmTypes;
	if(style == 0){
		if(returnClasses.size() > 1 || returnClasses[0] != "void"){
			for(unsigned i = 0; i < returnClasses.size(); i++){
				ClassInfo *clazz = getClass(returnClasses[i]);
				if(clazz == NULL){
					throwError(this);
				}
				returnInfos.push_back(clazz);
				returnllvmTypes.push_back(clazz->llvmType);
			}
		}
	}else{
		for(unsigned i = 0; i < returnDecls.size(); i++){
			ClassInfo *clazz = getClass(returnDecls[i]->typeName);
			if(clazz == NULL){
				throwError(this);
			}
			returnInfos.push_back(clazz);
			returnllvmTypes.push_back(clazz->llvmType);
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
	
	vector<ClassInfo*> argInfos;
	vector<Type*> argllvmTypes;
	if(classInfo != NULL){
		argInfos.push_back(classInfo);
		argllvmTypes.push_back(classInfo->llvmType);
	}
	for(unsigned i = 0; i < argDecls.size(); i++){
		SimpleVarDecl *argDecl = argDecls[i];
		ClassInfo *clazz = getClass(argDecl->typeName);
		if(clazz == NULL){
			throwError(argDecl);
		}
		argInfos.push_back(clazz);
		argllvmTypes.push_back(clazz->llvmType);
	}
	
	FunctionType *functionType = NULL;
	if(argllvmTypes.size() == 0){
		functionType = FunctionType::get(returnType,false);
	}else{
		ArrayRef<Type*> argTypeArrayRef(argllvmTypes);
		functionType = FunctionType::get(returnType,argTypeArrayRef,false);
	}
	Function *function = Function::Create(functionType,Function::ExternalLinkage,funcName+"_sp",&module);
	FunctionInfo *functionInfo = new FunctionInfo(funcName,function,returnInfos,
		argInfos,style,ClassInfo);
	if(classInfo != NULL){
		if(!classInfo.addMethod(functionInfo)){
			throwError(this);
		}
	}else{
		if(!addFunction(functionInfo)){
			throwError(this);
		}
	}
	return function;
}

Function* FuncDef::declGen(AstContext* astContext, ClassInfo* classInfo){
	funcDecl->codeGen(astContext,classInfo);
}

void FuncDef::codeGen(AstContext* astContext, ClassInfo* classInfo){
	FunctionInfo *functionInfo = NULL;
	if(classInfo != NULL){
		functionInfo = classInfo->getMethod(funcDecl->funcName);
	}else{
		functionInfo = getFunction(funcDecl->funcName);
	}
	Function *function = functionInfo->llvmFunction;
	vector<ClassInfo*> &returnClasses = functionInfo->returnClasses;
	vector<ClassInfo*> &argClasses = functionInfo->argClasses;
	AstContext newContext(&astContext);
	builder.SetInsertPoint(BasicBlock::Create(context,"entry",function));
	unsigned i = 0;
	ClassContext *classContext = NULL;
	for(Function::arg_iterator ai = function->arg_begin();ai != function->arg_end(); ai++,i++){
		if(i == 0 && classInfo != NULL){
			classContext = new ClassContext(classInfo,ai);
		}else{
			SimpleVarDecl *argDecl = funcDecl->argDecls[i];
			ClassInfo *argClazz = argClasses[i];
			Value *alloc = builder.CreateAlloca(argClazz->llvmType);
			builder.CreateStore(ai,alloc);
			if(!newContext.addVar(argDecl->varName,AValue(alloc,argClazz))){
				throwError(argDecl);
			}
		}
	}
	if(funcDecl->style == 1){
		for(i = 0; i < returnClasses.size(); i++){
			SimpleVarDecl *retDecl = funcDecl->returnDecls[i];
			ClassInfo *retClazz = returnClasses[i];
			Value *alloc = builder.CreateAlloca(retClazz->llvmType);
			builder.CreateStore(retClazz->getInitial(),alloc);
			if(!newContext.addReturnVar(retDecl->varName,AValue(alloc,retClazz))){
				throwError(retDecl);
			}
		}
	}

	newContext.currentFunc = functionInfo;
	newContext.classContext = classContext;
	stmtBlock->codeGen(astContext);
	
	if(functionInfo->returnNum == 0){
		builder.CreateRetVoid();
	}else if(funcDecl->style == 0){
		if(functionInfo->returnNum == 1){
			Value *retVal = returnClasses[0]->getInitial();
			builder.CreateRet(retVal);
		}else{
			Value *alloc = builder.CreateAlloca(FunctionInfo->returnType);
			for(i = 0; i < returnClasses.size(); i++){
				Value *element = builder.CreateStructGEP(alloc,i);	
				builder.CreateStore(returnClasses[i]->getInitial(),element);
			}
			builder.CreateRet(builder.CreateLoad(alloc));
		}
	}else{
		if(functionInfo->returnNum == 1){
			builder.CreateRet(builder.CreateLoad());
		}else{
			Value *alloc = builder.CreateAlloca(FunctionInfo->returnType);
			for(i = 0; i < newContext.returnVars.size(); i++){
				Value *element = builder.CreateStructGEP(alloc,i);	
				Value *elemVal = builder.CreateLoad(newContext.returnVars[i])
				builder.CreateStore(elemVal,element);
			}
			builder.CreateRet(builder.CreateLoad(alloc));
		}
	}		
}