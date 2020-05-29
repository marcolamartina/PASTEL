

%{
#  include <stdio.h>
#  include <stdlib.h>
#  include "utils.h"
int yylex();
%}

%union {
  struct ast *a;
  double d;
  int i;
  char * st;
  struct symbol *s;		/* which symbol */
  char c;
}

/* declare tokens */
%token IF ELSE WHILE FOR IN DEF
%token STRING_TYPE
%token INTEGER_TYPE
%token REAL_TYPE
%token DEVICE_TYPE
%token <d> REAL_VAL
%token <i> INTEGER_VAL
%token <st> STRING_VAL
%token <st> NAME

%type <c> type
%type <a> exp stmt list explist

%start program

%%

program: /* nothing */
  | program stmt ';' { eval($2);}
  | program DEF NAME '(' symlist ')' '{' list '}' { printf("Defined %s\n> ", $3); }
  | program error '\n' { yyerrok; printf("> "); }
 ;

stmt: IF '(' exp ')' '{' list '}'   { }
   | IF '(' exp ')' '{' list '}' ELSE '{' list '}'  { }
   | WHILE '(' exp ')' '{' list '}'   { }
   | exp
;

list: /* nothing */ { $$ = NULL; }
   | stmt ';' list { if ($3 == NULL)
	                     $$ = $1;
                     else
			                 $$ = $1;
                    }
   ;

exp: '(' exp ')'          { $$ = $2; }
   | type NAME  { insert($2,$1);}
   | NAME '=' value { }
   | NAME '(' explist ')' { }
;

type: STRING_TYPE { $$='s';}
  | INTEGER_TYPE { $$='i';}
  | REAL_TYPE { $$='r';}
  | DEVICE_TYPE { $$='d';}
;

value: STRING_VAL
  | INTEGER_VAL
  | REAL_VAL
;

explist: NAME { }
 | NAME ',' explist { }
 | value { }
 | value ',' explist { }
;

symlist: NAME { }
 | NAME ',' symlist { }
;


%%
