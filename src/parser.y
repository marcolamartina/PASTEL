

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
%token STRING_TYPE
%token INTEGER_TYPE
%token REAL_TYPE
%token DEVICE_TYPE
%token <v> REAL_VAL
%token <v> INTEGER_VAL
%token <v> STRING_VAL
%token <s> NAME

%type <c> type
%type <a> exp stmt list explist decl
%type <v> value

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
   | value { $$=newvalue($1);}
;

decl: type NAME  { $$=newdecl($2,$1); }
| NAME '=' exp   { $$=newasgn($1,$3);}
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



%%
