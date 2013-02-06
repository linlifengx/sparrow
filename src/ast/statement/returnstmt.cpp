#include "statement.h"
#include "expression.h"

void ReturnStmt::codeGen(AstContext &astContext) {
	FunctionInfo *currentFunc = astContext.currentFunc;
	if (currentFunc->style == 0) {
		vector<ClassInfo*> &returnClasses = currentFunc->returnClasses;
		if (exprList.size() < returnClasses.size()) {
			errorMsg = "too few values to return in function '"
					+ currentFunc->name + "'";
			throwError(this);
		} else if (exprList.size() > returnClasses.size()) {
			errorMsg = "too many values to return in function '"
					+ currentFunc->name + "'";
			throwError(this);
		}

		vector<Value*> &returnVars = astContext.getReturnVars();
		for (unsigned i = 0; i < exprList.size(); i++) {
			AValue v = exprList[i]->codeGen(astContext);
			if (!v.castTo(returnClasses[i])) {
				throwError(exprList[i]);
			}
			builder.CreateStore(v.llvmValue, returnVars[i]);
		}
		if (returnClasses.size() == 0) {
			builder.CreateRetVoid();
		} else {
			builder.CreateRet(builder.CreateLoad(astContext.returnAlloc));
		}
	} else if (currentFunc->style == 1) {
		if (exprList.size() > 0) {
			errorMsg =
					"needn't declare any expression behind 'return' in style 1 function";
			throwError(this);
		}
		if (currentFunc->returnNum == 0) {
			builder.CreateRetVoid();
		} else {
			builder.CreateRet(builder.CreateLoad(astContext.returnAlloc));
		}
	} else {
		if (exprList.size() > 0) {
			errorMsg =
					"needn't declare any expression behind 'return' in constructor";
			throwError(this);
		}
		builder.CreateRet(astContext.classContext->thisObject);
	}
	BasicBlock *anonyBB = BasicBlock::Create(context, "after_return",
			currentFunc->llvmFunction);
	builder.SetInsertPoint(anonyBB);
}
