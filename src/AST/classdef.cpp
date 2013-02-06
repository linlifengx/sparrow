#include <llvm/Constant.h>

#include "statement.h"
#include "support.h"

using namespace llvm;

void ClassDef::declGen(AstContext* astContext){
	ClassInfo *classInfo = getClass(*className);
	vector<Constant*> fieldNameStrs;
	for(unsigned i = 0; i < body->fieldDefs.size(); i++){
		VarDef *fieldDef = body->fieldDefs[i];
		ClassInfo *fieldClass = getClass(fieldDef->typeName);
		if(fieldClass == NULL){
			throwError(fieldDef);
		}
		for(unsigned  j = 0; j < fieldDef->varInitList.size(); j++){
			VarInit *fieldInitDecl = fieldDef->varInitList[j];
			if(!classInfo->addField(fieldInitDecl->varName,fieldClass)){
				throwError(fieldInitDecl);
			}else{
				fieldNameStrs.push_back(builder.CreateGlobalStringPtr(*fieldInitDecl->varName));
			}
		}
	}
	fieldNameStrs.push_back(ptr_null);
	ArrayType *arrayType = ArrayType::get(ptrType,fieldNameStrs.size());
	ArrayRef<Constant*> fieldNameStrsRef(fieldNameStrs); 
	GlobalVariable *fieldNameTable = new GlobalVariable(module,arrayType,true,
		GlobalValue::ExternalLinkage,ConstantArray::get(arrayType,fieldNameStrsRef));
	
	vector<Constant*> methodNameStrs;
	vector<Constant*> methodPtrs;
	for(unsigned i = 0; i < body->methodDefs.size(); i++){
		Value* methodName = builder.CreateGlobalStringPtr(body->methodDefs[i]->funcDecl->funcName);
		methodNameStrs.push_back(methodName);
		Function *method = body->methodDefs[i]->declGen(astContext,classInfo);
		methodPtrs.push_back(builder.CreateBitCast(method,ptrType));
	}
	methodNameStrs.push_back(ptr_null);
	methodPtrs.push_back(ptr_null);
	arrayType = ArrayType::get(ptrType,methodNameStrs.size());
	ArrayRef<Constant*> methodNameStrsRef(methodNameStrs);
	GlobalVariable *methodNameTable = new GlobalVariable(module,arrayType,true,
		GlobalValue::ExternalLinkage,ConstantArray::get(arrayType,methodNameStrsRef));
	
	arrayType = ArrayType::get(ptrType,methodPtrs.size());
	ArrayRef<Constant*> methodPtrsRef(methodPtrs);
	GlobalVariable *methodTable = new GlobalVariable(module,arrayType,true,
		GlobalValue::ExternalLinkage,ConstantArray::get(arrayType,methodPtrsRef));
	
	Constant *nameStr = static_cast<Constant*>(builder.CreateGlobalStringPtr(classInfo->name));
	
	arrayType = ArrayType::get(ptrType,5);
	Constant *super = NULL;
	if(superName == NULL){
		super = ptr_null;
	}else{
		super = new GlobalVariable(module,arrayType,true,GlobalValue::ExternalLinkage,NULL,"$"+*superName);
	}
	
	vector<Constant*> infos;
	infos.push_back(nameStr);
	infos.push_back(builder.CreateBitCast(super,ptrType));
	infos.push_back(builder.CreateBitCast(fieldNameTable,ptrType));
	infos.push_back(builder.CreateBitCast(methodNameTable,ptrType));
	infos.push_back(builder.CreateBitCast(methodTable,ptrType));
	ArrayRef<Constant*> infosRef(infos);
	GlobalVariable *classV = new GlobalVariable(module,arrayType,true,
		GlobalValue::ExternalLinkage,ConstantArray::get(arrayType,infosRef),"$"+*className);
	classInfo->info = classV;
	
	//init decl gen
	vector<Type*> argllvmTypes;
	argllvmTypes.push_back(classInfo->llvmType);
	if(classInfo->superClassInfo != NULL){
		argllvmTypes.push_back(classInfo->superClassInfo->llvmType);
	}
	ArrayRef<Type*> argTypesRef(argllvmTypes);
	FunctionType *initFuncType = FunctionType::get(voidType,argTypesRef,false);
	Function *initFunc = Function::Create(initFuncType,Function::ExternalLinkage,"",&module);
	classInfo->initor = initFunc;
	
	//constructor decl gen
	if(body->constructors.size() > 0){
		for(unsigned i = 0; i < body->constructors.size(); i++){
			body->constructors[i].declGen(astContext,classInfo);
		}
	}else{
		vector<ClassInfo*> returnClasses;
		returnClasses.push_back(classInfo);	
		vector<ClassInfo*> argClasses;
		FunctionType *constructorType = FunctionType::get(classInfo->llvmType,false);
		Function *constructor = Function::Create(constructorType,Function::ExternalLinkage,"",&module);
		FunctionInfo *constructorInfo = new FunctionInfo("new$"+*className,
			constructor,returnClasses,argClasses,classInfo,3);
		classInfo->constructor = constructorInfo;
		defaultContructors.push_back(constructorInfo);
	}
}

void ClassDef::codeGen(AstContext* astContext){
	ClassInfo *classInfo = getClass(*className);
	for(unsigned i = 0; i < body->methodDefs.size(); i++){
		body->methodDefs[i]->codeGen(astContext,classInfo);
	}
	
	for(unsigned i = 0; i < body->constructors.size(); i++){
		body->constructors[i]->codeGen(astContext,classInfo);
	}
	
	//initor gen
	builder.SetInsertPoint(BasicBlock::Create(context,"entry",classInfo->initor));
	unsigned i = 0;
	Function::arg_iterator ai = classInfo->initor->arg_begin();
	ClassContext classContext = new ClassContext(classInfo,ai);
	if(classInfo->superClassInfo != NULL){
		ai++;
		classContext.superObject = ai;
	}
	astContext->classContext = classContext;
	
	for(unsigned i = 0; i < body->fieldDefs.size(); i++){
		body->fieldDefs[i]->fieldGen(astContext);
	}
	builder.CreateRetVoid();
}