#include "obeject_code.h"
#include "ir.h"
#include <assert.h>
void generate_oc(SyntaxTreeNode *root)
{
	initReg();
	generate_head();
	InterCodes* codes = translate_Program(root);
	code_optimization(codes);
	generate_text(codes);
}

void generate_head()
{
	printf(".data\n");
	printf("_prompt: .asciiz \"Enter an integer:\"\n");
	printf("_ret: .asciiz \"\\n\"\n");
	printf(".globl main\n");
	printf(".text\n");

	//function:read
	printf("read:\n");
	printf("\tli $v0, 4\n");
	printf("\tla $a0, _prompt\n");
	printf("\tsyscall\n");
	printf("\tli $v0, 5\n");
	printf("\tsyscall\n");
	printf("\tjr $ra\n");

	//function:write
	printf("\nwrite:\n");
	printf("\tli $v0, 1\n");
	printf("\tsyscall\n");
	printf("\tli $v0, 4\n");
	printf("\tla $a0, _ret\n");
	printf("\tsyscall\n");
	printf("\tmove $v0, $0\n");
	printf("\tjr $ra\n");
}

void genarate_call(char *function_name)
{
	printf("\taddi $sp, $sp, -4\n");
	printf("\tsw $ra, 0($sp)\n");
	printf("\tjal %s\n", function_name);
	printf("\tlw $ra, 0($sp)\n");
	printf("\taddi $sp, $sp, 4\n");
}

void generate_prologue()
{
	printf("\taddi $sp, $sp, -4\n");
	printf("\tsw $fp, 0($sp)\n");
	printf("\tmove $fp, $sp\n");
}

void generate_epilogue()
{
	printf("\tmove $sp, $fp\n");
	printf("\tlw $fp, 0($sp)\n");
	printf("\taddi $sp, $sp, 4\n");
}

void generate_text(InterCodes* codes)
{
	for (InterCodes* p = codes; p != NULL; p = p->next)
	{
		int t0, t1, t2;
		t0 = t1 = t2 = -1;
		switch (p->code.kind) {
		case IC_LABEL: {
			printf("label%d:\n", p->code.result.u.label_id);
			break;
		}
		case IC_FUNC: {
			//every time begin a func, reset offset
			offset2fp = 0;
			offset2sp = 8;
			varList = NULL;
			printf("\n%s:\n", p->code.result.u.name);
			generate_prologue();
			break;
		}
		case IC_ASSIGN: {
			t0 = getReg(p->code.result);
			if (p->code.arg1.kind == CONSTANT) {
				printf("\tli $t%d, %d\n", t0, p->code.arg1.u.value);
			} else {
				t1 = getReg(p->code.arg1);
				printf("\tmove $t%d, $t%d\n", t0, t1);
			}
			spillReg(t0);
			break;
		}
		case IC_PLUS: {
			t0 = getReg(p->code.result);
			if (p->code.arg1.kind == CONSTANT && p->code.arg2.kind == CONSTANT)
				assert(0);
			if (p->code.arg1.kind == CONSTANT) {
				t1 = getReg(p->code.arg2);
				printf("\taddi $t%d, $t%d, %d\n", t0, t1, p->code.arg1.u.value);
			} else if (p->code.arg2.kind == CONSTANT) {
				t1 = getReg(p->code.arg1);
				printf("\taddi $t%d, $t%d, %d\n", t0, t1, p->code.arg2.u.value);
			} else {
				t1 = getReg(p->code.arg1);
				t2 = getReg(p->code.arg2);
				printf("\tadd $t%d, $t%d, $t%d\n", t0, t1, t2);
			}
			spillReg(t0);
			break;
		}
		case IC_MINUS: {
			t0 = getReg(p->code.result);
			if (p->code.arg1.kind == CONSTANT && p->code.arg2.kind == CONSTANT)
				assert(0);
			if (p->code.arg1.kind == CONSTANT) {
				t1 = getReg(p->code.arg2);
				printf("\taddi $t%d, $t%d, %d\n", t0, t1, 0 - p->code.arg1.u.value);
			} else if (p->code.arg2.kind == CONSTANT) {
				t1 = getReg(p->code.arg1);
				printf("\taddi $t%d, $t%d, %d\n", t0, t1, 0 - p->code.arg2.u.value);
			} else {
				t1 = getReg(p->code.arg1);
				t2 = getReg(p->code.arg2);
				printf("\tsub $t%d, $t%d, $t%d\n", t0, t1, t2);
			}
			spillReg(t0);
			break;
		}
		case IC_MUL: {
			t0 = getReg(p->code.result);
			t1 = getReg(p->code.arg1);
			t2 = getReg(p->code.arg2);
			printf("\tmul $t%d, $t%d, $t%d\n", t0, t1, t2);
			spillReg(t0);
			break;
		}
		case IC_DIV: {
			t0 = getReg(p->code.result);
			t1 = getReg(p->code.arg1);
			t2 = getReg(p->code.arg2);
			printf("\tdiv $t%d, $t%d\n", t1, t2);
			printf("\tmflo $t%d\n", t0);
			spillReg(t0);
			break;
		}
		case IC_ADDR: {
			t0 = getReg(p->code.result);
			Variable* var = allocate(p->code.arg1);
			printf("\tla $t%d, %d($fp)\n", t0, var->offset);
			spillReg(t0);
			break;
		}
		case IC_DEREF_R: {
			t0 = getReg(p->code.result);
			t1 = getReg(p->code.arg1);
			printf("\tlw $t%d, 0($t%d)\n", t0, t1);  //t1 is address
			spillReg(t0);
			break;
		}
		case IC_DEREF_L: {
			t0 = getReg(p->code.result);
			t1 = getReg(p->code.arg1);
			printf("\tsw $t%d, 0($t%d)\n", t1, t0);  //t0 is address
			break;
		}
		case IC_GOTO: {
			printf("\tj label%d\n", p->code.result.u.label_id);
			break;
		}
		case IC_RELOP: {
			t1 = getReg(p->code.arg1);
			t2 = getReg(p->code.arg2);
			switch (p->code.relop) {
			case LT: printf("\tblt "); break;
			case LE: printf("\tble "); break;
			case EQ: printf("\tbeq "); break;
			case GT: printf("\tbgt "); break;
			case GE: printf("\tbge "); break;
			case NE: printf("\tbne "); break;
			default: assert(0);
			}
			printf("$t%d, $t%d, label%d\n", t1, t2, p->code.result.u.label_id);
			break;
		}
		case IC_DEC: {
			Variable* var = (Variable*)malloc(sizeof(Variable));
			var->kind = OP_VARIABLE;
			var->u.name = p->code.result.u.name;
			var->offset = (offset2fp -= p->code.arg1.u.value);
			var->next = varList;
			varList = var;
			printf("\taddi $sp, $sp, -%d\n", p->code.arg1.u.value);
			break;
		}
		case IC_RETURN: {
			t0 = getReg(p->code.result);
			printf("\tmove $v0, $t%d\n", t0);
			generate_epilogue();
			printf("\tjr $ra\n");
			break;
		}
		case IC_ARG: {
			/* the arg's order is opposite to param,
			 * so put all the arg into stack, and in IC_PARAM,
			 * just read parameter from stack by order
			 */
			t0 = getReg(p->code.result);
			printf("\taddi $sp, $sp, -4\n");
			printf("\tsw $t%d, 0($sp)\n", t0);
			break;
		}
		case IC_CALL: {
			genarate_call(p->code.arg1.u.name);
			// be care of the change of sp.
			// because I use the offset2sp to read the parameter
			t0 = getReg(p->code.result);
			printf("\tmove $t%d, $v0\n", t0);
			spillReg(t0);
			break;
		}
		case IC_PARAM: {
			t0 = getReg(p->code.result);
			printf("\tlw $t%d, %d($fp)\n", t0, offset2sp);  //fp is call function's fp
			offset2sp += 4;   //only integer can be parameter
			spillReg(t0);
			break;
		}
		case IC_READ: {
			t0 = getReg(p->code.result);
			genarate_call("read");
			printf("\tmove $t%d, $v0\n", t0);
			spillReg(t0);
			break;
		}
		case IC_WRITE: {
			t0 = getReg(p->code.result);
			printf("\tmove $a0, $t%d\n", t0);
			genarate_call("write");
			break;
		}
		default: assert(0);
		}
		if (t0 >= 0) freeReg(t0);
		if (t1 >= 0) freeReg(t1);
		if (t2 >= 0) freeReg(t2);
	}
}

