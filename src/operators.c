#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include "utils.h"
#include "types.h"
#include "operators.h"

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
      case 'l':
        result=valuedup(a);
        if(length(result)>0){
          (get_element(result,length(result)-1))->next=listdup(b)->next;
        } else {
          result->next=listdup(b)->next;
        }
        result->next=a->next;
        break;
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
      case 'a': result=strcmp(a->string_val,b->string_val); break;
      case 'd': result=(strcmp(a->string_val,b->string_val) || (a->port_val - b->port_val)); break;
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
