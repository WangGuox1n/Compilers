#include "syntaxtree.h"

SyntaxTreeNode* createTreeNode(char* name, char* content, int firstline, int childrenNum)
{
	SyntaxTreeNode* treenode = (SyntaxTreeNode*)malloc(sizeof(SyntaxTreeNode));
	treenode->type = (Type)malloc(sizeof(struct Type_));
	treenode->type->isFunction = 0;
	treenode->type->isParameter = 0;
	strcpy(treenode->name, name);

	if (strcmp(name, "ID") == 0)
		strcpy(treenode->content, content);
	else if (strcmp(name, "INT") == 0)
	{
		strcpy(treenode->content, content);
		treenode->type = (Type)malloc(sizeof(struct Type_));
		treenode->type->kind = BASIC;
		treenode->type->u.basic = INT_;
		//printtype(treenode->type);
	}
	else if (strcmp(name, "FLOAT") == 0)
	{
		strcpy(treenode->content, content);
		treenode->type = (Type)malloc(sizeof(struct Type_));
		treenode->type->kind = BASIC;
		treenode->type->u.basic = FLOAT_;
	}
	else if (strcmp(name, "TYPE") == 0)
	{
		strcpy(treenode->content, content);
		treenode->type = (Type)malloc(sizeof(struct Type_));
		treenode->type->kind = BASIC;
		if (strcmp(content, "int") == 0)
			treenode->type->u.basic = INT_;
		else
			treenode->type->u.basic = FLOAT_;
		//printtype(treenode->type);
	}

	treenode->firstline = firstline;
	treenode->childrenNum = childrenNum;

	for (int i = 0; i < 16; ++i)
		treenode->children[i] = NULL;
	return treenode;
}
int hex2dec(char * content)
{
	int num = 0;
	for (int i = 2; content[i] != '\0'; i++)
	{
		num *= 16;
		if (content[i] == 'a' || content[i] == 'A')
			num += 10;
		else if (content[i] == 'b' || content[i] == 'B')
			num += 11;
		else if (content[i] == 'c' || content[i] == 'C')
			num += 12;
		else if (content[i] == 'd' || content[i] == 'D')
			num += 13;
		else if (content[i] == 'e' || content[i] == 'E')
			num += 14;
		else if (content[i] == 'f' || content[i] == 'F')
			num += 15;
		else
			num += content[i] - '0';
	}
	return num;
}
int oct2dec(char *content)
{
	int num = 0;
	for (int i = 1; content[i] != '\0'; i++)
	{
		num *= 8;
		num += content[i] - '0';
	}
	return num;
}

void output(SyntaxTreeNode* p, int indentlength)
{
	//printtype(p->type);
	if (p == NULL)
		return;

	for (int i = 0; i < indentlength; ++i)
		printf(" ");
	printf("%d ",p->type->isParameter);
	if (p->childrenNum > 0) /* non-teminal sysbol*/
	{
		printf("%s (%d)\n", p->name, p->firstline);

		for (int i = 0; i < p->childrenNum; ++i)
			output(p->children[i], indentlength + 2);
	}
	else /* tokens */
	{
		if (strcmp(p->name, "ID") == 0 || strcmp(p->name, "TYPE") == 0)
		{
			printf("%s: %s\n", p->name, p->content);
		}
		else if (strcmp(p->name, "INT") == 0)
		{
			int num = atoi(p->content);;
			printf("%s: %d\n", p->name, num);
		}
		else if (strcmp(p->name, "FLOAT") == 0)
		{
			float num = atof(p->content);
			printf("%s: %f\n", p->name, num);
		}
		else
		{
			printf("%s\n", p->name);
		}
	}
}