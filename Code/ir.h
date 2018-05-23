#ifndef __IR_H__
#define __IR_H__

#include "symbol.h"

typedef struct Operand_ Operand;
typedef struct InterCode InterCode;
typedef struct InterCodes_ InterCodes;
typedef struct ArgList ArgList;

enum RELOP_kind { LT, LE, EQ, NE, GE, GT };
struct Operand_
{
	enum { TEMP, VARIABLE, CONSTANT, FUNCTION, LABEL} kind;
	union {
		char *name;    //VARIABLE'name  FUNCTION'name
		int value;     //CONSTANT's value
		int temp_id;   //TEMP's id
		int label_id;  //LABEL's id
	} u;
};

struct InterCode {
	//Three address codes
	enum {
		IC_LABEL,   // LABEL result :
		IC_FUNC,    // FUNCTION result :
		IC_ASSIGN,  // result := arg1
		IC_PLUS,    // result := arg1 + arg2
		IC_MINUS,   // result := arg1 - arg2
		IC_MUL,     // result := arg1 * arg2
		IC_DIV,     // result := arg1 / arg2

		IC_ADDR,    // result  := &arg1
		IC_DEREF_R, // result  := *arg1, dereference at right
		IC_DEREF_L, // *result :=  arg1, dereference at left

		IC_GOTO,    // GOTO result
		IC_RELOP,   // IF arg1 [relop] arg2 GOTO result
		IC_RETURN,  // RETURN result
		IC_DEC,     // DEC result [size]
		IC_ARG,     // ARG result
		IC_CALL,    // result := CALL arg1
		IC_PARAM,   // PARAM result

		IC_READ,    // READ result
		IC_WRITE    // WRITE result
	} kind;

	enum RELOP_kind relop;
	struct Operand_ result, arg1, arg2;
};

struct InterCodes_ {
	int count;
	InterCode code;
	struct InterCodes_ *prev,*next;
};

struct ArgList{
	int count;
	int id[MAX_ARGS];
};

int tempId;
int labelId;

FieldList lookupSymbol(SyntaxTreeNode *ID);
void gen_InterCode(SyntaxTreeNode *root);
void printOperand(Operand operand);
InterCodes* newInterCodes();

InterCodes* translate_Exp(SyntaxTreeNode *Exp, int place);
InterCodes* translate_Stmt(SyntaxTreeNode *Stmt);
InterCodes* translate_StmtList(SyntaxTreeNode *StmtList);
InterCodes* translate_CompSt(SyntaxTreeNode *CompSt);
InterCodes* translate_Cond(SyntaxTreeNode *Exp, int label_true, int label_false);
InterCodes* translate_Args(SyntaxTreeNode *Arg, ArgList* arglist);
InterCodes* translate_Program(SyntaxTreeNode *Program);
InterCodes* translate_ExtDefList(SyntaxTreeNode *ExtDefList);
InterCodes* translate_ExtDef(SyntaxTreeNode *ExtDef);
InterCodes* translate_ExtDecList(SyntaxTreeNode *ExtDecList);
InterCodes* translate_VarDec(SyntaxTreeNode *VarDec);
InterCodes* translate_FunDec(SyntaxTreeNode *FunDec);
InterCodes* translate_VarList(SyntaxTreeNode *VarList);
InterCodes* translate_ParamDec(SyntaxTreeNode *ParamDec);
InterCodes* translate_DefList(SyntaxTreeNode *DefList);
InterCodes* translate_Def(SyntaxTreeNode *Def);
InterCodes* translate_DecList(SyntaxTreeNode *DecList);
InterCodes* translate_Dec(SyntaxTreeNode *Dec);
InterCodes* translate_DEREF_L(SyntaxTreeNode *Exp,int place);
InterCodes* translate_ADDR(SyntaxTreeNode *Exp,int place);

#define Map_Max 5
struct Constant_Map
{
	int value;
	int temp_id;
}constant_map[Map_Max];

struct Variable_Map
{
	char* name;
	int temp_id;
}variable_map[Map_Max];

int constant_count;
int variable_count;
void code_optimization(InterCodes* codes);
void replace_constant(InterCodes* codes);
void replace_variable(InterCodes* codes);
InterCodes* deleteCode(InterCodes* codes);
#endif