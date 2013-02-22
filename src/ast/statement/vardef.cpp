#include "statement.h"
#include "expression.h"
#include "support.h"

void VarDef::globalGen() {
	ClassInfo *classInfo = typeDecl->getClassInfo();
	Constant *initial = classInfo->getInitial();
	for (unsigned i = 0; i < varInitList.size(); i++) {
		VarInit *varInit = varInitList[i];
		Value *llvmVar = new GlobalVariable(module, classInfo->llvmType, false,
				GlobalValue::ExternalLinkage, initial);
		AstContext astContext;
		if (varInit->expr != NULL) {
			varInit->expr->expectedType = classInfo;
			AValue v = varInit->expr->codeGen(astContext);
			builder.CreateStore(v.llvmValue, llvmVar);
		}
		globalContext.addVar(varInit->varName, AValue(llvmVar, classInfo));
	}
}

void VarDef::fieldGen(AstContext &astContext) {
	for (unsigned i = 0; i < varInitList.size(); i++) {
		VarInit *varInit = varInitList[i];
		AValue field = astContext.getVar(varInit->varName);
		Value *value = NULL;
		if (varInit->expr != NULL) {
			varInit->expr->expectedType = field.clazz;
			AValue v = varInit->expr->codeGen(astContext);
			value = v.llvmValue;
		} else {
			value = field.clazz->getInitial();
		}
		builder.CreateStore(value, field.llvmValue);
	}
}

void VarDef::codeGen(AstContext &astContext) {
	ClassInfo *classInfo = typeDecl->getClassInfo();
	for (unsigned i = 0; i < varInitList.size(); i++) {
		VarInit *varInit = varInitList[i];
		Value *value = NULL;
		if (varInit->expr != NULL) {
			varInit->expr->expectedType = classInfo;
			AValue v = varInit->expr->codeGen(astContext);
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
