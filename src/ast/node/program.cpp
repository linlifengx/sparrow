#include "statement.h"
#include "support.h"

static void classDeclGen(ClassInfo *clazz);

void Program::codeGen() {
	// add clazz
	for (unsigned i = 0; i < classDefs.size(); i++) {
		ClassInfo *clazz = new ClassInfo(classDefs[i]->className, classDefs[i]);
		if (!globalContext.addClass(clazz)) {
			throwError(classDefs[i]);
		}
	}
	for (unsigned i = 0; i < classDefs.size(); i++) {
		ClassDef *classDef = classDefs[i];
		ClassInfo *clazz = classDef->classInfo;
		if (classDef->superName != "") {
			ClassInfo *superClass = globalContext.getClass(classDef->superName);
			if (superClass == NULL) {
				throwError(classDef);
			}
			if (superClass->isLongType() || superClass->isDoubleType()
					|| superClass->isBoolType() || superClass->isCharType()) {
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
	vector<Type*> mainArgs;
	mainArgs.push_back(ptrType);
	FunctionType *mainFuncType = FunctionType::get(int64Type,
			ArrayRef<Type*>(mainArgs), false);
	mainFunc = Function::Create(mainFuncType, Function::ExternalLinkage,
			"start_program", &module);
	builder.SetInsertPoint(BasicBlock::Create(context, "entry", mainFunc));
	for (unsigned i = 0; i < varDefs.size(); i++) {
		varDefs[i]->globalGen();
	}
	string mainStr = "main";
	AFunction mainF = globalContext.getFunctionV(mainStr);
	Function *mainf = (Function*) mainF.llvmFunc;
	if (mainf == NULL) {
		cout << errorMsg << endl;
		builder.CreateRet(int64_0);
	} else {
		builder.CreateRet(builder.CreateCall(mainf, mainFunc->arg_begin()));
	}

	// class gen
	for (unsigned i = 0; i < classDefs.size(); i++) {
		classDefs[i]->codeGen();
	}

	// function gen
	for (unsigned i = 0; i < funcDefs.size(); i++) {
		funcDefs[i]->codeGen();
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

ClassInfo* TypeDecl::getClassInfo() {
	ClassInfo *classInfo = globalContext.getClass(typeName);
	if (classInfo == NULL) {
		throwError(this);
	}

	for (unsigned i = 0; i < dimension; i++) {
		classInfo = classInfo->getArrayClass();
	}
	return classInfo;
}
