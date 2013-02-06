#include "statement.h"
#include "expression.h"

FunctionInfo* FuncDecl::codeGen(ClassInfo *classInfo) {
	vector<ClassInfo*> returnInfos;
	vector<Type*> returnllvmTypes;
	if (style == 0) {
		for (unsigned i = 0; i < returnClasses.size(); i++) {
			ClassInfo *clazz = getClass(returnClasses[i]);
			if (clazz == NULL) {
				throwError(this);
			}
			returnInfos.push_back(clazz);
			returnllvmTypes.push_back(clazz->llvmType);
		}
	} else {
		for (unsigned i = 0; i < returnDecls.size(); i++) {
			ClassInfo *clazz = getClass(returnDecls[i]->typeName);
			if (clazz == NULL) {
				throwError(this);
			}
			returnInfos.push_back(clazz);
			returnllvmTypes.push_back(clazz->llvmType);
		}

	}

	Type *returnType = NULL;
	if (returnllvmTypes.size() == 0) {
		returnType = builder.getVoidTy();
	} else if (returnllvmTypes.size() == 1) {
		returnType = returnllvmTypes[0];
	} else {
		ArrayRef<Type*> typesArray(returnllvmTypes);
		returnType = StructType::create(context, typesArray);
	}

	vector<ClassInfo*> argInfos;
	vector<Type*> argllvmTypes;
	if (classInfo != NULL) {
		argInfos.push_back(classInfo);
		argllvmTypes.push_back(classInfo->llvmType);
	}
	for (unsigned i = 0; i < argDecls.size(); i++) {
		SimpleVarDecl *argDecl = argDecls[i];
		ClassInfo *clazz = getClass(argDecl->typeName);
		if (clazz == NULL) {
			throwError(argDecl);
		}
		argInfos.push_back(clazz);
		argllvmTypes.push_back(clazz->llvmType);
	}

	FunctionType *functionType = NULL;
	if (argllvmTypes.size() == 0) {
		functionType = FunctionType::get(returnType, false);
	} else {
		ArrayRef<Type*> argTypeArrayRef(argllvmTypes);
		functionType = FunctionType::get(returnType, argTypeArrayRef, false);
	}
	Function *function = Function::Create(functionType,
			Function::ExternalLinkage, funcName + "_sp", &module);
	FunctionInfo *functionInfo = new FunctionInfo(funcName, function,
			returnInfos, argInfos, style, classInfo);
	if (classInfo != NULL) {
		if (!classInfo->addMethod(functionInfo)) {
			throwError(this);
		}
	} else {
		if (!addFunction(functionInfo)) {
			throwError(this);
		}
	}
	this->functionInfo = functionInfo;
	return functionInfo;
}

Function* FuncDef::declGen(ClassInfo* classInfo) {
	functionInfo = funcDecl->codeGen(classInfo);
	return functionInfo->llvmFunction;
}

void FuncDef::codeGen(AstContext &astContext) {
	ClassInfo *classInfo = functionInfo->dominateClass;
	unsigned isMethod = classInfo == NULL ? 0 : 1;
	Function *function = functionInfo->llvmFunction;
	vector<ClassInfo*> &returnClasses = functionInfo->returnClasses;
	vector<ClassInfo*> &argClasses = functionInfo->argClasses;
	AstContext newContext(&astContext);

	BasicBlock *allocBB = BasicBlock::Create(context, "alloc", function);
	BasicBlock *entryBB = BasicBlock::Create(context, "entry", function);
	newContext.allocBB = allocBB;
	builder.SetInsertPoint(allocBB);
	unsigned i = 0;
	ClassContext *classContext = NULL;
	for (Function::arg_iterator ai = function->arg_begin();
			ai != function->arg_end(); ai++, i++) {
		if (i == 0 && isMethod) {
			classContext = new ClassContext(classInfo, allocBB, ai);
		} else {
			SimpleVarDecl *argDecl = funcDecl->argDecls[i - isMethod];
			ClassInfo *argClazz = argClasses[i];
			Value *alloc = builder.CreateAlloca(argClazz->llvmType);
			builder.CreateStore(ai, alloc);
			if (!newContext.addVar(argDecl->varName, AValue(alloc, argClazz))) {
				throwError(argDecl);
			}
		}
	}

	if (functionInfo->returnNum > 0) {
		Value *retAlloc = builder.CreateAlloca(functionInfo->returnType);
		newContext.returnAlloc = retAlloc;
		for (i = 0; i < functionInfo->returnNum; i++) {
			ClassInfo *retClazz = returnClasses[i];
			Value *retElement = NULL;
			if (functionInfo->returnNum == 1) {
				retElement = retAlloc;
			} else {
				retElement = builder.CreateStructGEP(retAlloc, i);
			}
			builder.CreateStore(retClazz->getInitial(), retElement);
			newContext.returnVars.push_back(retElement);
			if (funcDecl->style == 1) {
				SimpleVarDecl *retDecl = funcDecl->returnDecls[i];
				if (!newContext.addVar(retDecl->varName,
						AValue(retElement, retClazz))) {
					throwError(retDecl);
				}
			}
		}
	}

	newContext.currentFunc = functionInfo;
	newContext.classContext = classContext;
	builder.SetInsertPoint(entryBB);
	stmtBlock->codeGen(newContext);

	if (functionInfo->returnNum == 0) {
		builder.CreateRetVoid();
	} else {
		builder.CreateRet(builder.CreateLoad(newContext.returnAlloc));
	}

	builder.SetInsertPoint(allocBB);
	builder.CreateBr(entryBB);
}
