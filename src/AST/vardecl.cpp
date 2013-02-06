#include "ast.hpp"
//#include "parser.hpp"

void VarDecl::codeGen(AstContext &astContext){
	AstType *type = astContext.getType(typeName);
	if(type == NULL){
		throwError(this);
	}
	
	for(unsigned i = 0; i < varInitList.size(); i++){
		VarInit *varInit = varInitList[i];
		AstValue *var = NULL;
		AstValue *v = NULL;
		if(varInit->expr != NULL){
			v = varInit->expr->codeGen(astContext);
			if(v->castTo(type)){
				throwError(varInit->expr);
			}
		}else{
			v = type->getInitialValue();
		}
		var = builder.CreateAlloca(type->llvmType);
		builder.CreateStore(v->llvmValue,var->llvmValue);
		if(!astContext.addVar(varInit->varName,var)){
			throwError(varInit);
		}
		delete v;
	}
}

void VarDecl::globalCodeGen(AstContext &astContext){
	AstType *type = astContext.getType(typeName);
	if(type == NULL){
		throwError(this);
	}
	Constant *initial = type->getInitial();
	for(unsigned i = 0; i < varInitList.size(); i++){
		VarInit *varInit = varInitList[i];
		Value *llvmVar = new GlobalVariable(module,type,false,GlobalValue::ExternalLinkage,initial);
		if(varInit->expr != NULL){
			AstValue *v = varInit->expr->codeGen(astContext);
			if(v->castTo(type)){
				throwError(varInit->expr);
			}
			builder.CreateStore(v->llvmValue,llvmVar);
			delete v;
		}
		astContext.addVar(varInit->varName,new AstValue(llvmVar,type));
	}
}