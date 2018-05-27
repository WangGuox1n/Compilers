#include <assert.h>
#include "ir.h"
#include "symbol.h"
void gen_InterCode(SyntaxTreeNode *root)
{
	tempId = 1;
	labelId = 1;
	constant_count = 0;
	variable_count = 0;
	InterCodes* codes = translate_Program(root);
	code_optimization(codes);
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
			printOperand(p->code.result); printf(" := ");
			printOperand(p->code.arg1);
			break;
		}
		case IC_PLUS: {
			printOperand(p->code.result); printf(" := ");
			printOperand(p->code.arg1);   printf(" + ");
			printOperand(p->code.arg2);
			break;
		}
		case IC_MINUS: {
			printOperand(p->code.result); printf(" := ");
			printOperand(p->code.arg1);   printf(" - ");
			printOperand(p->code.arg2);
			break;
		}
		case IC_MUL: {
			printOperand(p->code.result); printf(" := ");
			printOperand(p->code.arg1);   printf(" * ");
			printOperand(p->code.arg2);
			break;
		}
		case IC_DIV: {
			printOperand(p->code.result); printf(" := ");
			printOperand(p->code.arg1);   printf(" / ");
			printOperand(p->code.arg2);
			break;
		}
		case IC_ADDR: {
			printOperand(p->code.result); printf(" := &");
			printOperand(p->code.arg1);
			break;
		}
		case IC_DEREF_R: {
			printOperand(p->code.result); printf(" := *");
			printOperand(p->code.arg1);
			break;
		}
		case IC_DEREF_L: {
			printf("*");   	printOperand(p->code.result);
			printf(" := "); printOperand(p->code.arg1);
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
			printOperand(p->code.arg2); printf(" GOTO ");
			printOperand(p->code.result);
			break;
		}
		case IC_DEC: {
			printf("DEC "); printOperand(p->code.result);
			printf(" %d", p->code.arg1.u.value);
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
			printOperand(p->code.result); printf(" := CALL ");
			printOperand(p->code.arg1);
			break;
		}
		case IC_PARAM: {
			printf("PARAM "); printOperand(p->code.result);
			break;
		}
		case IC_READ: {
			printf("READ ");  printOperand(p->code.result);
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

int SizeOfType(Type type)
{
	int size = 0;
	switch (type->kind)
	{
	case BASIC : return sizeof(int);
	case ARRAY : return SizeOfType(type->u.array.elem) * type->u.array.size;
	case STRUCTURE :
	{
		for (FieldList p = type->u.structure->tail; p != NULL; p = p->tail)
		{
			size += SizeOfType(p->type);
		}
		return size;
	}
	default : assert(0);
	}
	return size;
}

void printOperand(Operand operand)
{
	switch (operand.kind)
	{
	case TEMP:		printf("t%d", 	 operand.u.temp_id); break;
	case VARIABLE: 	printf("v_%s", 	 operand.u.name); break;
	case FUNCTION:	printf("%s", 	 operand.u.name); break;
	case CONSTANT: 	printf("#%d", 	 operand.u.value); break;
	case LABEL:		printf("label%d", operand.u.label_id); break;
	default:		assert(0);
	}
}

InterCodes* newInterCodes()
{
	InterCodes *codes = malloc(sizeof(struct InterCodes_));
	codes->prev = NULL;
	codes->next = NULL;
	return codes;
}

//link intercode
InterCodes* linkInterCode(InterCodes* code1, InterCodes* code2)
{
	if (NULL == code1)
		return code2;
	if (NULL == code2)
		return code1;

	InterCodes* tail = code1;
	while (tail->next != NULL)
		tail = tail->next;

	tail->next = code2;
	code2->prev = tail;

	return code1;
}

int newTempId() { return tempId++; }
int newLabelId() { return labelId++; }

enum RELOP_kind get_relop(SyntaxTreeNode *node)
{
	if (strcmp(node->content, "<") == 0)
		return LT;
	else if (strcmp(node->content, "<=") == 0)
		return LE;
	else if (strcmp(node->content, "==") == 0)
		return EQ;
	else if (strcmp(node->content, "!=") == 0)
		return NE;
	else if (strcmp(node->content, ">=") == 0)
		return GE;
	else if (strcmp(node->content, ">") == 0)
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
	return translate_ExtDefList(Program->children[0]);
}

InterCodes* translate_ExtDefList(SyntaxTreeNode *ExtDefList)
{
	if (NULL == ExtDefList)
		return NULL;
	InterCodes *code1 = translate_ExtDef(ExtDefList->children[0]);
	InterCodes *code2 = translate_ExtDefList(ExtDefList->children[1]);
	return linkInterCode(code1, code2);
}

InterCodes* translate_ExtDef(SyntaxTreeNode *ExtDef)
{
	if (ExtDef->productionNum == 2)
	{
		InterCodes *code1 = translate_FunDec(ExtDef->children[1]);
		assert(code1 != NULL);
		InterCodes *code2 = translate_CompSt(ExtDef->children[2]);
		return linkInterCode(code1, code2);
	}
	return NULL;
}

InterCodes* translate_VarDec(SyntaxTreeNode *VarDec)
{
	/*这里仅用于解析定义变量的时候内存空间的申请操作，函数形参留在Param里解析*/
	if (0 == VarDec->productionNum)
	{
		FieldList symbol = lookupSymbol(VarDec->children[0]);
		if (STRUCTURE == symbol->type->kind)
		{
			int size = SizeOfType(symbol->type);
			InterCodes *codes = newInterCodes();
			codes->code.kind = IC_DEC;
			codes->code.result.kind = VARIABLE;
			codes->code.result.u.name = symbol->name;
			codes->code.arg1.kind = CONSTANT;
			codes->code.arg1.u.value = size;
			return codes;
		}
		else if (ARRAY == symbol->type->kind)
		{
			int size = SizeOfType(symbol->type);
			InterCodes *codes = newInterCodes();
			codes->code.kind = IC_DEC;
			codes->code.result.kind = VARIABLE;
			codes->code.result.u.name = symbol->name;
			codes->code.arg1.kind = CONSTANT;
			codes->code.arg1.u.value = size;
			return codes;
		}
		else {
			return NULL;
		}
	}
	else
		return translate_VarDec(VarDec->children[0]);
}

InterCodes* translate_FunDec(SyntaxTreeNode *FunDec)
{
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
	InterCodes *code1 = translate_ParamDec(VarList->children[0]);
	InterCodes *code2 = NULL;
	if (VarList->productionNum == 0)
		code2 = translate_VarList(VarList->children[2]);

	return linkInterCode(code1, code2);
}

InterCodes* translate_ParamDec(SyntaxTreeNode *ParamDec)
{
	// only support basic/struct type varible, not arrry type.
	//assert(ParamDec->children[1]->productionNum == 0);
	ParamDec->type->isParameter = 1;
	SyntaxTreeNode *VarDec = ParamDec->children[1];
	SyntaxTreeNode *ID;
	if (0 == VarDec->productionNum) {
		ID = VarDec->children[0];
	} else {
		printf("Cannot translate:Code contains array as functon parameter.\n");
		exit(0);
	}
	assert(ID);
	FieldList symbol = lookupSymbol(ID);
	InterCodes *codes = newInterCodes();
	codes->code.kind = IC_PARAM;
	codes->code.result.kind = VARIABLE;
	codes->code.result.u.name = symbol->name;
	return codes;
}

InterCodes* translate_CompSt(SyntaxTreeNode *CompSt)
{
	assert(CompSt != NULL);
	InterCodes *code1 = translate_DefList(CompSt->children[1]);
	InterCodes *code2 = translate_StmtList(CompSt->children[2]);
	if (code1 == NULL)
		return code2;
	return linkInterCode(code1, code2);
}

InterCodes* translate_StmtList(SyntaxTreeNode *StmtList)
{
	if (NULL == StmtList)
		return NULL;
	InterCodes *code1 = translate_Stmt(StmtList->children[0]);
	InterCodes *code2 = translate_StmtList(StmtList->children[1]);
	return linkInterCode(code1, code2);
}

InterCodes* translate_Stmt(SyntaxTreeNode *Stmt)
{
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
		code2->code.result.u.temp_id = t1;
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
		            linkInterCode(
		                linkInterCode(gen_Label(label2), code3),
		                gen_Label(label3))
		        );
	}
	case 5:
	{
		int label1 = newLabelId();
		int label2 = newLabelId();
		int label3 = newLabelId();
		InterCodes *code1 = translate_Cond(Stmt->children[2], label2, label3);
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

InterCodes* translate_DefList(SyntaxTreeNode *DefList)
{
	if (NULL == DefList)
		return NULL;
	InterCodes *code1 = translate_Def(DefList->children[0]);
	InterCodes *code2 = translate_DefList(DefList->children[1]);
	return linkInterCode(code1, code2);

}

InterCodes* translate_Def(SyntaxTreeNode *Def)
{
	Def->type->isParameter = 0;
	return translate_DecList(Def->children[1]);
}

InterCodes* translate_DecList(SyntaxTreeNode *DecList)
{
	assert(DecList);
	if (DecList->productionNum == 0)
		return translate_Dec(DecList->children[0]);
	else
	{
		InterCodes *code1 = translate_Dec(DecList->children[0]);
		InterCodes *code2 = translate_DecList(DecList->children[2]);
		return linkInterCode(code1, code2);
	}
}

InterCodes* translate_Dec(SyntaxTreeNode *Dec)
{
	//Dec -> VarDec ASSIGNOP Exp
	assert(Dec);
	if (0 == Dec->productionNum)
		return translate_VarDec(Dec->children[0]);
	else
	{
		//只支持基本类型在定义的时候初始化
		assert(0 == Dec->children[0]->productionNum);
		assert(BASIC == Dec->children[0]->type->kind);

		int t1 = newTempId();
		//Dec -> VarDec ASSIGNOP Exp || VarDec -> ID
		FieldList symbol = lookupSymbol(Dec->children[0]->children[0]);
		InterCodes* code1 = translate_Exp(Dec->children[2], t1);
		InterCodes* code2 = newInterCodes();
		code2->code.kind = IC_ASSIGN;
		code2->code.result.kind = VARIABLE;
		code2->code.result.u.name = symbol->name;
		code2->code.arg1.kind = TEMP;
		code2->code.arg1.u.temp_id = t1;
		return linkInterCode(code1, code2);
	}
}

InterCodes* translate_Exp(SyntaxTreeNode *Exp, int place)
{
	switch (Exp->productionNum)
	{
	case 0:/*Exp -> Exp1 ASSIGNOP Exp2*/
	{
		if (Exp->children[0]->productionNum == 15) //左值为基本类型的变量(没有结构体类型的赋值，所以这里是基本类型)
		{
			FieldList symbol = lookupSymbol(Exp->children[0]->children[0]);
			int t1 = newTempId();
			InterCodes *code1 = translate_Exp(Exp->children[2], t1);
			InterCodes *code2 = newInterCodes();
			code2->code.kind = IC_ASSIGN;
			code2->code.result.kind = VARIABLE;
			code2->code.result.u.name = symbol->name;
			code2->code.arg1.kind = TEMP;
			code2->code.arg1.u.temp_id = t1;

			InterCodes *code3 = newInterCodes();
			code3->code.kind = IC_ASSIGN;
			code3->code.result.kind = TEMP;
			code3->code.result.u.temp_id = place;
			code3->code.arg1.kind = VARIABLE;
			code3->code.arg1.u.name = symbol->name;
			return linkInterCode(linkInterCode(code1, code2), code3);
		}
		else if (Exp->children[0]->productionNum == 13 ||
		         Exp->children[0]->productionNum == 14) //左值为结构体或数组
		{
			int t1 = newTempId();
			int t2 = newTempId();
			InterCodes *code1 = translate_ADDR(Exp->children[0], t1);
			InterCodes *code2 = translate_Exp(Exp->children[2], t2);
			InterCodes *code3 = newInterCodes();
			InterCodes *code4 = newInterCodes();
			code3->code.kind = IC_DEREF_L;
			code3->code.result.kind = TEMP;
			code3->code.result.u.temp_id = t1;
			code3->code.arg1.kind = TEMP;
			code3->code.arg1.u.temp_id = t2;
			//这里为为了应对出现 a=b=c 这种连续赋值的情况
			code4->code.kind = IC_DEREF_R;
			code4->code.result.kind = TEMP;
			code4->code.result.u.temp_id = place;
			code4->code.arg1.kind = TEMP;
			code4->code.arg1.u.temp_id = t1;
			return linkInterCode(linkInterCode(code1, code2), linkInterCode(code3, code4));
		}
		else
			assert(0); //连续赋值?
	}
	case 1:/*Exp -> Exp AND Exp*/
	case 2:/*Exp -> Exp OR Exp*/
	case 3:/*Exp -> Exp RELOP Exp*/
	case 10: /*Exp -> NOT Exp*/
	{
		int label1 = newLabelId();
		int label2 = newLabelId();
		InterCodes* code0 = newInterCodes();
		code0->code.kind = IC_ASSIGN;
		code0->code.result.kind = TEMP;
		code0->code.result.u.temp_id = place;
		code0->code.arg1.kind = CONSTANT;
		code0->code.arg1.u.value = 0;
		InterCodes* code1 = translate_Cond(Exp, label1, label2);
		InterCodes* code2 = newInterCodes();
		code2->code.kind = IC_ASSIGN;
		code2->code.result.kind = TEMP;
		code2->code.result.u.temp_id = place;
		code2->code.arg1.kind = CONSTANT;
		code2->code.arg1.u.value = 1;
		return linkInterCode(
		           linkInterCode(
		               linkInterCode(code0, code1),
		               linkInterCode(gen_Label(label1), code2)),
		           gen_Label(label2)
		       );
	}
	case 4:/*Exp -> Exp PLUS Exp*/
	{
		int t1 = newTempId();
		int t2 = newTempId();
		InterCodes* code1 = translate_Exp(Exp->children[0], t1);
		InterCodes* code2 = translate_Exp(Exp->children[2], t2);
		InterCodes* code3 = newInterCodes();
		code3->code.kind = IC_PLUS;
		code3->code.result.kind = TEMP;
		code3->code.result.u.temp_id = place;
		code3->code.arg1.kind = TEMP;
		code3->code.arg1.u.temp_id = t1;
		code3->code.arg2.kind = TEMP;
		code3->code.arg2.u.temp_id = t2;

		return linkInterCode(linkInterCode(code1, code2), code3);
	}
	case 5:/*Exp -> Exp MINUS Exp*/
	{
		int t1 = newTempId();
		int t2 = newTempId();
		InterCodes* code1 = translate_Exp(Exp->children[0], t1);
		InterCodes* code2 = translate_Exp(Exp->children[2], t2);
		InterCodes* code3 = newInterCodes();
		code3->code.kind = IC_MINUS;
		code3->code.result.kind = TEMP;
		code3->code.result.u.temp_id = place;
		code3->code.arg1.kind = TEMP;
		code3->code.arg1.u.temp_id = t1;
		code3->code.arg2.kind = TEMP;
		code3->code.arg2.u.temp_id = t2;

		return linkInterCode(linkInterCode(code1, code2), code3);
	}
	case 6:/*Exp -> Exp STAR Exp*/
	{
		int t1 = newTempId();
		int t2 = newTempId();
		InterCodes* code1 = translate_Exp(Exp->children[0], t1);
		InterCodes* code2 = translate_Exp(Exp->children[2], t2);
		InterCodes* code3 = newInterCodes();
		code3->code.kind = IC_MUL;
		code3->code.result.kind = TEMP;
		code3->code.result.u.temp_id = place;
		code3->code.arg1.kind = TEMP;
		code3->code.arg1.u.temp_id = t1;
		code3->code.arg2.kind = TEMP;
		code3->code.arg2.u.temp_id = t2;

		return linkInterCode(linkInterCode(code1, code2), code3);
	}
	case 7:/*Exp -> Exp DIV Exp*/
	{
		int t1 = newTempId();
		int t2 = newTempId();
		InterCodes* code1 = translate_Exp(Exp->children[0], t1);
		InterCodes* code2 = translate_Exp(Exp->children[2], t2);
		InterCodes* code3 = newInterCodes();
		code3->code.kind = IC_DIV;
		code3->code.result.kind = TEMP;
		code3->code.result.u.temp_id = place;
		code3->code.arg1.kind = TEMP;
		code3->code.arg1.u.temp_id = t1;
		code3->code.arg2.kind = TEMP;
		code3->code.arg2.u.temp_id = t2;

		return linkInterCode(linkInterCode(code1, code2), code3);
	}
	case 8: /*Exp -> LP Exp RP*/
	{
		return translate_Exp(Exp->children[1], place);
	}
	case 9:/*Exp -> MINUS Exp*/
	{
		int t1 = newTempId();
		InterCodes* code1 = translate_Exp(Exp->children[1], t1);
		InterCodes* code2 = newInterCodes();
		code2->code.kind = IC_MINUS;
		code2->code.result.kind = TEMP;
		code2->code.result.u.temp_id = place;
		code2->code.arg1.kind = CONSTANT;
		code2->code.arg1.u.value = 0;
		code2->code.arg2.kind = TEMP;
		code2->code.arg2.u.temp_id = t1;
		return linkInterCode(code1, code2);
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
			code2->code.result.u.temp_id = arglist->id[0];
			//here
			return linkInterCode(code1, code2);
		}
		//Arg传入参数的顺序和PARAM声明参数的顺序相反
		for (int i = arglist->count - 1; i >= 0; --i)
		{
			InterCodes *code2 = newInterCodes();
			code2->code.kind = IC_ARG;
			code2->code.result.kind = TEMP;
			code2->code.result.u.temp_id = arglist->id[i];
			code1 = linkInterCode(code1, code2);
		}
		InterCodes *code3 = newInterCodes();
		code3->code.kind = IC_CALL;
		code3->code.result.kind = TEMP;
		code3->code.result.u.temp_id = place;
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
			code1->code.result.u.temp_id = place;
		}
		else
		{
			code1->code.kind = IC_CALL;
			code1->code.result.kind = TEMP;
			code1->code.result.u.temp_id = place;
			code1->code.arg1.kind = FUNCTION;
			code1->code.arg1.u.name = func.name;
		}
		return code1;

		break;
	}
	case 13:/*Exp -> Exp LB Exp RB*/
	case 14:/*Exp -> Exp DOT ID*/
	{
		int t1 = newTempId();
		InterCodes* code1 = translate_ADDR(Exp, t1);
		InterCodes* code2 = newInterCodes();
		code2->code.kind = IC_DEREF_R;
		code2->code.result.kind = TEMP;
		code2->code.result.u.temp_id = place;
		code2->code.arg1.kind = TEMP;
		code2->code.arg1.u.temp_id = t1;
		return linkInterCode(code1, code2);
	}
	case 15:/*Exp -> ID*/
	{
		FieldList symbol = lookupSymbol(Exp->children[0]);
		InterCodes* codes = newInterCodes();

		if (symbol->type->kind == BASIC) {
			codes->code.kind = IC_ASSIGN;
			codes->code.result.kind = TEMP;
			codes->code.result.u.temp_id = place;
			codes->code.arg1.kind = VARIABLE;
			codes->code.arg1.u.name = symbol->name;
		} else {
			if (symbol->type->isParameter) {
				//当引用的是函数的形参的时候，传过来的已经是地址了，不需要再取地址
				codes->code.kind = IC_ASSIGN;
				codes->code.result.kind = TEMP;
				codes->code.result.u.temp_id = place;
				codes->code.arg1.kind = VARIABLE;
				codes->code.arg1.u.name = symbol->name;
			} else {
				codes->code.kind = IC_ADDR;
				codes->code.result.kind = TEMP;
				codes->code.result.u.temp_id = place;
				codes->code.arg1.kind = VARIABLE;
				codes->code.arg1.u.name = symbol->name;
			}
		}

		return codes;
	}
	case 16: /*Exp -> INT*/
	{
		int num = atoi(Exp->children[0]->content);
		InterCodes* codes = newInterCodes();
		codes->code.kind = IC_ASSIGN;
		codes->code.result.kind = TEMP;
		codes->code.result.u.temp_id = place;
		codes->code.arg1.kind = CONSTANT;
		codes->code.arg1.u.value = num;
		return codes;
		break;
	}
	case 17: /*Exp -> FLOAT*/
	{
		assert(0);
		break;
	}
	}
}

