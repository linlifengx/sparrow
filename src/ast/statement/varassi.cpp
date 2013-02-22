#include "statement.h"
#include "expression.h"
#include "support.h"

void VarAssi::codeGen(AstContext &astContext) {
	AValue var = leftExpr->lvalueGen(astContext);
	if (var.isReadOnly) {
		errorMsg = "read-only variable is not assignable";
		throwError(leftExpr);
	}
	expr->expectedType = var.clazz;
	AValue value = expr->codeGen(astContext);
	builder.CreateStore(value.llvmValue, var.llvmValue);
}

void MultiVarAssi::codeGen(AstContext &astContext) {
	vector<AValue> vars;
	for (unsigned i = 0; i < leftExprList.size(); i++) {
		LeftValueExpr *leftExpr = leftExprList[i];
		if (leftExpr == NULL) {
			vars.push_back(AValue());
		} else {
			AValue var = leftExpr->lvalueGen(astContext);
			if (var.isReadOnly) {
				errorMsg = "read-only variable is not assignable";
				throwError(leftExpr);
			}
			vars.push_back(var);
		}
	}

	vector<AValue> values = funcInvoke->multiCodeGen(astContext);

	if (values.size() < vars.size()) {
		errorMsg = "too few values returned from function '"
				+ funcInvoke->funcName + "'";
		throwError(funcInvoke);
	}
	for (unsigned i = 0; i < vars.size(); i++) {
		if (vars[i].llvmValue == NULL) {
			continue;
		}
		AValue v = values[i];
		if (!v.castTo(vars[i].clazz)) {
			throwError(this);
		}
		builder.CreateStore(v.llvmValue, vars[i].llvmValue);
	}
}

void ArrayAssi::codeGen(AstContext &astContext) {
	AValue arrayV = leftExpr->codeGen(astContext);
	if (!arrayV.isArray()) {
		errorMsg = "left expression must be a array";
		throwError(leftExpr);
	}
	for (unsigned i = 0; i < exprList.size(); i++) {
		if (exprList[i] == NULL) {
			continue;
		}
		exprList[i]->expectedType = arrayV.clazz->originClassInfo;
		AValue v = exprList[i]->codeGen(astContext);
		Value *elementPtr = builder.CreateCall2(sysArrayElement,
				arrayV.llvmValue, builder.getInt64(i));
		elementPtr = builder.CreateBitCast(elementPtr,
				PointerType::getUnqual(
						arrayV.clazz->originClassInfo->llvmType));
		builder.CreateStore(v.llvmValue, elementPtr);
	}
}
