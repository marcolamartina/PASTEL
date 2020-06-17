
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

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
	case 'J': printf("list "); break;
    /* constant */
  case 'K': string=toString(((struct value_val *)a)->v);
    printf("value %s\n", string); free(string); break;

    /* name reference */
  case 'N': string=toString((lookup(((struct symref *)a)->s))->value);
            printf("ref %s", (lookup(((struct symref *)a)->s))->name);
            if((lookup(((struct symref *)a)->s))->value){
              printf("=%s\n", string );
              free(string);
            }else{
              printf("\n");
            }

            break;

  /* name declaration */
  case 'D': printf("decl %s\n", ((struct symref *)a)->s); break;

    /* assignment */
  case '=': printf("= %s\n", ((struct symasgn *)a)->s);
    dumpast( ((struct symasgn *)a)->v, level); break;

  case 'd': printf("= %s\n", ((struct symdeclasgn *)a)->s);
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
    printf("%s[index]=\n", ((struct symasgn_l *)a)->s);
    printf("%*s", 2*level, "");
    printf("index =\n");
    dumpast(((struct symasgn_l *)a)->i, level+1);
    dumpast(((struct symasgn_l *)a)->v, level);
    break;

  case 'n':
    printf("%s[index]\n", ((struct symref *)a)->s);
    printf("%*s", 2*level, "");
    printf("index =\n");
    dumpast(((struct symref_l *)a)->i, level+1);
    break;

  case 'C':
    printf("call %s\n", ((struct ufncall *)a)->s);
    dumpast(a->l, level);
    break;

  default: printf("bad %c\n", a->nodetype);
    break;
  }
}
