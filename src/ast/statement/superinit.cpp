#include "statement.h"
#include "expression.h"
#include "support.h"

void SuperInit::codeGen(AstContext &astContext) {
	if (classContext == NULL) {
		errorMsg =
				"invalid use of 'super(...)' not in the beginning of a constructor";
		throwError(this);
	}
	ClassInfo *classInfo = classContext->currentClass;
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
		exprList[i]->expectedType = argClasses[i];
		AValue v = exprList[i]->codeGen(astContext);
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

	classContext->superObject = superObject;
}
