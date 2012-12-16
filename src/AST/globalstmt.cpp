#include "ast.hpp"
#include "parser.hpp"

void FuncDecl::globalDeclGen(AstContext &astContext){
	vector<Type*> returnTypes;
	if(retTypeNameList.size() > 1 || *retTypeNameList[0] != "void"){
		for(unsigned i = 0; i < retTypeNameList.size(); i++){
			string &typeName = *retTypeNameList[i];
			Type *type = NULL;
			try{
				type = astContext.getType(typeName);
			}catch(string msg){
				throwError(this,msg);
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
		Type *type = NULL;
		try{
			type = astContext.getType(argDecl->typeName);
		}catch(string msg){
			throwError(argDecl,msg);
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
	Function *function = Function::Create(functionType,Function::ExternalLinkage,funcName,&module);
	MyFunction *myFunction = new MyFunction(funcName,function,returnTypes,argTypes);
	try{
		astContext.addFunction(funcName,myFunction);
	}catch(string msg){
		throwError(this,msg);
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
		try{
			newContext.addVar(argDecl->varName,alloc);
		}catch(string msg){
			throwError(argDecl,msg);
		}
	}
	newContext.currentFunc = myFunction;
	for(i = 0; i < stmtList.size(); i++){
		stmtList[i]->codeGen(newContext);
	}
	if(myFunction->isReturnVoid){
		builder.CreateRetVoid();
	}else if(myFunction->isReturnSingle){
		try{
			Value *retVal = getInitial(myFunction->returnType);
			builder.CreateRet(retVal);
		}catch(string msg){
			throwError(this,msg);
		}
	}else{
		Value *alloc = builder.CreateAlloca(myFunction->returnType);
		try{
			for(i = 0; i < argTypes.size(); i++){
				Value *element = builder.CreateStructGEP(alloc,i);
				Value *elemVal = getInitial(argTypes[i]);
				builder.CreateStore(elemVal,element);
			}
		}catch(string msg){
			throwError(this,msg);
		}
		builder.CreateRet(builder.CreateLoad(alloc));
	}
}

void FuncDecl2::globalDeclGen(AstContext &astContext){
	vector<Type*> returnTypes;
	for(unsigned i = 0; i < retDeclList.size(); i++){
		SimpleVarDecl *retDecl = retDeclList[i];
		Type *type = NULL;
		try{
			type = astContext.getType(retDecl->typeName);
		}catch(string msg){
			throwError(retDecl,msg);
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
		Type *type = NULL;
		try{
			type = astContext.getType(argDecl->typeName);
		}catch(string msg){
			throwError(argDecl,msg);
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
	Function *function = Function::Create(functionType,Function::ExternalLinkage,funcName,&module);
	MyFunction *myFunction = new MyFunction(funcName,function,returnTypes,argTypes,2);
	try{
		astContext.addFunction(funcName,myFunction);
	}catch(string msg){
		throwError(this,msg);
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
		try{
			newContext.addVar(argDecl->varName,alloc);
		}catch(string msg){
			throwError(argDecl,msg);
		}
	}
	vector<Value*> retVarList;
	for(i = 0; i < retDeclList.size(); i++){
		SimpleVarDecl *retDecl = retDeclList[i];
		Value *alloc = builder.CreateAlloca(retTypes[i]);
		try{
			builder.CreateStore(getInitial(retTypes[i]),alloc);
			newContext.addVar(retDecl->varName,alloc);
		}catch(string msg){
			throwError(retDecl,msg);
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
	Type *type = NULL;
	try{
		type = astContext.getType(typeName);
	}catch(string msg){
		throwError(this,msg);
	}
	Constant *initial = NULL;
	try{
		initial =  getInitial(type);}
	catch(string msg){
		throwError(this,msg);
	}
	for(unsigned i = 0; i < varInitList.size(); i++){
		VarInit *varInit = varInitList[i];
		Value *var = new GlobalVariable(module,type,false,GlobalValue::ExternalLinkage,initial);
		astContext.addVar(varInit->varName,var);
	}
}

void VarDecl::globalCodeGen(AstContext &astContext){
	Type *type = NULL;
	try{
		type = astContext.getType(typeName);
	}catch(string msg){
		throwError(this,msg);
	}
	for(unsigned i = 0; i < varInitList.size(); i++){
		VarInit *varInit = varInitList[i];
		if(varInit->expr != NULL){
			Value *var = astContext.getVar(varInit->varName);
			Value *v = varInit->expr->codeGen(astContext);
			try{
				v = createCast(v,type);
			}catch(string msg){
				throwError(varInit->expr,msg);
			}
			builder.CreateStore(v,var);
		}
	}
}
