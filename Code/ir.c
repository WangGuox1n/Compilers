#include <assert.h>
#include "ir.h"
#include "symbol.h"
void gen_InterCode(SyntaxTreeNode *root)
{
	InterCodes* codes = translate_Program(root);
	assert(codes != NULL);
	for (InterCodes* p = codes; p != NULL; p = p->next)
	{
		switch (p->code.kind) {
		case IC_LABEL: {
			printf("LABEL label%d :", p->code.result.u.label_id);
			break;
		}
		case IC_FUNC: {
			printf("FUNCTION %s :", p->code.result.u.name);
			break;
		}
		case IC_ASSIGN: {
			/*if (p->code.result.kind == TEMP && p->code.result.u.var_id == VAR_NULL) {
			    break;
			}*/
			printOperand(p->code.result);
			printf(" := ");
			printOperand(p->code.arg1);
			break;
		}
		case IC_ADD: {
			printOperand(p->code.result); printf(" := ");
			printOperand(p->code.arg1);  printf(" + "); printOperand(p->code.arg2);
			break;
		}
		case IC_SUB: {
			printOperand(p->code.result); printf(" := ");
			printOperand(p->code.arg1);  printf(" - "); printOperand(p->code.arg2);
			break;
		}
		case IC_MUL: {
			printOperand(p->code.result); printf(" := ");
			printOperand(p->code.arg1);  printf(" * "); printOperand(p->code.arg2);
			break;
		}
		case IC_DIV: {
			printOperand(p->code.result); printf(" := ");
			printOperand(p->code.arg1);  printf(" / "); printOperand(p->code.arg2);
			break;
		}
		case IC_GOTO: {
			printf("GOTO "); printOperand(p->code.result);
			break;
		}
		case IC_RELOP: {
			printf("IF ");
			printOperand(p->code.arg1);
			switch (p->code.relop) {
			case LT: printf(" < "); break;
			case LE: printf(" <= "); break;
			case EQ: printf(" == "); break;
			case GT: printf(" > "); break;
			case GE: printf(" >= "); break;
			case NE: printf(" != "); break;
			default: assert(0);
			}
			printOperand(p->code.arg2); printf(" GOTO "); printOperand(p->code.result);
			break;
		}
		case IC_DEC: {
			//printf("DEC ");printOperand(p->code.result);printf(" %d", p->code.size);
			break;
		}
		case IC_RETURN: {
			printf("RETURN "); printOperand(p->code.result);
			break;
		}
		case IC_ARG: {
			printf("ARG "); printOperand(p->code.result);
			break;
		}
		case IC_CALL: {
			printOperand(p->code.result);
			printf(" := CALL "); printOperand(p->code.arg1);
			break;
		}
		case IC_PARAM: {
			printf("PARAM "); printOperand(p->code.result);
			break;
		}
		case IC_READ: {
			printf("READ "); printOperand(p->code.result);
			break;
		}
		case IC_WRITE: {
			printf("WRITE "); printOperand(p->code.result);
			break;
		}
		default: assert(0);
		}
		printf("\n");
	}
}

FieldList lookupSymbol(SyntaxTreeNode *ID)
{
	unsigned int index = hash_pjw(ID->content);
	FieldList f = symbolTable[index];
	for (; f != NULL; f = f->tail)
	{
		if (strcmp(f->name, ID->content) == 0)
			return f;
	}
	return NULL;
}

void printOperand(Operand operand)
{
	if (operand.kind == TEMP) {
		printf("t%d", operand.u.var_id);
	} else if (operand.kind == VARIABLE) {
		printf("v_%s", operand.u.name);
	} else if (operand.kind == FUNCTION) {
		printf("%s", operand.u.name);
	} else if (operand.kind == CONSTANT) {
		printf("#%d", operand.u.value);
	} else if (operand.kind == LABEL) {
		printf("label%d", operand.u.label_id);
	} else {
		assert(0);
	}
}

InterCodes* newInterCodes()
{
	InterCodes *codes = malloc(sizeof(struct InterCodes_));
	codes->count = interCount++;
	codes->prev = NULL;
	codes->next = NULL;
	return codes;
}

//get the tail of a link construct of struct InterCodes_
InterCodes* get_tail(InterCodes* codes)
{
	printf("in get_tail\n");
	assert(codes != NULL);
	InterCodes* p = codes;
	while (p->next != NULL)
		p = p->next;
	return p;
}

//link intercode
InterCodes* linkInterCode(InterCodes* code1, InterCodes* code2)
{
	assert(code1);
	get_tail(code1)->next = code2;
	return code1;
}

