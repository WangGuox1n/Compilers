#ifndef __IR_H__
#define __IR_H__

#include "symbol.h"

typedef struct Operand_ Operand;
typedef struct InterCode InterCode;
typedef struct InterCodes_ InterCodes;

struct Operand_
{
	enum { VARIABLE, CONSTANT, ADDRESS, FUNCTION} kind;
	union {
		char *name;
		int value;
		int var_id;
		int label_id;
	} u;
};

struct InterCode {
	enum {
		IC_LABEL,   // LABEL result :
		IC_FUNC,    // FUNCTION result :
		IC_ASSIGN,  // result := arg1
		IC_ADD,     // result := arg1 + arg2
		IC_SUB,     // result := arg1 - arg2
		IC_MUL,     // result := arg1 * arg2
		IC_DIV,     // result := arg1 / arg2

		IC_ADDR,    // result  := &arg1
		IC_DEREF_R, // result  := *arg1, dereference at right :-)
		IC_DEREF_L, // *result :=  arg1, dereference at left :-)

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

	struct Operand_ result, arg1, art2;
};

struct InterCodes_ {
	InterCode code;
	struct InterCodes_ *prev,*next;
};

int tempId = 0;
int labelId = 0;

void gen_InterCode(SyntaxTreeNode *root);

InterCodes* newInterCode();

InterCodes* translate_Exp(SyntaxTreeNode *Exp, int place);
InterCodes* translate_Stmt(SyntaxTreeNode *Stmt);
InterCodes* translate_StmtList(SyntaxTreeNode *StmtList);
InterCodes* translate_CompSt(SyntaxTreeNode *CompSt);
InterCodes* translate_Cond(SyntaxTreeNode *Exp, int label_true, int label_false);
InterCodes* translate_Args(SyntaxTreeNode *Args, SyntaxTreeNode** arg_list);
InterCodes* translate_Program(SyntaxTreeNode *Program);
InterCodes* translate_ExtDefList(SyntaxTreeNode *ExtDefList);
InterCodes* translate_ExtDef(SyntaxTreeNode *ExtDef);
InterCodes* translate_ExtDecList(SyntaxTreeNode *ExtDecList);
InterCodes* translate_VarDec(SyntaxTreeNode *VarDec);
InterCodes* translate_FunDec(SyntaxTreeNode *FunDec);
InterCodes* translate_VarList(SyntaxTreeNode *VarList);
InterCodes* translate_ParamDec(SyntaxTreeNode *ParamDec);


#endif