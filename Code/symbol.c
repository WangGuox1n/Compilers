#include "symbol.h"
#include "syntaxtree.h"
#include <string.h>
#include <assert.h>
unsigned int hash_pjw(char* name)
{
	unsigned int val = 0, i;
	for (; *name; ++name)
	{
		val = (val << 2) + *name;
		if (i == val & ~MAX_SYMBOLTABLE)
			val = (val ^ (i >> 12)) & MAX_SYMBOLTABLE;
	}
	return val % MAX_SYMBOLTABLE;
}

void traverseTree(SyntaxTreeNode* root)
{
	if (root == NULL || root->childrenNum == 0)
		return;
	if (strcmp(root->name, "Program") == 0)
	{
		traverseTree(root->children[0]);
		for (int i = 0; i < funcCount; i++)
			if (funcTable[i].returnType->flag == Declared)
				printf("Error type 18 at Line %d: Undefined function \"%s\".\n", funcTable[i].firstline, funcTable[i].name);
	}
	else if (strcmp(root->name, "ExtDefList") == 0)
	{
		if (root->productionNum == 0)
		{
			traverseTree(root->children[0]);
			traverseTree(root->children[1]);
		}
	}
	else if (strcmp(root->name, "ExtDef") == 0)
	{
		if (root->productionNum == 0)
		{
			traverseTree(root->children[0]);
			root->type = root->children[0]->type;
			root->children[1]->type = root->type;
			traverseTree(root->children[1]);
		}
		else if (root->productionNum == 1)
		{
			traverseTree(root->children[0]);
			root->type = root->children[0]->type;
		}
		else if (root->productionNum == 2)
		{
			traverseTree(root->children[0]);
			root->type = root->children[0]->type;
			root->type->isFunction = 1;
			root->type->flag = Defined;
			root->children[1]->type = root->type;
			currentFunc.returnType = root->type;
			if (isFuncExist(root->children[1]->children[0]->content))
			{
				printf("Error type 4 at line %d: redefinition of function \"%s\".\n", root->firstline, root->children[1]->children[0]->content);
				return;
			}
			traverseTree(root->children[1]);
			traverseTree(root->children[2]);
		}
		else if (root->productionNum == 3)
		{
			traverseTree(root->children[0]);
			root->type = root->children[0]->type;
			root->type->isFunction = 1;
			root->type->flag = Declared;
			root->children[1]->type = root->type;
			currentFunc.returnType = root->type;
			traverseTree(root->children[1]);
		}
	}
	else if (strcmp(root->name, "ExtDecList") == 0)
	{
		root->children[0]->type = root->type;
		traverseTree(root->children[0]);
		if (root->productionNum == 1)
		{
			root->children[2]->type = root->type;
			traverseTree(root->children[2]);
		}
	}
	else if (strcmp(root->name, "Specifier") == 0)
	{
		if (root->productionNum == 0)
		{
			root->type = root->children[0]->type;
		}
		else if (root->productionNum == 1)
		{
			traverseTree(root->children[0]);
			root->type = root->children[0]->type;
		}
	}
	else if (strcmp(root->name, "StructSpecifier") == 0)
	{
		if (root->productionNum == 0)
		{
			traverseTree(root->children[3]);
			root->type->kind = STRUCTURE;
			root->type->u.structure = (FieldList)malloc(sizeof(struct FieldList_));
			buildStruct(root->children[3], root->type->u.structure, root->type->u.structure);
			for(FieldList p = root->type->u.structure;p!=NULL;p=p->tail)
			{
				if(p->type==NULL)
					printf("NULL ");
				else
					printf("%s", p->name);
			}
			printf("\n");
			if (root->children[1] != NULL) //OptTag -> ID
			{
				root->children[1]->type = root->type;
				root->children[1]->children[0]->type = root->type;
				insertSymbolTable(root->children[1]->children[0]);
			}
		}
		if (root->productionNum == 1)
		{
			Type type = getTypeByID(root->children[1]->children[0], 0);
			if (type == NULL)
			{
				printf("Error type 17 at line %d: Undefined structures \"%s\".\n", root->firstline, root->children[1]->children[0]->content);
				root->type->kind = NON_EXIST;
			}
			else
				root->type = type;
		}
	}
	else if (strcmp(root->name, "VarDec") == 0)
	{
		if (root->productionNum == 0)
		{
			root->children[0]->type = root->type;
			if (root->type->isParameter)
			{
				currentFunc.argv[currentFunc.argc] = (FieldList)malloc(sizeof(struct FieldList_));
				currentFunc.argv[currentFunc.argc]->name = root->children[0]->content;
				currentFunc.argv[currentFunc.argc]->type = root->type;
				currentFunc.argc++;
			}
			else
				insertSymbolTable(root->children[0]);
		}
		else if (root->productionNum == 1)
		{
			root->children[0]->type->isParameter = root->type->isParameter;
			root->children[0]->type->kind = ARRAY;
			root->children[0]->type->u.array.elem = root->type;
			root->children[0]->type->u.array.size = atoi(root->children[3]->content);
			traverseTree(root->children[0]);
		}
	}
	else if (strcmp(root->name, "FunDec") == 0)
	{
		currentFunc.name = root->children[0]->content;
		if (root->productionNum == 0)
		{
			root->children[0]->type = root->type;
			currentFunc.argc = 0;
			traverseTree(root->children[2]);
			insertFuncTable(root->children[0]);
		}
		else if (root->productionNum == 1)
		{
			root->children[0]->type = root->type;
			currentFunc.argc = 0;
			insertFuncTable(root->children[0]);
		}
	}
	else if (strcmp(root->name, "VarList") == 0)
	{
		traverseTree(root->children[0]);
		if (root->productionNum == 0)
		{
			traverseTree(root->children[2]);
		}
	}
	else if (strcmp(root->name, "ParamDec") == 0)
	{
		traverseTree(root->children[0]);
		root->type = root->children[0]->type;
		root->children[1]->type = root->type;
		root->children[1]->type->isParameter = 1;
		traverseTree(root->children[1]);
	}
	else if (strcmp(root->name, "CompSt") == 0)
	{
		traverseTree(root->children[1]);
		traverseTree(root->children[2]);
	}
	else if (strcmp(root->name, "StmtList") == 0)
	{
		traverseTree(root->children[0]);
		traverseTree(root->children[1]);
	}
	else if (strcmp(root->name, "Stmt") == 0)
	{
		switch (root->productionNum)
		{
		case 0: traverseTree(root->children[0]); break;
		case 1: traverseTree(root->children[0]); break;
		case 2:
		{
			traverseTree(root->children[1]);
			if(currentFunc.returnType->kind == NON_EXIST || root->children[1]->type->kind == NON_EXIST)
				break;
			if (!checkTypeEqual(currentFunc.returnType, root->children[1]->type))
				printf("Error type 8 at line %d: Type mismatched for return.\n", root->firstline);
			break;
		}
		case 3:
		case 5:
		{
			traverseTree(root->children[2]);
			if (root->children[2]->type->kind != BASIC && root->children[2]->type->u.basic != INT_)
				printf("Error type 7 at line %d: Type mismatched for operands.\n", root->firstline);
			traverseTree(root->children[4]);
			break;
		}
		case 4:
		{
			traverseTree(root->children[2]);
			if (root->children[2]->type->kind != BASIC && root->children[2]->type->u.basic != INT_)
				printf("Error type 7 at line %d: Type mismatched for operands.\n", root->firstline);
			traverseTree(root->children[4]);
			traverseTree(root->children[6]);
			break;
		}
		}
	}
	else if (strcmp(root->name, "DefList") == 0)
	{
		traverseTree(root->children[0]);
		traverseTree(root->children[1]);
	}
	else if (strcmp(root->name, "Def") == 0)
	{
		traverseTree(root->children[0]);
		root->type = root->children[0]->type;
		root->children[1]->type = root->type;
		root->children[1]->type->isParameter = 0;
		traverseTree(root->children[1]);
	}
	else if (strcmp(root->name, "DecList") == 0)
	{
		root->children[0]->type = root->type;
		traverseTree(root->children[0]);
		if (root->productionNum == 1)
		{
			root->children[2]->type = root->type;
			traverseTree(root->children[2]);
		}
	}
	else if (strcmp(root->name, "Dec") == 0)
	{
		root->children[0]->type = root->type;
		traverseTree(root->children[0]);
		if (root->productionNum == 1)
		{
			traverseTree(root->children[2]);
			if (!checkTypeEqual(root->type, root->children[2]->type))
				printf("Error type 5 at line %d: Type mismatched for assignment\n", root->firstline);
		}
	}
	else if (strcmp(root->name, "Exp") == 0)
	{
		switch (root->productionNum)
		{
		case 0:/*Exp -> Exp ASSIGNOP Exp*/
		{
			traverseTree(root->children[0]);
			traverseTree(root->children[2]);
			if (root->children[0]->productionNum < 13 || root->children[0]->productionNum > 15)
			{
				printf("Error type 6 at line %d: The left-hand side of an assignment must be a variable.\n", root->firstline);
				break;
			}
			if (root->children[0]->type->kind == NON_EXIST || root->children[2]->type->kind == NON_EXIST)
				break;
			if (!checkTypeEqual(root->children[0]->type, root->children[2]->type))
			{
				printf("Error type 5 at line %d: Type mismatched for assignment.\n", root->firstline);
				break;
			}
			root->type = root->children[0]->type;
			break;
		}
		case 1:/*Exp -> Exp AND Exp*/
		case 2:/*Exp -> Exp OR Exp*/
		case 3:/*Exp -> Exp RELOP Exp*/
		{
			traverseTree(root->children[0]);
			traverseTree(root->children[2]);
			if (root->children[0]->type->kind == NON_EXIST || root->children[2]->type->kind == NON_EXIST)
				break;
			if (root->children[0]->type->kind != BASIC && root->children[0]->type->u.basic != INT_)
			{
				printf("Error type 7 at line %d: Type mismatched for operands.\n", root->firstline);
				break;
			}
			if (root->children[2]->type->kind != BASIC && root->children[2]->type->u.basic != INT_)
			{
				printf("Error type 7 at line %d: Type mismatched for operands.\n", root->firstline);
				break;
			}
			root->type = root->children[0]->type;
			break;
		}
		case 4:/*Exp -> Exp PLUS Exp*/
		case 5:/*Exp -> Exp MINUS Exp*/
		case 6:/*Exp -> Exp STAR Exp*/
		case 7:/*Exp -> Exp DIV Exp*/
		{
			traverseTree(root->children[0]);
			traverseTree(root->children[2]);
			if (root->children[0]->type->kind == NON_EXIST || root->children[2]->type->kind == NON_EXIST)
				break;
			if (root->children[0]->type->kind != BASIC)
			{
				printf("Error type 7 at line %d: Type mismatched for operands.\n", root->firstline);
				break;
			}
			if (root->children[2]->type->kind != BASIC)
			{
				printf("Error type 7 at line %d: Type mismatched for operands.\n", root->firstline);
				break;
			}
			if (!checkTypeEqual(root->children[0]->type, root->children[2]->type))
			{
				printf("Error type 7 at line %d: Type mismatched for operands.\n", root->firstline);
				break;
			}
			root->type = root->children[0]->type;
			break;
		}
		case 8: /*Exp -> LP Exp RP*/
		{
			traverseTree(root->children[1]);
			root->type = root->children[1]->type;
			break;
		}
		case 9:/*Exp -> MINUS Exp*/
		case 10:/*Exp -> NOT Exp*/
		{
			traverseTree(root->children[1]);
			if (root->children[1]->type->kind == NON_EXIST)
				break;
			if (root->children[1]->type->kind != BASIC && root->children[1]->type->u.basic != INT_)
			{
				printf("Error type 7 at line %d: Type mismatched for operands.\n", root->firstline);
				break;
			}
			root->type = root->children[1]->type;
			break;
		}
		case 11:/*Exp -> ID LP Args RP*/
		{
			root->children[0]->type->isFunction = 1;
			Function func = getFuncByID(root->children[0]);
			if (func.returnType == NULL)
			{
				Type type = getTypeByID(root->children[0], 0);
				if (type != NULL)
					printf("Error type 11 at line %d: \"%s\" is not a function.\n", root->firstline, root->children[0]->content);
				else
					printf("Error type 2 at line %d: Undefined function \"%s\".\n", root->firstline, root->children[0]->content);
				break;
			}
			root->type = func.returnType;
			stackTop++;
			calledFuncStack[stackTop].argc = 0;
			traverseTree(root->children[2]);
			if (!checkArgsEqual(func, calledFuncStack[stackTop]))
			{
				printf("Error type 9 at line %d: Conflicting types for \"%s\".\n", root->firstline, root->children[0]->content);
			}
			calledFuncStack[stackTop--].argc = 0;
			break;
		}
		case 12:/*Exp -> ID LP RP*/
		{
			root->children[0]->type->isFunction = 1;
			Function func = getFuncByID(root->children[0]);
			if (func.returnType == NULL)
			{
				Type type = getTypeByID(root->children[0], 0);
				if (type != NULL)
					printf("Error type 11 at line %d: \"%s\" is not a function.\n", root->firstline, root->children[0]->content);
				else
					printf("Error type 2 at line %d: Undefined function \"%s\".\n", root->firstline, root->children[0]->content);
				break;
			}
			root->type = func.returnType;
			if (func.argc != 0)
			{
				printf("Error type 9 at line %d: Conflicting types for \"%s\".\n", root->firstline, root->children[0]->content);
				break;
			}
			break;
		}
		case 13:/*Exp -> Exp LB Exp RB*/
		{
			traverseTree(root->children[0]);
			traverseTree(root->children[2]);
			if (root->children[0]->type->kind != ARRAY)
			{
				printf("Error type 10 at line %d: Illegal use of \"[]\".\n", root->firstline);
				//root->type->kind = ERROR;
				break;
			}
			if (root->children[2]->type->kind != BASIC)
			{
				printf("Error type 12 at line %d: Array argument must be an integer.\n", root->firstline);
				//root->type->kind = ERROR;
				break;
			}
			else if (root->children[2]->type->u.basic != INT_)
			{
				printf("Error type 12 at line %d: Array argument must be an integer.\n", root->firstline);
				//root->type->kind = ERROR;
				break;
			}
			root->type = root->children[0]->type->u.array.elem;
			break;
		}
		case 14:/*Exp -> Exp DOT ID*/
		{
			traverseTree(root->children[0]);
			if(root->children[0]->type->kind == NON_EXIST)
				break;
			if (root->children[0]->type->kind != STRUCTURE)
			{
				printf("Error type 13 at line %d: Illegal use of \".\".\n", root->firstline);
				break;
			}
			FieldList f = root->children[0]->type->u.structure->tail;
			for (; f != NULL; f = f->tail)
			{
				if (strcmp(f->name, root->children[2]->content) == 0)
				{
					root->type = f->type;
					break;
				}
			}
			if (f == NULL)
			{
				printf("Error type 14 at line %d: Non-existeent field \"%s\".\n", root->firstline, root->children[2]->content);
				root->type->kind = NON_EXIST;
			}
			break;
		}
		case 15:/*Exp -> ID*/
		{
			if (root->type->isFunction)
			{
				Type type = getTypeByID(root->children[0], 1);
				if (type == NULL)
				{
					printf("Error type 2 at line %d: Undefined function \"%s\".\n", root->firstline, root->children[0]->content);
					root->type->kind = NON_EXIST;
				}
				else
					root->type = type;
			}
			else
			{
				Type type = getTypeByID(root->children[0], 0);
				if (type == NULL)
				{
					printf("Error type 1 at line %d: Undefined variable \"%s\".\n", root->firstline, root->children[0]->content);
					root->type->kind = NON_EXIST;
				}
				else
					root->type = type;
			}
			break;
		}
		case 16: /*Exp -> INT*/
		{
			root->type->kind = BASIC;
			root->type->u.basic = INT_;
			break;
		}
		case 17: /*Exp -> FLOAT*/
		{
			root->type->kind = BASIC;
			root->type->u.basic = FLOAT_;
			break;
		}
		}
	}
	else if (strcmp(root->name, "Args") == 0)
	{
		traverseTree(root->children[0]);
		int argc = calledFuncStack[stackTop].argc;
		if (calledFuncStack[stackTop].argv[argc] == NULL)
			calledFuncStack[stackTop].argv[argc] = (FieldList)malloc(sizeof(struct FieldList_));
		calledFuncStack[stackTop].argv[argc]->type = root->children[0]->type;
		calledFuncStack[stackTop].argc++;
		if (root->productionNum == 0)
		{
			traverseTree(root->children[2]);
		}
	}
}

