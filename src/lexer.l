
%option noyywrap nodefault yylineno
%option stack
%x string_state
%x comment_state
%x IFILE


%{
#include "utils.h"
#include "debuginfo.h"
#include "builtin.h"
#include "types.h"
#include "parser.tab.h"

struct bufstack {
  struct bufstack *prev;
  YY_BUFFER_STATE bs;
  int lineno;
  char *filename;
  FILE *f;
} *curbs = 0;

char *curfilename;

int newfile(char *fn);
int popfile(void);

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
"strip"       { yylval.fn = B_strip; return FUNC; }
"split"       { yylval.fn = B_split; return FUNC; }
"sleep"       { yylval.fn = B_sleep; return FUNC; }
"typeof"      { yylval.fn = B_typeof; return FUNC; }
"isConnected" { yylval.fn = B_isConnected; return FUNC; }


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
 "TRUE"   { yylval.v=new_int(1); return VALUE; }
 "FALSE"  { yylval.v=new_int(0); return VALUE; }

 /* device */
((([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5]))|"localhost" { yylval.v=new_address(yytext); return VALUE; }

 /* names */
[a-zA-Z_][a-zA-Z0-9_]*  { yylval.str = strdup(yytext); return NAME; }

 /* include */
 ^"#"[ \t]*include[ \t]*[\"<] { BEGIN IFILE; }
 <IFILE>[^ \t\n\">]+ {
                        { int c;
                          while((c = input()) && c != '\n') ;
                        }
                        yylineno++;
                        if(!newfile(yytext))
                          yyterminate(); /* no such file */
                        BEGIN INITIAL;
                      }
<IFILE>.|\n {
              fprintf(stderr, "%4d bad include line\n", yylineno);
              yyterminate();
            }

<<EOF>> { if(!popfile()) yyterminate();}

"//".*  /* ignore comments */


[ \t]   /* ignore white space */
"\n"    { printf("%s", file_mod ? "" : "> "); yylineno++; }

.	{ yyerror("Mystery character %c\n", *yytext); }
%%

int newfile(char *fn) {
  FILE *f;
  if(strcmp(fn,"stdin")){
    f=fopen(fn, "r");
  }else{
    f=stdin;
  }
  file_mod++;
  struct bufstack *bs = malloc(sizeof(struct bufstack));
  /* die if no file or no room */
  if(!f) { perror(fn); return 0; }
  if(!bs) { perror("malloc"); exit(1); }

  /* remember state */
  if(curbs)curbs->lineno = yylineno;
  bs->prev = curbs;

  /* set up current entry */
  bs->bs = yy_create_buffer(f, YY_BUF_SIZE);
  bs->f = f;
  bs->filename = fn;
  yy_switch_to_buffer(bs->bs);
  curbs = bs;
  yylineno = 1;
  curfilename = fn;
  return 1;
}

int popfile(void) {
  struct bufstack *bs = curbs;
  struct bufstack *prevbs;
  file_mod--;
  if(!bs) return 0;

  /* get rid of current entry */
  fclose(bs->f);
  yy_delete_buffer(bs->bs);

  /* switch back to previous */
  prevbs = bs->prev;
  free(bs);
  if(!prevbs) return 0;
  yy_switch_to_buffer(prevbs->bs);
  curbs = prevbs;
  yylineno = curbs->lineno;
  curfilename = curbs->filename;
  return 1;
}

int main(int argc, char **argv){
	symstack = calloc(1, sizeof(struct symtable_stack));
	symstack->next = NULL;
	symstack->symtab = NULL;
	inner_scope();
  file_mod=0;
	for(int i = 1; i<argc; i++){
		if((strcmp(argv[i]+strlen(argv[i])-3,".pa")!=0)){
      fprintf(stderr, "Insert a .pa file");
      return 1;
    }else{
      file_mod++;
      if(newfile(argv[i])) yyparse();
      file_mod--;
    }
	}
	yyrestart(stdin);
	yylineno = 1;
  printf("%s", file_mod ? "" : "> ");
  yyparse();
	outer_scope();
	return 0;
}
