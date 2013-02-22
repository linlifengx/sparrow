#include "statement.h"
#include "expression.h"
#include "support.h"

AValue ArrayElement::lvalueGen(AstContext &astContext) {
	AValue arrayValue = array->codeGen(astContext);
	if (!arrayValue.isArray()) {
		errorMsg = "subscripted value is not an array";
		throwError(array);
	}
	index->expectedType = longClass;
	AValue indexValue = index->codeGen(astContext);

	Value *elementPtr = builder.CreateCall2(sysArrayElement,
			arrayValue.llvmValue, indexValue.llvmValue);
	elementPtr = builder.CreateBitCast(elementPtr,
			PointerType::getUnqual(
					arrayValue.clazz->originClassInfo->llvmType));
	return AValue(elementPtr, arrayValue.clazz->originClassInfo);
}

AValue ArrayElement::gen(AstContext &astContext) {
	AValue value = lvalueGen(astContext);
	value.llvmValue = builder.CreateLoad(value.llvmValue);
	return value;
}

AValue NewArray::gen(AstContext &astContext) {
	ClassInfo *classInfo = typeDecl->getClassInfo();
	sizeExpr->expectedType = longClass;
	AValue sizeV = sizeExpr->codeGen(astContext);
	Value *arrayObject = builder.CreateCall3(sysArrayAlloca, sizeV.llvmValue,
			builder.getInt8(dataLayout->getTypeStoreSize(classInfo->llvmType)),
			ptr_null);
	return AValue(arrayObject, classInfo->getArrayClass());
}

AValue ArrayInit::gen(AstContext &astContext) {
	if (expectedType == NULL) {
		errorMsg = "array expression must use in assistant statement";
		throwError(this);
	}
	ClassInfo *elementClass = expectedType->originClassInfo;
	if (elementClass == NULL) {
		errorMsg = "no viable conversion from array to class '"
				+ expectedType->name + "'";
		throwError(this);
	}
	Value *arrayObject = builder.CreateCall3(sysArrayAlloca,
			builder.getInt64(exprList.size()),
			builder.getInt8(
					dataLayout->getTypeStoreSize(elementClass->llvmType)),
			ptr_null);
	for (unsigned i = 0; i < exprList.size(); i++) {
		exprList[i]->expectedType = elementClass;
		AValue elementV = exprList[i]->codeGen(astContext);
		Value *elementPtr = builder.CreateCall2(sysArrayElement, arrayObject,
				builder.getInt64(i));
		elementPtr = builder.CreateBitCast(elementPtr,
				PointerType::getUnqual(elementClass->llvmType));
		builder.CreateStore(elementV.llvmValue, elementPtr);
	}
	return AValue(arrayObject, expectedType);
}
