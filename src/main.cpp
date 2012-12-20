#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/PassManager.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/CodeGen/CommandFlags.h>

#include "ast.hpp"
#include "parser.hpp"

extern Program *program;
extern int yyparse();
extern void createSystemFunctions(AstContext &astContext);
extern FILE *yyin, *yyout;

LLVMContext &context = getGlobalContext();
Module module("test",context);
IRBuilder<> builder(context);
Function *startFunc = NULL;
string errorMsg;

int main(int argc,char **argv){
	bool runJit = false;
	bool irOutput = false;
	bool asmOutput = false;
	bool objOutput = false;
	TargetMachine::CodeGenFileType outputFileType = TargetMachine::CGFT_Null;
	char *outputFileName = NULL;
	int option;
	while((option = getopt(argc,argv,"o:scS")) != -1){
		switch(option){
		case 'o':
			if(outputFileName != NULL){
				cout<<"warning: ignoring '-o "<<optarg<<"' because '-o "
						<<outputFileName<<"' has set before"<<endl;
			}else{
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
		default:
			;
		}
	}
	if(irOutput){
		if(asmOutput){
			cout<<"warning: ignoring '-s' because '-S' has set"<<endl;
		}
		if(objOutput){
			cout<<"warning: ignoring '-c' because '-S' has set"<<endl;
		}
	}else if(asmOutput){
		if(objOutput){
			cout<<"warning: ignoring '-c' because '-s' has set"<<endl;
		}
		outputFileType = TargetMachine::CGFT_AssemblyFile;
	}else if(objOutput){
		outputFileType = TargetMachine::CGFT_ObjectFile;
	}else{
		if(outputFileName != NULL){
			cout<<"warning: ignoring '-o "<<outputFileName
					<<"' because '-s' or '-S' or '-c' has not set"<<endl;
		}
		runJit = true;
	}
	
	char *inputFileName = NULL;
	for(;optind < argc; optind++){
		if(inputFileName == NULL){
			inputFileName = argv[optind];
		}else{
			cout<<"warning: ignoring input file "<<argv[optind]<<endl;
		}
	}
	
	if(inputFileName != NULL){
		yyin = fopen(inputFileName,"r");
		if(yyin == NULL){
			cout<<"can not open file '"<<inputFileName<<"'"<<endl;
			exit(1);
		}
	}
	
	if(yyin == NULL){
		cout<<"input program>>"<<endl;
	}
	yyparse();
	
	if(yyin != NULL){
		fclose(yyin);
	}
	
	AstContext astContext;
	astContext.addType("long",builder.getInt64Ty());
	astContext.addType("double",builder.getDoubleTy());
	astContext.addType("bool",builder.getInt1Ty());
	createSystemFunctions(astContext);
	
	program->codeGen(astContext);
	//module.dump();cout<<endl;
	
	InitializeNativeTarget();
	InitializeAllTargets();
	InitializeAllTargetMCs();
	InitializeAllAsmPrinters();
	InitializeAllAsmParsers();
	
	if(irOutput){
		string opFileName;
		if(outputFileName == NULL){
			if(inputFileName == NULL){
				opFileName = "temp.ir";
			}else{
				opFileName = string(basename(inputFileName)) + ".ir";
			}
		}else{
			opFileName = outputFileName;
		}
		string errorMsg;
		tool_output_file outputFile(opFileName.c_str(),errorMsg);
		if(!errorMsg.empty()){
			cout<<errorMsg<<endl;
			return 1;
		}
		outputFile.os()<<module;
		outputFile.keep();
	}
	
	if(outputFileType != TargetMachine::CGFT_Null){
		Triple triple(sys::getDefaultTargetTriple());
		string errorMsg;
		const Target *target = TargetRegistry::lookupTarget(MArch,triple,errorMsg);
		if(target == NULL){
			cout<<errorMsg<<endl;
			return 1;
		}
		TargetOptions targetOptions;
		TargetMachine *targetMachine = 
				target->createTargetMachine(triple.getTriple(),MCPU,"",targetOptions);
		
		string opFileName;
		if(outputFileName == NULL){
			if(inputFileName == NULL){
				if(asmOutput){
					opFileName = "temp.s";
				}else{
					opFileName = "temp.o";
				}
			}else{
				if(asmOutput){
					opFileName = string(basename(inputFileName)) + ".s";
				}else{
					opFileName = string(basename(inputFileName)) + ".o";
				}
			}
		}else{
			opFileName = outputFileName;
		}
		string errorMsg2;
		tool_output_file *outputFile = new tool_output_file(opFileName.c_str(),errorMsg2);
		if(!errorMsg2.empty()){
			cout<<errorMsg2<<endl;
			return 1;
		}
		PassManager passManager;
		passManager.add(new DataLayout(&module));
		formatted_raw_ostream fos(outputFile->os());
		targetMachine->addPassesToEmitFile(passManager,fos,outputFileType);
		passManager.run(module);
		outputFile->keep();
	}
	
	if(runJit){
		string errStr;
		ExecutionEngine *execEngine = EngineBuilder(&module).setErrorStr(&errStr).setEngineKind(EngineKind::JIT).create();	
		if(execEngine == NULL){
			cout<<"Could not create ExecutionEngine: "<<errStr<<endl;
			exit(1);
		}
		vector<GenericValue> argValues;
		execEngine->runFunction(startFunc,argValues);
	}
	
	return 0;
}

void createSystemFunctions(AstContext &astContext){
	//insert printf func decl
	vector<Type*> printfFuncArgTypes;
	printfFuncArgTypes.push_back(builder.getInt8PtrTy());
	ArrayRef<Type*> printfFuncArgTypesRef(printfFuncArgTypes);
	FunctionType *printfFuncType = FunctionType::get(builder.getInt32Ty(),printfFuncArgTypesRef,true);
	Constant *printfFunc = module.getOrInsertFunction("printf",printfFuncType);
	
	vector<Type*> emptyTypes;
	
	//create print long func
	vector<Type*> printfLongFuncArgTypes;
	printfLongFuncArgTypes.push_back(builder.getInt64Ty());
	ArrayRef<Type*> printfLongFuncArgTypesRef(printfLongFuncArgTypes);
	FunctionType *printfLongFuncType = FunctionType::get(builder.getVoidTy(),printfLongFuncArgTypesRef,false);
	Function *printfLongFunc = Function::Create(printfLongFuncType,Function::ExternalLinkage,"printL",&module);
	builder.SetInsertPoint(BasicBlock::Create(context,"entry",printfLongFunc));
	Value *longFormat = builder.CreateGlobalStringPtr("%ld");
	builder.CreateCall2(printfFunc,longFormat,printfLongFunc->arg_begin());
	builder.CreateRetVoid();
	MyFunction *printfL = new MyFunction("printL",printfLongFunc,emptyTypes,printfLongFuncArgTypes);
	
	//create print double func
	vector<Type*> printfDoubleFuncArgTypes;
	printfDoubleFuncArgTypes.push_back(builder.getDoubleTy());
	ArrayRef<Type*> printfDoubleFuncArgTypesRef(printfDoubleFuncArgTypes);
	FunctionType *printfDoubleFuncType = FunctionType::get(builder.getVoidTy(),printfDoubleFuncArgTypesRef,false);
	Function *printfDoubleFunc = Function::Create(printfDoubleFuncType,Function::ExternalLinkage,"printD",&module);
	builder.SetInsertPoint(BasicBlock::Create(context,"entry",printfDoubleFunc));
	Value *doubleFormat = builder.CreateGlobalStringPtr("%lf");
	builder.CreateCall2(printfFunc,doubleFormat,printfDoubleFunc->arg_begin());
	builder.CreateRetVoid();
	MyFunction *printfD = new MyFunction("printD",printfDoubleFunc,emptyTypes,printfDoubleFuncArgTypes);
	
	//create print bool func
	vector<Type*> printfBoolFuncArgTypes;
	printfBoolFuncArgTypes.push_back(builder.getInt1Ty());
	ArrayRef<Type*> printfBoolFuncArgTypesRef(printfBoolFuncArgTypes);
	FunctionType *printfBoolFuncType = FunctionType::get(builder.getVoidTy(),printfBoolFuncArgTypesRef,false);
	Function *printfBoolFunc = Function::Create(printfBoolFuncType,Function::ExternalLinkage,"printB",&module);
	builder.SetInsertPoint(BasicBlock::Create(context,"entry",printfBoolFunc));
	BasicBlock *trueBB = BasicBlock::Create(context,"true",printfBoolFunc);
	BasicBlock *falseBB = BasicBlock::Create(context,"false",printfBoolFunc);
	builder.CreateCondBr(printfBoolFunc->arg_begin(),trueBB,falseBB);
	builder.SetInsertPoint(trueBB);
	Value *trueStr = builder.CreateGlobalStringPtr("true");
	builder.CreateCall(printfFunc,trueStr);
	builder.CreateRetVoid();
	builder.SetInsertPoint(falseBB);
	Value *falseStr = builder.CreateGlobalStringPtr("false");
	builder.CreateCall(printfFunc,falseStr);
	builder.CreateRetVoid();
	MyFunction *printfB = new MyFunction("printB",printfBoolFunc,emptyTypes,printfBoolFuncArgTypes);
	
	//create println func
	FunctionType *printlnFuncType = FunctionType::get(builder.getVoidTy(),false);
	Function *printlnFunc = Function::Create(printlnFuncType,Function::ExternalLinkage,"println",&module);
	builder.SetInsertPoint(BasicBlock::Create(context,"entry",printlnFunc));
	Value *lnFormat = builder.CreateGlobalStringPtr("\n");
	builder.CreateCall(printfFunc,lnFormat);
	builder.CreateRetVoid();
	MyFunction *println = new MyFunction("println",printlnFunc,emptyTypes,emptyTypes);
	
	//astContext.addFunction("printf",cast<Function>(printfFunc));
	astContext.addFunction("printL",printfL);
	astContext.addFunction("printD",printfD);
	astContext.addFunction("printB",printfB);
	astContext.addFunction("println",println);
}
