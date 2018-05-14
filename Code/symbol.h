#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#include "syntaxtree.h"

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;
typedef struct Function_ Function;
typedef struct SyntaxTreeNode SyntaxTreeNode;

#define MAX_SYMBOLTABLE 16384
#define MAX_FUNCTABLE 128
#define MAX_ARGS 16

struct FieldList_
{
	char* name;
	Type type;
	FieldList tail;
};

struct Type_
{
	int isFunction;
	int isParameter;
	enum { Defined, Declared } flag;
	enum { NON_EXIST, ERROR, BASIC, ARRAY, STRUCTURE } kind;
	union
	{
		// 基本类型
		enum { VAlID_, INT_, FLOAT_ } basic;
		// 数组类型信息包括元素类型与数组大小构成
		struct { Type elem; int size; } array;
		// 结构体类型信息是一个链表
		FieldList structure;
	} u;
};

struct Function_
{
	char* name;
	Type returnType;
	int argc;
	FieldList argv[MAX_ARGS];
	int firstline;
};

FieldList symbolTable[MAX_SYMBOLTABLE];
Function funcTable[MAX_FUNCTABLE];
Function calledFuncStack[MAX_FUNCTABLE];
Function currentFunc; //当前所在函数,保存返回类型、参数个数、参数类型，用于填表前的检查
int funcCount;
int stackTop;

void insertSymbolTable(SyntaxTreeNode * ID);
void insertFuncTable(SyntaxTreeNode * root);
int checkArgsEqual(Function f1, Function f2);
int checkTypeEqual(Type t1, Type t2);
int isSymbolExist(char* name);
int isFuncExist(char* name);
Type getTypeByID(SyntaxTreeNode* ID, int flag);
Function getFuncByID(SyntaxTreeNode* ID);
void buildStruct(SyntaxTreeNode * root, FieldList head, FieldList current);
void printtype(Type type);
void addReadAndWrite();
#endif