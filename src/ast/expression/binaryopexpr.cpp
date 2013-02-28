#include "expression.h"
#include "statement.h"
#include "support.h"
#include "parser.hpp"

AValue BinaryOpExpr::gen(AstContext &astContext) {
	AValue lv = leftExpr->codeGen(astContext);
	AValue rv = rightExpr->codeGen(astContext);
	AValue res;
	if (op == '%') {
		if ((lv.isLong() || lv.isChar()) && (rv.isLong() || rv.isChar())) {
			if (lv.isLong()) {
				rv.castTo(longClass);
			} else {
				lv.castTo(rv.clazz);
			}
			res = AValue(builder.CreateSRem(lv.llvmValue, rv.llvmValue),lv.clazz);
		}
	} else if (lv.isBool() && rv.isBool()) {
		switch (op) {
		case EQUAL:
			res = AValue(builder.CreateICmpEQ(lv.llvmValue, rv.llvmValue),
					boolClass);
			break;
		case NEQUAL:
			res = AValue(builder.CreateICmpNE(lv.llvmValue, rv.llvmValue),
					boolClass);
			break;
		}
	} else if ((lv.isLong() || lv.isDouble() || lv.isChar())
			&& (rv.isLong() || rv.isDouble() || rv.isChar())) {
		if (lv.isDouble() || rv.isDouble()) {
			if (!lv.castTo(doubleClass)) {
				throwError(leftExpr);
			}
			if (!rv.castTo(doubleClass)) {
				throwError(rightExpr);
			}
		} else if (lv.isLong() || rv.isLong()) {
			if (!lv.castTo(longClass)) {
				throwError(leftExpr);
			}
			if (!rv.castTo(longClass)) {
				throwError(rightExpr);
			}
		}
		if (lv.isDouble()) {
			switch (op) {
			case '+':
				res = AValue(builder.CreateFAdd(lv.llvmValue, rv.llvmValue),
						doubleClass);
				break;
			case '-':
				res = AValue(builder.CreateFSub(lv.llvmValue, rv.llvmValue),
						doubleClass);
				break;
			case '*':
				res = AValue(builder.CreateFMul(lv.llvmValue, rv.llvmValue),
						doubleClass);
				break;
			case '/':
				res = AValue(builder.CreateFDiv(lv.llvmValue, rv.llvmValue),
						doubleClass);
				break;
			case EQUAL:
				res = AValue(builder.CreateFCmpOEQ(lv.llvmValue, rv.llvmValue),
						boolClass);
				break;
			case NEQUAL:
				res = AValue(builder.CreateFCmpONE(lv.llvmValue, rv.llvmValue),
						boolClass);
				break;
			case '<':
				res = AValue(builder.CreateFCmpOLT(lv.llvmValue, rv.llvmValue),
						boolClass);
				break;
			case '>':
				res = AValue(builder.CreateFCmpOGT(lv.llvmValue, rv.llvmValue),
						boolClass);
				break;
			case LE:
				res = AValue(builder.CreateFCmpOLE(lv.llvmValue, rv.llvmValue),
						boolClass);
				break;
			case GE:
				res = AValue(builder.CreateFCmpOGE(lv.llvmValue, rv.llvmValue),
						boolClass);
				break;
			}
		} else {
			switch (op) {
			case '+':
				res = AValue(builder.CreateAdd(lv.llvmValue, rv.llvmValue),
						lv.clazz);
				break;
			case '-':
				res = AValue(builder.CreateSub(lv.llvmValue, rv.llvmValue),
						lv.clazz);
				break;
			case '*':
				res = AValue(builder.CreateMul(lv.llvmValue, rv.llvmValue),
						lv.clazz);
				break;
			case '/':
				res = AValue(builder.CreateSDiv(lv.llvmValue, rv.llvmValue),
						lv.clazz);
				break;
			case EQUAL:
				res = AValue(builder.CreateICmpEQ(lv.llvmValue, rv.llvmValue),
						boolClass);
				break;
			case NEQUAL:
				res = AValue(builder.CreateICmpNE(lv.llvmValue, rv.llvmValue),
						boolClass);
				break;
			case '<':
				res = AValue(builder.CreateICmpSLT(lv.llvmValue, rv.llvmValue),
						boolClass);
				break;
			case '>':
				res = AValue(builder.CreateICmpSGT(lv.llvmValue, rv.llvmValue),
						boolClass);
				break;
			case LE:
				res = AValue(builder.CreateICmpSLE(lv.llvmValue, rv.llvmValue),
						boolClass);
				break;
			case GE:
				res = AValue(builder.CreateICmpSGE(lv.llvmValue, rv.llvmValue),
						boolClass);
				break;
			}
		}
	} else if ((lv.isObject() || lv.isArray())
			&& (rv.isObject() || rv.isArray())) {
		if (op == EQUAL) {
			res = AValue(builder.CreateICmpEQ(lv.llvmValue, rv.llvmValue),
					boolClass);
		} else if (op == NEQUAL) {
			res = AValue(builder.CreateICmpNE(lv.llvmValue, rv.llvmValue),
					boolClass);
		}
	}
	if (res.llvmValue == NULL) {
		errorMsg = "invalid operands to binary expression (" + lv.clazz->name
				+ " " + getOperatorName(op) + " " + rv.clazz->name + ")";
		throwError(this);
	}

	return res;
}

