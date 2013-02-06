#include "expression.h"
#include "statement.h"
#include "parser.hpp"

AValue BinaryLogicExpr::codeGen(AstContext &astContext) {
	Function *currentFunc = astContext.currentFunc->llvmFunction;
	Value *res = createAlloca(boolType, astContext.allocBB);
	AValue lv = leftExpr->codeGen(astContext);
	if (!lv.castTo(boolClass)) {
		throwError(leftExpr);
	}
	builder.CreateStore(lv.llvmValue, res);
	BasicBlock *rexprBB = BasicBlock::Create(context, "", currentFunc);
	BasicBlock *endBB = BasicBlock::Create(context, "");
	if (op == AND) {
		builder.CreateCondBr(lv.llvmValue, rexprBB, endBB);
	} else {
		builder.CreateCondBr(lv.llvmValue, endBB, rexprBB);
	}

	builder.SetInsertPoint(rexprBB);
	AValue rv = rightExpr->codeGen(astContext);
	if (!rv.castTo(boolClass)) {
		throwError(rightExpr);
	}
	builder.CreateStore(rv.llvmValue, res);
	builder.CreateBr(endBB);

	currentFunc->getBasicBlockList().push_back(endBB);
	builder.SetInsertPoint(endBB);
	return AValue(builder.CreateLoad(res), boolClass);
}

