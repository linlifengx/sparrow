#include "ast.hpp"

AstValue* NewObject::codeGen(AstContext &astContext){
	string className = callExpr.funcName;
	if(astContext.getType(className) == NULL){
		throwError(this);
	}
	callExpr.funcName = "new$"+className;
	return callExpr.codeGen(astContext);
}

AstValue* ObjectField::codeGen(AstContext& astContext){
	AstValue *object = expr.codeGen(astContext);
	AstType *fieldType = object->type->getFieldType(fieldName);
	if(fieldType == NULL){
		throwError(this);
	}
	Value *fieldNameStr = builder.CreateGlobalStringPtr(fieldName);
	Value *field = builder.CreateCall2(sysObjectField,object->llvmValue,fieldNameStr);
	return new AstValue(field,fieldType);
}

AstValue* ObjectMethod::codeGen(AstContext& astContext){
	AstValue *object = expr.codeGen(astContext);
	AstFunction *method = object->type->getMethod(callExpr.funcName);
	if(method == NULL){
		throwError(this);
	}
	Value *methodNameStr = builder.CreateGlobalStringPtr(callExpr.funcName);
	Value *methodPtr = builder.CreateCall2(sysObjectMethod,object->llvmValue,methodNameStr);
	methodPtr = builder.CreateBitCast(methodPtr,method->llvmFunction->getType());
	method = new AstFunction(method->name,static_cast<Function*>(methodPtr),
		method->returnTypes,method->argTypes,method->dominateType,method->style);
	callExpr.function = method;
	return callExpr.codeGen(astContext);
}