int newTempId() { return tempId++; }
int newLabelId() { return labelId++; }

enum RELOP_kind get_relop(SyntaxTreeNode *node)
{
	if (strcmp(node->content, "<"))
		return LT;
	else if (strcmp(node->content, "<="))
		return LE;
	else if (strcmp(node->content, "=="))
		return EQ;
	else if (strcmp(node->content, "!="))
		return NE;
	else if (strcmp(node->content, ">="))
		return GE;
	else if (strcmp(node->content, ">"))
		return GT;
	assert(0);
	return GT;
}

InterCodes* gen_Goto(int label_id)
{
	InterCodes* codes = newInterCodes();
	codes->code.kind = IC_GOTO;
	codes->code.result.kind = LABEL;
	codes->code.result.u.label_id = label_id;
	return codes;
}

InterCodes* gen_Label(int label_id)
{
	InterCodes* codes = newInterCodes();
	codes->code.kind = IC_LABEL;
	codes->code.result.kind = LABEL;
	codes->code.result.u.label_id = label_id;
	return codes;
}

InterCodes* translate_Program(SyntaxTreeNode *Program)
{
	printf("in Program\n");
	return translate_ExtDefList(Program->children[0]);
}

InterCodes* translate_ExtDefList(SyntaxTreeNode *ExtDefList)
{
	printf("in ExtDefList\n");
	if (NULL == ExtDefList)
		return NULL;
	InterCodes *code1 = translate_ExtDef(ExtDefList->children[0]);
	InterCodes *code2 = translate_ExtDefList(ExtDefList->children[1]);
	return linkInterCode(code1, code2);
}

InterCodes* translate_ExtDef(SyntaxTreeNode *ExtDef)
{
	printf("in ExtDef\n");
	if (ExtDef->productionNum == 2)
	{
		InterCodes *code1 = translate_FunDec(ExtDef->children[1]);
		assert(code1 != NULL);
		InterCodes *code2 = translate_CompSt(ExtDef->children[2]);
		return linkInterCode(code1, code2);
	}
	return NULL;
}

InterCodes* translate_FunDec(SyntaxTreeNode *FunDec)
{
	printf("in FunDec\n");
	InterCodes *code1 = newInterCodes();
	code1->code.kind = IC_FUNC;
	code1->code.result.kind = FUNCTION;
	code1->code.result.u.name = FunDec->children[0]->content;

	InterCodes *code2 = NULL;
	if (FunDec->productionNum == 0)
		code2 = translate_VarList(FunDec->children[2]);

	return linkInterCode(code1, code2);
}

InterCodes* translate_VarList(SyntaxTreeNode *VarList)
{
	printf("in VarList\n");
	InterCodes *code1 = translate_ParamDec(VarList->children[0]);
	InterCodes *code2 = NULL;
	if (VarList->productionNum == 0)
		code2 = translate_VarList(VarList->children[2]);

	return linkInterCode(code1, code2);
}

InterCodes* translate_ParamDec(SyntaxTreeNode *ParamDec)
{
	printf("in ParamDec\n");
	InterCodes *codes = newInterCodes();
	codes->code.kind = IC_PARAM;
	codes->code.result.kind = VARIABLE;
	codes->code.result.u.var_id = newTempId();
	return codes;
}

InterCodes* translate_CompSt(SyntaxTreeNode *CompSt)
{
	printf("in CompSt\n");
	return translate_StmtList(CompSt->children[2]);
}

InterCodes* translate_StmtList(SyntaxTreeNode *StmtList)
{
	printf("in StmtList\n");
	if (NULL == StmtList)
		return NULL;
	InterCodes *code1 = translate_Stmt(StmtList->children[0]);
	InterCodes *code2 = translate_StmtList(StmtList->children[1]);
	return linkInterCode(code1, code2);
}

