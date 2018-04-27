%{
#include <stdio.h>
#include "lex.yy.c"
#include "syntaxtree.h"
void yyerror(char *msg);
%}
/* declared types */
%union {
   struct SyntaxTreeNode* type_SyntaxTreeNode;
}

/* declared tokens */
%token <type_SyntaxTreeNode> INT 
%token <type_SyntaxTreeNode> FLOAT
%token <type_SyntaxTreeNode> PLUS MINUS STAR DIV
%token <type_SyntaxTreeNode> SEMI COMMA
%token <type_SyntaxTreeNode> ASSIGNOP RELOP
%token <type_SyntaxTreeNode> AND OR NOT
%token <type_SyntaxTreeNode> DOT SPACE
%token <type_SyntaxTreeNode> ID TYPE RETURN STRUCT IF ELSE WHILE
%token <type_SyntaxTreeNode> LP RP LB RB LC RC

/* declared non-terminals */
%type <type_SyntaxTreeNode> Program ExtDefList ExtDef ExtDecList 
%type <type_SyntaxTreeNode> Specifier StructSpecifier OptTag Tag 
%type <type_SyntaxTreeNode> VarDec FunDec VarList ParamDec
%type <type_SyntaxTreeNode> CompSt StmtList Stmt 
%type <type_SyntaxTreeNode> DefList Def DecList Dec 
%type <type_SyntaxTreeNode> Exp Args


%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left MINUS PLUS
%left DIV STAR
%right NOT 
%left LP RP LB RB DOT
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%

/* High-level Definitions */
Program
: ExtDefList {
	$$ = createTreeNode("Program", NULL, @$.first_line, 1);
	$$->children[0] = $1;
	$$->productionNum = 0;
	treeroot = $$;
}
;

ExtDefList
: ExtDef ExtDefList {
	$$ = createTreeNode("ExtDefList", NULL, @$.first_line, 2);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->productionNum = 0;
}
|/*empty (epsilon)*/ {
	$$ = NULL;
}
;

ExtDef
: Specifier ExtDecList SEMI {
	$$ = createTreeNode("ExtDef", NULL, @$.first_line, 3);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->productionNum = 0;
}
| Specifier SEMI {
	$$ = createTreeNode("ExtDef", NULL, @$.first_line, 2);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->productionNum = 1;
}
| Specifier FunDec CompSt {
	$$ = createTreeNode("ExtDef", NULL, @$.first_line, 3);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3; 
	$$->productionNum = 2;
} /* Function declare */
| Specifier FunDec SEMI {
	$$ = createTreeNode("ExtDef", NULL, @$.first_line, 3);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->productionNum = 3;
}
| error SEMI {
	printf("Error type B at Line %d: Syntax error.\n", @$.first_line);
}
| Specifier FunDec error {
	printf("Error type B at Line %d: Syntax error.\n", @$.first_line);
}
;

ExtDecList
: VarDec {
	$$ = createTreeNode("ExtDecList", NULL, @$.first_line, 1);
	$$->children[0] = $1;
	$$->productionNum = 0;
}
| VarDec COMMA ExtDecList {
	$$ = createTreeNode("ExtDecList", NULL, @$.first_line, 3);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->productionNum = 1;
}
;

/* Specifiers */
Specifier
: TYPE {
	$$ = createTreeNode("Specifier", NULL, @$.first_line, 1);
	$$->children[0] = $1;
	$$->productionNum = 0;
}
| StructSpecifier {
	$$ = createTreeNode("Specifier", NULL, @$.first_line, 1);
	$$->children[0] = $1;
	$$->productionNum = 1;
}
;

StructSpecifier
: STRUCT OptTag LC DefList RC {
	$$ = createTreeNode("StructSpecifier", NULL, @$.first_line, 5);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->children[3] = $4;
	$$->children[4] = $5;
	$$->productionNum = 0;
}
| STRUCT Tag {
	$$ = createTreeNode("StructSpecifier", NULL, @$.first_line, 2);
	$$->children[0] = $1;
	$$->children[1] = $2;	
	$$->productionNum = 1;
}
;

OptTag
: ID {
	$$ = createTreeNode("OptTag", NULL, @$.first_line, 1);
	$$->children[0] = $1;
}
| /*empty (epsilon)*/ {
	$$ = NULL;
}
;

