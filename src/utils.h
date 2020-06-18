#ifndef UTILS_H
#define UTILS_H

enum bifs {			/* built-in functions */
  B_print = 1,
  B_quit,
  B_connect,
  B_disconnect,
  B_send,
  B_receive,
  B_insert,
  B_remove,
  B_length,
  B_port,
  B_address,
  B_s2i,
  B_s2r,
  B_s2d,
  B_s2a,
  B_toString,
  B_console,
  B_split,
  B_strip,
  B_sleep,
  B_typeof,
  B_isConnected
};

/* symbol table */
struct symbol {					/* a variable name */
  char *name;
  struct val *value;
  struct ast *func;			/* stmt for the function */
  struct symlist *syms; /* list of dummy args */
};


struct symtable_stack{
	struct symbol * symtab;
	struct symtable_stack * next;
};

/* simple symtab of fixed size */
#define NHASH 9997

struct symtable_stack * symstack;

struct symbol *lookup(char*);
struct symbol *insert_symbol(char*);

/* list of symbols, for an argument list */
struct symlist {
  char *sym;
  struct symlist *next;
};

struct symlist *newsymlist(char *sym, struct symlist *next);
void symlistfree(struct symlist *sl);

/* node types
 *  + - * / |
 *  0-7 comparison ops, bit coded 04 equal, 02 less, 01 greater
 *  M unary minus
 *  L statement list
 *  I IF statement
 *  W WHILE statement
 *  N symbol ref
 *  = assignment
 *  S list of symbols
 *  F built in function call
 *  C user function call
 *  d type declaration and assignement
 *  # assignement to list position
 *  n list element ref
 *  J static list
 */


/* nodes in the Abstract Syntax Tree */
/* all have common initial nodetype */

struct ast {
  int nodetype;
  struct ast *l;
  struct ast *r;
};

struct fncall {			/* built-in function */
  int nodetype;			/* type F */
  struct ast *l;
  enum bifs functype;
};

struct ufncall {		/* user function */
  int nodetype;			/* type C */
  struct ast *l;		/* list of arguments */
  char *s;
};

struct flow {
  int nodetype;			/* type I or W */
  struct ast *cond;	/* condition */
  struct ast *tl;		/* then or do list */
  struct ast *el;		/* optional else list */
};

struct value_val {
  int nodetype;			/* type K */
  struct val * v;
};

struct symref_l {
  int nodetype;			/* type n */
  struct ast *i;		/* index*/
  char *s;
};
struct symref {
  int nodetype;			/* type N */
  char *s;
};

struct symdecl {
  int nodetype;			/* type D */
  char *s;
  char type;
};

struct symasgn {
  int nodetype;			/* type = */
  char *s;
  struct ast *v;		/* value */
};

struct symdeclasgn {
  int nodetype;			/* type d */
  char *s;
  struct ast *v;		/* value */
  char type;
};

struct symasgn_l {
  int nodetype;			/* type # */
  char *s;
  struct ast *i;
  struct ast *v;		/* value */
};

struct foreach {
  int nodetype;			/* type f */
  char *i;
  struct ast *list;
  struct ast *l;		/* command list */
};

/* build an AST */
struct ast *newast(int nodetype, struct ast *l, struct ast *r);
struct ast *newcmp(int cmptype, struct ast *l, struct ast *r);
struct ast *newfunc(int functype, struct ast *l);
struct ast *newcall(char *s, struct ast *l);
struct ast *newref(char *s);
struct ast *newdecl(char *s, char type);
struct ast *newasgn(char *s, struct ast *v);
struct ast *newdeclasgn(char *s, char type, struct ast *v);
struct ast *newvalue(struct val *v);
struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *tr);
struct ast *newforeach(int nodetype, char *i, struct ast *list, struct ast *l);
struct ast *newref_l(char *s, struct ast *i);
struct ast *newasgn_l(char *s, struct ast *i, struct ast *v);

/* define a function */
void dodef(char *name, struct symlist *syms, struct ast *stmts);


/* delete and free an AST */
void treefree(struct ast *);

/* interface to the lexer */
extern int yylineno; /* from lexer */
void yyerror(const char *s, ...);
void yyrestart  (FILE * input_file );
int file_mod;

void inner_scope();
void outer_scope();
void free_lost(struct val * v);
void remove_symbol(struct symbol * s);
struct val * calluser(struct ufncall *);

char typeof_v(struct val * v);
char typeof_s(struct symbol * s);
char * toString(struct val * v);
struct val * valuedup(struct val * v);
struct val * listdup(struct val * v);
struct val * eval(struct ast *a);
struct val * get_element(struct val * list, int index);
int length(struct val * list);
int arg_len(struct ast * l);


#endif /* UTILS_H */