void insertSymbolTable(SyntaxTreeNode* ID)
{
	if (isSymbolExist(ID->content))
	{
		switch (ID->type->kind)
		{
		case BASIC:
		case ARRAY:
			printf("Error type 3 at line %d: Redefined variable \"%s\".\n", ID->firstline, ID->content);
			return;
		case STRUCTURE:
			printf("Error type 16 at line %d: Dupicated name \"%s\".\n", ID->firstline, ID->content);
			return;
		default:
			return;
		}
	}
	FieldList newSymbol = (FieldList)malloc(sizeof(struct FieldList_));
	newSymbol->type = ID->type;
	newSymbol->name = ID->content;
	unsigned int index = hash_pjw(ID->content);
	if (symbolTable[index] == NULL)
		symbolTable[index] = newSymbol;
	else
	{
		newSymbol->tail = symbolTable[index];
		symbolTable[index] = newSymbol;
	}
}

void insertFuncTable(SyntaxTreeNode * root)
{
	if (root->type->flag == Defined)
	{
		for (int i = 0; i < funcCount; i++)
		{
			if (strcmp(funcTable[i].name, currentFunc.name) == 0)
			{
				if (funcTable[i].returnType->flag == Defined)
				{
					printf("Error type 4 at line %d: redefinition of function \"%s\".\n", root->firstline, root->content);
				}
				else if (funcTable[i].returnType->flag == Declared)
				{
					if (checkArgsEqual(funcTable[i], currentFunc))
					{
						funcTable[i] = currentFunc;
						funcTable[i].returnType->flag = Defined;
					}
					else
						printf("Error type 19 at line %d: Inconsistent definition of function \"%s\".\n", root->firstline, root->content);

				}
				return;
			}
		}
		funcTable[funcCount++] = currentFunc;
	}
	else if (root->type->flag == Declared)
	{
		for (int i = 0; i < funcCount; i++)
		{
			if (strcmp(funcTable[i].name, currentFunc.name) == 0)
			{
				if (checkArgsEqual(funcTable[i], currentFunc) == 0)
					printf("Error type 19 at line %d: Inconsistent definition of function \"%s\".\n", root->firstline, root->content);
				return;
			}
		}
		currentFunc.firstline = root->firstline;
		funcTable[funcCount++] = currentFunc;
	}
	for (int i = 0; i < currentFunc.argc; i++)
	{
		SyntaxTreeNode * ID = malloc(sizeof(struct SyntaxTreeNode));
		strcpy(ID->content, currentFunc.argv[i]->name);
		ID->type = currentFunc.argv[i]->type;
		insertSymbolTable(ID);
	}
}

