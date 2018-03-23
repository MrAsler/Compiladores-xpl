%{
// $Id: xpl_parser.y,v 1.12 2017/05/15 20:04:57 ist181926 Exp $
//-- don't change *any* of these: if you do, you'll break the compiler.
#include <cdk/compiler.h>
#include "ast/all.h"
#define LINE       compiler->scanner()->lineno()
#define yylex()    compiler->scanner()->scan()
#define yyerror(s) compiler->scanner()->error(s)
#define YYPARSE_PARAM_TYPE std::shared_ptr<cdk::compiler>
#define YYPARSE_PARAM      compiler
//-- don't change *any* of these --- END!
#define DEFVOID  new basic_type(0, basic_type::TYPE_VOID)

int USE = 1, PUBLIC = 2;

bool toImport(int qualifier) {
  return qualifier == USE ? true : false;
}

bool toExport(int qualifier) {
  return qualifier == PUBLIC ? true : false;
}

cdk::expression_node* omissionValue(int line, basic_type *type) {
      switch(type->name()) {

        case basic_type::TYPE_INT:     
          return new cdk::integer_node(line, 0);    
          break;
        case basic_type::TYPE_POINTER:  
          return nullptr;    
          break;

        default: 
          return nullptr;
      }
}

%}

%union {
  int                  i;	        /* integer value */
  double               d;         /* real value */
  bool                 b;         /* boolean value */
  basic_type           *t;        /* primitive type of declarations */
  std::string          *s;	      /* symbol name or string literal */
  cdk::basic_node      *node;	    /* node pointer */
  cdk::sequence_node   *sequence; /* List of nodes */
  cdk::expression_node *expression; /* expression nodes */
  cdk::lvalue_node     *lvalue;   
};

%token <i> tINTEGER
%token <d> tREAL
%token <s> tIDENTIFIER tSTRING
%token tINT tTYPEREAL tTYPESTRING tPROCEDURE tPUBLIC tUSE
%token tPRINT tNEXT tSTOP tRETURN tIF tWHILE tSWEEP tNULL


%nonassoc tIFX
%nonassoc tELSE tELSIF


%right '=' 
%left '|'
%left '&'
%nonassoc '~'  
%left tEQ tNE 
%left '<' '>' tLE tGE 
%left '+' '-'
%left '*' '/' '%'
%nonassoc tUNARY '?'
%nonassoc '(' ')' '[' ']' 

%type <node> stmt retrn cond elsif iter body block decl decl_v decl_f
%type <sequence> lst_decl lst_decl_v lst_stmt f_lst_var f_decl_v lst_expr
%type <expression> expr literal funcall
%type <lvalue> lval
%type <s> word
%type <t> type
%type <i> qualifier
%type <b> sign
%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%

file : lst_decl             { compiler->ast($1); }
     ;
     
lst_decl : lst_decl decl    { $$ = new cdk::sequence_node(LINE, $2, $1); }
         | /* EMPTY */      { $$ = new cdk::sequence_node(LINE, new cdk::nil_node(LINE)); }
         ;

decl : decl_v ';'   { $$ = $1; }
     | decl_f       { $$ = $1; }
     ;

decl_v : qualifier type tIDENTIFIER '=' expr { $$ = new xpl::decl_variable_node(LINE, toImport($1), toExport($1), $2, $3, $5); }
       | qualifier type tIDENTIFIER          { $$ = new xpl::decl_variable_node(LINE, toImport($1), toExport($1), $2, $3, nullptr); }
       
       | type tIDENTIFIER '=' expr { $$ = new xpl::decl_variable_node(LINE, false, false, $1, $2, $4); }
       | type tIDENTIFIER          { $$ = new xpl::decl_variable_node(LINE, false, false, $1, $2, nullptr); }
       ;

