#include "expression.h"
#include "statement.h"
#include "support.h"
#include "parser.hpp"

AValue BinaryLogicExpr::gen(AstContext &astContext) {
	Function *currentFunc = astContext.currentFunc->llvmFunction;
	Value *res = createAlloca(boolType, astContext.allocBB);
	leftExpr->expectedType = boolClass;
	AValue lv = leftExpr->codeGen(astContext);
	builder.CreateStore(lv.llvmValue, res);
	BasicBlock *rexprBB = BasicBlock::Create(context, "", currentFunc);
	BasicBlock *endBB = BasicBlock::Create(context, "");
	if (op == AND) {
		builder.CreateCondBr(lv.llvmValue, rexprBB, endBB);
	} else {
		builder.CreateCondBr(lv.llvmValue, endBB, rexprBB);
	}

	builder.SetInsertPoint(rexprBB);
	rightExpr->expectedType = boolClass;
	AValue rv = rightExpr->codeGen(astContext);
	builder.CreateStore(rv.llvmValue, res);
	builder.CreateBr(endBB);

	currentFunc->getBasicBlockList().push_back(endBB);
	builder.SetInsertPoint(endBB);
	return AValue(builder.CreateLoad(res), boolClass);
}

