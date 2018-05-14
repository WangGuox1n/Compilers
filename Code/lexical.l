%{
	#include "syntax.tab.h"
	#include "syntaxtree.h"
 	int yycolumn = 1;
 	#define YY_USER_ACTION \
		yylloc.first_line = yylloc.last_line = yylineno; \
		yylloc.first_column = yycolumn; \
		yylloc.last_column = yycolumn + yyleng - 1; \
		yycolumn += yyleng;
%}
%option yylineno
INT [1-9][0-9]*|[0]
FLOAT [0-9]+?[.][0-9]+
ID [a-zA-Z_][a-zA-Z_0-9]*
SEMI ";"
COMMA ","
ASSIGNOP "="
RELOP ">"|"<"|">="|"<="|"=="|"!="
PLUS "+"
MINUS "-"
STAR "*"
DIV "/"
AND "&&"
OR "||"
DOT "."
NOT "!"
TYPE "int"|"float"
LP "("
RP ")"
LB "["
RB "]"
LC "{"
RC "}"
STRUCT "struct"
RETURN "return"
IF "if"
ELSE "else" 
WHILE "while"
SPACE [ \n\t]
ID_error [0-9]+{ID}
%%
\n { yycolumn = 1; }
"//" {
	char c = input();
	while(c != '\n'){
		c = input();
		if(c == EOF)
			break;
	}
}
"/*" {
	char c1 = input();
	char c2 = input();
	while(c1 != '*' || c2 != '/')
	{
	    c1 = c2;
	    c2 = input();
	    if(c1 == EOF || c2==EOF){
	    	isError = 1;
	    	printf("Error type A at Line %d: unterminated comment /*\n",yylineno);
	    	break;
	    }
	}
}
"*/" {
	isError = 1;
	printf("Error type A at Line %d: expected expression before ‘/’ token\n",yylineno);
}
{INT} {
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("INT", yytext, yylineno, 0);
	return INT; 
}
{FLOAT} {
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("FLOAT", yytext, yylineno, 0);
	return FLOAT; 
}
{SEMI} {
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("SEMI", NULL, yylineno, 0);
	return SEMI;
} 
{COMMA} { 
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("COMMA", NULL, yylineno, 0);
	return COMMA; 
} 
{ASSIGNOP} { 
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("ASSIGNOP", NULL, yylineno, 0);
	return ASSIGNOP; 
} 
{RELOP} { 
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("RELOP", yytext, yylineno, 0);
	return RELOP;
}
{PLUS} { 
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("PLUS", NULL, yylineno, 0);	
	return PLUS; 
} 
{MINUS} { 
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("MINUS", NULL, yylineno, 0);
	return MINUS;
} 
{STAR} { 
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("STAR", NULL, yylineno, 0);
	return STAR; 
} 
{DIV} { 
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("DIV", NULL, yylineno, 0);
	return DIV; 
} 
{AND} { 
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("AND", NULL, yylineno, 0);
	return AND; 
} 
{OR} { 
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("OR", NULL, yylineno, 0);
	return OR;
} 
{DOT} { 
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("DOT", NULL, yylineno, 0);
	return DOT; 
} 
{NOT} { 
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("NOT", NULL, yylineno, 0);
	return NOT; 
} 
{TYPE} {
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("TYPE", yytext, yylineno, 0);
	return TYPE;
}
{LP} { 
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("LP", NULL, yylineno, 0);
	return LP; 
}
{RP} { 
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("RP", NULL, yylineno, 0);
	return RP; 
}
{LB} { 
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("LB", NULL, yylineno, 0);
	return LB; 
} 
{RB} { 
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("RB", NULL, yylineno, 0);
	return RB; 
} 
{LC} { 
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("LC", NULL, yylineno, 0);
	return LC; 
} 
{RC} { 
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("RC", NULL, yylineno, 0);
	return RC; 
} 
{STRUCT} { 
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("STRUCT", NULL, yylineno, 0);
	return STRUCT; 
} 
{RETURN} { 
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("RETURN", NULL, yylineno, 0);
	return RETURN; 
} 
{IF} { 
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("IF", NULL, yylineno, 0);
	return IF; 
} 
{ELSE} { 
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("ELSE", NULL, yylineno, 0);
	return ELSE; 
} 
{WHILE} { 
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("WHILE", NULL, yylineno, 0);
	return WHILE; 
}
{ID} {
	yylval.type_SyntaxTreeNode = (struct SyntaxTreeNode*)createTreeNode("ID", yytext, yylineno, 0);
	return ID; 
}
{SPACE} {}
{ID_error} {
	isError = 1;
	printf("Error type A at Line %d: wrong identifier \"%s\".\n",yylineno, yytext);
	return ID; 
}
. {
	isError = 1;
	printf("Error type A at Line %d: Mysterious characters %s.\n",yylineno, yytext);
}
%%