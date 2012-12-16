#include "ast.hpp"
#include "parser.hpp"

void VarDecl::codeGen(AstContext &astContext){
	Type *type = NULL;
	try{
		type = astContext.getType(typeName);
	}catch(string msg){
		throwError(this,msg);
	}
	
	for(unsigned i = 0; i < varInitList.size(); i++){
		VarInit *varInit = varInitList[i];
		Value *var = NULL;
		Value *v = NULL;
		if(varInit->expr != NULL){
			v = varInit->expr->codeGen(astContext);
			try{
				v = createCast(v,type);
			}catch(string msg){
				throwError(varInit->expr,msg);
			}
		}else{
			try{
				v = getInitial(type);}
			catch(string msg){
				throwError(this,msg);
			}
		}
		var = builder.CreateAlloca(type);
		builder.CreateStore(v,var);
		try{
			astContext.addVar(varInit->varName,var);
		}catch(string msg){
			throwError(varInit,msg);
		}
	}
}

void VarAssi::codeGen(AstContext &astContext){
	Value *var = NULL;
	try{
		var = astContext.getVar(varName);
	}catch(string msg){
		throwError(this,msg);
	}
	
	Value *value = expr.codeGen(astContext);
	PointerType *pt = static_cast<PointerType*>(var->getType());
	try{
		value = createCast(value,pt->getElementType());
	}catch(string msg){
		throwError(&expr,msg);
	}
	builder.CreateStore(value,var);
}

void MultiVarAssi::codeGen(AstContext &astContext){
	vector<Value*> vars;
	for(unsigned i=0; i < varNameList.size(); i++){
		string &varName = *varNameList[i];
		if(varName == ""){
			vars.push_back(NULL);
		}else{
			Value *var = NULL;
			try{
				var = astContext.getVar(varName);
			}catch(string msg){
				throwError(this,msg);
			}
			vars.push_back(var);
		}
	}
	
	vector<Value*> values = callExpr.multiCodeGen(astContext);
	if(values.size() < vars.size()){
		throwError(&callExpr,"too few values returned from function '"+callExpr.funcName+"'");
	}
	for(unsigned i=0; i < vars.size(); i++){
		if(vars[i] == NULL){
			continue;
		}
		Value *v = values[i];
		PointerType *pt = static_cast<PointerType*>(vars[i]->getType());
		try{
			v = createCast(v,pt->getElementType());
		}catch(string msg){
			throwError(&callExpr,msg);
		}
		builder.CreateStore(v,vars[i]);
	}
}

void SimpleStmtList::codeGen(AstContext &astContext){
	for(unsigned i = 0; i < stmtList.size(); i++){
		stmtList[i]->codeGen(astContext);
	}
}

void SimpleStmtList::add(Statement *stmt){
	stmtList.push_back(stmt);
}

void ExprStmt::codeGen(AstContext &astContext){
	expr.codeGen(astContext);
}

void IfElseStmt::codeGen(AstContext &astContext){
	Value *cond = condExpr.codeGen(astContext);
	try{
		cond = createCast(cond,builder.getInt1Ty());
	}catch(string msg){
		throwError(&condExpr,msg);
	}
	Function *func = astContext.currentFunc->llvmFunction;
	BasicBlock *thenBB = BasicBlock::Create(context,"then",func);
	BasicBlock *elseBB = BasicBlock::Create(context,"else");
	BasicBlock *outBB = BasicBlock::Create(context,"outif");
	builder.CreateCondBr(cond,thenBB,elseBB);
	builder.SetInsertPoint(thenBB);
	AstContext ifContext(&astContext);
	for(unsigned i=0; i < thenStmts.size(); i++){
		thenStmts[i]->codeGen(ifContext);
	}
	builder.CreateBr(outBB);
	func->getBasicBlockList().push_back(elseBB);
	builder.SetInsertPoint(elseBB);
	AstContext elseContext(&astContext);
	for(unsigned i=0; i < elseStmts.size(); i++){
		elseStmts[i]->codeGen(elseContext);
	}
	builder.CreateBr(outBB);
	func->getBasicBlockList().push_back(outBB);
	builder.SetInsertPoint(outBB);
}

