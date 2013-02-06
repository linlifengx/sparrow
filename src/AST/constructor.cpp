#include "statement.h"
#include "support.h"

void Constructor::declGen(AstContext *astContext, ClassInfo* classInfo){
	if(name != ClassInfo.name){
		errorMsg = "method '"+name + "' require return decl";
		throwError(this);
	}
	if(classInfo->constructor != NULL){
		errorMsg = "redefine constructor for class '"+name+"'";
		throwError(this);
	}
	
	vector<ClassInfo*> returnClasses;
	vector<Type*> returnllvmTypes;
	returnClasses.push_back(classInfo);
	returnllvmTypes.push_back(classInfo->llvmType);
	Type *returnType = classInfo->llvmType;
	
	vector<ClassInfo*> argClasses;
	vector<Type*> argllvmTypes;
	for(unsigned i = 0; i < argDeclList.size(); i++){
		SimpleVarDecl *argDecl = argDeclList[i];
		ClassInfo *argClass = getClass(argDecl->typeName);
		if(argClass == NULL){
			throwError(argDecl);
		}
		argClasses.push_back(argClass);
		argllvmTypes.push_back(argClass->llvmType);
	}
	
	ArrayRef<Type*> argTypeArrayRef(argllvmTypes);
	FunctionType *functionType = FunctionType::get(returnType,argTypeArrayRef,false);
	Function *function = Function::Create(functionType,Function::ExternalLinkage,"new$"+name,&module);
	FunctionInfo *functionInfo = new FunctionInfo("new$"+name,function,returnClasses,1,classInfo);
	classInfo->constructor = functionInfo;
}

void Constructor::codeGen(AstContext* astContext,ClassInfo *classInfo){
	FunctionInfo *constructor = classInfo->constructor;
	Function *function = constructor->llvmFunction;
	vector<ClassInfo*> &argClasses = constructor->argClasses;
	AstContext newContext(&astContext);
	builder.SetInsertPoint(BasicBlock::Create(context,"entry",function));
	unsigned i = 0;
	for(Function::arg_iterator ai = function->arg_begin();ai != function->arg_end(); ai++,i++){
		SimpleVarDecl *argDecl = argDeclList[i];
		Value *alloc = builder.CreateAlloca(argClasses[i]->llvmType);
		builder.CreateStore(ai,alloc);
		if(!newContext.addVar(argDecl->varName,AValue(alloc,argClasses[i]))){
			throwError(argDecl);
		}
	}
	
	
}