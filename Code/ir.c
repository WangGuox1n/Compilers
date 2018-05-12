#include "ir.h"

void gen_InterCode(SyntaxTreeNode *root){
	InterCodes* interCodes = translate_Program(root);
    for (InterCodes* p = codes; p != NULL; p = p->next) 
    {
        switch(p->code.kind) {
            case IR_LABEL: {
                printf("LABEL label%d :\n", p->code.result.u.label_id);
                break;
            }
            case IR_FUNC: {
                printf("FUNCTION %s :\n", p->code.result.symbol->name);
                break;
            }
            case IR_ASSIGN: {
                if (p->code.result.kind == OP_TEMP && p->code.result.u.var_id == VAR_NULL) {
                    break;
                }
                printOperand(p->code.result);
                printf(" := ");
                printOperand(p->code.arg1);
                break;
            }
            case IR_ADD: {
                printOperand(p->code.result);printf(" := ");
                printOperand(p->code.arg1);  printf(" + ");printOperand(p->code.arg2);
                break;
            }
            case IR_SUB: {
                printOperand(p->code.result);printf(" := ");
                printOperand(p->code.arg1);  printf(" - ");printOperand(p->code.arg2);
                break;
            }
            case IR_MUL: {
                printOperand(p->code.result);printf(" := ");
                printOperand(p->code.arg1);  printf(" * ");printOperand(p->code.arg2);
                break;
            }
            case IR_DIV: {
                printOperand(p->code.result);printf(" := ");
                printOperand(p->code.arg1);  printf(" / ");printOperand(p->code.arg2);
                break;
            }
            case IR_GOTO: {
                printf("GOTO ");printOperand(p->code.result);
                break;
            }
            case IR_RELOP: {
                printf("IF ");
                printOperand(p->code.arg1);
                switch (p->code.relop) {
                    case RELOP_LT: printf(" < "); break;
                    case RELOP_LE: printf(" <= "); break;
                    case RELOP_EQ: printf(" == "); break;
                    case RELOP_GT: printf(" > "); break;
                    case RELOP_GE: printf(" >= "); break;
                    case RELOP_NE: printf(" != "); break;
                    default: assert(0);
                }
                printOperand(p->code.arg2);printf(" GOTO ");printOperand(p->code.result);
                break;
            }
            case IR_DEC: {
                printf("DEC ");printOperand(p->code.result);printf(" %d", p->code.size);
                break;
            }
            case IR_RETURN: {
                printf("RETURN ");printOperand(p->code.result);
                break;
            }
            case IR_ARG: {
                printf("ARG ");printOperand(p->code.result);
                break;
            }
            case IR_CALL: {
                printOperand(p->code.result);
                printf(" := CALL ");printOperand(p->code.arg1);
                break;
            }
            case IR_PARAM: {
                printf("PARAM ");printOperand(p->code.result);
                break;
            }
            case IR_READ: {
                printf("READ ");printOperand(p->code.result);
                break;
            }
            case IR_WRITE: {
                printf("WRITE ");printOperand(p->code.result);
                break;
            }
            default: assert(0);
        }
        printf("\n");
    }
}
InterCodes* newInterCode(){

}
InterCodes* concat_Code(int index,InterCodes* code1,InterCodes* code2,InterCodes* code3)
{	
	if(index==2)
	{
		assert(code3==NULL);
		code1->next = code2;		
		if(codeHead == NULL)
			codeHead = code1;
		else
			codeTail->next = code1;
		codeTail = code2;
	}
}

int newTempId(){ return tempId++; }
int newLabelId(){ return labelId++; }

InterCodes* translate_Exp(SyntaxTreeNode *Exp, int place)
{
	switch (Exp->productionNum)
	{
	case 0:/*Exp -> Exp ASSIGNOP Exp*/
	{
		if(Exp->productionNum == 15)
		{
			FieldList symbol = lookupSymbol(Exp->children[0]->children[0]);
			int t1 = newTempId();
			InterCodes *code1 = translate_Exp(Exp->children[2],t1);
			InterCodes *code2 = newInterCode();
			code2->code.kind = IC_ASSIGN;
			code2->code.result.kind = VARIABLE;
			code2->code.result.u.name = symbol.name;
			code2->code.arg1.kind = TEMP;
			code2->code.arg1.u.var_id = t1;
			return concat_Code(2,code1,code2,NULL);
		}
		break;
	}
	case 1:/*Exp -> Exp AND Exp*/
	case 2:/*Exp -> Exp OR Exp*/
	case 3:/*Exp -> Exp RELOP Exp*/
	{
		int t1 = newLabelId();
		int t2 = newLabelId();
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
	case 10:/*Exp -> NOT Exp*/
	{
		break;
	}
	case 11:/*Exp -> ID LP Args RP*/
	{
		break;
	}
	case 12:/*Exp -> ID LP RP*/
	{
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
	}
	case 16: /*Exp -> INT*/
	{
		break;
	}
	case 17: /*Exp -> FLOAT*/
	{
		break;
	}
	}
}