InterCodes* translate_Cond(SyntaxTreeNode *Exp, int label_true, int label_false)
{
	switch (Exp->productionNum)
	{
	case 1:/*Exp -> Exp AND Exp*/
	{
		int label1 = newLabelId();
		InterCodes* code1 = translate_Cond(Exp->children[0], label1, label_false);
		InterCodes* code2 = translate_Cond(Exp->children[2], label_true, label_false);
		return linkInterCode(linkInterCode(code1, gen_Label(label1)), code2);
	}
	case 2:/*Exp -> Exp OR Exp*/
	{
		int label1 = newLabelId();
		InterCodes* code1 = translate_Cond(Exp->children[0], label_true, label1);
		InterCodes* code2 = translate_Cond(Exp->children[2], label_true, label_false);
		return linkInterCode(linkInterCode(code1, gen_Label(label1)), code2);
	}
	case 3:/*Exp -> Exp Relop Exp*/
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
		code3->code.arg1.u.temp_id = t1;
		code3->code.arg2.kind = TEMP;
		code3->code.arg2.u.temp_id = t2;

		//generate GOTO
		InterCodes* code4 = newInterCodes();
		code4->code.kind = IC_GOTO;
		code4->code.result.kind = LABEL;
		code4->code.result.u.label_id = label_false;

		return linkInterCode(
		           linkInterCode(code1, code2),
		           linkInterCode(code3, code4));
	}
	case 10: /*Exp -> NOT Exp*/
	{
		return translate_Cond(Exp->children[1], label_false, label_true);
	}
	default:/*other case*/
	{
		int t1 = newTempId();
		InterCodes* code1 = translate_Exp(Exp, t1);
		InterCodes* code2 = newInterCodes();
		code2->code.kind = IC_RELOP;
		code2->code.relop = NE;
		code2->code.result.kind = LABEL;
		code2->code.result.u.label_id = label_true;
		code2->code.arg1.kind = TEMP;
		code2->code.arg1.u.temp_id = t1;
		code2->code.arg2.kind = CONSTANT;
		code2->code.arg2.u.value = 0;
		return linkInterCode(linkInterCode(code1, code2), gen_Goto(label_false));
	}
	}
}

