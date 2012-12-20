#include "ast.hpp"
#include "parser.hpp"

Value* BinaryExpr::codeGen(AstContext &astContext){
	Value *lv = lexpr.codeGen(astContext);
	Value *rv = rexpr.codeGen(astContext);
	if(lv->getType()->isIntegerTy(1) && lv->getType()->isIntegerTy(1)){
		switch(op){
		case EQUAL:
			return builder.CreateICmpEQ(lv,rv);
		case NEQUAL:
			return builder.CreateICmpNE(lv,rv);
		default:
			;
		}
	}else if( (lv->getType()->isDoubleTy() || lv->getType()->isIntegerTy(64))
		&& (lv->getType()->isDoubleTy() || lv->getType()->isIntegerTy(64)) ){
		if(lv->getType()->isDoubleTy()){
			rv = createCast(rv,lv->getType());
			if(rv == NULL){
				throwError(&rexpr);
			} 
		}else{
			lv = createCast(lv,rv->getType());
			if(lv == NULL){
				throwError(&lexpr);
			}
		}
		if(lv->getType()->isDoubleTy()){
			switch(op){
			case '+':
				return builder.CreateFAdd(lv,rv);
			case '-':
				return builder.CreateFSub(lv,rv);
			case '*':
				return builder.CreateFMul(lv,rv);
			case '/':
				return builder.CreateFDiv(lv,rv);
			case EQUAL:
				return builder.CreateFCmpOEQ(lv,rv);
			case NEQUAL:
				return builder.CreateFCmpONE(lv,rv);
			case '<':
				return builder.CreateFCmpOLT(lv,rv);
			case '>':
				return builder.CreateFCmpOGT(lv,rv);
			case LE:
				return builder.CreateFCmpOLE(lv,rv);
			case GE:
				return builder.CreateFCmpOGE(lv,rv);
			default:
				;
			}
		}else{
			switch(op){
			case '+':
				return builder.CreateAdd(lv,rv);
			case '-':
				return builder.CreateSub(lv,rv);
			case '*':
				return builder.CreateMul(lv,rv);
			case '/':
				return builder.CreateSDiv(lv,rv);
			case EQUAL:
				return builder.CreateICmpEQ(lv,rv);
			case NEQUAL:
				return builder.CreateICmpNE(lv,rv);
			case '<':
				return builder.CreateICmpSLT(lv,rv);
			case '>':
				return builder.CreateICmpSGT(lv,rv);
			case LE:
				return builder.CreateICmpSLE(lv,rv);
			case GE:
				return builder.CreateICmpSGE(lv,rv);
			default:
				;
			}
		}
	}
	errorMsg = "invalid operands to binary expression ("+getTypeName(lv->getType())+
				 " "+getOperatorName(op)+" "+getTypeName(rv->getType())+")";
	throwError(this);
}

Value* LogicExpr::codeGen(AstContext &astContext){
	Function *currentFunc = astContext.currentFunc->llvmFunction;
	Value *res = builder.CreateAlloca(builder.getInt1Ty());
	Value *lv = lexpr.codeGen(astContext);
	lv = createCast(lv,builder.getInt1Ty());
	if(lv == NULL){
		throwError(&lexpr);
	}
	builder.CreateStore(lv,res);
	BasicBlock *rexprBB = BasicBlock::Create(context,"",currentFunc);
	BasicBlock *endBB = BasicBlock::Create(context,"");
	if(op == AND){
		builder.CreateCondBr(lv,rexprBB,endBB);
	}else{
		builder.CreateCondBr(lv,endBB,rexprBB);
	}

	builder.SetInsertPoint(rexprBB);
	Value *rv = rexpr.codeGen(astContext);
	rv = createCast(rv,builder.getInt1Ty());
	if(rv == NULL){
		throwError(&rexpr);
	}
	builder.CreateStore(rv,res);
	builder.CreateBr(endBB);
	
	currentFunc->getBasicBlockList().push_back(endBB);
	builder.SetInsertPoint(endBB);
	return builder.CreateLoad(res);
}

Value* PrefixExpr::codeGen(AstContext &astContext){
	Value *val = expr.codeGen(astContext);
	if(op == '-'){
		if(val->getType()->isDoubleTy()){
			return builder.CreateFNeg(val);
		}else if(val->getType()->isIntegerTy(64)){
			return builder.CreateNeg(val);
		}
	}else if(op == '!'){
		if(val->getType()->isIntegerTy(1)){
			return builder.CreateNot(val);
		}
	}
	errorMsg = "invalid argument type '"+getTypeName(val->getType())+
				 "' to unary '"+getOperatorName(op)+"'expression";
	throwError(this);
}

Value* IdentExpr::codeGen(AstContext &astContext){
	Value *var = astContext.getVar(ident);
	if(var == NULL){
		throwError(this);
	}
	return builder.CreateLoad(var);
}

vector<Value*> CallExpr::multiCodeGen(AstContext &astContext){
	MyFunction *myfunc = astContext.getFunction(funcName);
	if(myfunc == NULL){
		throwError(this);
	}
	vector<Type*> &argTypes = myfunc->argTypes;
	vector<Value*> exprListValues;
	for(unsigned i=0; i < exprList.size(); i++){
		Expression *expr = exprList[i];
		exprListValues.push_back(expr->codeGen(astContext));
	}
	if(exprListValues.size() < argTypes.size()){
		errorMsg = "too few arguments to function '"+funcName+"''";
		throwError(this);
	}else if(exprListValues.size() > argTypes.size()){
		cout<<"too many arguments to function '"<<funcName<<"'"<<endl;
	}
	
	Value *callResult = NULL;
	if(argTypes.size() == 0){
		callResult = builder.CreateCall(myfunc->llvmFunction);
	}else{
		vector<Value*> argValues;
		for(unsigned i=0; i < argTypes.size(); i++){
			Value *v = createCast(exprListValues[i],argTypes[i]);
			if(v == NULL){
				throwError(exprList[i]);
			}
			argValues.push_back(v);
		}
		ArrayRef<Value*> args(argValues);
		callResult = builder.CreateCall(myfunc->llvmFunction,args);
	}
	
	vector<Value*> resultValues;
	vector<Type*> &resultTypes = myfunc->returnTypes;
	if(myfunc->isReturnVoid){
		resultValues.push_back(callResult);
	}else if(myfunc->isReturnSingle){
		resultValues.push_back(callResult);
	}else{
		Value *alloc = builder.CreateAlloca(myfunc->returnType);
		builder.CreateStore(callResult,alloc);
		for(unsigned i=0; i < resultTypes.size(); i++){
			Value *element = builder.CreateStructGEP(alloc,i);
			resultValues.push_back(builder.CreateLoad(element));
		}
	}
	return resultValues;
}

Value* CallExpr::codeGen(AstContext &astContext){
	vector<Value*> resultValues = multiCodeGen(astContext);
	return resultValues[0];
}

Value* Long::codeGen(AstContext &astContext){
	return builder.getInt64(atol(valStr.c_str()));
}

Value* Double::codeGen(AstContext &astContext){
	return ConstantFP::get(builder.getDoubleTy(),atof(valStr.c_str()));
}

Value* Bool::codeGen(AstContext &astContext){
	return builder.getInt1(value);
}
