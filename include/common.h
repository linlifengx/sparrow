#ifndef COMMON_H_
#define COMMON_H_

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <map>
#include <vector>
#include <string>
#include <iostream>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Type.h>

class ClassInfo;
class FunctionInfo;
class AValue;
class AFunction;
class AstContext;
class ClassContext;
class GlobalContext;

class Node;
class Program;
class VarInit;
class SimpleVarDecl;
class TypeDecl;
class ClassDef;
class ClassBody;
class FuncDef;
class FuncDecl;
class Constructor;

class Statement;
class StmtBlock;
class VarDef;
class VarAssi;
class MultiVarAssi;
class SimpleStmtList;
class ExprStmt;
class IfElseStmt;
class ForStmt;
class NullStmt;
class ReturnStmt;
class BreakStmt;
class ContinueStmt;
class SuperInit;
class ArrayAssi;

class Expression;
class LeftValueExpr;
class IdentExpr;
class BinaryOpExpr;
class BinaryLogicExpr;
class PrefixOpExpr;
class FuncInvoke;
class Long;
class Char;
class Double;
class Bool;
class Nil;
class ThisExpr;
class SuperExpr;
class ArrayElement;
class NewArray;
class String;
class ArrayInit;
class DynamicCast;

using namespace std;

#endif
