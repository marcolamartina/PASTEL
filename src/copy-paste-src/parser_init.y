%{
  #define YYSTYPE char*
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  void yyerror(const char *s);
  FILE * out;
  int yylex();
  char * substring(char * s, int start, int end );
  void send_command(char * ip, char * port, char * command);
  void open_server_mac(char * port);

%}

%token TEXT
%token EOL
%token FOR WHILE
%token STRING_TYPE
%token INTEGER_TYPE
%token REAL_TYPE
%token DEVICE_TYPE
%token REAL_VAL
%token INTEGER_VAL
%token IP
%token PORT
%token OP CP
%token CONNECT_FUN DISCONECT_FUN
%token SEND_FUN RECEIVE_FUN
%token PLUS MINUS MUL DIV
%token OCB CCB
%token SEMICOLON COLON
%token STRING_O STRING_C
%token IN

%define parse.error verbose

%%

compiler: /* nothing */
| compiler line { fprintf(out, "%s\n", $2); }
| compiler multiline { fprintf(out, "%s\n", $2); }
;

multiline: line
| FOR OP testo IN list CP block { asprintf(&$$, "for %s in %s ", $3, $5); }
| WHILE OP exp CP block { asprintf(&$$, "while %s ", $3);}
;

list: testo
;

block: multiline CCB
| OCB multiline CCB {$$=$2; }
| OCB CCB { asprintf(&$$, ""); }
| OCB multiline block  { asprintf(&$$, "%s%s", $2, $3);}
;

line: exp SEMICOLON
| exp line { asprintf(&$$, "%s%s", $1, $2);}
| declaration SEMICOLON
;


exp: testo
| exp exp { asprintf(&$$, "%s%s", $1, $2);}
;

declaration: REAL_TYPE testo {asprintf(&$$, "declaration of %s", $2);}
| INTEGER_TYPE testo {asprintf(&$$, "declaration of %s", $2);}
| STRING_TYPE testo {asprintf(&$$, "declaration of %s", $2);}
| DEVICE_TYPE testo {asprintf(&$$, "declaration of %s", $2);}
;



testo: TEXT
| testo testo { asprintf(&$$, "%s%s", $1, $2);}
;


%%
int main(int argc, char **argv)
{
  extern FILE * yyin;
  char * output_file_name="a.c";

  if(argc>1){
    if((strcmp(argv[1]+strlen(argv[1])-3,"pa")==0)){
      fprintf(stderr, "Insert a .pa file");
      return 1;
    }else{
      yyin=fopen(argv[1], "r");
      asprintf(&output_file_name,"%s.c",substring(argv[1], 0, 3));
      out = fopen(output_file_name, "w+");
    }
  }else{
    yyin=stdin;
    out=stdout;
  }


  if (!yyin){
      yyerror("Error on opening source file");
      return 1;
  }

  if (!out){
      yyerror("Error on creating output file");
      return 1;
  }


  yyparse();
  fclose(yyin);
  fclose(out);
  /*char * command;
  asprintf(&command,"make %s", substring(output_file_name,0,2));
  system(command);*/
  return 0;
}

void yyerror(const char *s){
  fprintf(stderr, "error: %s\n", s);
}

char * substring(char * s, int start, int end ){
  int l=strlen(s)-start-end;
  char result[l];
  strncpy(result, s+start, l);
  result[l]='\0';
  return strdup(result);
}

void send_command(char * ip, char * port, char * command){
  char * string;
  asprintf(&string, "echo \"%s\" | nc %s %s ", command, ip, port );
  system(string);
}

void open_server_mac(char * port){
  char * string;
  asprintf(&string, "osascript -e \'tell application \"Terminal\" to do script \"nc -lk %s\"\'", port );
  system(string);

}
