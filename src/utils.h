/* symbol table */
struct symbol {		/* a variable name */
  char *name;
  struct val *value;
  struct ast *func;	/* stmt for the function */
  struct symlist *syms; /* list of dummy args */
};

struct val {
  char type;
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
 */

enum bifs {			/* built-in functions */
  B_print = 1,
  B_connect, B_disconnect, B_send, B_listen
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
  struct ast *cond;		/* condition */
  struct ast *tl;		/* then or do list */
  struct ast *el;		/* optional else list */
};

struct value_val {
  int nodetype;			/* type K */
  struct val * v;
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

/* build an AST */
struct ast *newast(int nodetype, struct ast *l, struct ast *r);
struct ast *newcmp(int cmptype, struct ast *l, struct ast *r);
struct ast *newfunc(int functype, struct ast *l);
struct ast *newcall(struct symbol *s, struct ast *l);
struct ast *newref(struct symbol *s);
struct ast *newdecl(struct symbol *s, char type);
struct ast *newasgn(struct symbol *s, struct ast *v);
struct ast *newvalue(struct val *v);
struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *tr);

/* define a function */
void dodef(struct symbol *name, struct symlist *syms, struct ast *stmts);

/* evaluate an AST */
struct val * eval(struct ast *);

/* delete and free an AST */
void treefree(struct ast *);

/* interface to the lexer */
extern int yylineno; /* from lexer */
void yyerror(const char *s, ...);

extern int debug;
void dumpast(struct ast *a, int level);
void free_lost(struct val * v);
void callbuiltin(struct fncall *);

char typeof_v(struct val * v);
char typeof_s(struct symbol * s);
char * toString(struct val * v);
struct val * sum(struct val * a, struct val * b);
struct val * sub(struct val * a, struct val * b);
struct val * mul(struct val * a, struct val * b);
struct val * division(struct val * a, struct val * b);
struct val * new_real(double a);
struct val * new_int(int a);
struct val * new_device(struct val * addr, struct val * port);
struct val * new_string(char * a);
struct val * new_address(char * a);
struct val * change_sign(struct val * a);
struct val * eval(struct ast *a);
struct val * and_logic(struct val * a, struct val * b);
struct val * or_logic(struct val * a, struct val * b);