InterCodes* translate_Args(SyntaxTreeNode *Args, ArgList* arglist)
{
	if (Args->productionNum == 1) /*Args -> Exp*/
	{
		int t1 = newTempId();
		if (Args->children[0]->type->kind == STRUCTURE)
		{
			//InterCodes *code1 =translate_Exp(Args->children[0], t1);
			//int t2 = newTempId();
			assert(Args->children[0]->productionNum == 15);
			FieldList symbol = lookupSymbol(Args->children[0]->children[0]);
			assert(symbol);
			InterCodes *code1 = newInterCodes();
			code1->code.kind = IC_ADDR;
			code1->code.result.kind = TEMP;
			code1->code.result.u.temp_id = t1;
			code1->code.arg1.kind = VARIABLE;
			code1->code.arg1.u.name = symbol->name;

			arglist->id[arglist->count++] = t1;
			return code1;
		}
		else {
			int t1 = newTempId();
			arglist->id[arglist->count++] = t1;
			return translate_Exp(Args->children[0], t1);
		}
	}
	else /*Args -> Exp COMMA Args*/
	{
		int t1 = newTempId();
		InterCodes *code1 = translate_Exp(Args->children[0], t1);
		arglist->id[arglist->count++] = t1;
		InterCodes *code2 = translate_Args(Args->children[2], arglist);
		return linkInterCode(code1, code2);
	}
}

