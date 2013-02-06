#include "statement.h"
#include "expression.h"

void VarDef::globalGen(AstContext &astContext) {
	ClassInfo *classInfo = getClass(typeName);
	if (classInfo == NULL) {
		throwError(this);
	}
	Constant *initial = classInfo->getInitial();
	for (unsigned i = 0; i < varInitList.size(); i++) {
		VarInit *varInit = varInitList[i];
		Value *llvmVar = new GlobalVariable(module, classInfo->llvmType, false,
				GlobalValue::ExternalLinkage, initial);
		if (varInit->expr != NULL) {
			AValue v = varInit->expr->codeGen(astContext);
			if (!v.castTo(classInfo)) {
				throwError(varInit->expr);
			}
			builder.CreateStore(v.llvmValue, llvmVar);
		}
		astContext.addVar(varInit->varName, AValue(llvmVar, classInfo));
	}
}

void VarDef::fieldGen(AstContext &astContext) {
	for (unsigned i = 0; i < varInitList.size(); i++) {
		VarInit *varInit = varInitList[i];
		AValue field = astContext.getVar(varInit->varName);
		Value *value = NULL;
		if (varInit->expr != NULL) {
			AValue v = varInit->expr->codeGen(astContext);
			if (!v.castTo(field.clazz)) {
				throwError(varInit->expr);
			}
			value = v.llvmValue;
		} else {
			value = field.clazz->getInitial();
		}
		builder.CreateStore(value, field.llvmValue);
	}
}

void VarDef::codeGen(AstContext &astContext) {
	ClassInfo *classInfo = getClass(typeName);
	if (classInfo == NULL) {
		throwError(this);
	}

	for (unsigned i = 0; i < varInitList.size(); i++) {
		VarInit *varInit = varInitList[i];
		Value *value = NULL;
		if (varInit->expr != NULL) {
			AValue v = varInit->expr->codeGen(astContext);
			if (!v.castTo(classInfo)) {
				throwError(varInit->expr);
			}
			value = v.llvmValue;
		} else {
			value = classInfo->getInitial();
		}
		Value *var = createAlloca(classInfo->llvmType, astContext.allocBB);
		if (!astContext.addVar(varInit->varName, AValue(var, classInfo))) {
			throwError(varInit);
		}
		builder.CreateStore(value, var);
	}
}
