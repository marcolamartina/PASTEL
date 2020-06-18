#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "utils.h"
#include "builtin.h"
#include "operators.h"
#include "types.h"
#include "eval.h"

int yyparse();

extern int file_mod;

static struct symbol * lookup_aux(char* sym, int print_error);

/* symbol table */
/* hash a symbol */
static unsigned symhash(char *sym)
{
  unsigned int hash = 0;
  unsigned c;

  while((c = *sym++)) hash = hash*9 ^ c;

  return hash;
}

void remove_symbol(struct symbol * s){
	struct symbol * symtab;
	symtab = symstack->symtab;
  struct symbol *sp = &symtab[symhash(s->name)%NHASH];
  int scount = NHASH;		/* how many have we looked at */

  while(--scount >= 0) {
    if(sp->name && !strcmp(sp->name, s->name)) {
			sp->value->aliases--;
			free(sp->name);
		//	free_lost(sp->value);
			sp->name=NULL;
			//free(s);
			return;
		}

    if(!sp->name) {		/* new entry */
			yyerror("Trying to delete a non existent symbol.");
			return;
    }
    if(++sp >= symtab+NHASH) sp = symtab; /* try the next entry */
  }
  yyerror("Trying to delete a non existent symbol.");
}

struct symbol * lookup(char* sym){
	return lookup_aux(sym, 0);
}

struct symbol * lookup_aux(char* sym, int print_error){
	struct symbol * symtab;
	struct symtable_stack * curr_scope = symstack;
  struct symbol *sp;
  int scount = NHASH;		/* how many have we looked at */

	while(curr_scope && curr_scope->symtab){
		symtab = curr_scope->symtab;
		sp = &symtab[symhash(sym)%NHASH];
		while(--scount >= 0) {
			if(sp->name && !strcmp(sp->name, sym)) { return sp; }
			if(!sp->name) {		/* new entry */
				break;
			}
			if(++sp >= symtab+NHASH) sp = symtab; /* try the next entry */
		}
		curr_scope = curr_scope->next;
	}
	if(print_error){
		yyerror("Symbol %s not defined", sym);
	}
	return NULL;
}

struct symbol * insert_symbol(char* sym){
	struct symbol * symtab;
	struct symtable_stack * curr_scope = symstack;
  struct symbol *sp;
  int scount = NHASH;		/* how many have we looked at */

