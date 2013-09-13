#include <llvm/IR/GlobalVariable.h>

#include "statement.h"
#include "support.h"

using namespace llvm;

static Constant *createGlobalStrPtr(string &str);

void ClassDef::declGen() {
	vector<Constant*> fieldNameStrs;
	for (unsigned i = 0; i < body->fieldDefs.size(); i++) {
		VarDef *fieldDef = body->fieldDefs[i];
		ClassInfo *fieldClass = fieldDef->typeDecl->getClassInfo();
		if (fieldClass == NULL) {
			throwError(fieldDef);
		}
		for (unsigned j = 0; j < fieldDef->varInitList.size(); j++) {
			VarInit *fieldInitDecl = fieldDef->varInitList[j];
			if (!classInfo->addField(fieldInitDecl->varName, fieldClass)) {
				throwError(fieldInitDecl);
			} else {
				fieldNameStrs.push_back(
						createGlobalStrPtr(fieldInitDecl->varName));
			}
		}
	}
	ArrayType *arrayType = ArrayType::get(ptrType, fieldNameStrs.size());
	ArrayRef<Constant*> fieldNameStrsRef(fieldNameStrs);
	GlobalVariable *fieldNameTable = new GlobalVariable(module, arrayType, true,
			GlobalValue::ExternalLinkage,
			ConstantArray::get(arrayType, fieldNameStrsRef));

	vector<Constant*> methodNameStrs;
	vector<Constant*> methodPtrs;
	for (unsigned i = 0; i < body->methodDefs.size(); i++) {
		methodNameStrs.push_back(
				createGlobalStrPtr(body->methodDefs[i]->funcDecl->funcName));
		Function *method = body->methodDefs[i]->declGen(classInfo);
		methodPtrs.push_back(
				static_cast<Constant*>(builder.CreateBitCast(method, ptrType)));
	}

	arrayType = ArrayType::get(ptrType, methodNameStrs.size());
	ArrayRef<Constant*> methodNameStrsRef(methodNameStrs);
	GlobalVariable *methodNameTable = new GlobalVariable(module, arrayType,
			true, GlobalValue::ExternalLinkage,
			ConstantArray::get(arrayType, methodNameStrsRef));

	arrayType = ArrayType::get(ptrType, methodPtrs.size());
	ArrayRef<Constant*> methodPtrsRef(methodPtrs);
	GlobalVariable *methodTable = new GlobalVariable(module, arrayType, true,
			GlobalValue::ExternalLinkage,
			ConstantArray::get(arrayType, methodPtrsRef));

	Constant *nameStr = createGlobalStrPtr(classInfo->name);

	arrayType = ArrayType::get(ptrType, 7);
	Constant *super = NULL;
	if (superName == "") {
		super = ptr_null;
	} else {
		super = static_cast<Constant*>(builder.CreateBitCast(
				module.getOrInsertGlobal("$" + superName, arrayType), ptrType));
	}

	vector<Constant*> infos;
	infos.push_back(nameStr);
	infos.push_back(super);
	infos.push_back(
			static_cast<Constant*>(builder.CreateBitCast(fieldNameTable,
					ptrType)));
	infos.push_back(
			static_cast<Constant*>(builder.CreateBitCast(methodNameTable,
					ptrType)));
	infos.push_back(
			static_cast<Constant*>(builder.CreateBitCast(methodTable, ptrType)));
	infos.push_back(
			static_cast<Constant*>(builder.CreateIntToPtr(
					builder.getInt64(fieldNameStrs.size()), ptrType)));
	infos.push_back(
			static_cast<Constant*>(builder.CreateIntToPtr(
					builder.getInt64(methodNameStrs.size()), ptrType)));

	ArrayRef<Constant*> infosRef(infos);
	classInfo->info = builder.CreateBitCast(
			new GlobalVariable(module, arrayType, true,
					GlobalValue::ExternalLinkage,
					ConstantArray::get(arrayType, infosRef), "$" + className),
			ptrType);

	//init decl gen
	vector<Type*> argllvmTypes;
	argllvmTypes.push_back(classInfo->llvmType);
	if (classInfo->superClassInfo != NULL) {
		argllvmTypes.push_back(classInfo->superClassInfo->llvmType);
	}
	ArrayRef<Type*> argTypesRef(argllvmTypes);
	FunctionType *initFuncType = FunctionType::get(voidType, argTypesRef,
			false);
	Function *initFunc = Function::Create(initFuncType,
			Function::ExternalLinkage, "", &module);
	classInfo->initor = initFunc;

	//constructor decl gen
	if (body->constructors.size() > 0) {
		for (unsigned i = 0; i < body->constructors.size(); i++) {
			body->constructors[i]->declGen(*classInfo);
		}
	} else {
		vector<ClassInfo*> returnClasses;
		returnClasses.push_back(classInfo);
		vector<ClassInfo*> argClasses;
		FunctionType *constructorType = FunctionType::get(classInfo->llvmType,
				false);
		Function *constructor = Function::Create(constructorType,
				Function::ExternalLinkage, "", &module);
		FunctionInfo *constructorInfo = new FunctionInfo("new$" + className,
				constructor, returnClasses, argClasses, 0, classInfo);
		classInfo->constructor = constructorInfo;
	}
}