decl_f : qualifier type tIDENTIFIER f_lst_var                  { $$ = new xpl::decl_function_node(LINE, toImport($1), toExport($1), $2, $3, $4); }
       | qualifier tPROCEDURE tIDENTIFIER f_lst_var            { $$ = new xpl::decl_function_node(LINE, toImport($1), toExport($1), DEFVOID, $3, $4); }
       | qualifier type tIDENTIFIER f_lst_var '=' literal body { $$ = new xpl::function_node(LINE, toImport($1), toExport($1), $2, $3, $4, $6, $7); }
       | qualifier type tIDENTIFIER f_lst_var body             { $$ = new xpl::function_node(LINE, toImport($1), toExport($1), $2, $3, $4, omissionValue(LINE, $2), $5); }
       | qualifier tPROCEDURE tIDENTIFIER f_lst_var body       { $$ = new xpl::function_node(LINE, toImport($1), toExport($1), DEFVOID, $3, $4, nullptr, $5); }

       | type tIDENTIFIER f_lst_var                   { $$ = new xpl::decl_function_node(LINE, false, false, $1, $2, $3); }
       | tPROCEDURE tIDENTIFIER f_lst_var             { $$ = new xpl::decl_function_node(LINE, false, false, DEFVOID, $2, $3); }
       | type tIDENTIFIER f_lst_var '=' literal body { $$ = new xpl::function_node(LINE, false, false, $1, $2, $3, $5, $6); }
       | type tIDENTIFIER f_lst_var body             { $$ = new xpl::function_node(LINE, false, false, $1, $2, $3, omissionValue(LINE, $1), $4); }
       | tPROCEDURE tIDENTIFIER f_lst_var body       { $$ = new xpl::function_node(LINE, false, false, DEFVOID, $2, $3, nullptr, $4); }
       ;

f_lst_var : '(' f_decl_v ')'    { $$ = $2; }
          | '(' ')'             { $$ = new cdk::sequence_node(LINE, nullptr); }
          ;

f_decl_v : f_decl_v ',' decl_v { $$ = new cdk::sequence_node(LINE, $3, $1); }
         | decl_v              { $$ = new cdk::sequence_node(LINE, $1); }
         ;

body  : '{' lst_decl_v lst_stmt retrn '}' { $$ = new xpl::body_node(LINE, $2, new cdk::sequence_node(LINE, $4, new cdk::sequence_node(LINE, $3))); }
      | '{' lst_decl_v retrn '}'          { $$ = new xpl::body_node(LINE, $2, new cdk::sequence_node(LINE, $3)); }
      | '{' lst_stmt retrn '}'            { $$ = new xpl::body_node(LINE, new cdk::sequence_node(LINE, nullptr), new cdk::sequence_node(LINE, $3, new cdk::sequence_node(LINE, $2))); }
      | '{' retrn '}'                     { $$ = new xpl::body_node(LINE, new cdk::sequence_node(LINE, nullptr), new cdk::sequence_node(LINE, $2)); }
      ;

block : '{' lst_decl_v lst_stmt retrn '}' { $$ = new xpl::block_node(LINE, $2, new cdk::sequence_node(LINE, $4, new cdk::sequence_node(LINE, $3))); }
      | '{' lst_decl_v retrn '}'          { $$ = new xpl::block_node(LINE, $2, new cdk::sequence_node(LINE, $3)); }
      | '{' lst_stmt retrn '}'            { $$ = new xpl::block_node(LINE, new cdk::sequence_node(LINE, nullptr), new cdk::sequence_node(LINE, $3, new cdk::sequence_node(LINE, $2))); }
      | '{' retrn '}'                     { $$ = new xpl::block_node(LINE, new cdk::sequence_node(LINE, nullptr), new cdk::sequence_node(LINE, $2)); }
      ;

lst_decl_v : lst_decl_v decl_v ';' { $$ = new cdk::sequence_node(LINE, $2, $1); }
           | decl_v ';'            { $$ = new cdk::sequence_node(LINE, $1); }
           ;

lst_stmt : lst_stmt stmt { $$ = new cdk::sequence_node(LINE, $2, $1); }
         | stmt          { $$ = new cdk::sequence_node(LINE, $1); }  
         ;

retrn: tRETURN                          { $$ = new xpl::return_node(LINE); }
     | /* EMPTY */                      { $$ = new cdk::nil_node(LINE); }
     ;

stmt : expr ';'                         { $$ = new xpl::evaluation_node(LINE, $1); }
     | expr '!'                         { $$ = new xpl::print_node(LINE, false, $1); }
     | expr tPRINT                      { $$ = new xpl::print_node(LINE, true, $1); }
     | tNEXT                            { $$ = new xpl::next_node(LINE); }
     | tSTOP                            { $$ = new xpl::stop_node(LINE); }
     | cond                             { $$ = $1; }
     | iter                             { $$ = $1; }
     | block                            { $$ = $1; }
     ;

cond : tIF '(' expr ')' stmt %prec tIFX   { $$ = new xpl::if_node(LINE, $3, $5); }
     | tIF '(' expr ')' stmt tELSIF elsif { $$ = new xpl::if_else_node(LINE, $3, $5, $7); }
     | tIF '(' expr ')' stmt tELSE stmt   { $$ = new xpl::if_else_node(LINE, $3, $5, $7); }
     ;

