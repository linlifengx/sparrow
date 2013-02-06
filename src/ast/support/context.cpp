#include "node.h"
#include "support.h"

bool AstContext::addVar(string& name, AValue value) {
	if (varTable[name].llvmValue != NULL) {
		errorMsg = "redefine variable named '" + name + "'";
		return false;
	}
	varTable[name] = value;
	return true;
}

AValue AstContext::getVar(string& name) {
	AValue var = varTable[name];
	if (var.llvmValue == NULL && classContext != NULL
			&& classContext->thisObject != NULL) {
		var = classContext->getField(name);
	}
	if (var.llvmValue == NULL && superior != NULL) {
		return superior->getVar(name);
	}
	if (var.llvmValue == NULL) {
		errorMsg = "undeclared identifier '" + name + "'";
	}
	return var;
}

AFunction AstContext::getFunc(string& name) {
	AFunction funcInfo;
	if (classContext != NULL && classContext->thisObject != NULL) {
		funcInfo = classContext->getMethod(name);
	}
	if (funcInfo.llvmFunc == NULL) {
		funcInfo = getFunctionV(name);
	}
	return funcInfo;
}

vector<Value*>& AstContext::getReturnVars() {
	if (returnVars.size() == 0 && superior != NULL) {
		return superior->getReturnVars();
	}
	return returnVars;
}

AValue ClassContext::getField(string& name) {
	AValue field;
	ClassInfo *fieldClass = currentClass->getFieldClass(name);
	if (fieldClass != NULL) {
		if (currentClass->fieldTable[name] == NULL && superObject != NULL) {
			return getSuperField(name);
		} else {
			field = fieldTable[name];
			if (field.llvmValue == NULL) {
				BasicBlock *currentBB = NULL;
				if (allocBB != NULL) {
					currentBB = builder.GetInsertBlock();
					builder.SetInsertPoint(allocBB);
				}
				field = getFieldV(AValue(thisObject, currentClass), name);
				fieldTable[name] = field;
				if (allocBB != NULL) {
					builder.SetInsertPoint(currentBB);
				}

			}
		}
	}
	return field;
}

AValue ClassContext::getSuperField(string& name) {
	AValue field;
	ClassInfo *fieldClass = currentClass->superClassInfo->getFieldClass(name);
	if (fieldClass != NULL) {
		field = superFieldTable[name];
		if (field.llvmValue == NULL) {
			BasicBlock *currentBB = NULL;
			if (allocBB != NULL) {
				currentBB = builder.GetInsertBlock();
				builder.SetInsertPoint(allocBB);
			}
			field = getFieldV(AValue(superObject, currentClass->superClassInfo),
					name);
			superFieldTable[name] = field;
			if (allocBB != NULL) {
				builder.SetInsertPoint(currentBB);
			}
		}
	}
	return field;
}

AFunction ClassContext::getMethod(string& name) {
	AFunction method;
	FunctionInfo *methodInfo = currentClass->getMethod(name);
	if (methodInfo != NULL) {
		if (currentClass->methodTable[name] == NULL && superObject != NULL) {
			return getSuperMethod(name);
		} else {
			method = methodTable[name];
			if (method.llvmFunc == NULL) {
				BasicBlock *currentBB = NULL;
				if (allocBB != NULL) {
					currentBB = builder.GetInsertBlock();
					builder.SetInsertPoint(allocBB);
				}
				method = getMethodV(AValue(thisObject, currentClass), name);
				methodTable[name] = method;
				if (allocBB != NULL) {
					builder.SetInsertPoint(currentBB);
				}
			}
		}
	}
	return method;
}

AFunction ClassContext::getSuperMethod(string& name) {
	AFunction method;
	FunctionInfo *methodInfo = currentClass->superClassInfo->getMethod(name);
	if (methodInfo != NULL) {
		method = superMethodTable[name];
		if (method.llvmFunc == NULL) {
			BasicBlock *currentBB = NULL;
			if (allocBB != NULL) {
				currentBB = builder.GetInsertBlock();
				builder.SetInsertPoint(allocBB);
			}
			method = getMethodV(
					AValue(superObject, currentClass->superClassInfo), name);
			superMethodTable[name] = method;
			if (allocBB != NULL) {
				builder.SetInsertPoint(currentBB);
			}
		}
	}
	return method;
}
