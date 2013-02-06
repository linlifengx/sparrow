#include "expression.h"
#include "support.h"

AValue IdentExpr::codeGen(AstContext &astContext) {
	AValue value = lvalueGen(astContext);
	value.llvmValue = builder.CreateLoad(value.llvmValue);
	return value;
}

AValue IdentExpr::lvalueGen(AstContext &astContext) {
	if (expr != NULL) {
		AValue object = expr->codeGen(astContext);
		if (astContext.classContext != NULL
				&& astContext.classContext->thisObject == object.llvmValue) {
			return astContext.classContext->getField(ident);
		} else if (astContext.classContext != NULL
				&& astContext.classContext->superObject == object.llvmValue) {
			return astContext.classContext->getSuperField(ident);
		} else {
			AValue fieldV = getFieldV(object, ident);
			if (fieldV.llvmValue == NULL) {
				throwError(this);
			}
			return fieldV;
		}
	} else {
		AValue var = astContext.getVar(ident);
		if (var.llvmValue == NULL) {
			throwError(this);
		}
		return var;
	}
}