/*
 place = 该引用的地址
*/
InterCodes* translate_ADDR(SyntaxTreeNode *Exp, int place)
{
	if (Exp->productionNum == 13) /*Exp -> Exp LB Exp RB*/
	{
		if (Exp->children[0]->productionNum == 13) {
			printf("Cannot translate:Code contains vriables of multi-dimensional arrry type.\n");
			exit(0);
		}
		FieldList symbol;
		if (Exp->children[0]->productionNum == 14)
			symbol = lookupSymbol(Exp->children[0]->children[2]);
		else if (Exp->children[0]->productionNum == 15)
			symbol = lookupSymbol(Exp->children[0]->children[0]);
		else
			assert(0);

		assert(symbol->type->kind == ARRAY);
		int t1 = newTempId();
		int t2 = newTempId();
		int t3 = newTempId();

		InterCodes* code1 = translate_ADDR(Exp->children[0], t1);
		InterCodes* code2 = translate_Exp(Exp->children[2], t2);
		InterCodes* code3 = newInterCodes();
		code3->code.kind = IC_MUL;        //计算偏移量
		code3->code.result.kind = TEMP;
		code3->code.result.u.temp_id = t3;
		code3->code.arg1.kind = TEMP;
		code3->code.arg1.u.temp_id = t2;
		code3->code.arg2.kind = CONSTANT;
		code3->code.arg2.u.value = SizeOfType(symbol->type->u.array.elem);

		InterCodes* code4 = newInterCodes();
		code4->code.kind = IC_PLUS;
		code4->code.result.kind = TEMP;
		code4->code.result.u.temp_id = place;
		code4->code.arg1.kind = TEMP;
		code4->code.arg1.u.temp_id = t1;
		code4->code.arg2.kind = TEMP;
		code4->code.arg2.u.temp_id = t3;
		return linkInterCode(linkInterCode(code1, code2), linkInterCode(code3, code4));
	}
	else if (Exp->productionNum == 14) /*Exp -> Exp DOT ID*/
	{
		int offset = 0;
		FieldList structHead;
		FieldList symbol;
		if (Exp->children[0]->productionNum == 13) /*Exp -> Exp LB Exp RB*/
		{
			if (Exp->children[0]->children[0]->productionNum == 14)
				symbol = lookupSymbol(Exp->children[0]->children[0]->children[2]);
			else if (Exp->children[0]->children[0]->productionNum == 15)
				symbol = lookupSymbol(Exp->children[0]->children[0]->children[0]);
			else
				assert(0);
		}
		else if (Exp->children[0]->productionNum == 15) /*Exp -> ID*/
			symbol = lookupSymbol(Exp->children[0]->children[0]);
		else
			assert(0);


		if (ARRAY == symbol->type->kind) {
			assert(symbol->type->u.array.elem->kind == STRUCTURE);
			structHead = symbol->type->u.array.elem->u.structure->tail;
		}
		else if (STRUCTURE == symbol->type->kind) {
			structHead = symbol->type->u.structure->tail;
		}
		else {
			printf("Cannot translate:Code contains vriables of multi-dimensional array type.\n");
			exit(0);
		}
		//Type symbol = Exp->children[0]->children[0]->type;
		assert(structHead);
		for (FieldList p = structHead; p != NULL; p = p->tail)
		{
			if (strcmp(p->name, Exp->children[2]->content) == 0)
				break;
			offset += SizeOfType(p->type);
		}
		int t1 = newTempId();
		InterCodes *code1 = translate_ADDR(Exp->children[0], t1);
		InterCodes *code2 = newInterCodes();
		code2->code.kind = IC_PLUS;
		code2->code.result.kind = TEMP;
		code2->code.result.u.temp_id = place;
		code2->code.arg1.kind = TEMP;
		code2->code.arg1.u.temp_id = t1;
		code2->code.arg2.kind = CONSTANT;
		code2->code.arg2.u.temp_id = offset;

		return linkInterCode(code1, code2);
	}
	else if (Exp->productionNum == 15) /*Exp -> ID*/
	{
		return translate_Exp(Exp, place);
	}
	assert(0);
}

