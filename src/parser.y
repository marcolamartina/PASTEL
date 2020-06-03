
%{
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
int yylex();
extern int file_mod;
%}

%union {
  struct ast *a;
  struct symbol *s;		/* which symbol */
  struct symlist *sl;
  struct val * v;
  char c;
  int fn;
}

/* declare tokens */
%token IF ELSE WHILE FOR IN DEF
%token <fn> FUNC
%token <s> NAME
%token <c> TYPE
%token <v> VALUE


%right '='
%left ':'
%left AND OR
%nonassoc <fn> CMP
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS


%type <a> exp stmt list explist decl

%define parse.error verbose
%start program

%%

program: /* nothing */
  | program stmt { if(debug) dumpast($2, 0);
					eval($2);
					treefree($2);}
  | program error '\n' { yyerrok; printf("%s", file_mod ? "" : "> "); }
 ;

stmt: IF '(' exp ')' '{' list '}'                   { $$ = newflow('I', $3, $6, NULL); }
   | IF '(' exp ')' '{' list '}' ELSE '{' list '}'  { $$ = newflow('I', $3, $6, $10); }
   | WHILE '(' exp ')' '{' list '}'                 { $$ = newflow('W', $3, $6, NULL);}
   | exp ';'
   | decl ';'
;

list: /* nothing */ { $$ = NULL; }
   | stmt list { if ($2 == NULL)
	                 $$ = $1;
                 else
			             $$ = newast('L', $1, $2);
                }
   ;

exp: '(' exp ')'          { $$ = $2; }
   | FUNC '(' explist ')' { $$ = newfunc($1, $3); }
   | VALUE                { $$ = newvalue($1);}
   | exp ':' exp          { $$ = newast(':', $1,$3); }
   | exp '+' exp          { $$ = newast('+', $1,$3); }
   | exp '-' exp          { $$ = newast('-', $1,$3); }
   | exp '*' exp          { $$ = newast('*', $1,$3); }
   | exp '/' exp          { $$ = newast('/', $1,$3); }
   | exp AND exp          { $$ = newast('&', $1,$3); }
   | exp OR exp           { $$ = newast('|', $1,$3); }
   | '-' exp %prec UMINUS { $$ = newast('M', $2, NULL); }
	 | NAME'[' exp ']'			{ $$ = newref_l($1,$3); }
   | NAME                 { $$ = newref($1); }
   | exp CMP exp          { $$ = newcmp($2, $1, $3); }
;

decl: TYPE NAME    					{ $$=newdecl($2,$1); }
  | NAME '=' exp  					{ $$=newasgn($1,$3); }
	| NAME'[' exp ']''=' exp	{	$$=newasgn_l($1,$3,$6);	}
;

explist: exp
 | exp ',' explist  { $$ = newast('L', $1, $3); }
;


%%
