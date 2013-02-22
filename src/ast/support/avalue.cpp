#include "node.h"
#include "support.h"

bool AValue::castTo(ClassInfo *destClass) {
	bool res = false;
	if (clazz == destClass) {
		res = true;
	} else if (clazz == nilClass
			&& (destClass->isObjectType() || destClass->isArrayType())) {
		res = true;
	} else if (isObject() && destClass->isObjectType()
			&& clazz->isSubClassOf(destClass)) {
		res = true;
	} else if (isLong() && destClass->isDoubleType()) {
		llvmValue = builder.CreateSIToFP(llvmValue, doubleType);
		res = true;
	} else if (isLong() && destClass->isCharType()) {
		llvmValue = builder.CreateTrunc(llvmValue, int32Type);
		res = true;
	} else if (isDouble() && destClass->isLongType()) {
		llvmValue = builder.CreateFPToSI(llvmValue, int64Type);
		res = true;
	} else if (isDouble() && destClass->isCharType()) {
		llvmValue = builder.CreateFPToSI(llvmValue, int32Type);
		res = true;
	} else if (isChar() && destClass->isLongType()) {
		llvmValue = builder.CreateSExt(llvmValue, int64Type);
		res = true;
	} else if (isChar() && destClass->isDoubleType()) {
		llvmValue = builder.CreateSIToFP(llvmValue, doubleType);
		res = true;
	}
	if (res) {
		clazz = destClass;
	} else {
		errorMsg = "no viable conversion from '" + clazz->name + "' to '"
				+ destClass->name + "'";
	}
	return res;
}

bool AValue::isBool() {
	return clazz->isBoolType();
}

bool AValue::isLong() {
	return clazz->isLongType();
}

bool AValue::isChar() {
	return clazz->isCharType();
}

bool AValue::isDouble() {
	return clazz->isDoubleType();
}

bool AValue::isObject() {
	return clazz->isObjectType();
}

bool AValue::isArray() {
	return clazz->isArrayType();
}