int isChanged = 0;
void code_optimization(InterCodes* codes)
{
	do {
		isChanged = 0;
		replace_constant(codes);
		replace_variable(codes);
	} while (isChanged);
}

int isConstantMap(int temp_id)
{
	for (int i = 0; i < Map_Max; ++i)
	{
		if (constant_map[i].temp_id == temp_id)
			return i;
	}
	return -1;
}

void replace_constant(InterCodes* codes)
{
	//临时变量 — 常量 之间的替换 t1 = #1, v1 = t1 -> v1 = #1
	for (InterCodes *p = codes; p != NULL; p = p->next)
	{
		switch (p->code.kind)
		{
		case IC_ASSIGN: {
			if (p->code.result.kind == TEMP && p->code.arg1.kind == CONSTANT)
			{
				constant_map[constant_count].value = p->code.arg1.u.value;
				constant_map[constant_count].temp_id = p->code.result.u.temp_id;
				if (++constant_count == Map_Max)
					constant_count = 0;
				p = deleteCode(p);
				isChanged = 1;
			}
			else if (p->code.arg1.kind == TEMP && isConstantMap(p->code.arg1.u.temp_id) >= 0)
			{
				int index = isConstantMap(p->code.arg1.u.temp_id);
				p->code.arg1.kind = CONSTANT;
				p->code.arg1.u.value = constant_map[index].value;
				isChanged = 1;
			}
			break;
		}
		case IC_PLUS:
		case IC_MINUS:
		case IC_MUL:
		case IC_DIV:
		case IC_RELOP: {
			if (p->code.arg1.kind == TEMP && isConstantMap(p->code.arg1.u.temp_id) >= 0)
			{
				int index = isConstantMap(p->code.arg1.u.temp_id);
				p->code.arg1.kind = CONSTANT;
				p->code.arg1.u.value = constant_map[index].value;
				isChanged = 1;
			}
			if (p->code.arg2.kind == TEMP && isConstantMap(p->code.arg2.u.temp_id) >= 0)
			{
				int index = isConstantMap(p->code.arg2.u.temp_id);
				p->code.arg2.kind = CONSTANT;
				p->code.arg2.u.value = constant_map[index].value;
				isChanged = 1;
			}
			break;
		}
		case IC_RETURN:
		case IC_ARG:
		case IC_WRITE: {
			if (p->code.result.kind == TEMP && isConstantMap(p->code.result.u.temp_id) >= 0)
			{
				int index = isConstantMap(p->code.result.u.temp_id);
				p->code.result.kind = CONSTANT;
				p->code.result.u.value = constant_map[index].value;
				isChanged = 1;
			}
			break;
		}
		case IC_DEREF_L: {
			if (p->code.arg1.kind == TEMP && isConstantMap(p->code.arg1.u.temp_id) >= 0)
			{
				int index = isConstantMap(p->code.arg1.u.temp_id);
				p->code.arg1.kind = CONSTANT;
				p->code.arg1.u.value = constant_map[index].value;
				isChanged = 1;
			}
			break;
		}
		default: break;
		}
	}

	// t1 = t2 + #0 or t1 = t1 - #0 ->t1 = t2
	for (InterCodes *p = codes; p != NULL; p = p->next)
	{
		if ((p->code.kind == IC_PLUS || p->code.kind == IC_MINUS)
		        && p->code.arg2.kind == CONSTANT && p->code.arg2.u.value == 0)
		{
			p->code.kind = IC_ASSIGN;
			isChanged = 1;
		}
	}

	//减少常量计算 t1 = #2 - #1, t2 = t1 + #1  -> t2 = #2
	for (InterCodes *p = codes; p != NULL; p = p->next)
	{
		if ((p->code.kind == IC_PLUS || p->code.kind == IC_MINUS || p->code.kind == IC_MUL || p->code.kind == IC_DIV)
		        && p->code.arg1.kind == CONSTANT && p->code.arg2.kind == CONSTANT)
		{
			int new_value = 0;
			switch (p->code.kind)
			{
			case IC_PLUS: new_value = p->code.arg1.u.value + p->code.arg2.u.value; break;
			case IC_MINUS: new_value = p->code.arg1.u.value - p->code.arg2.u.value; break;
			case IC_MUL:  new_value = p->code.arg1.u.value * p->code.arg2.u.value; break;
			case IC_DIV:  new_value = p->code.arg1.u.value / p->code.arg2.u.value; break;
			default: break;
			}
			p->code.kind = IC_ASSIGN;
			p->code.arg1.u.value = new_value;
			isChanged = 1;
		}
	}
}

