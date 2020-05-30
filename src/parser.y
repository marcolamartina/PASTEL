

%{
#  include <stdio.h>
#  include <stdlib.h>
#  include "utils.h"
int yylex();
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
%token <s> NAME
%token <c> TYPE
%token <v> VALUE

%right '='
%left '+' '-'
%left '*' '/'
%nonassoc '|' UMINUS

%type <a> exp stmt list explist decl

%start program

%%

program: /* nothing */
  | program stmt { if(debug) dumpast($2, 0); eval($2);}
  | program error '\n' { yyerrok; printf("> "); }
 ;

stmt: IF '(' exp ')' '{' list '}'   { }
   | IF '(' exp ')' '{' list '}' ELSE '{' list '}'  { }
   | WHILE '(' exp ')' '{' list '}'   { }
   | exp ';'
   | decl ';'
;

list: /* nothing */ { $$ = NULL; }
   | stmt ';' list { if ($3 == NULL)
	                     $$ = $1;
                     else
			                 $$ = $1;
                    }
   ;

exp: '(' exp ')'          { $$ = $2; }
   | NAME '(' explist ')' { }
   | VALUE { $$=newvalue($1);}
   | exp '+' exp          { $$ = newast('+', $1,$3); }
   | exp '-' exp          { $$ = newast('-', $1,$3); }
   | exp '*' exp          { $$ = newast('*', $1,$3); }
   | exp '/' exp          { $$ = newast('/', $1,$3); }
   | '-' exp %prec UMINUS { $$ = newast('M', $2, NULL); }
   | NAME                 { $$ = newref($1); }
;

decl: TYPE NAME  { $$=newdecl($2,$1); }
  | NAME '=' exp   { $$=newasgn($1,$3); }
;


explist: NAME { }
 | NAME ',' explist { }
 | VALUE { }
 | VALUE ',' explist { }
;



%%
