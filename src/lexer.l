/* recognize tokens for the calculator */

%option noyywrap nodefault yylineno
%option stack
%x string_state
%x comment_state


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
"," |
";" |
":" |
"{" |
"}" |
"[" |
"]" |
"(" |
")"     { return yytext[0]; }

 /* comparison ops */
">"     { yylval.fn = 1; return CMP; }
"<"     { yylval.fn = 2; return CMP; }
"!="    { yylval.fn = 3; return CMP; }
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

".port"    { yylval.fn = B_port; return PORT; }
".address" { yylval.fn = B_address; return ADDR; }

"string"  	{ yylval.c='s'; return TYPE; }
"int"     	{ yylval.c='i'; return TYPE; }
"real"    	{ yylval.c='r'; return TYPE; }
"device" 		{ yylval.c='d'; return TYPE; }
"address"		{ yylval.c='a'; return TYPE; }
"list"		  { yylval.c='l'; return TYPE; }

	/* Built-in functions */
"print"  			{ yylval.fn = B_print; return FUNC; }
"quit"  			{ yylval.fn = B_quit; return FUNC; }
"connect" 		{ yylval.fn = B_connect; return FUNC; }
"disconnect"	{ yylval.fn = B_disconnect; return FUNC; }
"receive"			{ yylval.fn = B_receive; return FUNC; }
"send"				{ yylval.fn = B_send; return FUNC; }
"insert"			{ yylval.fn = B_insert; return FUNC; }
"remove"			{ yylval.fn = B_remove; return FUNC; }
"length"			{ yylval.fn = B_length; return FUNC; }
"s2i"			    { yylval.fn = B_s2i; return FUNC; }
"s2r"			    { yylval.fn = B_s2r; return FUNC; }
"s2d"			    { yylval.fn = B_s2d; return FUNC; }
"s2a"			    { yylval.fn = B_s2a; return FUNC; }
"toString"		{ yylval.fn = B_toString; return FUNC; }
"console"     { yylval.fn = B_console; return FUNC; }
"load"        { yylval.fn = B_load; return FUNC; }
"strip"       { yylval.fn = B_strip; return FUNC; }
"split"       { yylval.fn = B_split; return FUNC; }
"sleep"       { yylval.fn = B_sleep; return FUNC; }


"debug"[0-9]+ { debug = atoi(&yytext[5]); printf("debug set to %d\n", debug); }

 /* numbers */
[0-9]+\.[0-9]+ { yylval.v=new_real(atof(yytext)); return VALUE; }
[0-9]+         { yylval.v=new_int(atoi(yytext)); return VALUE; }

 /* strings */
\"                    {yy_push_state(string_state); }
\"\"                  {yylval.v=new_string(""); return VALUE;}
<string_state>[^\"]*  {yylval.v=new_string(yytext); return VALUE;}
<string_state>\"      {yy_pop_state(); }

"/*"                      {yy_push_state(comment_state); }
<comment_state>"*/"       {yy_pop_state(); }
<comment_state>[^*\/]*    {/* ignore comments */}
<comment_state>.          {/* ignore comments */}

 /* constants */
 "NL" { yylval.v=new_string("\n"); return VALUE; }

 /* device */
((([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5]))|"localhost" { yylval.v=new_address(yytext); return VALUE; }

 /* names */
[a-zA-Z_][a-zA-Z0-9_]*  { yylval.str = strdup(yytext); return NAME; }


"//".*  /* ignore comments */


[ \t]   /* ignore white space */
"\n"    { printf("%s", file_mod ? "" : "> "); }

.	{ yyerror("Mystery character %c\n", *yytext); }
%%