void ForStmt::codeGen(AstContext &astContext){
	Function *func = astContext.currentFunc->llvmFunction;
	AstContext headContext(&astContext);
	initStmt.codeGen(headContext);
	BasicBlock *forHeadBB = BasicBlock::Create(context,"forhead",func);
	BasicBlock *forBodyBB = BasicBlock::Create(context,"forbody");
	BasicBlock *forFootBB = BasicBlock::Create(context,"forfoot");
	BasicBlock *outBB = BasicBlock::Create(context,"outfor");
	builder.CreateBr(forHeadBB);
	
	builder.SetInsertPoint(forHeadBB);
	Value *cond = condExpr.codeGen(headContext);
	try{
		cond = createCast(cond,builder.getInt1Ty());
	}catch(string msg){
		throwError(&condExpr,msg);
	}
	builder.CreateCondBr(cond,forBodyBB,outBB);
	
	func->getBasicBlockList().push_back(forBodyBB);
	builder.SetInsertPoint(forBodyBB);
	AstContext bodyContext(headContext);
	bodyContext.breakOutBB = outBB;
	bodyContext.continueBB = forFootBB;
	for(unsigned i = 0; i < stmtList.size(); i++){
		stmtList[i]->codeGen(bodyContext);
	}
	builder.CreateBr(forFootBB);
	
	func->getBasicBlockList().push_back(forFootBB);
	builder.SetInsertPoint(forFootBB);
	loopStmt.codeGen(headContext);
	builder.CreateBr(forHeadBB);
	
	func->getBasicBlockList().push_back(outBB);
	builder.SetInsertPoint(outBB);
}

void ReturnStmt::codeGen(AstContext &astContext){	
	MyFunction *currentFunc = astContext.currentFunc;
	if(currentFunc->style == 1){
		vector<Type*> &returnTypes = currentFunc->returnTypes;
		
		vector<Value*> exprListValues;
		for(unsigned i=0; i < exprList.size(); i++){
			Expression *expr = exprList[i];
			exprListValues.push_back(expr->codeGen(astContext));
		}
		if(exprListValues.size() < returnTypes.size()){
			throwError(this,"too few values to return in function '"+currentFunc->name+"'");
		}else if(exprListValues.size() > returnTypes.size()){
			cout<<"Warning: too many values to return in function '"<<currentFunc->name<<"'"<<endl;
		}
		if(returnTypes.size() == 0){
			builder.CreateRetVoid();
		}else if(returnTypes.size() == 1){
			try{
				builder.CreateRet(createCast(exprListValues[0],returnTypes[0]));
			}catch(string msg){
				throwError(exprList[0],msg);
			}
		}else{
			Value *alloc = builder.CreateAlloca(currentFunc->returnType);
			for(unsigned i=0; i < returnTypes.size(); i++){
				Value *element = builder.CreateStructGEP(alloc,i);
				try{
					builder.CreateStore(createCast(exprListValues[i],returnTypes[i]),element);
				}catch(string msg){
					throwError(exprList[i],msg);
				}
			}
			builder.CreateRet(builder.CreateLoad(alloc));
		}
	}else{
		if(exprList.size() > 0){
			cout<<"Warning: needn't declare any expression behind 'return' in style 2 function"<<endl;
		}
		if(currentFunc->isReturnVoid){
			builder.CreateRetVoid();
		}else if(currentFunc->isReturnSingle){
			Value *v = builder.CreateLoad(currentFunc->returnVars[0]);
			builder.CreateRet(v);
		}else{
			Value *alloc = builder.CreateAlloca(currentFunc->returnType);
			for(unsigned i = 0; i < currentFunc->returnVars.size(); i++){
				Value *element = builder.CreateStructGEP(alloc,i);
				Value *v = builder.CreateLoad(currentFunc->returnVars[i]);
				builder.CreateStore(v,element);
			}
			builder.CreateRet(builder.CreateLoad(alloc));
		}
	}
	BasicBlock *anonyBB = BasicBlock::Create(context,"after_return",currentFunc->llvmFunction);
	builder.SetInsertPoint(anonyBB);
}


void BreakStmt::codeGen(AstContext &astContext){
	if(astContext.breakOutBB == NULL){
		throwError(this,"break statement not within for");
	}
	builder.CreateBr(astContext.breakOutBB);
	BasicBlock *anonyBB = BasicBlock::Create(context,"after_break",astContext.currentFunc->llvmFunction);
	builder.SetInsertPoint(anonyBB);
}

void ContinueStmt::codeGen(AstContext &astContext){
	if(astContext.continueBB == NULL){
		throwError(this,"continue statement not within for");
	}
	builder.CreateBr(astContext.continueBB);
	BasicBlock *anonyBB = BasicBlock::Create(context,"after_continue",astContext.currentFunc->llvmFunction);
	builder.SetInsertPoint(anonyBB);
}