elsif : '(' expr ')' stmt %prec tIFX   { $$ = new xpl::if_node(LINE, $2, $4); }          // Acaba elsif
      | '(' expr ')' stmt tELSIF elsif { $$ = new xpl::if_else_node(LINE, $2, $4, $6); } // Serie de elsifs
      | '(' expr ')' stmt tELSE stmt   { $$ = new xpl::if_else_node(LINE, $2, $4, $6); } // Acaba elsif com else
      ;

iter : tWHILE '(' expr ')' stmt                                  { $$ = new xpl::while_node(LINE, $3, $5); }
     | tSWEEP sign '(' lval ':' expr ':' expr ')' stmt           { $$ = new xpl::sweep_node(LINE, $2, $4, $6, $8, new cdk::integer_node(LINE, 1), $10); }
     | tSWEEP sign '(' lval ':' expr ':' expr ':' expr ')' stmt  { $$ = new xpl::sweep_node(LINE, $2, $4, $6, $8, $10, $12); }
     ;

expr : literal                 { $$ = $1; }
     | funcall                 { $$ = $1; }
     | '-' expr %prec tUNARY   { $$ = new cdk::neg_node(LINE, $2); }
     | '+' expr %prec tUNARY   { $$ = new xpl::identity_node(LINE, $2); }
     | '~' expr                { $$ = new cdk::not_node(LINE, $2); }  
     | expr '*' expr	         { $$ = new cdk::mul_node(LINE, $1, $3); }
     | expr '/' expr	         { $$ = new cdk::div_node(LINE, $1, $3); }
     | expr '%' expr	         { $$ = new cdk::mod_node(LINE, $1, $3); }
     | expr '+' expr           { $$ = new cdk::add_node(LINE, $1, $3); }
     | expr '-' expr           { $$ = new cdk::sub_node(LINE, $1, $3); }
     | expr '<' expr	         { $$ = new cdk::lt_node(LINE, $1, $3); }
     | expr '>' expr	         { $$ = new cdk::gt_node(LINE, $1, $3); }
     | expr tGE expr	         { $$ = new cdk::ge_node(LINE, $1, $3); }
     | expr tNE expr           { $$ = new cdk::ne_node(LINE, $1, $3); }
     | expr tLE expr           { $$ = new cdk::le_node(LINE, $1, $3); }
     | expr tEQ expr	         { $$ = new cdk::eq_node(LINE, $1, $3); } 
     | expr '&' expr           { $$ = new cdk::and_node(LINE, $1, $3); }
     | expr '|' expr           { $$ = new cdk::or_node(LINE, $1, $3); } 
     | '(' expr ')'            { $$ = $2; } 
     | '[' expr ']'            { $$ = new xpl::memalloc_node(LINE, $2); } 
     | '@'                     { $$ = new xpl::read_node(LINE); } 
     | lval                    { $$ = new cdk::rvalue_node(LINE, $1); } 
     | lval '=' expr           { $$ = new cdk::assignment_node(LINE, $1, $3); } 
     | lval '?'                { $$ = new xpl::address_node(LINE, $1); }
     | tNULL                   { $$ = nullptr; }
     ;


funcall : tIDENTIFIER '(' ')'          { $$ = new xpl::funcall_node(LINE, $1, nullptr); }
        | tIDENTIFIER '(' lst_expr ')' { $$ = new xpl::funcall_node(LINE, $1, $3); }
        ;

lst_expr : lst_expr ',' expr  { $$ = new cdk::sequence_node(LINE, $3, $1); }
         | expr               { $$ = new cdk::sequence_node(LINE, $1); }
         ;

lval : tIDENTIFIER            { $$ = new cdk::identifier_node(LINE, $1); }
     | expr '[' expr ']'      { $$ = new xpl::index_node(LINE, $1, $3); }
     ;

sign : '+'          { $$ = true; }
     | '-'          { $$ = false; }
     ;

literal : tINTEGER  { $$ = new cdk::integer_node(LINE, $1); }
        | tREAL     { $$ = new cdk::double_node(LINE, $1); }
        | word      { $$ = new cdk::string_node(LINE, $1); }
        ; 

word : word tSTRING  { $$ = new std::string(*$1 + *$2); delete $1; delete $2; }
     | tSTRING       { $$ = $1; }
     ; 


type : tINT         { $$ = new basic_type(4, basic_type::TYPE_INT); }
     | tTYPEREAL    { $$ = new basic_type(8, basic_type::TYPE_DOUBLE); }
     | tTYPESTRING  { $$ = new basic_type(4, basic_type::TYPE_STRING); }
     | '[' type ']' { basic_type *p = new basic_type(4, basic_type::TYPE_POINTER); p->_subtype = $2; $$ = p; }
     ;

qualifier : tUSE                { $$ = USE; }
          | tPUBLIC             { $$ = PUBLIC; }
          ;

%%
