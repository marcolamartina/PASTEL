#  include <stdio.h>
#  include <stdlib.h>
#  include <stdarg.h>
#  include <string.h>
#  include <math.h>
#  include "utils.h"
int yyparse();

/* symbol table */
/* hash a symbol */
static unsigned symhash(char *sym)
{
  unsigned int hash = 0;
  unsigned c;

  while((c = *sym++)) hash = hash*9 ^ c;

  return hash;
}

struct symbol * lookup(char* sym){
  struct symbol *sp = &symtab[symhash(sym)%NHASH];
  int scount = NHASH;		/* how many have we looked at */

  while(--scount >= 0) {
    if(sp->name && !strcmp(sp->name, sym)) { return sp; }

    if(!sp->name) {		/* new entry */
      sp->name = strdup(sym);
      sp->value = malloc(sizeof(struct val));
      sp->func = NULL;
      sp->syms = NULL;
      return sp;
    }

    if(++sp >= symtab+NHASH) sp = symtab; /* try the next entry */
  }
  yyerror("symbol table overflow\n");
  abort(); /* tried them all, table is full */

}

struct ast * newast(int nodetype, struct ast *l, struct ast *r){
  struct ast *a = malloc(sizeof(struct ast));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = nodetype;
  a->l = l;
  a->r = r;
  return a;
}

struct ast * newvalue(struct val * v){
  struct value_val *a = malloc(sizeof(struct value_val));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'K';
  a->v = v;
  return (struct ast *)a;
}

struct ast * newcmp(int cmptype, struct ast *l, struct ast *r){
  struct ast *a = malloc(sizeof(struct ast));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = '0' + cmptype;
  a->l = l;
  a->r = r;
  return a;
}

struct ast * newfunc(int functype, struct ast *l){
  struct fncall *a = malloc(sizeof(struct fncall));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'F';
  a->l = l;
  a->functype = functype;
  return (struct ast *)a;
}

struct ast * newcall(struct symbol *s, struct ast *l) {
  struct ufncall *a = malloc(sizeof(struct ufncall));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'C';
  a->l = l;
  a->s = s;
  return (struct ast *)a;
}

struct ast * newref(struct symbol *s){
  struct symref *a = malloc(sizeof(struct symref));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'N';
  a->s = s;
  return (struct ast *)a;
}

struct ast * newdecl(struct symbol *s, char type){
  struct symref *a = malloc(sizeof(struct symref));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'D';
  a->s = s;
  a->s->value->type = type ;
  return (struct ast *)a;
}

struct ast * newasgn(struct symbol *s, struct ast *v){
  struct symasgn *a = malloc(sizeof(struct symasgn));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = '=';
  a->s = s;
  a->v = v;
  return (struct ast *)a;
}

struct ast * newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *el){
  struct flow *a = malloc(sizeof(struct flow));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = nodetype;
  a->cond = cond;
  a->tl = tl;
  a->el = el;
  return (struct ast *)a;
}

struct symlist * newsymlist(struct symbol *sym, struct symlist *next){
  struct symlist *sl = malloc(sizeof(struct symlist));

  if(!sl) {
    yyerror("out of space");
    exit(0);
  }
  sl->sym = sym;
  sl->next = next;
  return sl;
}

void symlistfree(struct symlist *sl){
  struct symlist *nsl;

  while(sl) {
    nsl = sl->next;
    free(sl);
    sl = nsl;
  }
}


static struct val * callbuiltin(struct fncall *);

char typeof_v(struct val * v){
  return v->type;
}

char typeof_s(struct symbol * s){
  return s->value->type;
}

char * toString(struct val * v){
  char * result;
  switch (typeof_v(v)) {
    case 'i': asprintf(&result, "%d", v->int_val); break;
    case 'r': asprintf(&result, "%f", v->real_val); break;
    case 's': result= v->string_val; break;
    default: yyerror("cannot print a value of type \"%c\"", typeof_v(v)); break;
  }
  return result;
}

struct val * sum(struct val * a, struct val * b){
  struct val * result=malloc(sizeof(struct val *));
  if(typeof_v(a)==typeof_v(b)){
    result->type=typeof_v(a);
    switch (typeof_v(a)) {
      case 'i': result->int_val=a->int_val+b->int_val; break;
      case 's': asprintf(&result->string_val, "%s%s", a->string_val, b->string_val); break;
      case 'r': result->real_val=a->real_val+b->real_val; break;
      default: yyerror("addiction not supported for these types (%c/%c)", typeof_v(a), typeof_v(b)); break;
    }
  }else{
    yyerror("addiction of incompatible types (%c+%c)", typeof_v(a), typeof_v(b));
  }
  return result;
}

