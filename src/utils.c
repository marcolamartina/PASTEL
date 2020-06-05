#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <arpa/inet.h>
#include "utils.h"
int yyparse();

extern int file_mod;


/* symbol table */
/* hash a symbol */
static void remove_symbol(struct symbol * s);
static void start_connection(struct val * v);
static void close_connection(struct val * v);
static void receive_from_connection(struct val * device, struct val * string);
static void send_to_connection(struct val * device, struct val * string);
static void list_append(struct val * list, struct val * value);

static unsigned symhash(char *sym)
{
  unsigned int hash = 0;
  unsigned c;

  while((c = *sym++)) hash = hash*9 ^ c;

  return hash;
}

void remove_symbol(struct symbol * s){
  struct symbol *sp = &symtab[symhash(s->name)%NHASH];
  int scount = NHASH;		/* how many have we looked at */

  while(--scount >= 0) {
    if(sp->name && !strcmp(sp->name, s->name)) {
			sp->value->aliases--;
		//	free_lost(sp->value);
			sp->name=NULL;
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
  struct symbol *sp = &symtab[symhash(sym)%NHASH];
  int scount = NHASH;		/* how many have we looked at */

  while(--scount >= 0) {
    if(sp->name && !strcmp(sp->name, sym)) { return sp; }

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

struct ast * newref_l(struct symbol *s, struct ast *i){
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

struct ast * newdeclasgn(struct symbol *s, char type, struct ast *v){
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

struct ast * newasgn_l(struct symbol *s, struct ast *i, struct ast *v){
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

struct ast * newforeach(int nodetype, struct symbol *i, struct ast *list, struct ast *l){
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
		case 'd': asprintf(&result, "%s:%d - %sconnected", v->string_val, v->port_val, (v->int_val != 0 ? "":"not " )); break;
		case 'u': result=strdup("uninstantiated variable"); break;
    default: yyerror("cannot print a value of type \"%c\"", typeof_v(v)); result=strdup(""); break;
  }
  return result;
}
 		
struct val * valuedup(struct val * v){
	struct val *v2;
	switch (typeof_v(v)){
		case 'i': v2 = new_int(v->int_val); break;
		case 'r': v2 = new_real(v->real_val); break;
		case 's': v2 = new_string(v->string_val); break;
		case 'a': v2 = new_address(v->string_val); break;
		default:
			v2=v;
			break;
	}
	return v2;
}

struct val * sum(struct val * a, struct val * b){
  struct val * result=malloc(sizeof(struct val));
  result->aliases=0;
  result->next=NULL;
  if(typeof_v(a)==typeof_v(b)){
    result->type=typeof_v(a);
    switch (typeof_v(a)) {
      case 'i': result->int_val=a->int_val+b->int_val; break;
      case 's': asprintf(&result->string_val, "%s%s", a->string_val, b->string_val); break;
      case 'r': result->real_val=a->real_val+b->real_val; break;
      default: yyerror("addiction not supported for these types (%c/%c)", typeof_v(a), typeof_v(b)); break;
    }
  }else{
		if(typeof_v(a)=='a' && typeof_v(b)=='i'){
			result->type = 'a';
			int temp ;
			result->string_val = malloc(sizeof(char)*15);
			inet_pton(AF_INET, a->string_val, &temp);
			temp += htonl(b->int_val);
			inet_ntop(AF_INET, &temp, result->string_val, sizeof(char)*15);
		} else {
				yyerror("addiction of incompatible types (%c+%c)", typeof_v(a), typeof_v(b));
		}
  }

  return result;
}

struct val * or_logic(struct val * a, struct val * b){
  struct val * result=malloc(sizeof(struct val));
  result->aliases=0;
  result->next=NULL;
  if(typeof_v(a)=='i' && typeof_v(b)=='i' ){
    result->type=typeof_v(a);
    result->int_val=a->int_val || b->int_val;
  }else{
    yyerror("logic or is not supported for types (%c || %c)", typeof_v(a), typeof_v(b));
  }
  return result;
}


struct val * and_logic(struct val * a, struct val * b){
  struct val * result=malloc(sizeof(struct val));
  result->aliases=0;
  result->next=NULL;
  if(typeof_v(a)=='i' && typeof_v(b)=='i' ){
    result->type=typeof_v(a);
    result->int_val=a->int_val && b->int_val;
  }else{
    yyerror("logic and is not supported for types (%c && %c)", typeof_v(a), typeof_v(b));
  }
  return result;
}


struct val * sub(struct val * a, struct val * b){
  struct val * result=malloc(sizeof(struct val));
  result->aliases=0;
  result->next=NULL;
  if(typeof_v(a)==typeof_v(b)){
    result->type=typeof_v(a);
    switch (typeof_v(a)) {
      case 'i': result->int_val=a->int_val-b->int_val; break;
      case 'r': result->real_val=a->real_val-b->real_val; break;
      default: yyerror("subtraction not supported for these types (%c/%c)", typeof_v(a), typeof_v(b)); break;
    }
  }else{
		if(typeof_v(a)=='a' && typeof_v(b)=='i'){
			result->type = 'a';
			int temp ;
			result->string_val = malloc(sizeof(char)*15);

			inet_pton(AF_INET, a->string_val, &temp);
			temp -= htonl(b->int_val);
			inet_ntop(AF_INET, &temp, result->string_val, sizeof(char)*15);
		} else {
    	yyerror("subtraction of incompatible types (%c-%c)", typeof_v(a), typeof_v(b));
		}
  }
  return result;
}

struct val * mul(struct val * a, struct val * b){
  struct val * result=malloc(sizeof(struct val));
  result->aliases=0;
  result->next=NULL;
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
  struct val * result=malloc(sizeof(struct val));
  result->aliases=0;
  result->next=NULL;
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

struct val * new_list(struct ast * list){
  struct val * result=malloc(sizeof(struct val));
  struct val * temp=result;
	result->next = NULL;
  result->aliases=0;
  result->type='l';
  if(!list || !list->l){
    return result;
  }
  do {
    temp->next=list->nodetype=='L' ? eval(list->l) : eval(list);
    if(typeof_v(temp->next)=='l'){
      yyerror("Cannot have nested lists");
      temp->next=NULL;
      return result;
    }
    temp=temp->next;
    temp->aliases=0;

  } while(list->nodetype=='L' && (list=list->r));
  return result;
}

struct val * new_real(double a){
  struct val * result=malloc(sizeof(struct val));
	result->next = NULL;
  result->aliases=0;
  result->type='r';
  result->real_val=a;
  return result;
}

struct val * new_int(int a){
  struct val * result=malloc(sizeof(struct val));
	result->next = NULL;
  result->aliases=0;
  result->type='i';
  result->int_val=a;
  return result;
}

struct val * new_string(char * a){
  struct val * result=malloc(sizeof(struct val));
	result->next = NULL;
  result->aliases=0;
  result->type='s';
  result->string_val=strdup(a);
  return result;
}

struct val * new_address(char * a){
  struct val * result=malloc(sizeof(struct val));
	result->next = NULL;
  result->aliases=0;
  result->type='a';
  result->string_val=strdup(a);
  return result;
}

struct val * new_device(struct val * addr, struct val * port){
  struct val * result=malloc(sizeof(struct val));
	if(typeof_v(addr) != 'a' || typeof_v(port) != 'i'){
		yyerror("Wrong types for address/port");
		return NULL;
	}
	result->next = NULL;
  result->aliases=0;
  result->type='d';
  result->string_val=strdup(addr->string_val);
  if(port->int_val>0 && port->int_val<((2<<16)-1)){
    result->port_val = port->int_val;
  } else {
    yyerror("invalid port number, found %d", port->int_val);
  }
	result->int_val = 0;
  return result;
}

struct val * change_sign(struct val * a){
  struct val * result=malloc(sizeof(struct val));
	result->next = NULL;
  result->aliases=0;
  result->type=typeof_v(a);
  switch (typeof_v(a)) {
    case 'i': result->int_val=-(a->int_val);
              break;
    case 'r': result->real_val=-(a->real_val);
              break;
    default: yyerror("change of sign not supported for this types (%c)", typeof_v(a)); break;
  }
  return result;
}

double compare(struct val * a, struct val * b){
  double result;
  if(typeof_v(a)==typeof_v(b)){
    switch (typeof_v(a)) {
      case 'i': result=a->int_val-b->int_val; break;
      case 'r': result=a->real_val-b->real_val; break;
      case 's': result=strcmp(a->string_val,b->string_val); break;
      default: yyerror("comparison not supported for these types (%c/%c)", typeof_v(a), typeof_v(b)); break;
    }
  }else if(typeof_v(a)=='i'&&typeof_v(b)=='r'){
    result=a->int_val-b->real_val;
  }else if(typeof_v(a)=='r'&&typeof_v(b)=='i'){
    result=a->real_val-b->int_val;
  }else{
    yyerror("comparison of incompatible types (%c-%c)", typeof_v(a), typeof_v(b));
  }
  return result;
}

struct val * eval(struct ast *a){
  struct val *v;
  struct val *temp;
  struct val *temp2;
  char * string;
	v = NULL;

  if(!a) {
    yyerror("internal error, null eval");
    return NULL;
  }

  switch(a->nodetype) {
    /* constant */
  case 'K': v = valuedup(((struct value_val *)a)->v); break;

    /* name reference */
	case 'n':
    v=((struct symref_l *)a)->s->value;
    temp=eval(((struct symref_l *)a)->i);
    if(typeof_s(((struct symref_l *)a)->s) == 'u'){
      yyerror("variable %s uninstantiated.", ((struct symref_l *)a)->s->name);
    } else if(typeof_s(((struct symref_l *)a)->s)!='l'){
      yyerror("variable is not a list, found %c", typeof_s(((struct symref_l *)a)->s));
    }else{
      if(typeof_v(temp)=='i' && temp->int_val>=0){
        v = ((struct symref_l *) a)->s->value;
        for(int i = temp->int_val; i > 0 && v->next != NULL; i--){
          v = v->next; /* auxiliary var */
        }
        if(v->next == NULL){
          yyerror("Index out of bounds");
          free_lost(temp);
          return NULL;
        }
        v=v->next;
      } else {
        string = toString(temp);
        yyerror("index is not a positive integer, found %c=%s", typeof_v(temp), string);
        free(string);
      }
    }
    break;
	case 'N':
		if(typeof_s(((struct symasgn *)a)->s) == 'u'){
			yyerror("variable %s uninstantiated.", ((struct symasgn *)a)->s->name);
			v = ((struct symref *)a)->s->value; break;
		} else {
			v = ((struct symref *)a)->s->value; break;
		}

  /* name declaration */
  case 'D':
      if(((struct symdecl *)a)->s->value->type != 'u'){
		     yyerror("%s already has type '%c'", ((struct symdecl *)a)->s->name,
                                             ((struct symdecl *)a)->s->value->type);
	    } else {
			   ((struct symdecl *)a)->s->value->type = ((struct symdecl *)a)->type ;
	    }

      break;

  case 'd':
      if(((struct symdeclasgn *)a)->s->value->type != 'u'){
		     yyerror("%s already has type '%c'", ((struct symdeclasgn *)a)->s->name,
                                             ((struct symdeclasgn *)a)->s->value->type);
	    } else {
        v=eval(((struct symdeclasgn *)a)->v);
				if(!v){
					yyerror("Cannot assign NULL value");
					return NULL;
				}
        if(typeof_v(v)==((struct symdeclasgn *)a)->type){
          v->aliases++;
          ((struct symdeclasgn *)a)->s->value->aliases--;
          free_lost(((struct symdeclasgn *)a)->s->value);
          ((struct symdeclasgn *)a)->s->value = v;
        }else{
           yyerror("assignement error for incompatible types (%c=%c)", typeof_s(((struct symdeclasgn *)a)->s), typeof_v(v) );
         }
	    }

      break;


    /* assignment */
  case '=':
      v=eval(((struct symasgn *)a)->v);
			if(typeof_s(((struct symasgn *)a)->s) == 'u'){
				yyerror("variable %s uninstantiated.", ((struct symasgn *)a)->s->name);
			} else if(typeof_v(v)==typeof_s(((struct symasgn *)a)->s)){
        v->aliases++;
        temp=v;
        while((temp=temp->next)){
          temp->aliases++;
        }
        ((struct symasgn *)a)->s->value->aliases--;
        free_lost(((struct symasgn *)a)->s->value);
        ((struct symasgn *)a)->s->value = v;
      }else{
        yyerror("assignement error for incompatible types (%c=%c)", typeof_s(((struct symasgn *)a)->s), typeof_v(v) );
      }
      break;

  case '#':
      temp=eval(((struct symasgn_l *)a)->v);
      temp2=eval(((struct symasgn_l *)a)->i);
			if(typeof_s(((struct symasgn_l *)a)->s) == 'u'){
				yyerror("variable %s uninstantiated.", ((struct symasgn_l *)a)->s->name);
			} else if(typeof_v(temp) == 'l'){
				yyerror("Cannot have nested lists");
			} else if(typeof_s(((struct symasgn_l *)a)->s)!='l'){
        yyerror("variable is not a list, found %c", typeof_s(((struct symasgn_l *)a)->s));
      }else{
        if(typeof_v(temp2)=='i' && temp2->int_val>=0){
          v = ((struct symasgn_l *) a)->s->value;
          for(int i = temp2->int_val; i > 0 && v->next != NULL; i--){
        		v = v->next; /* auxiliary var */
        	}
          if(v->next == NULL){
            yyerror("Index out of bounds");
            free_lost(temp);
            return NULL;
          }
          temp->aliases++;
          v->next->aliases--;
          temp->next=v->next->next;
          free_lost(v->next);
          v->next = temp;
        } else {
          string = toString(temp2);
          yyerror("index is not a positive integer, found %c=%s", typeof_v(temp2), string);
          free(string);
        }
      }
      break;

    /* expressions */
  case ':': v = new_device(eval(a->l),eval(a->r)); break;
  case '+': v = sum(eval(a->l),eval(a->r)); break;
  case '-': v = sub(eval(a->l),eval(a->r)); break;
  case '*': v = mul(eval(a->l),eval(a->r)); break;
  case '/': v = division(eval(a->l),eval(a->r)); break;
  case 'M': v = change_sign(eval(a->l)); break;

  /* logic operations */
  case '&': v = and_logic(eval(a->l),eval(a->r)); break;
  case '|': v = or_logic(eval(a->l),eval(a->r)); break;

    /* comparisons */
  case '1': v = new_int(compare(eval(a->l), eval(a->r))>0); break;
  case '2': v = new_int(compare(eval(a->l), eval(a->r))<0); break;
  case '3': v = new_int(compare(eval(a->l), eval(a->r))!=0); break;
  case '4': v = new_int(compare(eval(a->l), eval(a->r))==0); break;
  case '5': v = new_int(compare(eval(a->l), eval(a->r))>=0); break;
  case '6': v = new_int(compare(eval(a->l), eval(a->r))<=0); break;



  /* control flow */
  /* null if/else/do expressions allowed in the grammar, so check for them */
  case 'I':
    temp=(eval( ((struct flow *)a)->cond));
    if( temp->int_val != 0) {
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
    free_lost(temp);
    break;

  case 'W':
    v = NULL;		/* a default value */

    if( ((struct flow *)a)->tl) {
      temp=(eval( ((struct flow *)a)->cond));
      while( temp->int_val != 0){
        free_lost(temp);
	      v = eval(((struct flow *)a)->tl);
        temp=(eval( ((struct flow *)a)->cond));
      }
      free_lost(temp);
    }
    break;			/* last value is value */

  case 'f':
    v = NULL;		/* a default value */

    if( ((struct foreach *)a)->l) {
      temp=(eval( ((struct foreach *)a)->list));
      if(!temp || typeof_v(temp)!='l'){
        yyerror("No valid list specified");
        return NULL;
      }
      if(typeof_s(((struct foreach *)a)->i)!='u'){
        yyerror("Variable %s already declared", ((struct foreach *)a)->i->name);
        return NULL;
      }
      temp2=temp;

      while( (temp=temp->next) ){
        ((struct foreach *)a)->i->value=temp;
				temp->aliases++;
	      v = eval(((struct foreach *)a)->l);
				temp->aliases--;
      }
			((struct foreach *)a)->i->value->aliases++;	/* the remove_symbol will decrement the aliases */
			remove_symbol(((struct foreach *)a)->i);
    }
    break;			/* last value is value */


  case 'L': temp=eval(a->l); v = eval(a->r); free_lost(temp);  break;

  case 'F': v=callbuiltin((struct fncall *)a); break;

  default: printf("internal error: bad node %c\n", a->nodetype);
  }
  return v;
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


struct val * callbuiltin(struct fncall *f) {
  struct val * result=NULL;
  enum bifs functype = f->functype;
  struct val * v = eval(f->l);
  struct val * v_temp;
  char * temp;
  int num_arg=arg_len(f->l);

  switch(functype) {
  case B_print:
    if (num_arg != 1) {
      yyerror("Wrong argument number, expected 1, found %d", num_arg);
    } else{
      temp=toString(v);
      printf("%s\n", temp);
      free(temp);
    }
    break;
	case B_connect:
    if (num_arg != 1) {
      yyerror("Wrong argument number, expected 1, found %d", num_arg);
    } else{
      start_connection(v);
    }
    break;
	case B_disconnect:
  if (num_arg != 1) {
    yyerror("Wrong argument number, expected 1, found %d", num_arg);
  } else{
    close_connection(v);
  }
  break;
	case B_receive:
    if (num_arg != 2) {
      yyerror("Wrong argument number, expected 2, found %d", num_arg);
    } else{
      v_temp=eval(f->l->l);
      receive_from_connection(v_temp,v);
      free_lost(v_temp);
    }
    break;

	case B_send:
    if (num_arg != 2) {
      yyerror("Wrong argument number, expected 2, found %d", num_arg);
    } else{
      v_temp=eval(f->l->l);
      send_to_connection(v_temp,v);
      free_lost(v_temp);
    }
    break;
  case B_append:
    if (num_arg != 2) {
      yyerror("Wrong argument number, expected 2, found %d", num_arg);
    } else{
      v_temp=eval(f->l->l);
      list_append(v_temp,v);
      free_lost(v_temp);
    }
    break;

  case B_length:
    if (num_arg != 1) {
      yyerror("Wrong argument number, expected 1, found %d", num_arg);
    } else{
      result=new_int(length(v));
    }
    break;
  default:
    yyerror("Unknown built-in function %d", functype);
  }
  return result;
}

void free_lost(struct val * v){
	if(!v){
		return;
	}
  if(v->aliases<=0){
		if(v->next){
				free_lost(v->next);
		}
    free(v);
  }
}

void start_connection(struct val * v){
	if(typeof_v(v) != 'd'){
		yyerror("Cannot connect to something that is not a device!");
		return;
	}
	if(v->int_val != 0){
		yyerror("There is already an active connection to the selected device");
		return;
	}
	struct sockaddr_in device_addr;
	int sock ;

	if((sock = socket(AF_INET, SOCK_STREAM, 0))<0){
		yyerror("Socket creation error");
		return;
	}

	device_addr.sin_family = AF_INET;
	device_addr.sin_port = htons(v->port_val);

	if(inet_pton(AF_INET, (!strcmp(v->string_val,"localhost")?"127.0.0.1":v->string_val), &device_addr.sin_addr)<=0)
	{
			yyerror("Invalid address/ Address not supported");
			return ;
	}

	if (connect(sock, (struct sockaddr *)&device_addr, sizeof(device_addr)) < 0)
	{
			yyerror("Connection Failed");
			return ;
	}

	v->int_val = sock;
}

void close_connection(struct val * v){
	if(typeof_v(v) != 'd'){
		yyerror("Cannot disconnect from something that is not a device!");
		return;
	}
	if(v->int_val == 0){
		yyerror("There is no active connection to the selected device");
		return;
	}

	close(v->int_val);
	v->int_val=0;

}

void send_to_connection(struct val * device, struct val * string){
	if(typeof_v(device) != 'd'){
		yyerror("Cannot write to something that is not a device!");
		return;
	}
	if(typeof_v(string) != 's'){
		yyerror("Can't write to something that is not a string");
		return;
	}
	if(device->int_val == 0){
		yyerror("There is no active connection to the selected device");
		return;
	}

	if(! string->string_val){
		yyerror("Cannot send an uninstantiated string");
		return;
	}
		send(device->int_val, string->string_val, sizeof(char)*strlen(string->string_val), 0);
}

void receive_from_connection(struct val * device, struct val * string){
	if(typeof_v(device) != 'd'){
		yyerror("Cannot receive from something that is not a device!");
		return;
	}
	if(device->int_val == 0){
		yyerror("There is no active connection to the selected device");
		return;
	}

	if(string->string_val){
		free(string->string_val);
	}

	if((string->string_val = calloc(1024,sizeof(char)))){
		recv(device->int_val, string->string_val, sizeof(char)*1024, 0);
	} else {
		yyerror("Out of space");
		exit(1);
	}
}

static void list_append(struct val * list, struct val * value){
  if(typeof_v(list)!='l'){
    yyerror("Function append is defined only for list, not for %c", typeof_v(list));
  } else if(typeof_v(value)=='l'){
    yyerror("Cannot have nested lists");
  } else {
    value->next=list->next;
    list->next=value;
    value->aliases++;
  }
}


void treefree(struct ast *a){
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
    //treefree(a->l);
    break;

    /* one subtree */

  case 'M': case 'C': case 'F':
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
  //exit(1);
}

int main(int argc, char **argv){
	extern FILE * yyin;
  file_mod=0;
	for(int i = 1; i<argc; i++){
		if((strcmp(argv[1]+strlen(argv[1])-3,".pa")!=0)){
      fprintf(stderr, "Insert a .pa file");
      return 1;
    }else{
      file_mod++;
      yyin=fopen(argv[i], "r");
    }
		if (!yyin){
				yyerror("Error on opening source file");
				return 1;
		}
		yyrestart(yyin);
		yylineno = 1;
		yyparse();
		fclose(yyin);
    file_mod--;
	}
	yyin=stdin;
	yyrestart(yyin);
	yylineno = 1;
  printf("%s", file_mod ? "" : "> ");
  return yyparse();
}

/* debugging: dump out an AST */
int debug = 0;

void dumpast(struct ast *a, int level){
  char * string;
  printf("%*s", 2*level, "");	/* indent to this level */
  level++;

  if(!a) {
    printf("NULL\n");
    return;
  }

  switch(a->nodetype) {
    /* constant */
  case 'K': string=toString(((struct value_val *)a)->v);
    printf("value %s\n", string); free(string); break;

    /* name reference */
  case 'N': string=toString(((struct symref *)a)->s->value);
            printf("ref %s", ((struct symref *)a)->s->name);
            if(((struct symref *)a)->s->value){
              printf("=%s\n", string );
              free(string);
            }else{
              printf("\n");
            }

            break;

  /* name declaration */
  case 'D': printf("decl %s\n", ((struct symref *)a)->s->name); break;

    /* assignment */
  case '=': printf("= %s\n", ((struct symasgn *)a)->s->name);
    dumpast( ((struct symasgn *)a)->v, level); break;

  case 'd': printf("= %s\n", ((struct symdeclasgn *)a)->s->name);
    dumpast( ((struct symdeclasgn *)a)->v, level); break;

    /* expressions */
  case '|': case '&':
  case '+': case '-': case '*': case '/': case 'L':
  case ':':
  case '1': case '2': case '3':
  case '4': case '5': case '6':
    printf("binop %c\n", a->nodetype);
    dumpast(a->l, level);
    dumpast(a->r, level);
    break;

  case 'M':
    printf("unop %c\n", a->nodetype);
    dumpast(a->l, level);
    break;

  case 'I': case 'W':
    printf("flow %c\n", a->nodetype);
    dumpast( ((struct flow *)a)->cond, level);
    if( ((struct flow *)a)->tl)
      dumpast( ((struct flow *)a)->tl, level);
    if( ((struct flow *)a)->el)
      dumpast( ((struct flow *)a)->el, level);
    break;

  case 'f':
    printf("foreach %c\n", a->nodetype);
    if( ((struct foreach *)a)->list)
      dumpast( ((struct foreach *)a)->list, level);
    if( ((struct foreach *)a)->l)
      dumpast( ((struct foreach *)a)->l, level);
    break;

  case 'F':
    printf("builtin %d\n", ((struct fncall *)a)->functype);
    dumpast(a->l, level);
    break;

  case '#':
    printf("%s[index]=\n", ((struct symasgn_l *)a)->s->name);
    printf("%*s", 2*level, "");
    printf("index =\n");
    dumpast(((struct symasgn_l *)a)->i, level+1);
    dumpast(((struct symasgn_l *)a)->v, level);


    break;

  case 'n':
    printf("%s[index]\n", ((struct symref_l *)a)->s->name);
    printf("%*s", 2*level, "");
    printf("index =\n");
    dumpast(((struct symref_l *)a)->i, level+1);
    break;


  default: printf("bad %c\n", a->nodetype);
    break;
  }

}