Tag
: ID {
	$$ = createTreeNode("Tag", NULL, @$.first_line, 1);
	$$->children[0] = $1;
}
;

/* Declarators */
VarDec
: ID {
	$$ = createTreeNode("VarDec", NULL, @$.first_line, 1);
	$$->children[0] = $1;
	$$->productionNum = 0;
}
| VarDec LB INT RB {
	$$ = createTreeNode("VarDec", NULL, @$.first_line, 4);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->children[3] = $4;
	$$->productionNum = 1;
}
;

FunDec
: ID LP VarList RP {
	$$ = createTreeNode("FunDec", NULL, @$.first_line, 4);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->children[3] = $4;
	$$->productionNum = 0;
}
| ID LP RP {
	$$ = createTreeNode("FunDec", NULL, @$.first_line, 3);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->productionNum = 1;
}
| error RP {
	printf("Error type B at Line %d: Syntax error.\n", @$.first_line);
}
;

VarList
: ParamDec COMMA VarList {
	$$ = createTreeNode("VarList", NULL, @$.first_line, 3);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->productionNum = 0;
}
| ParamDec {
	$$ = createTreeNode("VarList", NULL, @$.first_line, 1);
	$$->children[0] = $1;
	$$->productionNum = 1;
}
;

ParamDec
: Specifier VarDec {
	$$ = createTreeNode("ParamDec", NULL, @$.first_line, 2);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->productionNum = 0;
}
;

/* Statements */
CompSt
: LC DefList StmtList RC {
	$$ = createTreeNode("CompSt", NULL, @$.first_line, 4);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->children[3] = $4;
	$$->productionNum = 0;
}
| LC error RC {
	printf("Error type B at Line %d: Syntax error.\n", @$.first_line);
}
| error RC {
	printf("Error type B at Line %d: Syntax error.\n", @$.first_line);
}
| LC error {
	printf("Error type B at Line %d: Syntax error.\n", @$.first_line);
}
;

StmtList
: Stmt StmtList {
	$$ = createTreeNode("StmtList", NULL, @$.first_line, 2);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->productionNum = 0;
}
| /*empty (epsilon)*/ {
	$$ = NULL;
}
;

Stmt
: Exp SEMI {
	$$ = createTreeNode("Stmt", NULL, @$.first_line, 2);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->productionNum = 0;	
}
| CompSt {
	$$ = createTreeNode("Stmt", NULL, @$.first_line, 1);
	$$->children[0] = $1;
	$$->productionNum = 1;
}
| RETURN Exp SEMI {
	$$ = createTreeNode("Stmt", NULL, @$.first_line, 3);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->productionNum = 2;
}
| IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {
	$$ = createTreeNode("Stmt", NULL, @$.first_line, 5);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->children[3] = $4;
	$$->children[4] = $5;
	$$->productionNum = 3;	
}
| IF LP Exp RP Stmt ELSE Stmt {
	$$ = createTreeNode("Stmt", NULL, @$.first_line, 7);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->children[3] = $4;
	$$->children[4] = $5;
	$$->children[5] = $6;
	$$->children[6] = $7;
	$$->productionNum = 4;
}
| WHILE LP Exp RP Stmt {
	$$ = createTreeNode("Stmt", NULL, @$.first_line, 5);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->children[3] = $4;
	$$->children[4] = $5;
	$$->productionNum = 5;
}
| error SEMI {
	printf("Error type B at Line %d: Syntax error.\n", @$.first_line);
}
| Exp error {
	printf("Error type B at Line %d: Missing \";\".\n", @$.first_line);
}
| RETURN Exp error {
	printf("Error type B at Line %d: Missing \";\".\n", @$.first_line);
}
;

/* Local Definitions */
DefList
: Def DefList {
	$$ = createTreeNode("DefList", NULL, @$.first_line, 2);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->productionNum = 0;
}
| /*empty (epsilon)*/ {
	$$ = NULL;
}
;

Def
: Specifier DecList SEMI {
	$$ = createTreeNode("Def", NULL, @$.first_line, 3);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->productionNum = 0;
}
| error SEMI {
	printf("Error type B at Line %d: Definition error.\n", @$.first_line);
}
| Specifier DecList error {
	printf("Error type B at Line %d: Missing \";\".\n", @$.first_line);
}
;

