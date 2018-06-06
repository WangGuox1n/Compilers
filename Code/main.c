#include "syntaxtree.h"
#include "syntax.tab.h"
#include "ir.h"
#include "obeject_code.h"
int main(int argc, char** argv)
{
	if(argc <= 1) 
		return 1;
	FILE* f = fopen(argv[1], "r");
	if(!f){
		perror(argv[1]);
		return 1;
	}
	isError = 0;
	funcCount = 0;
	yyrestart(f);
	//yydebug = 1;
	yyparse();
	addReadAndWrite();
	traverseTree(treeroot);
	//output(treeroot,0);
	//gen_InterCode(treeroot);
	generate_oc(treeroot);
	return 0;
}