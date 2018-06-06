#ifndef __OBJECT_CODE_H__
#define __OBJECT_CODE_H__

#include "syntaxtree.h"
#include "ir.h"

typedef struct Variable Variable;
struct Variable{
	enum { OP_TEMP, OP_VARIABLE } kind;
	union {
		char *name;
		int id;
	} u;
	int offset;	     // offset to $fp
	Variable *next;
};

typedef struct {
	Variable *var;
	int valid;
} Reg;

Reg regs[10];
Variable *varList;
int offset2fp;
int offset2sp;  //use to load param from stack

void generate_oc(SyntaxTreeNode *root);
void generate_head();
void genarate_call(char *function_name);
void generate_text(InterCodes* codes);
void generate_prologue();
void generate_epilogue();
void initReg();
int getReg(Operand operand);
void freeReg(int i);
void spillReg(int i);
Variable* allocate(Operand operand);

#endif