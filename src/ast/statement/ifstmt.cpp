#include "statement.h"
#include "expression.h"
#include "support.h"

void IfElseStmt::codeGen(AstContext &astContext) {
	AValue cond = condExpr->codeGen(astContext);
	if (!cond.castTo(boolClass)) {
		throwError(condExpr);
	}
	Function *func = astContext.currentFunc->llvmFunction;
	BasicBlock *thenBB = BasicBlock::Create(context, "then", func);
	BasicBlock *elseBB = BasicBlock::Create(context, "else");
	BasicBlock *outBB = BasicBlock::Create(context, "outif");
	builder.CreateCondBr(cond.llvmValue, thenBB, elseBB);

	builder.SetInsertPoint(thenBB);
	AstContext ifContext(&astContext);
	thenBlock->codeGen(ifContext);
	builder.CreateBr(outBB);
	func->getBasicBlockList().push_back(elseBB);

	builder.SetInsertPoint(elseBB);
	AstContext elseContext(&astContext);
	if (elseBlock != NULL) {
		elseBlock->codeGen(elseContext);
	}
	builder.CreateBr(outBB);
	func->getBasicBlockList().push_back(outBB);
	builder.SetInsertPoint(outBB);
}

