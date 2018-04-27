#ifndef __SYNTAXTREENODE_H__
#define __SYNTAXTREENODE_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol.h"

typedef struct Type_* Type;

typedef struct SyntaxTreeNode
{
	//int type;
	char name[16];   
	char content[32]; //ID INT FLOAT's content
//	char* name;
//	char* content;
	int firstline;
	int childrenNum;
	int productionNum;
	struct SyntaxTreeNode* children[16];
	Type type;
}SyntaxTreeNode;

SyntaxTreeNode* treeroot;
int isError;
SyntaxTreeNode* createTreeNode(char* name, char* content, int firstline, int childrenNum);
void output(SyntaxTreeNode* p, int indentlength);

#endif