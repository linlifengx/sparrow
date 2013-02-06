#include "expression.h"
#include "support.h"

AValue PrefixOpExpr::codeGen(AstContext &astContext) {
	AValue val = expr->codeGen(astContext);
	if (op == '-') {
		if (val.isDouble()) {
			val.llvmValue = builder.CreateFNeg(val.llvmValue);
			return val;
		} else if (val.isLong()) {
			val.llvmValue = builder.CreateNeg(val.llvmValue);
			return val;
		}
	} else if (op == '!') {
		if (val.isBool()) {
			val.llvmValue = builder.CreateNot(val.llvmValue);
			return val;
		}
	}
	errorMsg = "invalid argument type '" + val.clazz->name + "' to unary '"
			+ getOperatorName(op) + "'expression";
	throwError(this);
	return val;
}

AValue Long::codeGen(AstContext &astContext) {
	return AValue(ConstantInt::getSigned(int64Type, value), longClass);
}

AValue Double::codeGen(AstContext &astContext) {
	return AValue(ConstantFP::get(doubleType, value), doubleClass);
}

AValue Bool::codeGen(AstContext &astContext) {
	return AValue(builder.getInt1(value), boolClass);
}

AValue Nil::codeGen(AstContext &astContext) {
	return AValue(ptr_null, nilClass);
}

AValue ThisExpr::codeGen(AstContext &astContext) {
	if (astContext.classContext == NULL) {
		errorMsg = "invalid use of 'this' outside of a class method";
		throwError(this);
	}
	if (astContext.classContext->thisObject == NULL) {
		errorMsg = "invalid use of 'this' before current object create";
		throwError(this);
	}
	return AValue(astContext.classContext->thisObject,
			astContext.classContext->currentClass);
}

AValue SuperExpr::codeGen(AstContext &astContext) {
	if (astContext.classContext == NULL) {
		errorMsg = "invalid use of 'super' outside of a constructor";
		throwError(this);
	}
	ClassInfo *currentClass = astContext.classContext->currentClass;
	if (astContext.classContext->superObject == NULL) {
		if (currentClass->superClassInfo == NULL) {
			errorMsg = "class '" + currentClass->name + "' has no super class";
			throwError(this);
		} else if (astContext.classContext->thisObject == NULL) {
			errorMsg = "invalid use of 'super' before super object create";
			throwError(this);
		} else {
			errorMsg = "invalid use of 'super' outside of a constructor";
			throwError(this);
		}
	}
	return AValue(astContext.classContext->superObject,
			astContext.classContext->currentClass->superClassInfo);
}
