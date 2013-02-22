#include "statement.h"
#include "expression.h"
#include "support.h"
#include "parser.hpp"

void throwError(Node *node) {
	cout << node->firstLine << ":" << node->firstColumn << ": error: "
			<< errorMsg << endl;
	exit(1);
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
