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

void generate_oc(FILE* fout,SyntaxTreeNode *root);
void generate_head(FILE* fout);
void genarate_call(FILE* fout,char *function_name);
void generate_text(FILE* fout,InterCodes* codes);
void generate_prologue(FILE* fout);
void generate_epilogue(FILE* fout);
void initReg();
int getReg(FILE* fout,Operand operand);
void freeReg(int i);
void spillReg(FILE* fout,int i);
Variable* allocate(FILE* fout,Operand operand);

#endif