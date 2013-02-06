#include "statement.h"
#include "expression.h"
#include "support.h"

void Constructor::declGen(ClassInfo &classInfo) {
	if (name != classInfo.name) {
		errorMsg = "method '" + name + "' require return decl";
		throwError(this);
	}
	if (classInfo.constructor != NULL) {
		errorMsg = "redefine constructor for class '" + name + "'";
		throwError(this);
	}

	vector<ClassInfo*> returnClasses;
	vector<Type*> returnllvmTypes;
	returnClasses.push_back(&classInfo);
	returnllvmTypes.push_back(classInfo.llvmType);
	Type *returnType = classInfo.llvmType;

	vector<ClassInfo*> argClasses;
	vector<Type*> argllvmTypes;
	for (unsigned i = 0; i < argDeclList.size(); i++) {
		SimpleVarDecl *argDecl = argDeclList[i];
		ClassInfo *argClass = getClass(argDecl->typeName);
		if (argClass == NULL) {
			throwError(argDecl);
		}
		argClasses.push_back(argClass);
		argllvmTypes.push_back(argClass->llvmType);
	}

	ArrayRef<Type*> argTypeArrayRef(argllvmTypes);
	FunctionType *functionType = FunctionType::get(returnType, argTypeArrayRef,
			false);
	Function *function = Function::Create(functionType,
			Function::ExternalLinkage, "new$" + name, &module);
	FunctionInfo *functionInfo = new FunctionInfo(name, function, returnClasses,
			argClasses, 2, &classInfo);
	this->functionInfo = functionInfo;
	classInfo.constructor = functionInfo;
}

void Constructor::codeGen(AstContext &astContext) {
	ClassInfo *classInfo = functionInfo->dominateClass;
	Function *function = functionInfo->llvmFunction;
	vector<ClassInfo*> &argClasses = functionInfo->argClasses;
	AstContext newContext(&astContext);
	BasicBlock *allocBB = BasicBlock::Create(context, "alloc", function);
	BasicBlock *entryBB = BasicBlock::Create(context, "entry", function);
	newContext.allocBB = allocBB;
	builder.SetInsertPoint(allocBB);
	unsigned i = 0;
	for (Function::arg_iterator ai = function->arg_begin();
			ai != function->arg_end(); ai++, i++) {
		SimpleVarDecl *argDecl = argDeclList[i];
		Value *alloc = builder.CreateAlloca(argClasses[i]->llvmType);
		builder.CreateStore(ai, alloc);
		if (!newContext.addVar(argDecl->varName,
				AValue(alloc, argClasses[i]))) {
			throwError(argDecl);
		}
	}

	newContext.currentFunc = functionInfo;
	newContext.classContext = new ClassContext(classInfo, allocBB);
	Value *thisObject = NULL;
	if (classInfo->superClassInfo != NULL
			&& (stmtBlock->statements.size() == 0
					|| stmtBlock->statements[0]->type != SUPER_INIT)) {
		FunctionInfo *superConstructor = classInfo->superClassInfo->constructor;
		if (superConstructor->argClasses.size() > 0) {
			errorMsg = "constructor for '" + classInfo->superClassInfo->name
					+ "' require some args";
			throwError(this);
		}
		Value *superObject = builder.CreateCall(superConstructor->llvmFunction);
		thisObject = builder.CreateCall(sysObjectAlloca, classInfo->info);
		Value *superElementPtr = builder.CreateGEP(thisObject,
				builder.getInt32(8));
		superElementPtr = builder.CreateBitCast(superElementPtr, ptrptrType);
		builder.CreateStore(superObject, superElementPtr);
		builder.CreateCall2(classInfo->initor, thisObject, superObject);
		newContext.classContext->thisObject = thisObject;
		newContext.classContext->superObject = superObject;
	} else if (classInfo->superClassInfo == NULL) {
		thisObject = builder.CreateCall(sysObjectAlloca, classInfo->info);
		builder.CreateCall(classInfo->initor, thisObject);
		newContext.classContext->thisObject = thisObject;
	}

	builder.SetInsertPoint(entryBB);
	stmtBlock->codeGen(newContext);
	builder.CreateRet(newContext.classContext->thisObject);

	builder.SetInsertPoint(allocBB);
	builder.CreateBr(entryBB);
}
