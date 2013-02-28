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
	AstContext newContext(&astContext);
	for (unsigned i = 0; i < statements.size(); i++) {
		statements[i]->codeGen(newContext);
	}
}

void IncStmt::codeGen(AstContext &astContext) {
	AValue var = expr->lvalueGen(astContext);
	if (var.isObject() || var.isArray() || var.isBool()) {
		if (inc) {
			errorMsg = "invalid operand to expression (" + var.clazz->name
					+ "++)";
		} else {
			errorMsg = "invalid operand to expression (" + var.clazz->name
					+ "--)";
		}
		throwError(this);
	}
	Value *v = builder.CreateLoad(var.llvmValue);
	if (inc) {
		if (var.isLong()) {
			v = builder.CreateAdd(v, builder.getInt64(1));
		} else if (var.isChar()) {
			v = builder.CreateAdd(v, builder.getInt32(1));
		} else {
			v = builder.CreateFAdd(v, ConstantFP::get(doubleType, 1));
		}
	} else {
		if (var.isLong()) {
			v = builder.CreateSub(v, builder.getInt64(1));
		} else if (var.isChar()) {
			v = builder.CreateSub(v, builder.getInt32(1));
		} else {
			v = builder.CreateFSub(v, ConstantFP::get(doubleType, 1));
		}
	}
	builder.CreateStore(v, var.llvmValue);
}