void ClassDef::codeGen() {
	for (unsigned i = 0; i < body->methodDefs.size(); i++) {
		body->methodDefs[i]->codeGen();
	}
	for (unsigned i = 0; i < body->constructors.size(); i++) {
		body->constructors[i]->codeGen();
	}

	// default constructor gen
	if (body->constructors.size() == 0) {
		builder.SetInsertPoint(
				BasicBlock::Create(context, "entry",
						classInfo->constructor->llvmFunction));
		Value *thisObject = NULL;
		if (classInfo->superClassInfo != NULL) {
			FunctionInfo *superCstor = classInfo->superClassInfo->constructor;
			if (superCstor->argClasses.size() > 0) {
				errorMsg = "constructor for '" + classInfo->superClassInfo->name
						+ "' require some args";
				throwError(this);
			}
			Value *superObject = builder.CreateCall(superCstor->llvmFunction);
			thisObject = builder.CreateCall(sysObjectAlloca, classInfo->info);
			Value *superElementPtr = builder.CreateGEP(thisObject,
					builder.getInt32(8));
			superElementPtr = builder.CreateBitCast(superElementPtr,
					ptrptrType);
			builder.CreateStore(superObject, superElementPtr);
			builder.CreateCall2(classInfo->initor, thisObject, superObject);
		} else {
			thisObject = builder.CreateCall(sysObjectAlloca, classInfo->info);
			builder.CreateCall(classInfo->initor, thisObject);
		}
		builder.CreateRet(thisObject);
	}

	//initor gen
	AstContext astContext;
	builder.SetInsertPoint(
			BasicBlock::Create(context, "entry", classInfo->initor));
	unsigned i = 0;
	Function::arg_iterator ai = classInfo->initor->arg_begin();
	ClassContext classContext(classInfo, NULL, ai);
	if (classInfo->superClassInfo != NULL) {
		ai++;
		classContext.superObject = ai;
	}
	astContext.classContext = &classContext;

	for (unsigned i = 0; i < body->fieldDefs.size(); i++) {
		body->fieldDefs[i]->fieldGen(astContext);
	}
	builder.CreateRetVoid();
}

static Constant *createGlobalStrPtr(string &str) {
	Constant *charArray = ConstantDataArray::getString(context, str);
	GlobalVariable *globalVar = new GlobalVariable(module, charArray->getType(),
			true, GlobalValue::PrivateLinkage, charArray);
	Value *zero = builder.getInt32(0);
	Value *idxs[] = { zero, zero };
	Value *strPtr = builder.CreateInBoundsGEP(globalVar, idxs);
	return static_cast<Constant*>(strPtr);
}
