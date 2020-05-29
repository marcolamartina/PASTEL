/* recognize tokens for the calculator */

%option noyywrap nodefault yylineno
%option stack
%x string_state

%{
# include "utils.h"
# include "parser.tab.h"
%}


%%

 /* keywords */

"if"    { return IF; }
"else"  { return ELSE; }
"while" { return WHILE; }
"for"   { return FOR; }
" in "  { return IN; }
"def"   { return DEF; }
"string"  { return STRING_TYPE; }
"integer" { return INTEGER_TYPE; }
"real"    { return REAL_TYPE; }
"device"  { return DEVICE_TYPE; }

/* numbers */
[0-9]+\.[0-9]+ { yylval=atof(yytext); return REAL_VAL; }
[0-9]+         { yylval=atoi(yytext); return INTEGER_VAL; }

/* strings */
\"                  {yy_push_state(string_state); }
<string_state>\"    {yy_pop_state(); }
<string_state>[^\"]*{yylval=strdup(yytext); return STRING_VAL;}


 /* names */
[a-zA-Z_][a-zA-Z0-9_]*  { yylval.s = strdup(yytext); return NAME; }


"//".*  /* ignore comments */
[ \t]   /* ignore white space */
"\n"    { printf("\n> "); }

.	{ yyerror("Mystery character %c\n", *yytext); }
%%
