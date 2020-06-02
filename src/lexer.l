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
":" |
"{" |
"}" |
"(" |
")"     { return yytext[0]; }

 /* comparison ops */
">"     { yylval.fn = 1; return CMP; }
"<"     { yylval.fn = 2; return CMP; }
"<>"    { yylval.fn = 3; return CMP; }
"=="    { yylval.fn = 4; return CMP; }
">="    { yylval.fn = 5; return CMP; }
"<="    { yylval.fn = 6; return CMP; }

 /* keywords */

"if"    { return IF; }
"else"  { return ELSE; }
"while" { return WHILE; }
"for"   { return FOR; }
"in"    { return IN; }
"def"   { return DEF; }
"and"   { return AND; }
"or"    { return OR; }
"string"  		{ yylval.c='s'; return TYPE; }
"int"     		{ yylval.c='i'; return TYPE; }
"real"    		{ yylval.c='r'; return TYPE; }
"device" 		{ yylval.c='d'; return TYPE; }
"address"		{ yylval.c='a'; return TYPE; }
"print"  		{ yylval.fn = B_print; return FUNC; }
"connect" 		{ yylval.fn = B_connect; return FUNC; }
"disconnect"	{ yylval.fn = B_disconnect; return FUNC; }

"debug"[0-9]+ { debug = atoi(&yytext[5]); printf("debug set to %d\n", debug); }

 /* numbers */
[0-9]+\.[0-9]+ { yylval.v=new_real(atof(yytext)); return VALUE; }
[0-9]+         { yylval.v=new_int(atoi(yytext)); return VALUE; }

 /* strings */
\"                    {yy_push_state(string_state); }
<string_state>\"      {yy_pop_state(); }
<string_state>[^\"]*  {yylval.v=new_string(yytext); return VALUE;}


 /* device */
((([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5]))|"localhost" { yylval.v=new_address(yytext); return VALUE; }

 /* names */
[a-zA-Z_][a-zA-Z0-9_]*  { yylval.s = lookup(yytext); return NAME; }


"//".*  /* ignore comments */
[ \t]   /* ignore white space */
"\n"    { printf("> "); }

.	{ yyerror("Mystery character %c\n", *yytext); }
%%
