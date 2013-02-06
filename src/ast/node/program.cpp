#include <iostream>

#include "statement.h"

static void classDeclGen(ClassInfo *clazz);

void Program::codeGen(AstContext &astContext) {
	// add clazz
	for (unsigned i = 0; i < classDefs.size(); i++) {
		ClassInfo *clazz = new ClassInfo(classDefs[i]->className, classDefs[i]);
		if (!addClass(clazz)) {
			throwError(classDefs[i]);
		}
	}
	for (unsigned i = 0; i < classDefs.size(); i++) {
		ClassDef *classDef = classDefs[i];
		ClassInfo *clazz = classDef->classInfo;
		if (classDef->superName != "") {
			ClassInfo *superClass = getClass(classDef->superName);
			if (superClass == NULL) {
				throwError(classDef);
			}
			if (superClass == longClass || superClass == doubleClass
					|| superClass == boolClass) {
				errorMsg = "can't extends class " + superClass->name;
				throwError(classDef);
			}
			clazz->superClassInfo = superClass;
		}
	}

	// class decl gen
	for (unsigned i = 0; i < classDefs.size(); i++) {
		classDeclGen(classDefs[i]->classInfo);
	}

	// func decl gen
	for (unsigned i = 0; i < funcDefs.size(); i++) {
		funcDefs[i]->declGen();
	}

	// create main func and global var gen
	FunctionType *mainFuncType = FunctionType::get(builder.getVoidTy(), false);
	mainFunc = Function::Create(mainFuncType, Function::ExternalLinkage, "main",
			&module);
	builder.SetInsertPoint(BasicBlock::Create(context, "entry", mainFunc));
	builder.CreateCall(sysGCinit);
	for (unsigned i = 0; i < varDefs.size(); i++) {
		varDefs[i]->globalGen(astContext);
	}
	string mainStr = "main";
	AFunction mainF = getFunctionV(mainStr);
	if (mainF.llvmFunc == NULL) {
		cout << errorMsg << endl;
	} else {
		builder.CreateCall(mainF.llvmFunc);
	}
	builder.CreateRetVoid();

	// class gen
	for (unsigned i = 0; i < classDefs.size(); i++) {
		classDefs[i]->codeGen(astContext);
	}

	// function gen
	for (unsigned i = 0; i < funcDefs.size(); i++) {
		funcDefs[i]->codeGen(astContext);
	}
}

void classDeclGen(ClassInfo *clazz) {
	if (clazz->status == 2) {
		return;
	}
	clazz->status = 1;
	ClassInfo *superClass = clazz->superClassInfo;
	if (superClass != NULL) {
		if (superClass->status == 1) {
			errorMsg = "cyclic inheritance involving class '" + superClass->name
					+ "'";
			throwError(superClass->classDef);
		} else if (superClass->status == 0) {
			classDeclGen(superClass);
		}
	}
	clazz->classDef->declGen();
	clazz->status = 2;
}
