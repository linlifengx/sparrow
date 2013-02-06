#include "statement.h"
#include "expression.h"

void VarAssi::codeGen(AstContext &astContext) {
	AValue var = identExpr->lvalueGen(astContext);
	AValue value = expr->codeGen(astContext);
	if (!value.castTo(var.clazz)) {
		throwError(expr);
	}
	builder.CreateStore(value.llvmValue, var.llvmValue);
}

void MultiVarAssi::codeGen(AstContext &astContext) {
	vector<AValue> vars;
	for (unsigned i = 0; i < identList.size(); i++) {
		IdentExpr *identExpr = identList[i];
		if (identExpr == NULL) {
			vars.push_back(AValue());
		} else {
			AValue var = identExpr->lvalueGen(astContext);
			if (var.llvmValue == NULL) {
				throwError(identExpr);
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
