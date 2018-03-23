%option c++ yyclass="xpl_scanner" outfile="xpl_scanner.cpp"
%option stack noyywrap yylineno 8bit debug
%{ 
/* $Id: xpl_scanner.l,v 1.6 2017/04/21 12:49:19 ist181926 Exp $ */
// make relevant includes before including the parser's tab file
#include <string>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>
#include "xpl_scanner.h"
#include "xpl_parser.tab.h"

#define CHECKOVERFLOW  if(errno == ERANGE) yyerror("The number causes an overflow.")
	
// don't change this
#define yyerror LexerError
%}

WHITESPACE              [ \t\n\r]
SYMBOLS                 [()[\]{}+-~?*%/<>=!&|@,;:.]

%x X_STRING X_SPECIAL_CHAR X_COMMENT

%%
 				{ set_debug(0); } 

"/*"            yy_push_state(X_COMMENT); // ignore comments
<X_COMMENT>"/*" yy_push_state(X_COMMENT); 
<X_COMMENT>"*/" yy_pop_state();
<X_COMMENT>.|\n ;

"//".*$            ; // ignore comments

">="                   return tGE; 
"<="                   return tLE; 
"=="                   return tEQ; 
"!="                   return tNE; 
"!!"				   return tPRINT;  // print_node

"int"				   return tINT;
"real"				   return tTYPEREAL;
"string" 			   return tTYPESTRING;
"procedure"			   return tPROCEDURE;
"public" 	  		   return tPUBLIC;
"use" 				   return tUSE;
"if"                   return tIF; 		 // if_node | if_else_node
"elsif"				   return tELSIF;
"else"                 return tELSE; 	 // if_else_node
"while"                return tWHILE; 	 // while_node
"sweep"				   return tSWEEP;	 // sweep_node
"next"				   return tNEXT; 	 // next_node
"null"				   return tNULL;
"stop"				   return tSTOP;	 // stop_node
"return"			   return tRETURN;	 // return_node

[A-Za-z_][A-Za-z0-9_]*  yylval.s = new std::string(yytext); return tIDENTIFIER;	   

\"                    yy_push_state(X_STRING); yylval.s = new std::string(""); 
<X_STRING>\"          yy_pop_state(); return tSTRING;
<X_STRING>\\          yy_push_state(X_SPECIAL_CHAR);
<X_STRING>.           *yylval.s += yytext;

<X_SPECIAL_CHAR>n						 yy_pop_state(); *yylval.s += "\n"; 
<X_SPECIAL_CHAR>r						 yy_pop_state(); *yylval.s += "\t"; 
<X_SPECIAL_CHAR>t 					   	 yy_pop_state(); *yylval.s += "\r";
<X_SPECIAL_CHAR>\"        			     yy_pop_state(); *yylval.s += "\"";
<X_SPECIAL_CHAR>[0-9a-fA-F][0-9a-fA-F]?  yy_pop_state(); *yylval.s += strtoul(yytext, nullptr, 16);
<X_SPECIAL_CHAR>. 				     	 yy_pop_state();


[1-9][0-9]*                 		yylval.i = strtol(yytext, nullptr, 10); CHECKOVERFLOW; return tINTEGER; // return int
0x[0-9A-Fa-f]+                      yylval.i = strtol(yytext, nullptr, 16); CHECKOVERFLOW; return tINTEGER; // return int hexa
0									yylval.i = strtol(yytext, nullptr, 10); return tINTEGER; // return int zero
[0-9]+\.?[0-9]*([eE][-+]?[0-9]+)?   yylval.d = strtod(yytext, nullptr); CHECKOVERFLOW; return tREAL;        // return double


{SYMBOLS}        	   return *yytext;
{WHITESPACE}*          ; /* ignore whitespace */


.                      yyerror("Unknown character");

%%
// Very, very dirty hack: flex is a mess generating C++ scanners.
int xpl_scanner::yywrap() { return 1; }
