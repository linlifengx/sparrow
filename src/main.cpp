#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Value.h>
#include <llvm/Type.h>
#include <llvm/Function.h>
#include <llvm/BasicBlock.h>
#include <llvm/IRBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/PassManager.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/CodeGen/CommandFlags.h>

#include "statement.h"
#include "expression.h"
#include "parser.hpp"

extern int yyparse();
extern FILE *yyin;

LLVMContext &context = getGlobalContext();
Module module("test", context);
IRBuilder<> builder(context);

map<string, ClassInfo*> classTable;
map<string, FunctionInfo*> functionTable;
Type *ptrType = NULL;
Type *ptrptrType = NULL;
Type *int64Type = NULL;
Type *doubleType = NULL;
Type *boolType = NULL;
Type *voidType = NULL;
ClassInfo *longClass = NULL;
ClassInfo *doubleClass = NULL;
ClassInfo *boolClass = NULL;
ClassInfo *nilClass = NULL;
ClassInfo *voidClass = NULL;

Constant *int64_0 = NULL;
Constant *double_0 = NULL;
Constant *bool_true = NULL;
Constant *bool_false = NULL;
Constant *ptr_null = NULL;

Constant *sysGCinit = NULL;
Constant *sysObjectField = NULL;
Constant *sysObjectAlloca = NULL;
Constant *sysObjectMethod = NULL;
Function *mainFunc = NULL;

string errorMsg;
Function *startFunc = NULL;
Program *program = NULL;

static void createSystemFunctions(AstContext &astContext);
static void initGlobals();

int main(int argc, char **argv) {
	bool irOutput = false;
	bool asmOutput = false;
	bool objOutput = false;
	TargetMachine::CodeGenFileType outputFileType = TargetMachine::CGFT_Null;
	char *outputFileName = NULL;
	int option;
	while ((option = getopt(argc, argv, "o:scS")) != -1) {
		switch (option) {
		case 'o':
			if (outputFileName != NULL) {
				cout << "warning: ignoring '-o " << optarg << "' because '-o "
						<< outputFileName << "' has set before" << endl;
			} else {
				outputFileName = optarg;
			}
			break;
		case 's':
			asmOutput = true;
			break;
		case 'c':
			objOutput = true;
			break;
		case 'S':
			irOutput = true;
			break;
		}
	}
	if (irOutput) {
		if (asmOutput) {
			cout << "warning: ignoring '-s' because '-S' has set" << endl;
		}
		if (objOutput) {
			cout << "warning: ignoring '-c' because '-S' has set" << endl;
		}
	} else if (asmOutput) {
		if (objOutput) {
			cout << "warning: ignoring '-c' because '-s' has set" << endl;
		}
		outputFileType = TargetMachine::CGFT_AssemblyFile;
	} else {
		outputFileType = TargetMachine::CGFT_ObjectFile;
		objOutput = true;
	}

	char *inputFileName = NULL;
	for (; optind < argc; optind++) {
		if (inputFileName == NULL) {
			inputFileName = argv[optind];
		} else {
			cout << "warning: ignoring input file " << argv[optind] << endl;
		}
	}

	if (inputFileName != NULL) {
		yyin = fopen(inputFileName, "r");
		if (yyin == NULL) {
			cout << "can not open file '" << inputFileName << "'" << endl;
			exit(1);
		}
	}

	if (yyin == NULL) {
		cout << "input program>>" << endl;
	}
	yyparse();

	if (yyin != NULL) {
		fclose(yyin);
	}

	initGlobals();
	AstContext astContext;
	createSystemFunctions(astContext);

	program->codeGen(astContext);
	//module.dump();cout<<endl;

	InitializeNativeTarget();
	InitializeAllTargets();
	InitializeAllTargetMCs();
	InitializeAllAsmPrinters();
	InitializeAllAsmParsers();

	if (irOutput) {
		string opFileName;
		if (outputFileName == NULL) {
			if (inputFileName == NULL) {
				opFileName = "temp.ir";
			} else {
				opFileName = string(basename(inputFileName)) + ".ir";
			}
		} else {
			opFileName = outputFileName;
		}
		string errorMsg;
		tool_output_file outputFile(opFileName.c_str(), errorMsg);
		if (!errorMsg.empty()) {
			cout << errorMsg << endl;
			return 1;
		}
		outputFile.os() << module;
		outputFile.keep();
	} else {
		string errorStr;
		const Target *target = TargetRegistry::lookupTarget(
				sys::getDefaultTargetTriple(), errorStr);
		if (target == NULL) {
			cout << errorStr << endl;
			return 1;
		}
		TargetOptions targetOptions;
		TargetMachine *targetMachine = target->createTargetMachine(
				sys::getDefaultTargetTriple(), sys::getHostCPUName(), "",
				targetOptions);

		string opFileName;
		if (outputFileName == NULL) {
			if (inputFileName == NULL) {
				if (asmOutput) {
					opFileName = "temp.s";
				} else {
					opFileName = "temp.o";
				}
			} else {
				if (asmOutput) {
					opFileName = string(basename(inputFileName)) + ".s";
				} else {
					opFileName = string(basename(inputFileName)) + ".o";
				}
			}
		} else {
			opFileName = outputFileName;
		}
		string errorStr2;
		tool_output_file *outputFile = new tool_output_file(opFileName.c_str(),
				errorStr2);
		if (!errorStr2.empty()) {
			cout << errorStr2 << endl;
			return 1;
		}
		PassManager passManager;
		passManager.add(new DataLayout(&module));
		formatted_raw_ostream fos(outputFile->os());
		targetMachine->addPassesToEmitFile(passManager, fos, outputFileType);
		passManager.run(module);
		outputFile->keep();
	}

	return 0;
}