int checkArgsEqual(Function f1, Function f2)
{
	if (f1.argc != f2.argc)
		return 0;
	for (int i = 0; i < f1.argc; i++)
	{
		if (!checkTypeEqual(f1.argv[i]->type, f2.argv[i]->type))
			return 0;
	}
	return 1;
}

int checkTypeEqual(Type t1, Type t2)
{
	if (t1->kind != t2->kind)
		return 0;
	if (t1->kind == BASIC && t1->u.basic != t2->u.basic)
		return 0;
	if (t1->kind == STRUCTURE)
	{
		return t1 == t2;
	}
	return 1;
}

int isSymbolExist(char* name)
{
	unsigned int index = hash_pjw(name);
	FieldList f = symbolTable[index];
	for (; f != NULL; f = f->tail)
		if (strcmp(f->name, name) == 0)
			return 1;
	return 0;
}

int isFuncExist(char* name)
{
	for (int i = 0; i < funcCount; i++)
	{
		if (strcmp(funcTable[i].name, name) == 0)
		{
			if (funcTable[i].returnType->flag == Defined)
				return 1;
		}
	}
	return 0;
}

Type getTypeByID(SyntaxTreeNode* ID, int flag)
{
	if (flag == 0)
	{
		unsigned int index = hash_pjw(ID->content);
		FieldList f = symbolTable[index];
		for (; f != NULL; f = f->tail)
		{
			if (strcmp(f->name, ID->content) == 0)
				return f->type;
		}
		for (int i = 0; i < funcCount; i++)
		{
			for (int j = 0; j < funcTable[i].argc; j++)
			{
				if (strcmp(funcTable[i].argv[j]->name, ID->content) == 0)
					return funcTable[i].argv[j]->type;
			}
		}
	}
	else
	{
		for (int i = 0; i < funcCount; i++)
			if (strcmp(funcTable[i].name, ID->content) == 0)
				return funcTable[i].returnType;
	}
	return NULL;
}

