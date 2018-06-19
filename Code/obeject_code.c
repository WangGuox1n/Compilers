#include "obeject_code.h"
#include "ir.h"
#include <assert.h>
void generate_oc(FILE* fout, SyntaxTreeNode *root)
{
	initReg();
	generate_head(fout);
	InterCodes* codes = translate_Program(root);
	code_optimization(codes);
	generate_text(fout, codes);
}

void generate_head(FILE* fout)
{
	fprintf(fout, ".data\n");
	fprintf(fout, "_prompt: .asciiz \"Enter an integer:\"\n");
	fprintf(fout, "_ret: .asciiz \"\\n\"\n");
	fprintf(fout, ".globl main\n");
	fprintf(fout, ".text\n");

	//function:read
	fprintf(fout, "read:\n");
	fprintf(fout, "\tli $v0, 4\n");
	fprintf(fout, "\tla $a0, _prompt\n");
	fprintf(fout, "\tsyscall\n");
	fprintf(fout, "\tli $v0, 5\n");
	fprintf(fout, "\tsyscall\n");
	fprintf(fout, "\tjr $ra\n");

	//function:write
	fprintf(fout, "\nwrite:\n");
	fprintf(fout, "\tli $v0, 1\n");
	fprintf(fout, "\tsyscall\n");
	fprintf(fout, "\tli $v0, 4\n");
	fprintf(fout, "\tla $a0, _ret\n");
	fprintf(fout, "\tsyscall\n");
	fprintf(fout, "\tmove $v0, $0\n");
	fprintf(fout, "\tjr $ra\n");
}

void genarate_call(FILE* fout, char *function_name)
{
	fprintf(fout, "\taddi $sp, $sp, -4\n");
	fprintf(fout, "\tsw $ra, 0($sp)\n");
	fprintf(fout, "\tjal %s\n", function_name);
	fprintf(fout, "\tlw $ra, 0($sp)\n");
	fprintf(fout, "\taddi $sp, $sp, 4\n");
}

void generate_prologue(FILE* fout)
{
	fprintf(fout, "\taddi $sp, $sp, -4\n");
	fprintf(fout, "\tsw $fp, 0($sp)\n");
	fprintf(fout, "\tmove $fp, $sp\n");
}

void generate_epilogue(FILE* fout)
{
	fprintf(fout, "\tmove $sp, $fp\n");
	fprintf(fout, "\tlw $fp, 0($sp)\n");
	fprintf(fout, "\taddi $sp, $sp, 4\n");
}

