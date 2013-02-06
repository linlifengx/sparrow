#include "expression.h"
#include "support.h"
#include "statement.h"
#include "parser.hpp"

AValue BinaryOpExpr::codeGen(AstContext &astContext) {
	AValue lv = leftExpr->codeGen(astContext);
	AValue rv = rightExpr->codeGen(astContext);
	AValue res;
	if (lv.isBool() && rv.isBool()) {
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
	} else if ((lv.isLong() || lv.isDouble())
			&& (rv.isLong() || rv.isDouble())) {
		if (lv.isDouble()) {
			if (!rv.castTo(lv.clazz)) {
				throwError(rightExpr);
			}
		} else {
			if (!lv.castTo(rv.clazz)) {
				throwError(leftExpr);
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
						longClass);
				break;
			case '-':
				res = AValue(builder.CreateSub(lv.llvmValue, rv.llvmValue),
						longClass);
				break;
			case '*':
				res = AValue(builder.CreateMul(lv.llvmValue, rv.llvmValue),
						longClass);
				break;
			case '/':
				res = AValue(builder.CreateSDiv(lv.llvmValue, rv.llvmValue),
						longClass);
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
	} else if (lv.isObject() && rv.isObject()) {
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