struct val * sub(struct val * a, struct val * b){
  struct val * result=malloc(sizeof(struct val *));
  if(typeof_v(a)==typeof_v(b)){
    result->type=typeof_v(a);
    switch (typeof_v(a)) {
      case 'i': result->int_val=a->int_val-b->int_val; break;
      case 'r': result->real_val=a->real_val-b->real_val; break;
      default: yyerror("subtraction not supported for these types (%c/%c)", typeof_v(a), typeof_v(b)); break;
    }
  }else{
    yyerror("subtraction of incompatible types (%c-%c)", typeof_v(a), typeof_v(b));
  }
  return result;
}

struct val * mul(struct val * a, struct val * b){
  struct val * result=malloc(sizeof(struct val *));
  if(typeof_v(a)==typeof_v(b)){
    result->type=typeof_v(a);
    switch (typeof_v(a)) {
      case 'i': result->int_val=a->int_val*b->int_val; break;
      case 'r': result->real_val=a->real_val*b->real_val; break;
      default: yyerror("multiplication not supported for these types (%c/%c)", typeof_v(a), typeof_v(b)); break;
    }
  }else if(typeof_v(a)=='s' && typeof_v(b)=='i'){
    if(b->int_val>0){
      result->type=typeof_v(a);
      asprintf(&result->string_val, "%s", a->string_val);
      for(int i=0; i<b->int_val-1; i++){
        asprintf(&(result->string_val), "%s%s", result->string_val, a->string_val);
      }
    }else{
      yyerror("string multiply allowed only for positive integer");
    }
  }else if(typeof_v(b)=='s' && typeof_v(a)=='i'){
    result->type=typeof_v(b);
    if(a->int_val>0){
      asprintf(&result->string_val, "%s", b->string_val);
      for(int i=0; i<a->int_val-1; i++){
        asprintf(&result->string_val, "%s%s", result->string_val, b->string_val);
      }
    }else{
      yyerror("string multiply allowed only for positive integer");
    }
  }else{
    yyerror("multiplication of incompatible types (%c*%c)", typeof_v(a), typeof_v(b));
  }
  return result;
}

struct val * division(struct val * a, struct val * b){
  struct val * result=malloc(sizeof(struct val *));
  if(typeof_v(a)==typeof_v(b)){
    result->type=typeof_v(a);
    switch (typeof_v(a)) {
      case 'i': if(b->int_val!=0){
                  result->int_val=a->int_val/b->int_val;
                }else{
                  yyerror("integer division for zero");
                }
                break;
      case 'r': if(b->real_val!=0){
                  result->real_val=a->real_val/b->real_val;
                }else{
                  yyerror("real division for zero");
                }
                break;
      default: yyerror("division not supported for these types (%c/%c)", typeof_v(a), typeof_v(b)); break;
    }
  }else{
    yyerror("division of incompatible types (%c/%c)", typeof_v(a), typeof_v(b));
  }
  return result;
}

struct val * eval(struct ast *a){
  struct val *v;

  if(!a) {
    yyerror("internal error, null eval");
    return NULL;
  }

  switch(a->nodetype) {
    /* constant */
  case 'K': v = ((struct value_val *)a)->v; break;

    /* name reference */
  case 'N': v = ((struct symref *)a)->s->value; break;

  /* name declaration */
  case 'D': break;

    /* assignment */
  case '=':
      v=eval(((struct symasgn *)a)->v);
      if(typeof_v(v)==typeof_s(((struct symasgn *)a)->s)){
        v = ((struct symasgn *)a)->s->value = eval(((struct symasgn *)a)->v);
      }else{
        yyerror("assignement error for incompatible types (%c=%c)", typeof_s(((struct symasgn *)a)->s), typeof_v(v) );
      }
      break;

    /* expressions */
  case '+': v = sum(eval(a->l),eval(a->r)); break;
  case '-': v = sub(eval(a->l),eval(a->r)); break;
  case '*': v = mul(eval(a->l),eval(a->r)); break;
  case '/': v = division(eval(a->l),eval(a->r)); break;
  /*case '|': v = fabs(eval(a->l)); break;
  case 'M': v = -eval(a->l); break;
*/
    /* comparisons */
  /*case '1': v = (eval(a->l) > eval(a->r))? 1 : 0; break;
  case '2': v = (eval(a->l) < eval(a->r))? 1 : 0; break;
  case '3': v = (eval(a->l) != eval(a->r))? 1 : 0; break;
  case '4': v = (eval(a->l) == eval(a->r))? 1 : 0; break;
  case '5': v = (eval(a->l) >= eval(a->r))? 1 : 0; break;
  case '6': v = (eval(a->l) <= eval(a->r))? 1 : 0; break;
*/
  /* control flow */
  /* null if/else/do expressions allowed in the grammar, so check for them */
  case 'I':
    if( eval( ((struct flow *)a)->cond) != 0) {
      if( ((struct flow *)a)->tl) {
	v = eval( ((struct flow *)a)->tl);
      } else
	v = NULL;		/* a default value */
    } else {
      if( ((struct flow *)a)->el) {
        v = eval(((struct flow *)a)->el);
      } else
	v = NULL;		/* a default value */
    }
    break;

  case 'W':
    v = NULL;		/* a default value */

    if( ((struct flow *)a)->tl) {
      while( eval(((struct flow *)a)->cond) != 0)
	v = eval(((struct flow *)a)->tl);
    }
    break;			/* last value is value */

  case 'L': eval(a->l); v = eval(a->r); break;

  case 'F': v = callbuiltin((struct fncall *)a); break;

  default: printf("internal error: bad node %c\n", a->nodetype);
  }
  return v;
}

