#include "syntaxtree.h"
#include "syntax.tab.h"
#include "ir.h"
#include "obeject_code.h"
int main(int argc, char** argv)
{
	if (argc <= 1)
		return 1;
	FILE* fin = fopen(argv[1], "r");
	FILE* fout = fopen(argv[2], "w");
	if (!fin) {
		perror(argv[1]);
		return 1;
	}
	isError = 0;
	funcCount = 0;
	yyrestart(fin);
	//yydebug = 1;
	yyparse();
	addReadAndWrite();
	traverseTree(treeroot);
	//output(treeroot,0);         //lab2's output
	gen_InterCode(treeroot);    //lab3's output
	//generate_oc(fout, treeroot);  //lab4's output
	return 0;
}