Function getFuncByID(SyntaxTreeNode* ID)
{
	assert(funcCount);
	Function f;
	f.returnType = NULL;
	for (int i = 0; i < funcCount; i++)
	{
		if (strcmp(funcTable[i].name, ID->content) == 0)
		{
			f = funcTable[i];
		}
	}
	return f;
}

void buildStruct(SyntaxTreeNode * root, FieldList head, FieldList current)
{
	if (root == NULL)
		return;
	if (strcmp(root->name, "Dec") == 0 && root->productionNum == 1)
	{
		SyntaxTreeNode * p = root->children[0];
		while (strcmp(p->name, "ID") != 0 && p != NULL)
			p = p->children[0];
		printf("Error type 15 at line %d: Initialized struct field \"%s\".\n", p->firstline, p->content);
	}
	if (strcmp(root->name, "ID") == 0)
	{
		for (FieldList p = head->tail; p != NULL; p = p->tail)
		{	
			if (strcmp(p->name, root->content) == 0)
			{
				printf("Error type 15 at line %d: Redefined field \"%s\".\n", root->firstline, root->content);
				return;
			}
		}
		FieldList f = (FieldList)malloc(sizeof(struct FieldList_));
		f->name = root->content;
		f->type = root->type;
		current->tail = f;
	}
	else
		for (int i = 0; i < root->childrenNum; i++)
		{
			if (current->tail == NULL)
				buildStruct(root->children[i], head, current);
			else
				buildStruct(root->children[i], head, current->tail);
		}

}

