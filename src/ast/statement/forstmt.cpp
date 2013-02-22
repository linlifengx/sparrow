#include "statement.h"
#include "expression.h"
#include "support.h"

void ForStmt::codeGen(AstContext &astContext) {
	Function *func = astContext.currentFunc->llvmFunction;
	AstContext headContext(&astContext);
	initStmt->codeGen(headContext);
	BasicBlock *forHeadBB = BasicBlock::Create(context, "forhead", func);
	BasicBlock *forBodyBB = BasicBlock::Create(context, "forbody");
	BasicBlock *forFootBB = BasicBlock::Create(context, "forfoot");
	BasicBlock *outBB = BasicBlock::Create(context, "outfor");
	builder.CreateBr(forHeadBB);

	builder.SetInsertPoint(forHeadBB);
	condExpr->expectedType = boolClass;
	AValue cond = condExpr->codeGen(headContext);
	builder.CreateCondBr(cond.llvmValue, forBodyBB, outBB);

	func->getBasicBlockList().push_back(forBodyBB);
	builder.SetInsertPoint(forBodyBB);
	AstContext bodyContext(&headContext);
	bodyContext.breakOutBB = outBB;
	bodyContext.continueBB = forFootBB;
	block->codeGen(bodyContext);
	builder.CreateBr(forFootBB);

	func->getBasicBlockList().push_back(forFootBB);
	builder.SetInsertPoint(forFootBB);
	loopStmt->codeGen(headContext);
	builder.CreateBr(forHeadBB);

	func->getBasicBlockList().push_back(outBB);
	builder.SetInsertPoint(outBB);
}