void initReg()
{
	for (int i = 0; i < 10; i ++) {
		regs[i].valid = 1;
		regs[i].var = (Variable*)malloc(sizeof(Variable));
		regs[i].var->kind = -1;
	}
}

int getReg(Operand operand)
{
	for (int i = 0; i < 10; i ++)
	{
		if (regs[i].valid)
		{
			if (operand.kind == CONSTANT) {
				printf("\tli $t%d, %d\n", i, operand.u.value);
			} else {
				int offset = offset2fp;
				regs[i].var = allocate(operand);

				//only when the operand is already send to stack,then you load it
				if (offset == offset2fp) {
					printf("\tlw $t%d, %d($fp)\n", i, regs[i].var->offset);
				}
			}
			regs[i].valid = 0;
			return i;
		}
	}
	assert(0);
}

Variable* allocate(Operand operand)
{
	for (Variable *p = varList; p != NULL; p = p->next)
	{
		if ((operand.kind == TEMP && p->kind == OP_TEMP
		        && operand.u.temp_id == p->u.id)
		        || (operand.kind == VARIABLE && p->kind == OP_VARIABLE
		            && strcmp(operand.u.name, p->u.name) == 0))
		{
			return p;
		}
	}

	Variable* var = (Variable*)malloc(sizeof(Variable));
	if (operand.kind == TEMP) {
		var->kind = OP_TEMP;
		var->u.id = operand.u.temp_id;
	} else if (operand.kind == VARIABLE) {
		var->kind = OP_VARIABLE;
		var->u.name = operand.u.name;
	} else
		assert(0);
	printf("\taddi $sp, $sp, -4\n");
	var->offset = (offset2fp -= sizeof(int));
	var->next = varList;
	varList = var;
	return var;
}

void freeReg(int i)
{
	assert(i >= 0);
	assert(i < 10);
	regs[i].valid = 1;
	regs[i].var = NULL;
}

void spillReg(int i)
{
	if (NULL == regs[i].var)
		return;
	assert(i >= 0);
	assert(i < 10);
	printf("\tsw $t%d, %d($fp)\n", i, regs[i].var->offset);
}