void generate_text(FILE* fout, InterCodes* codes)
{
	for (InterCodes* p = codes; p != NULL; p = p->next)
	{
		int t0, t1, t2;
		t0 = t1 = t2 = -1;
		switch (p->code.kind) {
		case IC_LABEL: {
			fprintf(fout, "label%d:\n", p->code.result.u.label_id);
			break;
		}
		case IC_FUNC: {
			//every time begin a func, reset offset
			offset2fp = 0;
			offset2sp = 8;
			varList = NULL;
			fprintf(fout, "\n%s:\n", p->code.result.u.name);
			generate_prologue(fout);
			break;
		}
		case IC_ASSIGN: {
			t0 = getReg(fout, p->code.result);
			if (p->code.arg1.kind == CONSTANT) {
				fprintf(fout, "\tli $t%d, %d\n", t0, p->code.arg1.u.value);
			} else {
				t1 = getReg(fout, p->code.arg1);
				fprintf(fout, "\tmove $t%d, $t%d\n", t0, t1);
			}
			spillReg(fout, t0);
			break;
		}
		case IC_PLUS: {
			t0 = getReg(fout, p->code.result);
			if (p->code.arg1.kind == CONSTANT && p->code.arg2.kind == CONSTANT)
				assert(0);
			if (p->code.arg1.kind == CONSTANT) {
				t1 = getReg(fout, p->code.arg2);
				fprintf(fout, "\taddi $t%d, $t%d, %d\n", t0, t1, p->code.arg1.u.value);
			} else if (p->code.arg2.kind == CONSTANT) {
				t1 = getReg(fout, p->code.arg1);
				fprintf(fout, "\taddi $t%d, $t%d, %d\n", t0, t1, p->code.arg2.u.value);
			} else {
				t1 = getReg(fout, p->code.arg1);
				t2 = getReg(fout, p->code.arg2);
				fprintf(fout, "\tadd $t%d, $t%d, $t%d\n", t0, t1, t2);
			}
			spillReg(fout, t0);
			break;
		}
		case IC_MINUS: {
			t0 = getReg(fout, p->code.result);
			if (p->code.arg1.kind == CONSTANT && p->code.arg2.kind == CONSTANT)
				assert(0);
			if (p->code.arg1.kind == CONSTANT) {
				t1 = getReg(fout, p->code.arg2);
				fprintf(fout, "\taddi $t%d, $t%d, %d\n", t0, t1, 0 - p->code.arg1.u.value);
			} else if (p->code.arg2.kind == CONSTANT) {
				t1 = getReg(fout, p->code.arg1);
				fprintf(fout, "\taddi $t%d, $t%d, %d\n", t0, t1, 0 - p->code.arg2.u.value);
			} else {
				t1 = getReg(fout, p->code.arg1);
				t2 = getReg(fout, p->code.arg2);
				fprintf(fout, "\tsub $t%d, $t%d, $t%d\n", t0, t1, t2);
			}
			spillReg(fout, t0);
			break;
		}
		case IC_MUL: {
			t0 = getReg(fout, p->code.result);
			t1 = getReg(fout, p->code.arg1);
			t2 = getReg(fout, p->code.arg2);
			fprintf(fout, "\tmul $t%d, $t%d, $t%d\n", t0, t1, t2);
			spillReg(fout, t0);
			break;
		}
		case IC_DIV: {
			t0 = getReg(fout, p->code.result);
			t1 = getReg(fout, p->code.arg1);
			t2 = getReg(fout, p->code.arg2);
			fprintf(fout, "\tdiv $t%d, $t%d\n", t1, t2);
			fprintf(fout, "\tmflo $t%d\n", t0);
			spillReg(fout, t0);
			break;
		}
		case IC_ADDR: {
			t0 = getReg(fout, p->code.result);
			Variable* var = allocate(fout, p->code.arg1);
			fprintf(fout, "\tla $t%d, %d($fp)\n", t0, var->offset);
			spillReg(fout, t0);
			break;
		}
		case IC_DEREF_R: {
			t0 = getReg(fout, p->code.result);
			t1 = getReg(fout, p->code.arg1);
			fprintf(fout, "\tlw $t%d, 0($t%d)\n", t0, t1); //t1 is address
			spillReg(fout, t0);
			break;
		}
		case IC_DEREF_L: {
			t0 = getReg(fout, p->code.result);
			t1 = getReg(fout, p->code.arg1);
			fprintf(fout, "\tsw $t%d, 0($t%d)\n", t1, t0); //t0 is address
			break;
		}
		case IC_GOTO: {
			fprintf(fout, "\tj label%d\n", p->code.result.u.label_id);
			break;
		}
		case IC_RELOP: {
			t1 = getReg(fout, p->code.arg1);
			t2 = getReg(fout, p->code.arg2);
			switch (p->code.relop) {
			case LT: fprintf(fout, "\tblt "); break;
			case LE: fprintf(fout, "\tble "); break;
			case EQ: fprintf(fout, "\tbeq "); break;
			case GT: fprintf(fout, "\tbgt "); break;
			case GE: fprintf(fout, "\tbge "); break;
			case NE: fprintf(fout, "\tbne "); break;
			default: assert(0);
			}
			fprintf(fout, "$t%d, $t%d, label%d\n", t1, t2, p->code.result.u.label_id);
			break;
		}
		case IC_DEC: {
			Variable* var = (Variable*)malloc(sizeof(Variable));
			var->kind = OP_VARIABLE;
			var->u.name = p->code.result.u.name;
			var->offset = (offset2fp -= p->code.arg1.u.value);
			var->next = varList;
			varList = var;
			fprintf(fout, "\taddi $sp, $sp, -%d\n", p->code.arg1.u.value);
			break;
		}
		case IC_RETURN: {
			t0 = getReg(fout, p->code.result);
			fprintf(fout, "\tmove $v0, $t%d\n", t0);
			generate_epilogue(fout);
			fprintf(fout, "\tjr $ra\n");
			break;
		}
		case IC_ARG: {
			/* the arg's order is opposite to param,
			 * so put all the arg into stack, and in IC_PARAM,
			 * just read parameter from stack by order
			 */
			t0 = getReg(fout, p->code.result);
			fprintf(fout, "\taddi $sp, $sp, -4\n");
			fprintf(fout, "\tsw $t%d, 0($sp)\n", t0);
			break;
		}
		case IC_CALL: {
			genarate_call(fout, p->code.arg1.u.name);
			// be care of the change of sp.
			// because I use the offset2sp to read the parameter
			t0 = getReg(fout, p->code.result);
			fprintf(fout, "\tmove $t%d, $v0\n", t0);
			spillReg(fout, t0);
			break;
		}
		case IC_PARAM: {
			t0 = getReg(fout, p->code.result);
			fprintf(fout, "\tlw $t%d, %d($fp)\n", t0, offset2sp); //fp is call function's fp
			offset2sp += 4;   //only integer can be parameter
			spillReg(fout, t0);
			break;
		}
		case IC_READ: {
			t0 = getReg(fout, p->code.result);
			genarate_call(fout, "read");
			fprintf(fout, "\tmove $t%d, $v0\n", t0);
			spillReg(fout, t0);
			break;
		}
		case IC_WRITE: {
			t0 = getReg(fout, p->code.result);
			fprintf(fout, "\tmove $a0, $t%d\n", t0);
			genarate_call(fout, "write");
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

int getReg(FILE* fout, Operand operand)
{
	for (int i = 0; i < 10; i ++)
	{
		if (regs[i].valid)
		{
			if (operand.kind == CONSTANT) {
				fprintf(fout, "\tli $t%d, %d\n", i, operand.u.value);
			} else {
				int offset = offset2fp;
				regs[i].var = allocate(fout, operand);

				//only when the operand is already send to stack,then you load it
				if (offset == offset2fp) {
					fprintf(fout, "\tlw $t%d, %d($fp)\n", i, regs[i].var->offset);
				}
			}
			regs[i].valid = 0;
			return i;
		}
	}
	assert(0);
}

Variable* allocate(FILE* fout, Operand operand)
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
	fprintf(fout, "\taddi $sp, $sp, -4\n");
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

void spillReg(FILE* fout, int i)
{
	if (NULL == regs[i].var)
		return;
	assert(i >= 0);
	assert(i < 10);
	fprintf(fout, "\tsw $t%d, %d($fp)\n", i, regs[i].var->offset);
}