InterCodes* translate_Stmt(SyntaxTreeNode *Stmt)
{
	printf("in Stmt %d\n", Stmt->productionNum);
	switch (Stmt->productionNum)
	{
	case 0: return translate_Exp(Stmt->children[0], 0);
	case 1:	return translate_CompSt(Stmt->children[0]);
	case 2: /*Stmt -> RETURN Exp SEMI*/
	{
		int t1 = newTempId();
		InterCodes* code1 = translate_Exp(Stmt->children[1], t1);
		InterCodes* code2 = newInterCodes();
		code2->code.kind = IC_RETURN;
		code2->code.result.kind = TEMP;
		code2->code.result.u.var_id = t1;
		return linkInterCode(code1, code2);
	}
	case 3:
	{
		int label1 = newLabelId();
		int label2 = newLabelId();
		InterCodes *code1 = translate_Cond(Stmt->children[2], label1, label2);
		InterCodes *code2 = translate_Stmt(Stmt->children[4]);
		return linkInterCode(
		           linkInterCode(code1, gen_Label(label1)),
		           linkInterCode(code2, gen_Label(label2))
		       );
	}
	case 4:
	{
		int label1 = newLabelId();
		int label2 = newLabelId();
		int label3 = newLabelId();
		InterCodes *code1 = translate_Cond(Stmt->children[2], label1, label2);
		InterCodes *code2 = translate_Stmt(Stmt->children[4]);
		InterCodes *code3 = translate_Stmt(Stmt->children[6]);
		return  linkInterCode(
		            linkInterCode(
		                linkInterCode(code1, gen_Label(label1)),
		                linkInterCode(code2, gen_Goto(label3))),
		            linkInterCode(code3, gen_Label(label3))
		        );
	}
	case 5:
	{
		int label1 = newLabelId();
		int label2 = newLabelId();
		int label3 = newLabelId();
		InterCodes *code1 = translate_Cond(Stmt->children[2], label1, label2);
		InterCodes *code2 = translate_Stmt(Stmt->children[4]);
		return linkInterCode(
		            linkInterCode(
		                linkInterCode(gen_Label(label1), code1),
		                linkInterCode(gen_Label(label2), code2)),
		            linkInterCode(gen_Goto(label1), gen_Label(label3))
		        );
	}
	default: assert(0);
	}
}

InterCodes* translate_Exp(SyntaxTreeNode *Exp, int place)
{
	printf("in Exp %d\n", Exp->productionNum);
	switch (Exp->productionNum)
	{
	case 0:/*Exp -> Exp ASSIGNOP Exp*/
	{
		if (Exp->children[0]->productionNum == 15)
		{
			FieldList symbol = lookupSymbol(Exp->children[0]->children[0]);
			int t1 = newTempId();
			InterCodes *code1 = translate_Exp(Exp->children[2], t1);
			InterCodes *code2 = newInterCodes();
			code2->code.kind = IC_ASSIGN;
			code2->code.result.kind = VARIABLE;
			code2->code.result.u.name = symbol->name;
			code2->code.arg1.kind = TEMP;
			code2->code.arg1.u.var_id = t1;
			return linkInterCode(code1, code2);
		}
		break;
	}
	case 1:/*Exp -> Exp AND Exp*/
	case 2:/*Exp -> Exp OR Exp*/
	case 3:/*Exp -> Exp RELOP Exp*/
	{
		int t1 = newLabelId();
		int t2 = newLabelId();
		InterCodes* code1 = translate_Cond(Exp, t1, t2);
		return code1;
		break;
	}
	case 4:/*Exp -> Exp PLUS Exp*/
	case 5:/*Exp -> Exp MINUS Exp*/
	case 6:/*Exp -> Exp STAR Exp*/
	case 7:/*Exp -> Exp DIV Exp*/
	{
		break;
	}
	case 8: /*Exp -> LP Exp RP*/
	{
		break;
	}
	case 9:/*Exp -> MINUS Exp*/
	{
		int t1 = newTempId();
		InterCodes* code1 = translate_Exp(Exp->children[1], t1);
		InterCodes* code2 = newInterCodes();
		code2->code.kind = IC_ASSIGN;
		code2->code.result.kind = TEMP;
		code2->code.result.u.var_id = place;
		code2->code.arg1.kind = CONSTANT;
		code2->code.arg1.u.value = 0;
		code2->code.arg2.kind = TEMP;
		code2->code.arg2.u.var_id = t1;
		return linkInterCode(code1, code2);
	}
	case 10:/*Exp -> NOT Exp*/
	{
		break;
	}
	case 11:/*Exp -> ID LP Args RP*/
	{
		Function func = getFuncByID(Exp->children[0]);  //lookup function
		ArgList* arglist = malloc(sizeof(struct ArgList));
		arglist->count = 0;
		InterCodes *code1 = translate_Args(Exp->children[2], arglist);
		if (strcmp(func.name, "write") == 0)
		{
			InterCodes *code2 = newInterCodes();
			assert(arglist->count == 1);
			code2->code.kind = IC_WRITE;
			code2->code.result.kind = TEMP;
			code2->code.result.u.var_id = arglist->id[0];
			//here
			return linkInterCode(code1, code2);
		}
		for (int i = 0; i < arglist->count; i++)
		{
			InterCodes *code2 = newInterCodes();
			code2->code.kind = IC_ARG;
			code2->code.result.kind = TEMP;
			code2->code.result.u.var_id = arglist->id[i];
			code1 = linkInterCode(code1, code2);
		}
		InterCodes *code3 = newInterCodes();
		code3->code.kind = IC_CALL;
		code3->code.result.kind = TEMP;
		code3->code.result.u.var_id = place;
		code3->code.arg1.kind = FUNCTION;
		code3->code.arg1.u.name = func.name;
		return linkInterCode(code1, code3);
		break;
	}
	case 12:/*Exp -> ID LP RP*/
	{
		Function func = getFuncByID(Exp->children[0]);  //lookup function
		InterCodes *code1 = newInterCodes();
		if (strcmp(func.name, "read") == 0)
		{
			code1->code.kind = IC_READ;
			code1->code.result.kind = TEMP;
			code1->code.result.u.var_id = place;
		}
		else
		{
			code1->code.kind = IC_CALL;
			code1->code.result.kind = TEMP;
			code1->code.result.u.var_id = place;
			code1->code.arg1.kind = FUNCTION;
			code1->code.arg1.u.name = func.name;
		}
		return code1;

		break;
	}
	case 13:/*Exp -> Exp LB Exp RB*/
	{
		break;
	}
	case 14:/*Exp -> Exp DOT ID*/
	{
		break;
	}
	case 15:/*Exp -> ID*/
	{

		FieldList symbol = lookupSymbol(Exp->children[0]);
		InterCodes* codes = newInterCodes();
		codes->code.kind = IC_ASSIGN;

		codes->code.result.kind = TEMP;
		codes->code.result.u.var_id = place;
		codes->code.arg1.kind = VARIABLE;
		codes->code.arg1.u.name = symbol->name;
		return codes;
	}
	case 16: /*Exp -> INT*/
	{
		int num = atoi(Exp->children[0]->content);
		printf("%d\n", num);
		InterCodes* codes = newInterCodes();
		codes->code.kind = IC_ASSIGN;

		codes->code.result.kind = TEMP;
		codes->code.result.u.var_id = place;
		codes->code.arg1.kind = CONSTANT;
		codes->code.arg1.u.value = num;
		return codes;
		break;
	}
	case 17: /*Exp -> FLOAT*/
	{
		break;
	}
	}
}

