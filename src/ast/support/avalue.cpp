#include "node.h"
#include "support.h"

bool AValue::castTo(ClassInfo *destClass) {
	if (destClass == clazz) {
		return true;
	} else if (destClass->llvmType == clazz->llvmType) {
		if (clazz->llvmType != ptrType || clazz == nilClass) {
			return true;
		} else if (clazz->llvmType == ptrType
				&& clazz->isSubClassOf(destClass)) {
			return true;
		}
	} else if (clazz->llvmType == int64Type
			&& destClass->llvmType == doubleType) {
		llvmValue = builder.CreateSIToFP(llvmValue, doubleType);
		clazz = doubleClass;
		return true;
	} else if (clazz->llvmType == doubleType
			&& destClass->llvmType == int64Type) {
		llvmValue = builder.CreateFPToSI(llvmValue, int64Type);
		clazz = doubleClass;
		return true;
	}
	errorMsg = "no viable conversion from '" + clazz->name + "' to '"
			+ destClass->name + "'";
	return false;
}

bool AValue::isBool() {
	return clazz == boolClass || clazz->llvmType == boolType;
}

bool AValue::isLong() {
	return clazz == longClass || clazz->llvmType == int64Type;
}

bool AValue::isDouble() {
	return clazz == doubleClass || clazz->llvmType == doubleType;
}

bool AValue::isObject() {
	return clazz->llvmType == ptrType;
}