int isVariableMap(int temp_id)
{
	for (int i = 0; i < Map_Max; ++i)
	{
		if (variable_map[i].temp_id == temp_id)
			return i;
	}
	return -1;
}

//临时变量 — 变量 之间的替换
void replace_variable(InterCodes* codes)
{
	InterCodes *p = codes;
	while (p != NULL)
	{
		switch (p->code.kind)
		{
		case IC_ASSIGN: {
			if (p->code.result.kind == TEMP && p->code.arg1.kind == VARIABLE)
			{
				if (p->code.result.u.temp_id == 0) {
					p = deleteCode(p);
					break;
				}
				variable_map[variable_count].name = p->code.arg1.u.name;
				variable_map[variable_count].temp_id = p->code.result.u.temp_id;
				if (++variable_count == Map_Max)
					variable_count = 0;
				p = deleteCode(p);
				isChanged = 1;
			}
			else if (p->code.arg1.kind == TEMP && isVariableMap(p->code.arg1.u.temp_id) >= 0)
			{
				int index = isVariableMap(p->code.arg1.u.temp_id);
				p->code.arg1.kind = VARIABLE;
				p->code.arg1.u.name = variable_map[index].name;
				isChanged = 1;
			}
			break;
		}
		case IC_PLUS:
		case IC_MINUS:
		case IC_MUL:
		case IC_DIV:
		case IC_RELOP: {
			if (p->code.arg1.kind == TEMP && isVariableMap(p->code.arg1.u.temp_id) >= 0)
			{
				int index = isVariableMap(p->code.arg1.u.temp_id);
				p->code.arg1.kind = VARIABLE;
				p->code.arg1.u.name = variable_map[index].name;
				isChanged = 1;
			}
			if (p->code.arg2.kind == TEMP && isVariableMap(p->code.arg2.u.temp_id) >= 0)
			{
				int index = isVariableMap(p->code.arg2.u.temp_id);
				p->code.arg2.kind = VARIABLE;
				p->code.arg2.u.name = variable_map[index].name;
				isChanged = 1;
			}
			break;
		}
		case IC_RETURN:
		case IC_ARG:
		case IC_WRITE: {
			if (p->code.result.kind == TEMP && isVariableMap(p->code.result.u.temp_id) >= 0)
			{
				int index = isVariableMap(p->code.result.u.temp_id);
				p->code.result.kind = VARIABLE;
				p->code.result.u.name = variable_map[index].name;
				isChanged = 1;
			}
			break;
		}
		case IC_DEREF_L: {
			if (p->code.arg1.kind == TEMP && isVariableMap(p->code.arg1.u.temp_id) >= 0)
			{
				int index = isVariableMap(p->code.arg1.u.temp_id);
				p->code.arg1.kind = VARIABLE;
				p->code.arg1.u.name = variable_map[index].name;
				isChanged = 1;
			}
			break;
		}
		default: break;
		}
		p = p->next;
	}
}

/*if t1 > t2 GOTO label1
  GOTO label2
	->
  if t1 <= t2 GOTO label2
*/
void delete_Goto(InterCodes* codes)
{
	for (InterCodes *p = codes; p != NULL; p = p->next)
	{
		if (p->code.kind == IC_RELOP)
		{
			switch (p->code.relop)
			{
			case LT: p->code.relop = GE; break;
			case LE: p->code.relop = GT; break;
			case EQ: p->code.relop = NE; break;
			case GT: p->code.relop = LE; break;
			case GE: p->code.relop = LT; break;
			case NE: p->code.relop = EQ; break;
			default: assert(0);
			}
			assert(p->next->code.kind == IC_GOTO);
			p->code.result.u.label_id = p->next->code.result.u.label_id;
			deleteCode(p->next);
		}
	}
}

InterCodes* deleteCode(InterCodes* codes)
{
	if (NULL == codes->prev) //head
		return codes->next;

	codes->prev->next = codes->next;
	if (codes->next != NULL) //not the tail
		codes->next->prev = codes->prev;
	return codes->prev;
}