InterCodes* translate_Cond(SyntaxTreeNode *Exp, int label_true, int label_false)
{
	printf("in Cond\n");
	switch (Exp->productionNum)
	{
	case 3:
	{
		int t1 = newTempId();
		int t2 = newTempId();
		InterCodes* code1 = translate_Exp(Exp->children[0], t1);
		InterCodes* code2 = translate_Exp(Exp->children[2], t2);
		InterCodes* code3 = newInterCodes();
		code3->code.kind = IC_RELOP;
		code3->code.relop = get_relop(Exp->children[1]);
		code3->code.result.kind = LABEL;
		code3->code.result.u.label_id = label_true;

		code3->code.arg1.kind = TEMP;
		code3->code.arg1.u.var_id = t1;

		code3->code.arg2.kind = TEMP;
		code3->code.arg2.u.var_id = t2;

		//generate GOTO
		InterCodes* code4 = newInterCodes();
		code4->code.kind = IC_GOTO;
		code4->code.result.kind = LABEL;
		code4->code.result.u.label_id = label_false;

		return linkInterCode(
		           linkInterCode(code1, code2),
		           linkInterCode(code3, code4));
	}

	}
}

InterCodes* translate_Args(SyntaxTreeNode *Args, ArgList* arglist)
{
	printf("in Arg %d\n", Args->productionNum);
	if (Args->productionNum == 1)
	{
		int t1 = newTempId();
		arglist->id[arglist->count++] = t1;
		return translate_Exp(Args->children[0], t1);
	}
	else
	{
		int t1 = newTempId();
		InterCodes *code1 = translate_Exp(Args->children[0], t1);
		arglist->id[arglist->count++] = t1;
		InterCodes *code2 = translate_Exp(Args->children[2], t1);
		return linkInterCode(code1, code2);
	}
}

void printLink()
{
	for (InterCodes* p = codeHead; p != NULL; p = p->next)
	{
		printf("code%d ", p->count);
	}
	printf("\n");
}