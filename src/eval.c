#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "builtin.h"
#include "types.h"
#include "operators.h"
#include "eval.h"

struct val * eval(struct ast *a){
  struct symbol *s;
  struct val *v;
  struct val *temp;
  struct val *temp2;
  char * string;
	v = NULL;

  if(!a) {
    //yyerror("internal error, null eval");
    return NULL;
  }

  switch(a->nodetype) {
		/* static list */
	case 'J':
		a = a->l;
		v=malloc(sizeof(struct val));
		temp=v;
		v->type='l';
		v->next = NULL;
		v->aliases=0;
		if(!a || !a->l){
			return v;
		}
		do {
			temp->next=(a->nodetype=='L' ? eval(a->l) : eval(a));
			if(typeof_v(temp->next)=='l'){
				yyerror("Cannot have nested lists");
				temp->next=NULL;
				return v;
			}   
			temp=temp->next;
			temp->aliases=0;

		} while(a->nodetype=='L' && (a=a->r));
		break;

    /* constant */
  case 'K': v = valuedup(((struct value_val *)a)->v); break;

		/* list element ref */
	case 'n':
		s = lookup(((struct symref_l *)a)->s);
    v=s->value;
    temp=eval(((struct symref_l *)a)->i);
    if(typeof_s(s) == 'u'){
      yyerror("variable %s uninstantiated.", s->name);
    } else if(typeof_s(s)!='l'){
      yyerror("variable is not a list, found %c", typeof_s(s));
    }else{
      if(typeof_v(temp)=='i' && temp->int_val>=0){
        v = s->value;
        for(int i = temp->int_val; i > 0 && v->next != NULL; i--){
          v = v->next; /* auxiliary var */
        }
        if(v->next == NULL){
          yyerror("Index out of bounds");
          free_lost(temp);
          return NULL;
        }
        v=valuedup(v->next);
				v->next = NULL;
      } else {
        string = toString(temp);
        yyerror("index is not a positive integer, found %c=%s", typeof_v(temp), string);
        free(string);
      }
    }
    break;
    /* name reference */
	case 'N':
		s = lookup(((struct symref *)a)->s);
		if(!s){
			yyerror("variable %s not found.",((struct symref *)a)->s);
		}
		if(typeof_s(s) == 'u'){
			yyerror("variable %s uninstantiated.", s->name);
			v = s->value; break;
		} else {
			v = s->value; break;
		}

  /* name declaration */
  case 'D':
			s = insert_symbol(((struct symdecl *)a)->s);
      if(s->value->type != 'u'){
		     yyerror("%s already has type '%c'", s->name,
                                             s->value->type);
	    } else {
			   s->value->type = ((struct symdecl *)a)->type ;
	    }

      break;

  case 'd':
			s = insert_symbol(((struct symdeclasgn *)a)->s);
			if(!s){
				 return NULL;
			}
      if(s->value->type != 'u'){
		     yyerror("%s already has type '%c'", s->name, s->value->type);
				 return NULL;
			}
			v=eval(((struct symdeclasgn *)a)->v);
			if(!v){
				yyerror("Cannot assign NULL value");
				return NULL;
			}
			if(typeof_v(v)!=((struct symdeclasgn *)a)->type){
				yyerror("assignement error for incompatible types (%c=%c)", typeof_s(s), typeof_v(v) );
				return NULL;
			}
			if(typeof_v(v)=='l'){ /* If v is a list sets the aliases for the other items*/
				v=listdup(v);
				temp=v;
				while((temp=temp->next)){
					temp->aliases++;
				}
				temp=s->value;
				while((temp=temp->next)){
					temp->aliases--;
				}
			}
			v->aliases++;
			s->value->aliases--;
			free_lost(s->value);
			s->value = v;
			break;


    /* assignment */
  case '=':
      v=eval(((struct symasgn *)a)->v);
			s = lookup(((struct symasgn *)a)->s);
			if(typeof_s(s) == 'u'){
				yyerror("variable %s uninstantiated.", ((struct symasgn *)a)->s);
			} else if(typeof_v(v)==typeof_s(s)){
        v->aliases++;
        if(typeof_v(v)=='l'){
          temp=v;
          while((temp=temp->next)){
            temp->aliases++;
          }
          temp=s->value;
          while((temp=temp->next)){
            temp->aliases--;
          }
        }

        s->value->aliases--;
        free_lost(s->value);
        s->value = v;

      }else{
        yyerror("assignement error for incompatible types (%c=%c)", typeof_s(s), typeof_v(v) );
      }
      break;

  case '#':
			s = lookup(((struct symasgn_l *)a)->s);
      temp=eval(((struct symasgn_l *)a)->v);
      temp2=eval(((struct symasgn_l *)a)->i);
			if(!temp || !temp2){
				yyerror("Cannot assign null value");
				return NULL;
			}
			if(typeof_s(s) == 'u'){
				yyerror("variable %s uninstantiated.", ((struct symasgn_l *)a)->s);
			} else if(typeof_v(temp) == 'l'){
				yyerror("Cannot have nested lists");
			} else if(typeof_s(s)!='l'){
        yyerror("variable is not a list, found %c", typeof_s(s));
      }else{
        if(typeof_v(temp2)=='i' && temp2->int_val>=0){
          v = s->value;
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
		s = insert_symbol(((struct foreach *)a)->i);
    if(!s){
      yyerror("Variable %s already declared", s->name);
      return NULL;
    }

    if( ((struct foreach *)a)->l) {
      temp=(eval( ((struct foreach *)a)->list));
      if(!temp || typeof_v(temp)!='l'){
        yyerror("No valid list specified");
        return NULL;
      }
      temp2=temp;

      while( (temp=temp->next) ){
        s->value=valuedup(temp);
				s->value->next = NULL;
				s->value->aliases++;
	      v = eval(((struct foreach *)a)->l);
				s->value->aliases--;
      }
			//s->value->aliases++;	/* the remove_symbol will decrement the aliases */
			remove_symbol(s);
    }
    break;			/* last value is value */


  case 'L': temp=eval(a->l); v = eval(a->r); free_lost(temp);  break;

  case 'F': v = callbuiltin((struct fncall *)a); break;

  case 'C': v = calluser((struct ufncall *)a); break;

  default: printf("internal error: bad node %c\n", a->nodetype);
  }
  return v;
}
