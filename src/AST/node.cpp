#include "node.h"
#include "support.h"

extern void classDeclGen(AstContext *astContext, ClassInfo *clazz);

void Program::codeGen(AstContext *astContext){
	// add clazz
	for(unsigned i = 0; i < classDefs.size(); i++){
		ClassInfo *clazz = new ClassInfo(*classDefs[i]->className,classDefs[i]);
		if(!addClass(clazz)){
			throwError(classDefs[i]);
		}
	}
	for(unsigned i = 0; i < classDefs.size(); i++){
		ClassDef *classDef = classDefs[i];
		ClassInfo *clazz = getClass(*classDef->className);
		if(classDef->superName != NULL){
			ClassInfo *superClass = getClass(*classDef->superName);
			if(superClass == NULL){
				throwError(classDef);
			}
			clazz->superClassInfo = superClass;
		}
	}
	
	// class decl gen
	for(unsigned i = 0; i < classDefs.size(); i++){
		ClassInfo *clazz = getClass(*classDefs[i]->className);
		classDeclGen(astContext,clazz);
	}
	
	// func decl gen
	for(unsigned i = 0; i < funcDefs.size(); i++){
		funcDefs[i]->declGen(astContext);
	}
	
	// create main func and global var gen
	FunctionType *mainFuncType = FunctionType::get(builder.getVoidTy(),false);
	mainFunc = Function::Create(mainFuncType,Function::ExternalLinkage,"main",&module);
	builder.SetInsertPoint(BasicBlock::Create(context,"entry",mainFunc));
	for(unsigned  i = 0; i < varDefs.size(); i++){
		varDefs[i]->globalGen(astContext);
	}
	FunctionInfo *mainF = getFunction("main");
	if(mainF == NULL){
		cout<<errorMsg<<endl;
	}else{
		builder.CreateCall(mainF->llvmFunction);
	}
	builder.CreateRetVoid();
	
	// class gen
	for(unsigned i = 0; i < classDefs.size(); i++){
		classDefs[i]->codeGen(astContext);
	}
	
	// function gen
	for(unsigned i = 0; i < funcDefs.size(); i++){
		funcDefs[i]->codeGen(astContext);
	}
	
	// class default constructor gen
	

}

void classDeclGen(AstContext* astContext, ClassInfo* clazz){
	if(clazz->status == 2){
		return;
	}
	clazz->status = 1;
	ClassInfo *superClass = clazz->superClassInfo;
	if(superClass != NULL){
		if(superClass->status == 1){
			errorMsg = "cyclic inheritance involving class '"+superClass->name+"'";
			throwError(superClass->classDef);
		}else if(superClass->status == 0){
			classDeclGen(astContext,superClass);
		}
	}
	clazz->classDef->declGen(astContext);
	clazz->status = 2;
}