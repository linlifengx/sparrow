#include <iostream>
#include <string>

#include "statement.h"
#include "expression.h"
#include "parser.hpp"

void throwError(Node *node) {
	cout << node->firstLine << ":" << node->firstColumn << ": error: "
			<< errorMsg << endl;
	exit(1);
}

bool addClass(ClassInfo* clazz) {
	if (classTable[clazz->name] != NULL) {
		errorMsg = "redefine type named '" + clazz->name + "'";
		return false;
	}
	classTable[clazz->name] = clazz;
	return true;
}

bool addFunction(FunctionInfo* func) {
	if (functionTable[func->name] != NULL) {
		errorMsg = "redefine function named '" + func->name + "'";
		return false;
	}
	functionTable[func->name] = func;
	return true;
}

ClassInfo* getClass(string& name) {
	ClassInfo *type = classTable[name];
	if (type == NULL) {
		if (name == "void") {
			errorMsg = "variable has incomplete type 'void'";
		} else {
			errorMsg = "undeclared type '" + name + "'";
		}
	}
	return type;
}

AFunction getFunctionV(string& name) {
	FunctionInfo *funcInfo = functionTable[name];
	if (funcInfo == NULL) {
		errorMsg = "undeclared function '" + name + "'";
		return AFunction();
	} else {
		return AFunction(funcInfo->llvmFunction, funcInfo);
	}
}

AFunction getMethodV(AValue object, string &methodName) {
	FunctionInfo *methodInfo = object.clazz->getMethod(methodName);
	if (methodInfo != NULL) {
		Value *methodllvmName = builder.CreateGlobalStringPtr(methodName);
		Value *llvmfuc = builder.CreateCall2(sysObjectMethod, object.llvmValue,
				methodllvmName);
		llvmfuc = builder.CreateBitCast(llvmfuc,
				methodInfo->llvmFunction->getType());
		return AFunction(llvmfuc, methodInfo);
	} else {
		return AFunction();
	}
}

AValue getFieldV(AValue object, string &fieldName) {
	ClassInfo *fieldClass = object.clazz->getFieldClass(fieldName);
	if (fieldClass != NULL) {
		Value *fieldllvmName = builder.CreateGlobalStringPtr(fieldName);
		Value *field = builder.CreateCall2(sysObjectField, object.llvmValue,
				fieldllvmName);
		field = builder.CreateBitCast(field,
				PointerType::getUnqual(fieldClass->llvmType));
		return AValue(field, fieldClass);
	} else {
		return AValue();
	}
}

Value* createAlloca(Type *type, BasicBlock *bb) {
	BasicBlock *currentBB = builder.GetInsertBlock();
	builder.SetInsertPoint(bb);
	Value *var = builder.CreateAlloca(type);
	builder.SetInsertPoint(currentBB);
	return var;
}

string getOperatorName(int op) {
	string name;
	if (op < 128) {
		name.push_back(op);
	} else {
		switch (op) {
		case AND:
			name = "&&";
			break;
		case OR:
			name = "||";
			break;
		case NEQUAL:
			name = "!=";
			break;
		case EQUAL:
			name = "==";
			break;
		case LE:
			name = "<=";
			break;
		case GE:
			name = ">=";
			break;
		}
	}
	return name;
}
