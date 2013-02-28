#include "expression.h"
#include "support.h"

AValue PrefixOpExpr::gen(AstContext &astContext) {
	AValue val = expr->codeGen(astContext);
	if (op == '-') {
		if (val.isDouble()) {
			val.llvmValue = builder.CreateFNeg(val.llvmValue);
			return val;
		} else if (val.isLong() || val.isChar()) {
			val.llvmValue = builder.CreateNeg(val.llvmValue);
			return val;
		}
	} else if (op == '!') {
		if (val.isBool()) {
			val.llvmValue = builder.CreateNot(val.llvmValue);
			return val;
		}
	}
	errorMsg = "invalid argument type '" + val.clazz->name + "' to unary '"
			+ getOperatorName(op) + "' expression";
	throwError(this);
	return val;
}

AValue Long::gen(AstContext &astContext) {
	return AValue(ConstantInt::getSigned(int64Type, value), longClass);
}

AValue Char::gen(AstContext &astContext) {
	return AValue(ConstantInt::getSigned(int32Type, value), charClass);
}

AValue Double::gen(AstContext &astContext) {
	return AValue(ConstantFP::get(doubleType, value), doubleClass);
}

AValue Bool::gen(AstContext &astContext) {
	return AValue(builder.getInt1(value), boolClass);
}

AValue Nil::gen(AstContext &astContext) {
	return AValue(ptr_null, nilClass);
}

AValue ThisExpr::gen(AstContext &astContext) {
	if (astContext.classContext == NULL) {
		errorMsg = "invalid use of 'this' outside of a class method";
		throwError(this);
	}
	if (astContext.classContext->thisObject == NULL) {
		errorMsg = "invalid use of 'this' before current object create";
		throwError(this);
	}
	return AValue(astContext.classContext->thisObject,
			astContext.classContext->currentClass);
}

AValue SuperExpr::gen(AstContext &astContext) {
	if (astContext.classContext == NULL) {
		errorMsg = "invalid use of 'super' outside of a constructor";
		throwError(this);
	}
	ClassInfo *currentClass = astContext.classContext->currentClass;
	if (astContext.classContext->superObject == NULL) {
		if (currentClass->superClassInfo == NULL) {
			errorMsg = "class '" + currentClass->name + "' has no super class";
			throwError(this);
		} else if (astContext.classContext->thisObject == NULL) {
			errorMsg = "invalid use of 'super' before super object create";
			throwError(this);
		} else {
			errorMsg = "invalid use of 'super' outside of a constructor";
			throwError(this);
		}
	}
	return AValue(astContext.classContext->superObject,
			astContext.classContext->currentClass->superClassInfo);
}

AValue String::gen(AstContext &astContext) {
	vector<Constant*> unicodes;
	for (unsigned i = 0; i < data->size(); i++) {
		int32_t unicode = data->at(i);
		unicodes.push_back(ConstantInt::get(int32Type, unicode));
	}
	if (unicodes.size() > 0) {
		ArrayType *arrayType = ArrayType::get(int32Type, unicodes.size());
		GlobalVariable *array = new GlobalVariable(module, arrayType, true,
				GlobalValue::PrivateLinkage,
				ConstantArray::get(arrayType, ArrayRef<Constant*>(unicodes)));
		Value *str = builder.CreateCall3(sysArrayAlloca,
				builder.getInt64(unicodes.size()),
				builder.getInt8(dataLayout->getTypeStoreSize(int32Type)),
				builder.CreateBitCast(array, ptrType));
		return AValue(str, charClass->getArrayClass());
	} else {
		Value *str = builder.CreateCall3(sysArrayAlloca, int64_0,
				builder.getInt8(dataLayout->getTypeStoreSize(int32Type)),
				ptr_null);
		return AValue(str, charClass->getArrayClass());
	}
}

AValue DynamicCast::gen(AstContext &astContext) {
	ClassInfo *destClass = typeDecl->getClassInfo();
	AValue v = expr->codeGen(astContext);
	if (v.castTo(destClass)) {
		return v;
	} else if (v.isObject() && destClass->isObjectType()) {
		v.llvmValue = builder.CreateCall2(sysDynamicCast, v.llvmValue,
				destClass->info);
		v.clazz = destClass;
		return v;
	} else {
		errorMsg = "no viable conversion from '" + v.clazz->name + "' to '"
				+ destClass->name + "'";
		throwError(expr);
	}
	return v;
}

AValue InstanceOf::gen(AstContext &astContext) {
	AValue v = expr->codeGen(astContext);
	ClassInfo *destClass = typeDecl->getClassInfo();
	if (v.clazz == nilClass
			&& (destClass->isObjectType() || destClass->isArrayType())) {
		return AValue(bool_true, boolClass);
	} else if (v.clazz->isSubClassOf(destClass)) {
		return AValue(bool_true, boolClass);
	} else if (destClass->isObjectType() && v.isObject()) {
		Value *res = builder.CreateCall2(sysInstanceOf, v.llvmValue,
				destClass->info);
		res = builder.CreateTrunc(res, boolType);
		return AValue(res, boolClass);
	} else {
		return AValue(bool_false, boolClass);
	}
}
