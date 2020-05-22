%option stack

%{
  #define YYSTYPE char*
  #include "parser.tab.h"
  #include <stdio.h>
%}

%%

%x string_state

"connect"/(\(.+\))      { return CONNECT_FUN; }
"disconnect"/(\(.+\))   { return DISCONECT_FUN; }
"send"/(\(.+\))         { return SEND_FUN; }
"receive"/(\(.+\))      { return RECEIVE_FUN; }
"string"  { return STRING; }
"integer" { return INTEGER; }
"real"    { return REAL; }
"device"  { return DEVICE; }
"+"   { return PLUS; }
"-"   { return MINUS; }
"*"   { return MUL; }
"/"   { return DIV; }
"("   { return OP; }
")"   { return CP; }
"{"   { return OGP; }
"}"   { return CGP; }
";"   { return SEMICOLON; }
","   { return COLON; }

\"                  {yy_push_state(string_state); return STRING_O;}
<string_state>\"    {yy_pop_state(); return STRING_C;}

<*>"\n" {}
<*>.    { yylval=strdup(yytext); return TEXT; }

%%
