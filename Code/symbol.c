#include "symbol.h"
#include "syntaxtree.h"
#include <string.h>
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
		printf("in Program\n");
		traverseTree(root->children[0]);
		printf("back to Program\n");
		for (int i = 0; i < funcCount; i++)
			if (funcTable[i].returnType->flag == Declared)
				printf("Error type 18 at Line %d: Undefined function \"%s\".\n", funcTable[i].firstline, funcTable[i].name);
	}
	else if (strcmp(root->name, "ExtDefList") == 0)
	{
		printf("in ExtDefList\n");
		if (root->productionNum == 0)
		{
			traverseTree(root->children[0]);
			printf("back to ExtDefList\n");
			traverseTree(root->children[1]);
			printf("back to ExtDefList\n");
		}
	}
	else if (strcmp(root->name, "ExtDef") == 0)
	{
		if (root->productionNum == 0)
		{
			printf("in ExtDef 0\n");
			traverseTree(root->children[0]);
			printf("back to ExtDef 0\n");
			root->type = root->children[0]->type;
			root->children[1]->type = root->type;
			traverseTree(root->children[1]);
			printf("back to ExtDef 0\n");
		}
		else if (root->productionNum == 1)
		{
			printf("in ExtDef 1\n");
			traverseTree(root->children[0]);
			printf("back to ExtDef 1\n");
			root->type = root->children[0]->type;
		}
		else if (root->productionNum == 2)
		{
			printf("in ExtDef 2\n");
			traverseTree(root->children[0]);
			printf("back to ExtDef 2\n");
			root->type = root->children[0]->type;
			root->type->isFunction = 1;
			root->type->flag = Defined;
			root->children[1]->type = root->type;
			currentFunc.returnType = root->type;
			traverseTree(root->children[1]);
			printf("back to ExtDef 2\n");
			traverseTree(root->children[2]);
			printf("back to ExtDef 2\n");
		}
		else if (root->productionNum == 3)
		{
			printf("in ExtDef 3\n");
			traverseTree(root->children[0]);
			printf("back to ExtDef 3\n");
			root->type = root->children[0]->type;
			root->type->isFunction = 1;
			root->type->flag = Declared;
			root->children[1]->type = root->type;
			currentFunc.returnType = root->type;
			traverseTree(root->children[1]);
			printf("back to ExtDef 3\n");
		}
	}
	else if (strcmp(root->name, "ExtDecList") == 0)
	{
		printf("in ExtDecList\n");
		root->children[0]->type = root->type;
		traverseTree(root->children[0]);
		printf("back to ExtDecList\n");
		if (root->productionNum == 1)
		{
			root->children[2]->type = root->type;
			traverseTree(root->children[2]);
			printf("back to ExtDecList\n");
		}
	}
	else if (strcmp(root->name, "Specifier") == 0)
	{
		if (root->productionNum == 0)
		{
			printf("in Specifier 0\n");
			root->type = root->children[0]->type;
		}
		else if (root->productionNum == 1)
		{
			printf("in Specifier 1\n");
			traverseTree(root->children[0]);
			printf("back to Specifier 1\n");
			root->type = root->children[0]->type;
		}
	}
	else if (strcmp(root->name, "StructSpecifier") == 0)
	{
		if (root->productionNum == 0)
		{
			/*problem: OptTag is empty*/
			printf("in StructSpecifier 0\n");
			traverseTree(root->children[3]);
			printf("back to StructSpecifier 0\n");
			root->type->kind = STRUCTURE;
			root->type->u.structure = (FieldList)malloc(sizeof(struct FieldList_));
			addStructDefList(root->children[3], root->type->u.structure);
			printf("back to StructSpecifier 0\n");
			if (root->children[1] != NULL) //OptTag -> ID
			{
				root->children[1]->type = root->type;
				root->children[1]->children[0]->type = root->type;
				insertSymbolTable(root->children[1]->children[0]);
				printf("back to StructSpecifier 0\n");
			}
		}
		if (root->productionNum == 1)
		{
			printf("in StructSpecifier 1\n");
			Type type = getTypeByID(root->children[1]->children[0], 0);
			if (type == NULL)
			{
				printf("Error type 17 at line %d: Undefined structures.\n", root->firstline);
				root->type->kind = NON_EXIST;
			}
			else
				root->type = type;
			printf("back to StructSpecifier 1\n");
		}
	}
	else if (strcmp(root->name, "VarDec") == 0)
	{
		if (root->productionNum == 0)
		{
			printf("in VarDec 0\n");
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
			printf("back to VarDec 0\n");
		}
		else if (root->productionNum == 1)
		{
			printf("in VarDec 1\n");
			/*problem: the way array structed*/
			root->children[0]->type->isParameter = root->type->isParameter;
			root->children[0]->type->kind = ARRAY;
			root->children[0]->type->u.array.elem = root->type;
			root->children[0]->type->u.array.size = atoi(root->children[3]->content);
			traverseTree(root->children[0]);
			printf("back to VarDec 1\n");
		}
	}
	else if (strcmp(root->name, "FunDec") == 0)
	{
		currentFunc.name = root->children[0]->content;
		if (root->productionNum == 0)
		{
			printf("in Fundec 0\n");
			root->children[0]->type = root->type;
			currentFunc.argc = 0;
			traverseTree(root->children[2]);
			printf("back to FunDec\n");
			insertFuncTable(root->children[0]);
			printf("back to FunDec\n");
		}
		else if (root->productionNum == 1)
		{
			printf("in Fundec 1\n");
			root->children[0]->type = root->type;
			currentFunc.argc = 0;
			insertFuncTable(root->children[0]);
			printf("back to FunDec\n");
		}
	}
	else if (strcmp(root->name, "VarList") == 0)
	{
		printf("in VarList\n");
		//root->children[0]->type = root->type;
		traverseTree(root->children[0]);
		printf("back to VarList\n");
		if (root->productionNum == 0)
		{
			traverseTree(root->children[2]);
			printf("back to VarList\n");
		}
	}
	else if (strcmp(root->name, "ParamDec") == 0)
	{
		printf("in ParamDec\n");
		traverseTree(root->children[0]);
		printf("back to ParamDec\n");
		root->type = root->children[0]->type;
		root->type->isParameter = 1;
		root->children[1]->type = root->type;
		traverseTree(root->children[1]);
		printf("back to ParamDec\n");
	}
	else if (strcmp(root->name, "CompSt") == 0)
	{
		printf("in CompSt\n");
		traverseTree(root->children[1]);
		printf("back to CompSt\n");
		traverseTree(root->children[2]);
		printf("back to CompSt\n");
	}
	else if (strcmp(root->name, "StmtList") == 0)
	{
		printf("in StmtList\n");
		traverseTree(root->children[0]);
		printf("back to StmtList\n");
		traverseTree(root->children[1]);
		printf("back to StmtList\n");
	}
	else if (strcmp(root->name, "Stmt") == 0)
	{
		switch (root->productionNum)
		{
		case 0: printf("in Stmt 0\n"); traverseTree(root->children[0]); printf("back to Stmt 0\n"); break;
		case 1: printf("in Stmt 1\n"); traverseTree(root->children[0]); printf("back to Stmt 1\n"); break;
		case 2:
		{
			printf("in Stmt 2\n");
			traverseTree(root->children[1]);
			printf("back to Stmt 2\n");
			if (!checkTypeEqual(currentFunc.returnType, root->children[1]->type))
				printf("Error type 8 at line %d: Type mismatched for return.\n", root->firstline);
			break;
		}
		case 3:
		case 5:
		{
			printf("in Stmt 3 5\n");
			traverseTree(root->children[2]);
			printf("back to Stmt 3 5\n");
			if (root->children[2]->type->kind != BASIC && root->children[2]->type->u.basic != INT_)
				printf("Error type 7 at line %d: Type mismatched for operands.\n", root->firstline);
			traverseTree(root->children[4]);
			printf("back to Stmt 3 5\n");
			break;
		}
		case 4:
		{
			printf("in Stmt 4\n");
			traverseTree(root->children[2]);
			printf("back to Stmt 4\n");
			if (root->children[2]->type->kind != BASIC && root->children[2]->type->u.basic != INT_)
				printf("Error type 7 at line %d: Type mismatched for operands.\n", root->firstline);
			traverseTree(root->children[4]);
			printf("back to Stmt 4\n");
			traverseTree(root->children[6]);
			printf("back to Stmt 4\n");
			break;
		}
		}
	}
	else if (strcmp(root->name, "DefList") == 0)
	{
		printf("in DefList\n");
		traverseTree(root->children[0]);
		printf("back to DefList\n");
		traverseTree(root->children[1]);
		printf("back to DefList\n");
	}
	else if (strcmp(root->name, "Def") == 0)
	{
		printf("in Def\n");
		traverseTree(root->children[0]);
		printf("back to Def\n");
		root->type = root->children[0]->type;
		root->children[1]->type = root->type;
		traverseTree(root->children[1]);
		printf("back to Def\n");
	}
	else if (strcmp(root->name, "DecList") == 0)
	{
		printf("in DecList\n");
		root->children[0]->type = root->type;
		traverseTree(root->children[0]);
		printf("back to DecList\n");
		if (root->productionNum == 1)
		{
			root->children[2]->type = root->type;
			traverseTree(root->children[2]);
			printf("back to DecList\n");
		}
	}
	else if (strcmp(root->name, "Dec") == 0)
	{
		printf("in Dec\n");
		root->children[0]->type = root->type;
		traverseTree(root->children[0]);
		printf("back to Dec\n");
		if (root->productionNum == 1)
		{
			traverseTree(root->children[2]);
			printf("back to Dec\n");
			if (!checkTypeEqual(root->type, root->children[2]->type))
				printf("Error type 5 at line %d: Type mismatched for assignment\n", root->firstline);
			printf("back to Dec\n");
		}
	}
	else if (strcmp(root->name, "Exp") == 0)
	{
		switch (root->productionNum)
		{
		case 0:/*Exp -> Exp ASSIGNOP Exp*/
		{
			printf("in Exp 0\n");
			traverseTree(root->children[0]);
			printf("back to Exp 0\n");
			traverseTree(root->children[2]);
			printf("back to Exp 0\n");
			if (root->children[0]->productionNum < 13 || root->children[0]->productionNum > 15)
			{
				printf("Error type 6 at line %d: The left-hand side of an assignment must be a variable.\n", root->firstline);
				break;
			}
			if(root->children[0]->type->kind == NON_EXIST || root->children[2]->type->kind == NON_EXIST)
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
			printf("in Exp 3\n");
			traverseTree(root->children[0]);
			printf("back to Exp 3\n");
			traverseTree(root->children[2]);
			printf("back to Exp 3\n");
			if(root->children[0]->type->kind == NON_EXIST || root->children[2]->type->kind == NON_EXIST)
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
			printf("in Exp 7\n");
			traverseTree(root->children[0]);
			printf("back to Exp 7\n");
			traverseTree(root->children[2]);
			printf("back to Exp 7\n");
			if(root->children[0]->type->kind == NON_EXIST || root->children[2]->type->kind == NON_EXIST)
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
			printf("in Exp 8\n");
			traverseTree(root->children[1]);
			printf("back to Exp 8\n");
			root->type = root->children[1]->type;
			break;
		}
		case 9:/*Exp -> MINUS Exp*/
		case 10:/*Exp -> NOT Exp*/
		{
			printf("in Exp 10\n");
			traverseTree(root->children[1]);
			printf("back to Exp 10\n");
			if(root->children[1]->type->kind == NON_EXIST)
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
			printf("in Exp 11 \n");
			root->children[0]->type->isFunction = 1;
			//Type type = getTypeByID(root->children[0], 1);
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
			//calledFunc.argc = 0;
			traverseTree(root->children[2]);
			if (!checkArgsEqual(func, calledFuncStack[stackTop]))
			{
				printf("Error type 9 at line %d: Conflicting types for \"%s\".\n", root->firstline, root->children[0]->content);
			}
			calledFuncStack[stackTop--].argc = 0;
			//calledFunc.argc = 0;

			break;
		}
		case 12:/*Exp -> ID LP RP*/
		{
			printf("in Exp 12 \n");
			root->children[0]->type->isFunction = 1;
			Function func = getFuncByID(root->children[0]);
			//Type type = getTypeByID(root->children[0], 1);
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
			printf("in Exp 13\n");
			traverseTree(root->children[0]);
			printf("back to Exp 13\n");
			traverseTree(root->children[2]);
			printf("back to Exp 13\n");
			if (root->children[0]->type->kind != ARRAY)
			{
				printf("Error type 10 at line %d: Wrong type before [].\n", root->firstline);
				//root->type->kind = ERROR;
				break;
			}
			if (root->children[2]->type->kind != BASIC)
			{
				printf("Error type 12 at line %d: Wrong type in [] .\n", root->firstline);
				//root->type->kind = ERROR;
				break;
			}
			else if (root->children[2]->type->u.basic != INT_)
			{
				printf("Error type 12 at line %d: Wrong type in [] .\n", root->firstline);
				//root->type->kind = ERROR;
				break;
			}
			//root->type = root->children[0]->type;
			root->type = root->children[0]->type->u.array.elem;
			break;
		}
		case 14:/*Exp -> Exp DOT ID*/
		{
			printf("in Exp 14\n");
			traverseTree(root->children[0]);
			printf("back to Exp 14\n");
			if (root->children[0]->type->kind != STRUCTURE)
			{
				printf("Error type 13 at line %d: Illegal use of \".\".\n", root->firstline);
				//root->type->kind = ERROR;
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
				printf("in Exp 15 Function\n");
				Type type = getTypeByID(root->children[0], 1);
				if (type == NULL)
				{	
					printf("Error type 2 at line %d: Undefined function \"%s\".\n", root->firstline, root->children[0]->content);
					root->type->kind = NON_EXIST;
				}
				else
					root->type = type;
				printf("back to Exp 15\n");
			}
			else
			{
				printf("in Exp 15 variable\n");
				Type type = getTypeByID(root->children[0], 0);
				if (type == NULL)
				{
					printf("Error type 1 at line %d: Undefined variable \"%s\".\n", root->firstline, root->children[0]->content);
					root->type->kind = NON_EXIST;
				}
				else
					root->type = type;
				printf("back to Exp 15\n");
			}
			break;
		}
		case 16: /*Exp -> INT*/
		{
			printf("in Exp 16\n");
			root->type->kind = BASIC;
			root->type->u.basic = INT_;
			break;
		}
		case 17: /*Exp -> FLOAT*/
		{
			printf("in Exp 17\n");
			root->type->kind = BASIC;
			root->type->u.basic = FLOAT_;
			break;
		}
		}
	}
	else if (strcmp(root->name, "Args") == 0)
	{
		printf("in Args\n");
		traverseTree(root->children[0]);
		int argc = calledFuncStack[stackTop].argc;
		if(calledFuncStack[stackTop].argv[argc]==NULL)
			calledFuncStack[stackTop].argv[argc] = (FieldList)malloc(sizeof(struct FieldList_));
		calledFuncStack[stackTop].argv[argc]->type = root->children[0]->type;
		calledFuncStack[stackTop].argc++;
		printf("back to Args\n");
		if (root->productionNum == 0)
		{
			traverseTree(root->children[2]);
			printf("back to Args\n");
		}
	}
}
void insertSymbolTable(SyntaxTreeNode* ID)
{
	printf("in insertSymbolTable\n");
	if (isExist(ID->content) && ID->type->kind == BASIC)
	{
		printf("Error type 3 at line %d: Redefined variable \"%s\".\n", ID->firstline, ID->content);
		return;
	}
	printf("back to insertSymbolTable\n");
	if (isExist(ID->content) && ID->type->kind == STRUCTURE)
	{
		printf("Error type 16 at line %d: Dupicated name \"%s\".\n", ID->firstline, ID->content);
		return;
	}
	printf("back to insertSymbolTable\n");
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
	printf("in insertFuncTable\n");
	if (root->type->flag == Defined)
	{
		for (int i = 0; i < funcCount; i++)
		{
			if (strcmp(funcTable[i].name, currentFunc.name) == 0)
			{
				if (funcTable[i].returnType->flag == Defined)
				{
					printf("Error type 19 at line %d: redefinition of function \"%s\".\n", root->firstline, root->content);
				}
				else if (funcTable[i].returnType->flag == Declared)
				{
					if (checkArgsEqual(funcTable[i], currentFunc))
						funcTable[i].returnType->flag = Defined;
					else
						printf("Error type 19 at line %d: redefinition of function \"%s\".\n", root->firstline, root->content);

				}
				return;
			}
		}
		printf("%d\n", currentFunc.argc);
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
	for(int i = 0;i<currentFunc.argc;i++)
	{
		SyntaxTreeNode * ID = malloc(sizeof(struct SyntaxTreeNode));
		strcpy(ID->content,currentFunc.argv[i]->name);
		ID->type = currentFunc.argv[i]->type;
		insertSymbolTable(ID);
	}			
}

int checkArgsEqual(Function f1, Function f2)
{
	printf("in checkArgsEqual\n");
	printf("%d %d\n", f1.argc,f2.argc);
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

int isExist(char* name)
{
	printf("in isExist\n");
	unsigned int index = hash_pjw(name);
	FieldList f = symbolTable[index];
	for (; f != NULL; f = f->tail)
		if (strcmp(f->name, name) == 0)
			return 1;
	return 0;
}

Type getTypeByID(SyntaxTreeNode* ID, int flag)
{
	printf("in getTypeByID\n");
	if (flag == 0)
	{
		unsigned int index = hash_pjw(ID->content);
		FieldList f = symbolTable[index];
		for (; f != NULL; f = f->tail)
		{
			if (strcmp(f->name, ID->content) == 0)
				return f->type;
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
	Function f;
	f.returnType = NULL;
	for (int i = 0; i < funcCount; i++)
	{
		if (strcmp(funcTable[i].name, ID->content) == 0)
			f = funcTable[i];
	}
	return f;
}

void addStructDefList(SyntaxTreeNode * root, FieldList structHead)
{
	if (root == NULL)
		return;
	//printf("in addStructDefList %s\n",root->name);
	if (strcmp(root->name, "Dec") == 0 && root->productionNum == 1)
	{
		SyntaxTreeNode * p = root->children[0];
		while (strcmp(p->name, "ID") != 0 && p != NULL)
			p = p->children[0];
		printf("Error type 15 at line %d: Can't initailizer at struct field '%s'\n", p->firstline, p->content);
	}
	if (strcmp(root->name, "ID") == 0)
	{
		FieldList f = (FieldList)malloc(sizeof(struct FieldList_));
		f->name = root->content;
		f->type = root->type;
		structHead->tail = f;
	}
	else
		for (int i = 0; i < root->childrenNum; i++)
		{
			if (structHead->tail == NULL)
				addStructDefList(root->children[i], structHead);
			else
				addStructDefList(root->children[i], structHead->tail);
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
	default: printf("type = NULL\n"); break;
	}
}