void initGlobals() {
	ptrType = builder.getInt8PtrTy();
	ptrptrType = PointerType::get(ptrType, 0);
	int64Type = builder.getInt64Ty();
	doubleType = builder.getDoubleTy();
	boolType = builder.getInt1Ty();
	voidType = builder.getVoidTy();

	int64_0 = ConstantInt::getSigned(int64Type, 0);
	double_0 = ConstantFP::get(doubleType, 0);
	bool_true = builder.getInt1(true);
	bool_false = builder.getInt1(false);
	ptr_null = GlobalValue::getNullValue(ptrType);

	longClass = new ClassInfo("long", NULL, int64Type);
	doubleClass = new ClassInfo("double", NULL, doubleType);
	boolClass = new ClassInfo("bool", NULL, boolType);
	nilClass = new ClassInfo("null", NULL, ptrType);
	voidClass = new ClassInfo("void");

	addClass(longClass);
	addClass(doubleClass);
	addClass(boolClass);
	addClass(nilClass);
	addClass(voidClass);

	vector<Type*> argTypes;
	FunctionType *funcType = NULL;

	funcType = FunctionType::get(voidType, false);
	sysGCinit = module.getOrInsertFunction("gcInit", funcType);

	argTypes.push_back(ptrType);
	argTypes.push_back(ptrType);
	funcType = FunctionType::get(ptrType, ArrayRef<Type*>(argTypes), false);
	sysObjectField = module.getOrInsertFunction("sysObjectField", funcType);

	sysObjectMethod = module.getOrInsertFunction("sysObjectMethod", funcType);

	argTypes.clear();
	argTypes.push_back(ptrType);
	funcType = FunctionType::get(ptrType, ArrayRef<Type*>(argTypes), false);
	sysObjectAlloca = module.getOrInsertFunction("sysObjectAlloca", funcType);
}

void createSystemFunctions(AstContext &astContext) {
	vector<Type*> argllvmTypes;
	vector<ClassInfo*> argClasses;
	vector<ClassInfo*> emptyClasses;

	//create print long func
	argllvmTypes.push_back(int64Type);
	argClasses.push_back(longClass);
	ArrayRef<Type*> printfLongArgTypesRef(argllvmTypes);
	FunctionType *printfLongFuncType = FunctionType::get(voidType,
			printfLongArgTypesRef, false);
	Function *printfLongFunc =
			static_cast<Function*>(module.getOrInsertFunction("printL",
					printfLongFuncType));
	FunctionInfo *printfL = new FunctionInfo("printL", printfLongFunc,
			emptyClasses, argClasses);

	//create print double func
	argllvmTypes.clear();
	argClasses.clear();
	argllvmTypes.push_back(doubleType);
	argClasses.push_back(doubleClass);
	ArrayRef<Type*> printfDoubleArgTypesRef(argllvmTypes);
	FunctionType *printfDoubleFuncType = FunctionType::get(voidType,
			printfDoubleArgTypesRef, false);
	Function *printfDoubleFunc =
			static_cast<Function*>(module.getOrInsertFunction("printD",
					printfDoubleFuncType));
	FunctionInfo *printfD = new FunctionInfo("printD", printfDoubleFunc,
			emptyClasses, argClasses);

	//create print bool func
	argllvmTypes.clear();
	argClasses.clear();
	argllvmTypes.push_back(boolType);
	argClasses.push_back(boolClass);
	ArrayRef<Type*> printfBoolArgTypesRef(argllvmTypes);
	FunctionType *printfBoolFuncType = FunctionType::get(builder.getVoidTy(),
			printfBoolArgTypesRef, false);
	Function *printfBoolFunc =
			static_cast<Function*>(module.getOrInsertFunction("printB",
					printfBoolFuncType));
	FunctionInfo *printfB = new FunctionInfo("printB", printfBoolFunc,
			emptyClasses, argClasses);

	//create println func
	argllvmTypes.clear();
	argClasses.clear();
	FunctionType *printlnFuncType = FunctionType::get(builder.getVoidTy(),
			false);
	Function *printlnFunc = static_cast<Function*>(module.getOrInsertFunction(
			"println", printlnFuncType));
	FunctionInfo *println = new FunctionInfo("println", printlnFunc,
			emptyClasses, emptyClasses);
	argllvmTypes.clear();
	argClasses.clear();

	addFunction(printfL);
	addFunction(printfD);
	addFunction(printfB);
	addFunction(println);
}