static struct val * callbuiltin(struct fncall *f) {
  enum bifs functype = f->functype;
  struct val * v = eval(f->l);
  struct val * result= malloc(sizeof(struct val));

 switch(functype) {
 case B_sqrt:
   result->real_val= sqrt(v->real_val);
 case B_exp:
   result->real_val= exp(v->real_val);
 case B_log:
   result->real_val= log(v->real_val);
 case B_print:
   printf("= %s\n", toString(v));
   result= v;
 default:
   yyerror("Unknown built-in function %d", functype);
   return NULL;
 }
 return result;
}



void treefree(struct ast *a){
  switch(a->nodetype) {

    /* two subtrees */
  case '+':
  case '-':
  case '*':
  case '/':
  case '1':  case '2':  case '3':  case '4':  case '5':  case '6':
  case 'L':
    treefree(a->r);

    /* one subtree */
  case '|':
  case 'M': case 'C': case 'F':
    treefree(a->l);

    /* no subtree */
  case 'K': case 'N': case 'D':
    break;

  case '=':
    free( ((struct symasgn *)a)->v);
    break;

  case 'I': case 'W':
    free( ((struct flow *)a)->cond);
    if( ((struct flow *)a)->tl) free( ((struct flow *)a)->tl);
    if( ((struct flow *)a)->el) free( ((struct flow *)a)->el);
    break;

  default: printf("internal error: free bad node %c\n", a->nodetype);
  }

  free(a); /* always free the node itself */

}

void yyerror(char *s, ...){
  va_list ap;
  va_start(ap, s);

  fprintf(stderr, "%d: error: ", yylineno);
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
  //exit(1);
}

int
main()
{
  printf("> ");
  return yyparse();
}

/* debugging: dump out an AST */
int debug = 1;

void dumpast(struct ast *a, int level){

  printf("%*s", 2*level, "");	/* indent to this level */
  level++;

  if(!a) {
    printf("NULL\n");
    return;
  }

  switch(a->nodetype) {
    /* constant */
  case 'K': printf("value %s\n", toString(((struct value_val *)a)->v)); break;

    /* name reference */
  case 'N': printf("ref %s", ((struct symref *)a)->s->name);
            if(((struct symref *)a)->s->value){
              printf("=%s\n", toString(((struct symref *)a)->s->value));
            }else{
              printf("\n");
            }
            break;

  /* name declaration */
  case 'D': printf("decl %s\n", ((struct symref *)a)->s->name); break;

    /* assignment */
  case '=': printf("= %s\n", ((struct symref *)a)->s->name);
    dumpast( ((struct symasgn *)a)->v, level); return;

    /* expressions */
  case '+': case '-': case '*': case '/': case 'L':
  case '1': case '2': case '3':
  case '4': case '5': case '6':
    printf("binop %c\n", a->nodetype);
    dumpast(a->l, level);
    dumpast(a->r, level);
    return;

  case '|': case 'M':
    printf("unop %c\n", a->nodetype);
    dumpast(a->l, level);
    return;

  case 'I': case 'W':
    printf("flow %c\n", a->nodetype);
    dumpast( ((struct flow *)a)->cond, level);
    if( ((struct flow *)a)->tl)
      dumpast( ((struct flow *)a)->tl, level);
    if( ((struct flow *)a)->el)
      dumpast( ((struct flow *)a)->el, level);
    return;

  case 'F':
    printf("builtin %d\n", ((struct fncall *)a)->functype);
    dumpast(a->l, level);
    return;


  default: printf("bad %c\n", a->nodetype);
    return;
  }
}