void printtype(Type type)
{
	if (type == NULL)
		return;
	switch (type->kind)
	{
	case BASIC:
	{
		if (type->u.basic == INT_)
			printf("type = INT\n");
		else if (type->u.basic == FLOAT_)
			printf("type = FLOAT\n");
		else
			printf("type = VALID\n");
		break;
	}
	case ARRAY: printf("type = ARRAY\n"); break;
	case STRUCTURE: printf("type = STRUCTURE\n"); break;
	case ERROR: printf("type = ERROR\n"); break;
	case NON_EXIST : printf("type = NON_EXIST\n"); break;
	default: printf("type = NULL\n"); break;
	}
}

void addReadAndWrite()
{
	Function read;
	read.name = "read";
	read.returnType = malloc(sizeof(struct Type_));
	read.returnType->flag = Defined;
	read.returnType->kind =BASIC;
	read.returnType->u.basic = INT_;
	read.argc = 0;
	funcTable[funcCount++]=read;

	Function write;
	write.name = "write";
	write.returnType = malloc(sizeof(struct Type_));
	write.returnType->flag = Defined;
	write.returnType->kind = BASIC;
	write.returnType->u.basic = INT_;
	write.argc = 1;
	write.argv[0] = malloc(sizeof(struct FieldList_));
	write.argv[0]->type = malloc(sizeof(struct Type_));
	write.argv[0]->type->kind = BASIC;
	write.argv[0]->type->u.basic = INT_;
	funcTable[funcCount++]=write;
}