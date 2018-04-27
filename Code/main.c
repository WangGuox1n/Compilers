#include "syntaxtree.h"
#include "symbol.h"
#include "syntax.tab.h"
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
	traverseTree(treeroot);	
/*	if(isError == 0)
		output(treeroot,0);*/
	return 0;
}