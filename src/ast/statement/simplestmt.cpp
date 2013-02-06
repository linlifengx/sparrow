#include "statement.h"
#include "expression.h"
#include "support.h"

void SimpleStmtList::codeGen(AstContext &astContext) {
	for (unsigned i = 0; i < stmtList.size(); i++) {
		stmtList[i]->codeGen(astContext);
	}
}

void ExprStmt::codeGen(AstContext &astContext) {
	expr->codeGen(astContext);
}

void BreakStmt::codeGen(AstContext &astContext) {
	if (astContext.breakOutBB == NULL) {
		errorMsg = "break statement not within for";
		throwError(this);
	}
	builder.CreateBr(astContext.breakOutBB);
	BasicBlock *anonyBB = BasicBlock::Create(context, "after_break",
			astContext.currentFunc->llvmFunction);
	builder.SetInsertPoint(anonyBB);
}

void ContinueStmt::codeGen(AstContext &astContext) {
	if (astContext.continueBB == NULL) {
		errorMsg = "continue statement not within for";
		throwError(this);
	}
	builder.CreateBr(astContext.continueBB);
	BasicBlock *anonyBB = BasicBlock::Create(context, "after_continue",
			astContext.currentFunc->llvmFunction);
	builder.SetInsertPoint(anonyBB);
}

void StmtBlock::codeGen(AstContext &astContext) {
	for (unsigned i = 0; i < statements.size(); i++) {
		statements[i]->codeGen(astContext);
	}
}
