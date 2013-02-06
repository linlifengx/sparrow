#include "statement.h"
#include "expression.h"
#include "support.h"

void SuperInit::codeGen(AstContext &astContext) {
	if (astContext.classContext == NULL) {
		errorMsg = "invalid use of 'super(...)' outside of a constructor";
		throwError(this);
	}
	ClassInfo *classInfo = astContext.classContext->currentClass;
	if (astContext.currentFunc != classInfo->constructor) {
		errorMsg = "invalid use of 'super(...)' outside of a constructor";
		throwError(this);
	}
	if (classInfo->superClassInfo == NULL) {
		errorMsg = "class '" + classInfo->name + "' has no super class";
		throwError(this);
	}
	if (astContext.classContext->thisObject != NULL) {
		errorMsg =
				"invalid use of 'super(...)' not the beginning of a constructor";
		throwError(this);
	}
	BasicBlock *currentBB = NULL;
	if (astContext.allocBB != NULL) {
		currentBB = builder.GetInsertBlock();
		builder.SetInsertPoint(astContext.allocBB);
	}

	ClassInfo *superClass = classInfo->superClassInfo;
	FunctionInfo *superCstor = superClass->constructor;
	vector<ClassInfo*> &argClasses = superCstor->argClasses;
	if (exprList.size() < argClasses.size()) {
		errorMsg = "too few arguments to constructor of '" + superClass->name
				+ "''";
		throwError(this);
	} else if (exprList.size() > argClasses.size()) {
		errorMsg = "too many arguments to function '" + superClass->name + "'";
		throwError(this);
	}

	vector<AValue> argValues;
	vector<Value*> argllvmValues;

	for (unsigned i = 0; i < exprList.size(); i++) {
		AValue v = exprList[i]->codeGen(astContext);
		if (!v.castTo(argClasses[i])) {
			throwError(exprList[i]);
		}
		argValues.push_back(v);
		argllvmValues.push_back(v.llvmValue);
	}

	Value *superObject = NULL;
	if (argValues.size() == 0) {
		superObject = builder.CreateCall(superCstor->llvmFunction);
	} else {
		ArrayRef<Value*> args(argllvmValues);
		superObject = builder.CreateCall(superCstor->llvmFunction, args);
	}
	Value *thisObject = builder.CreateCall(sysObjectAlloca, classInfo->info);
	Value *superElementPtr = builder.CreateGEP(thisObject, builder.getInt32(8));
	superElementPtr = builder.CreateBitCast(superElementPtr, ptrptrType);
	builder.CreateStore(superObject, superElementPtr);
	builder.CreateCall2(classInfo->initor, thisObject, superObject);
	astContext.classContext->thisObject = thisObject;
	astContext.classContext->superObject = superObject;

	if (astContext.allocBB != NULL) {
		builder.SetInsertPoint(currentBB);
	}
}
