#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "types.h"



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
  if(port->int_val>0 && port->int_val<((1<<16)-1)){
    result->port_val = port->int_val;
  } else {
    yyerror("invalid port number, found %d", port->int_val);
  }
	result->int_val = 0;
  return result;
}
