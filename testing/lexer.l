%option stack

%{
  #define YYSTYPE char*
  #include "parser.tab.h"
  #include <stdio.h>
%}

%%

"\n" { return EOL;}

<*>.|\n    { yylval=strdup(yytext); return TEXT; }

%%
