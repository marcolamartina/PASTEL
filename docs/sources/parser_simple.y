
%{
%}

/* declare tokens */
%token IF ELSE WHILE FOR IN DEF
%token <fn> FUNC ADDR PORT
%token <str> NAME
%token <c> TYPE
%token <v> VALUE


%right '='
%nonassoc ADDR PORT
%left AND OR
%nonassoc <fn> CMP
%left ':'
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS


%type <a> exp stmt list explist decl
%type <sl> symlist

%start program

%%

program: /* nothing */
  | program stmt 
  | program DEF NAME  symlist   list  
  | program error
;
stmt: IF  exp  list        
   | IF  exp   list  ELSE  list  
   | WHILE  exp   list                
   | FOR  NAME IN exp   list         
   | exp 
   | decl 
;
%%
