#include "ast.hpp"

void ClassDecl::declGen(AstContext &astContext){
	AstType *astType = astContext.getType(className);
	
	vector<VarDecl*> fieldDeclList;
	vector<FuncDeclStmt*> methodDeclList;
	for(unsigned i = 0; i < stmtList.size(); i++){
		Statement *stmt = stmtList[i];
		if(stmt->stmtType() == VAR_DECL){
			fieldDeclList.push_back(static_cast<VarDecl*>(stmt));
		}else{
			methodDeclList.push_back(static_cast<FuncDeclStmt*>(stmt));
		}
	}
	vector<Constant*> fieldNameList;
	for(unsigned i = 0; i < fieldDeclList.size(); i++){
		VarDecl *fieldDecl = fieldDeclList[i];
		AstType *fieldType = astContext.getType(fieldDecl->typeName);
		if(fieldType == NULL){
			throwError(fieldDecl);
		}
		for(unsigned  j = 0; j < fieldDecl->varInitList.size(); j++){
			VarInit *fieldInitDecl = fieldDecl->varInitList[j];
			if(!astType->addField(fieldInitDecl->varName,fieldType)){
				throwError(fieldInitDecl);
			}else{
				fieldNameList.push_back(builder.CreateGlobalStringPtr(fieldInitDecl->varName));
			}
		}
	}
	fieldNameList.push_back(ptr_null);
	ArrayType *arrayType = ArrayType::get(ptrType,fieldNameList.size());
	ArrayRef<Constant*> fieldNameListRef(fieldNameList); 
	GlobalVariable *fieldNameTable = new GlobalVariable(module,arrayType,true,
		GlobalValue::ExternalLinkage,ConstantArray::get(arrayType,fieldNameListRef));
	
	vector<Constant*> methodAndNameList;
	for(unsigned i = 0; i < methodDeclList.size(); i++){
		Value* methodName = builder.CreateGlobalStringPtr(methodDeclList[i]->funcName);
		methodAndNameList.push_back(methodName);
		AstFunction *method = methodDeclList[i]->declGen(astContext,astType);
		methodAndNameList.push_back(builder.CreateBitCast(method->llvmFunction,ptrType));
	}
	methodAndNameList.push_back(ptr_null);
	arrayType = ArrayType::get(ptrType,methodDeclList.size());
	ArrayRef<Constant*> methodAndNameListRef(methodAndNameList);
	GlobalVariable *methodTable = new GlobalVariable(module,arrayType,true,
		GlobalValue::ExternalLinkage,ConstantArray::get(arrayType,methodAndNameListRef));
	
	Constant *nameStr = static_cast<Constant*>(builder.CreateGlobalStringPtr(className));
	
	arrayType = ArrayType::get(ptrType,4);
	Constant *super = NULL;
	if(superName == ""){
		super = ptr_null;
	}else{
		super = new GlobalVariable(module,arrayType,true,GlobalValue::ExternalLinkage,NULL,"$"+className);
	}
	
	vector<Constant*> infos;
	infos.push_back(nameStr);
	infos.push_back(builder.CreateBitCast(super,ptrType));
	infos.push_back(builder.CreateBitCast(fieldNameTable,ptrType));
	infos.push_back(builder.CreateBitCast(methodTable,ptrType));
	ArrayRef<Constant*> infosRef(infos);
	GlobalVariable *classInfo = new GlobalVariable(module,arrayType,true,
		GlobalValue::ExternalLinkage,ConstantArray::get(arrayType,infosRef),"$"+className);
	astType->info = classInfo;
	
	
	//init decl gen
	vector<Type*> argllvmTypes;
	argllvmTypes.push_back(astType->llvmType);
	if(astType->superClass != NULL){
		argllvmTypes.push_back(astType->llvmType);
	}
	ArrayRef<Type*> argTypesRef(argllvmTypes);
	FunctionType *initFuncType = FunctionType::get(voidType,argTypesRef,false);
	Function *initFunc = Function::Create(initFuncType,Function::ExternalLinkage,"",&module);
	astType->initFunc = initFunc;
	
	
	//default constructor decl gen
	if(astContext.getFunction("new$"+className) == NULL){
		vector<AstType*> returnTypes;
		returnTypes.push_back(astType);	
		vector<AstType*> argTypes;
		FunctionType *constructorType = FunctionType::get(astType->llvmType,false);
		Function *constr = Function::Create(constructorType,Function::ExternalLinkage,"",&module);
		AstFunction *constructor = new AstFunction("new$"+className,constr,returnTypes,argTypes,astType,3);
		astContext.addFunction(constructor->name,constructor);
		defaultContructors.push_back(constructor);
	}
}

void ClassDecl::codeGen(AstContext &astContext){
	AstType *astType = astContext.getType(className);
	for(unsigned i = 0; i < stmtList.size(); i++){
		if(stmtList[i]->stmtType() == FUNC_DECL){
			FuncDeclStmt *method = static_cast<FuncDeclStmt*>(stmtList[i]);
			method->codeGen(astContext,astType);
		}
	}
	
	//initor gen
	builder.SetInsertPoint(BasicBlock::Create(context,"entry",astType->initFunc));
	unsigned i = 0;
	Function::arg_iterator ai = astType->initFunc->arg_begin();
	TypeContext typeContext = new TypeContext(new AstValue(ai,astType));
	if(astType->superClass != NULL){
		ai++;
		typeContext.superObject = new AstValue(ai,astType->superClass);
	}
	astContext.typeContext = typeContext;
	
	for(unsigned i = 0; i < stmtList.size(); i++){
		if(stmtList[i]->stmtType() == VAR_DECL){
			FuncDeclStmt *method = static_cast<FuncDeclStmt*>(stmtList[i]);
			method->codeGen(astContext,astType);
		}
	}
	builder.CreateRetVoid();
}