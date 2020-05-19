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
  void open_server(char * port);

%}

%token TEXT
%token EOL

%%

compiler: /* nothing */
| compiler block EOL { fprintf(out, "%s\n", $2); }
;

block: line
| block block  { asprintf(&$$, "%s%s", $1, $2);}
;

line: testo
| line line { asprintf(&$$, "%s%s", $1, $2);}
;

testo: TEXT
| testo testo { asprintf(&$$, "%s%s", $1, $2);}
;


%%
int main(int argc, char **argv)
{
  extern FILE * yyin;
  char * output_file_name="a.out";

  if(argc>1){
    if((strcmp(argv[1]+strlen(argv[1])-3,"pa")==0)){
      fprintf(stderr, "Insert a .pa file");
      return 1;
    }else{
      yyin=fopen(argv[1], "r");
      asprintf(&output_file_name,"%s.out",substring(argv[1], 0, 3));
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
  open_server("8888");
  sleep(3);
  send_command("localhost", "8888", "ciao\nprova\nprova prova");
  send_command("localhost", "8888", "va bene funziona");
  yyparse();

  fclose(yyin);
  fclose(out);
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

void open_server(char * port){
  char * string;
  asprintf(&string, "osascript -e \'tell application \"Terminal\" to do script \"while true; do nc -l  %s; done\"\'", port );
  system(string);

}
