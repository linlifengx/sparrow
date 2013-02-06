#include "statement.h"

ClassInfo::ClassInfo(string name, ClassDef *classDef, Type *llvmType) {
	this->name = name;
	this->llvmType = llvmType;
	this->classDef = classDef;
	this->info = NULL;

	status = 0;
	superClassInfo = NULL;
	initor = NULL;
	constructor = NULL;

	if (classDef != NULL) {
		classDef->classInfo = this;
	}
}

bool ClassInfo::isSubClassOf(ClassInfo* superClazz) {
	if (superClazz == NULL) {
		return false;
	} else if (this == superClazz) {
		return true;
	} else if (superClassInfo == NULL) {
		return false;
	} else {
		return superClassInfo->isSubClassOf(superClazz);
	}
}

bool ClassInfo::addField(string& fieldName, ClassInfo* fieldClass) {
	if (fieldTable[name] != NULL) {
		errorMsg = "redefine field named '" + fieldName + "'";
		return false;
	} else {
		if (superClassInfo != NULL) {
			ClassInfo *superFieldClass = superClassInfo->getFieldClass(
					fieldName);
			if (superFieldClass != NULL
					&& !fieldClass->isSubClassOf(superFieldClass)) {
				errorMsg = "field named '" + fieldName + "' of class '" + name
						+ "' can't override the field of super, "
						+ fieldClass->name + " is not subclass of "
						+ superFieldClass->name;
				return false;
			}
		}
		fieldTable[fieldName] = fieldClass;
		return true;
	}
}

bool ClassInfo::addMethod(FunctionInfo* method) {
	if (methodTable[method->name] != NULL) {
		errorMsg = "redefine method named '" + method->name + "'";
		return false;
	} else {
		if (superClassInfo != NULL) {
			FunctionInfo *superMethod = superClassInfo->getMethod(method->name);
			if (superMethod != NULL) {
				if (method->returnClasses.size()
						!= superMethod->returnClasses.size()
						|| method->argClasses.size()
								!= superMethod->argClasses.size()) {
					errorMsg = "method '" + method->name + "' of class '" + name
							+ "' can't override the method of super";
					return false;
				} else {
					for (unsigned i = 0; i < method->returnClasses.size();
							i++) {
						ClassInfo *clazz = method->returnClasses[i];
						ClassInfo *clazz2 = superMethod->returnClasses[i];
						if (!clazz->isSubClassOf(clazz2)) {
							errorMsg = "method '" + method->name
									+ "' of class '" + name
									+ "' can't override the method of super";
							return false;
						}
					}
					for (unsigned i = 1; i < method->argClasses.size(); i++) {
						ClassInfo *clazz = method->argClasses[i];
						ClassInfo *clazz2 = superMethod->argClasses[i];
						if (clazz2->isSubClassOf(clazz)) {
							errorMsg = "method '" + method->name
									+ "' of class '" + name
									+ "' can't override the method of super";
							return false;
						}
					}
				}
			}
		}
		methodTable[method->name] = method;
		return true;
	}
}

ClassInfo* ClassInfo::getFieldClass(string& fieldName) {
	ClassInfo *clazz = fieldTable[fieldName];
	if (clazz == NULL && superClassInfo != NULL) {
		clazz = superClassInfo->getFieldClass(fieldName);
	}
	if (clazz == NULL) {
		errorMsg = "undeclared field '" + fieldName + "' in class '" + name
				+ "'";
	}
	return clazz;
}

Constant* ClassInfo::getInitial() {
	if (llvmType == int64Type) {
		return int64_0;
	} else if (llvmType == doubleType) {
		return double_0;
	} else if (llvmType == boolType) {
		return bool_false;
	} else if (llvmType == ptrType) {
		return ptr_null;
	} else {
		errorMsg = "can't init void type";
		return NULL;
	}
}

FunctionInfo* ClassInfo::getMethod(string& methodName) {
	FunctionInfo *method = methodTable[methodName];
	if (method == NULL && superClassInfo != NULL) {
		method = superClassInfo->getMethod(methodName);
	}
	if (method == NULL) {
		errorMsg = "undeclared method '" + methodName + "' of class '" + name
				+ "'";
	}
	return method;
}
