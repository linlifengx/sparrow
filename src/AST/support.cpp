#include <map>

#include "node.h"
#include "support.h"

void throwError(Node *node){
	cout<<node->firstLine<<":"<<node->firstColumn<<": error: "<<errorMsg<<endl;
	exit(1);
}

bool addClass(ClassInfo* clazz){
	if(classTable[clazz->name] != NULL){
		errorMsg =  "redefine type named '"+clazz->name+"'";
		return false;
	}
	classTable[clazz->name] = clazz;
	return true;
}

bool addFunction(FunctionInfo* func){
	if(functionTable[func->name] != NULL){
		errorMsg = "redefine function named '"+func->name+"'";
		return false;
	}
	functionTable[func->name] = func;
	return true;
}

ClassInfo* getClass(string& name){
	ClassInfo *type = classTable[name];
	if(type == NULL){
		if(name == "void"){
			errorMsg = "variable has incomplete type 'void'";
		}else{
			errorMsg = "undeclared type '"+name+"'";
		}
	}
	return type;
}

AFunction getFunction(string& name){
	FunctionInfo *funcInfo = functionTable[name];
	if(funcInfo == NULL){
		errorMsg = "undeclared function '"+name+"'";
		return AFunction();
	}else{
		return AFunction(funcInfo->llvmFunction,funcInfo);
	}
}

bool ClassInfo::isSubClassOf(ClassInfo* superClazz){
	if(superClassInfo == NULL || superClazz == NULL){
		return false;
	}else if(superClassInfo == superClazz){
		return true;
	}else{
		return superClassInfo->isSubClassOf(superClazz);
	}
}

bool ClassInfo::addField(string& fieldName, ClassInfo* fieldClass){
	if(fieldTable[name] != NULL){
		errorMsg = "redefine field named '"+fieldName+"'";
		return false;
	}else{
		if(superClassInfo != NULL){
			ClassInfo *superFieldClass = superClassInfo->getFieldClass(fieldName);
			if(superFieldClass != NULL && !fieldClass->isSubClassOf(superFieldClass)){
				errorMsg = "field named '"+fieldName+"' of class '"+name+
					"' can't override the field of super";
				return false;
			}
		}
		fieldTable[fieldName] = fieldClass;
		return true;
	}
}

bool ClassInfo::addMethod(string methodName, FunctionInfo* method){
	if(methodTable[methodName] != NULL){
		errorMsg = "redefine method named '"+methodName+"'";
		return false;
	}else{
		if(superClassInfo != NULL){
			FunctionInfo *superMethod = superClassInfo->getMethod(methodName);
			if(superMethod != NULL){
				if(method->returnClasses.size() != superMethod->returnClasses.size()
					|| method->argClasses.size() != superMethod->argClasses.size()){
					errorMsg = "method '"+methodName+"' of class '"+
						name+"' can't override the method of super";
					return false;
				}else{
					for(unsigned i = 0; i < method->returnClasses.size(); i++){
						ClassInfo *clazz = method->returnClasses[i];
						ClassInfo *clazz2 = superMethod->returnClasses[i];
						if(!clazz->isSubClassOf(clazz2)){
							errorMsg = "method '"+methodName+"' of class '"+
								name+"' can't override the method of super";
							return false;
						}
					}
					for(unsigned i = 1; i < method->argClasses.size(); i++){
						ClassInfo *clazz = method->argClasses[i];
						ClassInfo *clazz2 = superMethod->argClasses[i];
						if(clazz2->isSubClassOf(clazz)){
							errorMsg = "method '"+methodName+"' of class '"+
								name+"' can't override the method of super";
							return false;
						}
					}
				}
			}
		}
		methodTable[methodName] = method;
		return true;
	}
}

ClassInfo* ClassInfo::getFieldClass(string& fieldName){
	ClassInfo *clazz = fieldTable[fieldName];
	if(clazz == NULL && superClassInfo != NULL){
		clazz = superClassInfo->getFieldClass(fieldName);
	}
	if(clazz == NULL){
		errorMsg = "undeclared field '"+fieldName+"' in class '"+name+"'";
	}
	return clazz;
}

FunctionInfo* ClassInfo::getMethod(string& methodName){
	AstFunction *method = fieldTable[methodName];
	if(method == NULL && superClassInfo != NULL){
		method = superClassInfo->getMethod(methodName);
	}
	if(method == NULL){
		errorMsg = "undeclared method '"+methodName+"'";
	}
	return method;
}

bool AValue::castTo(ClassInfo *destClass){
	if(destClass == clazz){
		return true;
	}else if(destClass->llvmType == clazz->llvmType	&& clazz->llvmType != ptrType){
		if(clazz->llvmType != ptrType || clazz->isSubClassOf(destClass)){
			return true;
		}
	}else{
		errorMsg = "no viable conversion from '"+clazz->name
					  +"' to '"+destClass->name+"'";
		return false;
	}
}

bool AstContext::addVar(string& name, AValue value){
	if(varTable[name] != NULL){
		errorMsg = "redefine variable named '"+name+"'";
		return false;
	}
	varTable[name] = value;
	return true;
}

AValue AstContext::getVar(string& name){
	AValue var = varTable[name];
	if(var == NULL && classContext != NULL){
		var = classContext->getField(name);
	}
	if(var == NULL && superior != NULL){
		return superior->getVar(name);
	}
	if(var == NULL){
		errorMsg = "undeclared identifier '"+name+"'";
	}
	return var;
}

AFunction AstContext::getFunc(string& name){
	AFunction funcInfo;
	if(classContext != NULL){
		funcInfo = classContext->getMethod(name);
	}
	if(funcInfo.llvmFunc == NULL){
		funcInfo = getFunction(name);
	}
	return funcInfo;
}

AValue ClassContext::getField(string& name){
	AValue field;
	ClassInfo *fieldClass = currentClass->getFieldClass(name);
	if(fieldClass != NULL){
		field = fieldTable[name];
		if(field.llvmValue == NULL){
			Value *fieldName = builder.CreateGlobalStringPtr(name);
			Value *fieldValue = builder.CreateCall2(sysObjectField, thisObject, fieldName);
			fieldValue = builder.CreateBitCast(fieldValue, PointerType::get(fieldClass->llvmType));
			field = AValue(fieldValue, fieldClass);
			fieldTable[name] = field;
		}
	}
	return field;
}

AFunction ClassContext::getMethod(string& name){
	FunctionInfo *methodInfo = currentClass->getMethod(name);
	if(methodInfo != NULL){
		AFunction method = methodTable[name];
		if(method.llvmFunc == NULL){
			Value *methodName = builder.CreateGlobalStringPtr(name);
			Value *func = builder.CreateCall2(sysObjectField, thisObject, methodName);
			func = builder.CreateBitCast(func, 
				PointerType::get(methodInfo->llvmFunction->getType()));
			method = AFunction(func,methodInfo);
			methodTable[name] = method;
		}
		return method;
	}else{
		return AFunction();
	}
}