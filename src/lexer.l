/* recognize tokens for the calculator */

%option noyywrap nodefault yylineno
%option stack
%x string_state

%{
# include "utils.h"
# include "parser.tab.h"
%}


%%

 /* single character ops */
"+" |
"-" |
"*" |
"/" |
"=" |
"|" |
"," |
";" |
"(" |
")"     { return yytext[0]; }

 /* keywords */

"if"    { return IF; }
"else"  { return ELSE; }
"while" { return WHILE; }
"for"   { return FOR; }
" in "  { return IN; }
"def"   { return DEF; }
"string"  { yylval.c='s'; return TYPE; }
"int"     { yylval.c='i'; return TYPE; }
"real"    { yylval.c='r'; return TYPE; }
"device"  { yylval.c='d'; return TYPE; }

 /* numbers */
[0-9]+\.[0-9]+ { yylval.v=malloc(sizeof(struct val)); yylval.v->real_val=atof(yytext); yylval.v->type='r'; return VALUE; }
[0-9]+         { yylval.v=malloc(sizeof(struct val)); yylval.v->int_val=atoi(yytext); yylval.v->type='i'; return VALUE; }

 /* strings */
\"                    {yy_push_state(string_state); }
<string_state>\"      {yy_pop_state(); }
<string_state>[^\"]*  {yylval.v=malloc(sizeof(struct val)); yylval.v->string_val=strdup(yytext); yylval.v->type='s'; return VALUE;}


 /* names */
[a-zA-Z_][a-zA-Z0-9_]*  { yylval.s = lookup(yytext); return NAME; }


"//".*  /* ignore comments */
[ \t]   /* ignore white space */
"\n"    { printf("> "); }

.	{ yyerror("Mystery character %c\n", *yytext); }
%%