DecList
: Dec {
	$$ = createTreeNode("DecList", NULL, @$.first_line, 1);
	$$->children[0] = $1;	
	$$->productionNum = 0;
}
| Dec COMMA DecList {
	$$ = createTreeNode("DecList", NULL, @$.first_line, 3);
	$$->children[0] = $1;
	$$->children[1] = $2;	
	$$->children[2] = $3;
	$$->productionNum = 1;
}
;

Dec
: VarDec {
	$$ = createTreeNode("Dec", NULL, @$.first_line, 1);
	$$->children[0] = $1;
	$$->productionNum = 0;
}
| VarDec ASSIGNOP Exp {
	$$ = createTreeNode("Dec", NULL, @$.first_line, 3);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->productionNum = 1;
}
;

/* Expressions */
Exp
: Exp ASSIGNOP Exp {
	$$ = createTreeNode("Exp", NULL, @$.first_line, 3);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->productionNum = 0;
}
| Exp AND Exp {
	$$ = createTreeNode("Exp", NULL, @$.first_line, 3);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->productionNum = 1;
}
| Exp OR Exp {
	$$ = createTreeNode("Exp", NULL, @$.first_line, 3);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->productionNum = 2;
}
| Exp RELOP Exp {
	$$ = createTreeNode("Exp", NULL, @$.first_line, 3);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->productionNum = 3;
}
| Exp PLUS Exp {
	$$ = createTreeNode("Exp", NULL, @$.first_line, 3);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->productionNum = 4;
}
| Exp MINUS Exp {
	$$ = createTreeNode("Exp", NULL, @$.first_line, 3);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->productionNum = 5;
}
| Exp STAR Exp {
	$$ = createTreeNode("Exp", NULL, @$.first_line, 3);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->productionNum = 6;	
}
| Exp DIV Exp {
	$$ = createTreeNode("Exp", NULL, @$.first_line, 3);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->productionNum = 7;
}
| LP Exp RP {
	$$ = createTreeNode("Exp", NULL, @$.first_line, 3);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->productionNum = 8;
}
| MINUS Exp {
	$$ = createTreeNode("Exp", NULL, @$.first_line, 2);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->productionNum = 9;	
}
| NOT Exp {
	$$ = createTreeNode("Exp", NULL, @$.first_line, 2);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->productionNum = 10;	
}
| ID LP Args RP {
	$$ = createTreeNode("Exp", NULL, @$.first_line, 4);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->children[3] = $4;
	$$->productionNum = 11;
}
| ID LP RP {
	$$ = createTreeNode("Exp", NULL, @$.first_line, 3);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->productionNum = 12;
}
| Exp LB Exp RB {
	$$ = createTreeNode("Exp", NULL, @$.first_line, 4);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->children[3] = $4;	
	$$->productionNum = 13;
}
| Exp DOT ID {
	$$ = createTreeNode("Exp", NULL, @$.first_line, 3);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->productionNum = 14;
}
| ID {
	$$ = createTreeNode("Exp", NULL, @$.first_line, 1);
	$$->children[0] = $1;
	$$->productionNum = 15;
}
| INT {
	$$ = createTreeNode("Exp", NULL, @$.first_line, 1);
	$$->children[0] = $1;
	$$->productionNum = 16;
}
| FLOAT {
	$$ = createTreeNode("Exp", NULL, @$.first_line, 1);
	$$->children[0] = $1;
	$$->productionNum = 17;
}
| ID error RP {
	printf("Error type B at Line %d: Syntax error.\n", @$.first_line);
}
;

Args
: Exp COMMA Args {
	$$ = createTreeNode("Args", NULL, @$.first_line, 3);
	$$->children[0] = $1;
	$$->children[1] = $2;
	$$->children[2] = $3;
	$$->productionNum = 0;
}
| Exp {
	$$ = createTreeNode("Args", NULL, @$.first_line, 1);
	$$->children[0] = $1;
	$$->productionNum = 1;
}
;

%%
void yyerror(char* msg) {
	isError = 1;
	fprintf(stderr, "yyerror  Error type B at Line %d: Syntax error.\n", yylineno);
}