/* symbol table */
struct symbol {					/* a variable name */
  char *name;
  struct val *value;
  struct ast *func;			/* stmt for the function */
  struct symlist *syms; /* list of dummy args */
};

struct val {
  char type;
	struct val * next;		/* next element if list */
  char* string_val;
  int int_val;
  double real_val;
  unsigned short port_val;
  size_t aliases;
};

/* simple symtab of fixed size */
#define NHASH 9997
struct symbol symtab[NHASH];

struct symbol *lookup(char*);

/* list of symbols, for an argument list */
struct symlist {
  struct symbol *sym;
  struct symlist *next;
};

struct symlist *newsymlist(struct symbol *sym, struct symlist *next);
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
 *
 */

enum bifs {			/* built-in functions */
  B_print = 1,
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
  B_console
};

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
  struct symbol *s;
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
  struct symbol *s;
};
struct symref {
  int nodetype;			/* type N */
  struct symbol *s;
};

struct symdecl {
  int nodetype;			/* type D */
  struct symbol *s;
  char type;
};

struct symasgn {
  int nodetype;			/* type = */
  struct symbol *s;
  struct ast *v;		/* value */
};

struct symdeclasgn {
  int nodetype;			/* type d */
  struct symbol *s;
  struct ast *v;		/* value */
  char type;
};

struct symasgn_l {
  int nodetype;			/* type # */
  struct symbol *s;
  struct ast *i;
  struct ast *v;		/* value */
};

struct foreach {
  int nodetype;			/* type f */
  struct symbol *i;
  struct ast *list;
  struct ast *l;		/* command list */
};

/* build an AST */
struct ast *newast(int nodetype, struct ast *l, struct ast *r);
struct ast *newcmp(int cmptype, struct ast *l, struct ast *r);
struct ast *newfunc(int functype, struct ast *l);
struct ast *newcall(struct symbol *s, struct ast *l);
struct ast *newref(struct symbol *s);
struct ast *newdecl(struct symbol *s, char type);
struct ast *newasgn(struct symbol *s, struct ast *v);
struct ast *newdeclasgn(struct symbol *s, char type, struct ast *v);
struct ast *newvalue(struct val *v);
struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *tr);
struct ast *newforeach(int nodetype, struct symbol *i, struct ast *list, struct ast *l);
struct ast * newref_l(struct symbol *s, struct ast *i);
struct ast * newasgn_l(struct symbol *s, struct ast *i, struct ast *v);

/* define a function */
void dodef(struct symbol *name, struct symlist *syms, struct ast *stmts);

/* evaluate an AST */
struct val * eval(struct ast *);

/* delete and free an AST */
void treefree(struct ast *);

/* interface to the lexer */
extern int yylineno; /* from lexer */
void yyerror(const char *s, ...);
void yyrestart  (FILE * input_file );
int file_mod;

extern int debug;
void dumpast(struct ast *a, int level);
void free_lost(struct val * v);
struct val * callbuiltin(struct fncall *);

char typeof_v(struct val * v);
char typeof_s(struct symbol * s);
char * toString(struct val * v);
struct val * valuedup(struct val * v);
struct val * listdup(struct val * v);
struct val * sum(struct val * a, struct val * b);
struct val * sub(struct val * a, struct val * b);
struct val * mul(struct val * a, struct val * b);
struct val * division(struct val * a, struct val * b);
struct val * new_real(double a);
struct val * new_int(int a);
struct val * new_device(struct val * addr, struct val * port);
struct val * new_string(char * a);
struct val * new_address(char * a);
struct val * new_list(struct ast * list);
struct val * change_sign(struct val * a);
struct val * eval(struct ast *a);
struct val * and_logic(struct val * a, struct val * b);
struct val * or_logic(struct val * a, struct val * b);
struct val * get_element(struct val * list, int index);
struct val * get_port(struct val * v);
struct val * get_address(struct val * v);
int length(struct val * list);
int arg_len(struct ast * l);
void list_remove(struct val * list, struct val * index);
void list_insert(struct val * list, struct val * value, struct val * index);
struct val * s2a(struct val * v);
struct val * s2d(struct val * v);
struct val * s2r(struct val * v);
struct val * s2i(struct val * v);
