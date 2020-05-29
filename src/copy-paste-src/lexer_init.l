%option stack
%x string_state

%{
  #define YYSTYPE char*
  #include "parser.tab.h"
  #include <stdio.h>
%}

%%

"connect"/\(      { return CONNECT_FUN; }
"disconnect"/\(   { return DISCONECT_FUN; }
"send"/\(         { return SEND_FUN; }
"receive"/\(      { return RECEIVE_FUN; }
"string"  { return STRING_TYPE; }
"integer" { return INTEGER_TYPE; }
"real"    { return REAL_TYPE; }
"device"  { return DEVICE_TYPE; }
" in "      { return IN; }
"while"/\(   { return WHILE; }
"for"/\(     { return FOR; }
[0-9]+\.[0-9]+ { /*yylval=atof(yytext);*/ return REAL_VAL; }
[0-9]+         { /*yylval=atoi(yytext);*/ return INTEGER_VAL; }
(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])/(:[0-9]+) { yylval=strdup(yytext); return IP; }
:[0-9]+ { yylval=strdup(yytext+1); return PORT;}
"+"   { return PLUS; }
"-"   { return MINUS; }
"*"   { return MUL; }
"/"   { return DIV; }
"("   { return OP; }
")"   { return CP; }
"{"   { return OCB; }
"}"   { return CCB; }
";"   { return SEMICOLON; }
","   { return COLON; }
" "   {}

\"                  {yy_push_state(string_state); return STRING_O;}
<string_state>\"    {yy_pop_state(); return STRING_C;}

<*>"\n" {}
<*>.    { yylval=strdup(yytext); return TEXT; }

%%