	curr_scope = symstack;
	symtab = curr_scope->symtab;
	sp = &symtab[symhash(sym)%NHASH];
	while(--scount >= 0) {
    if(sp->name && !strcmp(sp->name, sym)) {
			yyerror("Symbol already defined in this scope");
			return NULL;
		 }
    if(!sp->name) {		/* new entry */
      sp->name = strdup(sym);
      sp->value = malloc(sizeof(struct val));
			sp->value->next = NULL;
      sp->value->aliases=1;
			sp->value->type = 'u';
			sp->value->string_val = NULL;
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


struct ast * newcall(char *s, struct ast *l) {
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

struct ast * newref_l(char *s, struct ast *i){
  struct symref_l *a = malloc(sizeof(struct symref_l));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'n';
  a->s = s;
	a->i = i;
  return (struct ast *)a;
}

struct ast * newref(char *s){
  struct symref *a = malloc(sizeof(struct symref));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'N';
  a->s = s;
  return (struct ast *)a;
}

struct ast * newdecl(char *s, char type){
  struct symdecl *a = malloc(sizeof(struct symdecl));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'D';
  a->s = s;
  a->type = type;

	return (struct ast *)a;
}

struct ast * newasgn(char *s, struct ast *v){
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

struct ast * newdeclasgn(char *s, char type, struct ast *v){
  struct symdeclasgn *a = malloc(sizeof(struct symdeclasgn));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'd';
  a->s = s;
  a->v = v;
  a->type = type;
  return (struct ast *)a;
}

struct ast * newasgn_l(char *s, struct ast *i, struct ast *v){
  struct symasgn_l *a = malloc(sizeof(struct symasgn_l));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = '#';
  a->s = s;
  a->i = i;
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

struct ast * newforeach(int nodetype, char *i, struct ast *list, struct ast *l){
  struct foreach *a = malloc(sizeof(struct foreach));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = nodetype;
  a->i = i;
  a->list = list;
  a->l = l;
  return (struct ast *)a;
}

struct symlist * newsymlist(char *sym, struct symlist *next){
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

struct val * get_element(struct val * list, int index){
  struct val * result=list;
  if(list){
    if(typeof_v(list)=='l'){
      for(int i=0; i<index && result->next; i++){
        result=result->next;
      }
      if(result->next == NULL){
        yyerror("Index out of bounds");
      }
    } else {
      yyerror("variable is not a list, found %c", list->type);
      return NULL;
    }
  } else {
    yyerror("list not initialized");
    return NULL;
  }
  return result->next;
}

int length(struct val * list){
  int len=0;
  if(list){
    if(typeof_v(list)=='l'){
      while((list=list->next)){
        len++;
      }
    } else {
      yyerror("variable is not a list, found %c", list->type);
      return 0;
    }
  } else {
    yyerror("list not initialized");
    return 0;
  }
  return len;
}


char typeof_v(struct val * v){
	if(v){
  	return v->type;
	} else {
		yyerror("NULL value detected");
		return 'u';
	}
}

char typeof_s(struct symbol * s){
	if(s){
  	return s->value->type;
	} else {
		yyerror("NULL value detected");
		return 'u';
	}
}

char * toString(struct val * v){
  char * result;
  char * temp1;
  char * temp2;
  switch (typeof_v(v)) {
    case 'i': asprintf(&result, "%d", v->int_val); break;
    case 'l':
      asprintf(&result, "[ ");
      for(int i=0; i<length(v); i++){
        temp1=toString(get_element(v,i));
        temp2=result;
        asprintf(&result, "%s%s%s,", temp2, (i==0?"":" ") ,temp1);
        free(temp1);
        free(temp2);
      }
      result[strlen(result)-1]=' ';
      temp2=result;
      asprintf(&result, "%s%s", temp2, "]");
      free(temp2);
      break;
    case 'r': asprintf(&result, "%f", v->real_val); break;
    case 's': result= strdup(v->string_val); break;
		case 'a': result= strdup(v->string_val); break;					//IP address
		case 'd': asprintf(&result, "%s:%d", v->string_val, v->port_val); break;
		case 'u': result=strdup("uninstantiated variable"); break;
    default: yyerror("cannot print a value of type \"%c\"", typeof_v(v)); result=strdup(""); break;
  }
  return result;
}

struct val * listdup(struct val * v){
  struct val *v2=NULL;
  if(typeof_v(v)=='l'){
    v2=malloc(sizeof(struct val));
    v2->aliases=0;
    v2->type='l';
    v2->next=valuedup(get_element(v,0));

    for(int i=0; i<length(v)-1; i++){
      get_element(v2,i)->next=valuedup(get_element(v,i+1));
    }
  }
  return v2;
}

struct val * valuedup(struct val * v){
	struct val *v2;
	switch (typeof_v(v)){
		case 'i': v2 = new_int(v->int_val); break;
		case 'r': v2 = new_real(v->real_val); break;
		case 's': v2 = new_string(v->string_val); break;
		case 'a': v2 = new_address(v->string_val); break;
		case 'd':
			 v2 = new_device(new_address(v->string_val), new_int(v->port_val));
			 break;
    //case 'l': v2 = listdup(v); break;
		default:
			v2=v;
			break;
	}
	return v2;
}



/* define a function */
void dodef(char *name, struct symlist *syms, struct ast *func){
	struct symbol * fname = insert_symbol(name);
	if(! fname){
		return;
	}
  fname->syms = syms;
  fname->func = func;
}


int arg_len(struct ast * list){
  int len=1;
  if(!list) return 0;
  while(list->nodetype=='L'){
    len++;
    list=list->r;
  }
  return len;
}


void inner_scope(){
	struct symtable_stack * new_scope = calloc(1, sizeof(struct symtable_stack));
	new_scope-> next = symstack;
	new_scope->symtab = calloc(NHASH, sizeof(struct symbol));
	if(!new_scope->symtab){
    yyerror("out of space");
    exit(0);
	}
	symstack = new_scope;
}

void outer_scope(){
	struct symtable_stack * inner = symstack;
	symstack = symstack->next;
	free(inner->symtab);
	free(inner);
}

struct val * calluser(struct ufncall *f){
  struct symbol *fn = lookup(f->s);	/* function name */
	if(!fn){
		yyerror("call to undefined function", f->s);
		return NULL;
	}
  struct symlist *sl;								/* dummy arguments */
  struct ast *args = f->l;					/* actual arguments */
  struct val **oldval, **newval;		/* saved arg values */
  struct val * v;
  int nargs;
  int i;

  if(!fn->func) {
    yyerror("call to undefined function", fn->name);
    return NULL;
  }
	inner_scope();
  /* count the arguments */
  sl = fn->syms;
  for(nargs = 0; sl; sl = sl->next){
    nargs++;
  }

  /* prepare to save them */
  oldval = (struct val **)malloc(nargs * sizeof(struct val));
  newval = (struct val **)malloc(nargs * sizeof(struct val));
  if(!oldval || !newval) {
    yyerror("Out of space in %s", fn->name); return NULL;
  }

  /* evaluate the arguments */
  for(i = 0; i < nargs; i++) {
    if(!args) {
      yyerror("too few args in call to %s", fn->name);
      free(oldval); free(newval);
      return NULL;
    }

    if(args->nodetype == 'L') {	/* if this is a list node */
      newval[i] = eval(args->l);
      args = args->r;
    } else {			/* if it's the end of the list */
      newval[i] = eval(args);
      args = NULL;
    }
  }

  /* save old values of dummies, assign new ones */
  sl = fn->syms;
  for(i = 0; i < nargs; i++) {
    struct symbol *s = lookup_aux(sl->sym,0);
	if(!s){
		s = insert_symbol(sl->sym);
	}
	oldval[i] = s->value;
	s->value = newval[i];
	sl = sl->next;
  }


  /* evaluate the function */
  v = eval(fn->func);

  /* put the dummies back */
  sl = fn->syms;
  for(i = 0; i < nargs; i++) {
    struct symbol *s = lookup(sl->sym);

    s->value = oldval[i];
    sl = sl->next;
  }
  /*
  for(int j=0; j<nargs; j++){
    free_lost(newval[j]);
  }
  */
  free(newval);
  /*
  for(int j=0; j<nargs; j++){
    free_lost(oldval[j]);
  }
  */
  free(oldval);
	outer_scope();
  return v;
}


void free_lost(struct val * v){
	if(!v){
		return;
	}
	if(!v->aliases){
		return;
	}
  if(v->aliases<=0){
		if(v->next){
				free_lost(v->next);
		}
    free(v);
  }
}



void treefree(struct ast *a){
	if(!a){
		return;
	}
  switch(a->nodetype) {

    /* two subtrees */
  case ':':
  case '+':
  case '-':
  case '*':
  case '/':
  case '|':
  case '&':
  case '1': case '2':  case '3':  case '4':  case '5':  case '6': treefree(a->r); treefree(a->l); break;

  case 'L':
    treefree(a->r);
    treefree(a->l);
    break;

    /* one subtree */

  case 'M': case 'C': case 'F': case 'J':
    treefree(a->l);
    break;

    /* no subtree */
  case 'N': case 'D':
    break;


  case 'n':
    treefree(((struct symref_l *)a)->i);
    break;

  case '#':
    treefree(((struct symasgn_l *)a)->i);
    treefree(((struct symasgn_l *)a)->v);
    break;

  case 'K':
    free_lost(((struct value_val *)a)->v);
    break;

  case '=':
    treefree( ((struct symasgn *)a)->v);
    break;

  case 'd':
    treefree( ((struct symdeclasgn *)a)->v);
    break;

  case 'I': case 'W':
    treefree( ((struct flow *)a)->cond);
    if( ((struct flow *)a)->tl) treefree( ((struct flow *)a)->tl);
    if( ((struct flow *)a)->el) treefree( ((struct flow *)a)->el);
    break;

  case 'f':
    if( ((struct foreach *)a)->list) treefree( ((struct foreach *)a)->list);
    if( ((struct foreach *)a)->l) treefree( ((struct foreach *)a)->l);
    break;

  default: printf("internal error: free bad node %c\n", a->nodetype);
  }

  free(a); /* always free the node itself */

}

void yyerror(const char *s, ...){
  va_list ap;
  va_start(ap, s);

  fprintf(stderr, "%d: error: ", yylineno);
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
  char * err=malloc(7*sizeof(char));
  strncpy(err,s,6);
  err[6]='\0';
  if(strcmp(err,"syntax")==0){
    free(err);
    exit(1);
  }
}
