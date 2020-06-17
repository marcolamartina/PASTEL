#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"
#include "builtin.h"
#include "types.h"

int yyparse();

#if defined(__linux__)
    #define OS 2
#elif defined(__APPLE__) && defined(__MACH__)
    #define OS 1
#else
    #define OS 0
#endif

static void start_connection(struct val * v);
static void close_connection(struct val * v);
static struct val * receive_from_connection(struct val * device);
static void send_to_connection(struct val * device, struct val * string);
static void list_remove(struct val * list, struct val * index);
static void list_insert(struct val * list, struct val * value, struct val * index);
static struct val * s2a(struct val * v);
static struct val * s2d(struct val * v);
static struct val * s2r(struct val * v);
static struct val * s2i(struct val * v);
static void load_file(struct val * file);
static void quit(struct val * arg);
static struct val * split_string(struct val * string, struct val * token);
static struct val * strip_string(struct val * string);
static struct val * get_port(struct val * v);
static struct val * get_address(struct val * v);
static void open_terminal(struct val * device);

struct val * callbuiltin(struct fncall *f) {
  struct val * result=NULL;
  enum bifs functype = f->functype;
  struct val * v = eval(f->l);
  struct val * v_temp;
  struct val * v_temp2;
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
	case B_quit:
    if (num_arg != 1) {
      yyerror("Wrong argument number, expected 1, found %d", num_arg);
    } else{
			quit(v);
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
    if (num_arg != 1) {
      yyerror("Wrong argument number, expected 1, found %d", num_arg);
    } else{
      result=receive_from_connection(v);
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
  case B_insert:
    if (num_arg != 3) {
      yyerror("Wrong argument number, expected 3, found %d", num_arg);
    } else{
      v_temp2=eval(f->l->r->l);
      v_temp=eval(f->l->l);
      list_insert(v_temp, v_temp2, v);
      free_lost(v_temp);
      free_lost(v_temp2);
    }
    break;

  case B_remove:
    if (num_arg != 2) {
      yyerror("Wrong argument number, expected 2, found %d", num_arg);
    } else{
      v_temp=eval(f->l->l);
      list_remove(v_temp,v);
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

  case B_port:
    if (num_arg != 1) {
      yyerror("Wrong argument number, expected 1, found %d", num_arg);
    } else{
      result=get_port(v);
    }
    break;
	case B_address:
  if (num_arg != 1) {
    yyerror("Wrong argument number, expected 1, found %d", num_arg);
  } else{
    result=get_address(v);
  }
  break;
  case B_s2i:
    if (num_arg != 1) {
      yyerror("Wrong argument number, expected 1, found %d", num_arg);
    } else{
      result=s2i(v);
    }
    break;
  case B_s2r:
    if (num_arg != 1) {
      yyerror("Wrong argument number, expected 1, found %d", num_arg);
    } else{
      result=s2r(v);
    }
    break;
  case B_s2d:
    if (num_arg != 1) {
      yyerror("Wrong argument number, expected 1, found %d", num_arg);
    } else{
      result=s2d(v);
    }
    break;
  case B_s2a:
    if (num_arg != 1) {
      yyerror("Wrong argument number, expected 1, found %d", num_arg);
    } else{
      result=s2a(v);
    }
    break;
  case B_toString:
    if (num_arg != 1) {
      yyerror("Wrong argument number, expected 1, found %d", num_arg);
    } else{
      temp=toString(v);
      result=new_string(temp);
      free(temp);
    }
    break;

  case B_console:
    if (num_arg != 1) {
      yyerror("Wrong argument number, expected 1, found %d", num_arg);
    } else{
      open_terminal(v);
    }
    break;
  case B_load:
    if (num_arg != 1) {
      yyerror("Wrong argument number, expected 1, found %d", num_arg);
    } else{
      load_file(v);
    }
    break;
  case B_strip:
    if (num_arg != 1) {
      yyerror("Wrong argument number, expected 1, found %d", num_arg);
    } else{
      result=strip_string(v);
    }
    break;
  case B_split:
    if (num_arg != 2) {
      yyerror("Wrong argument number, expected 2, found %d", num_arg);
    } else{
      v_temp=eval(f->l->l);
      result=split_string(v_temp,v);
      free_lost(v_temp);
    }
    break;
  case B_sleep:
    if (num_arg != 1) {
      yyerror("Wrong argument number, expected 1, found %d", num_arg);
    } else{
			if(typeof_v(v) != 'i'){
				yyerror("Wrong argument type, expected int, found %c", typeof_v(v));
				return NULL;
			}
			sleep(v->int_val);
    }
    break;

  case B_typeof:
    if (num_arg != 1) {
      yyerror("Wrong argument number, expected 1, found %d", num_arg);
    } else{
			switch(typeof_v(v)){
        case 'u': result=new_string("undefined"); break;
        case 'i': result=new_string("int"); break;
        case 'r': result=new_string("real"); break;
        case 's': result=new_string("string"); break;
        case 'a': result=new_string("address"); break;
        case 'l': result=new_string("list"); break;
        case 'd': result=new_string("device"); break;
        default: result=new_string("unrecognized"); break;
			}
    }
    break;
  case B_isConnected:
    if (num_arg != 1) {
      yyerror("Wrong argument number, expected 1, found %d", num_arg);
    } else{
			if(typeof_v(v) != 'd'){
				yyerror("Wrong argument type, expected device, found %c", typeof_v(v));
				return NULL;
			}
			result=new_int(v->int_val!=0);
    }
    break;
  default:
    yyerror("Unknown built-in function %d", functype);
  }
  return result;
}

void open_terminal(struct val * device){
  if(typeof_v(device)!='d'){
    yyerror("Cannot open terminal");
    return;
  }
  if(strcmp(device->string_val, "localhost") && strcmp(device->string_val, "127.0.0.1")){
    yyerror("Cannot open terminal");
    return;
  }

  char * string;
  char * title;

  asprintf(&title, "%s:%hu", device->string_val, device->port_val);

  switch(OS){
    case 1:
      asprintf(&string, "osascript -e \'tell application \"Terminal\" to do script \"clear; echo %s; nc -lk %hu \"\'",  title, device->port_val );
      break;
    case 2:
      asprintf(&string, "gnome-terminal -- sh -c \"clear; echo %s; nc -lk %hu; bash\"", title, device->port_val);
      break;
    default:
      yyerror("Function not supported in this OS");
      return;
  }
  system(string);
  free(string);
  free(title);
  sleep(1);  
}

void quit(struct val * arg){
	if(typeof_v(arg) != 'i'){
			printf("%s\n", toString(arg));
			exit(1);
	} else {
			exit(arg->int_val);
	}
}


struct val * split_string(struct val * string, struct val * token){
  if(typeof_v(string)!='s'){
    yyerror("Cannot split a non-string value, found %c", typeof_v(string));
    return NULL;
  }
  if(typeof_v(token)!='s'){
    yyerror("Cannot split a string with a non-string value token, found %c", typeof_v(token));
    return NULL;
  }

  struct val *v=NULL;
  struct val *result=NULL;
  v=malloc(sizeof(struct val));
  result=v;
  v->aliases=0;
  v->type='l';
  char * temp=strdup(string->string_val);
  char *t;
  t = strtok(temp, token->string_val);
  while( t != NULL ) {
      v->next=new_string(t);
      v=v->next;
      t = strtok(NULL, token->string_val);
   }

  return result;
}

struct val * strip_string(struct val * string){
  if(typeof_v(string)!='s'){
    yyerror("Cannot strip a non-string value, found %c", typeof_v(string));
    return NULL;
  }
  char * s=strdup(string->string_val);
  while(isspace(*s)){
    s++;
  }
  char* back = s + strlen(s);
  while(isspace(*--back)){
    /*do nothing*/
  }
  *(back+1) = '\0';

  return new_string(s);
}

void load_file(struct val * file){
  int temp=yylineno;
  if(typeof_v(file)!='s'){
    yyerror("Wrong filename specified, found value of type %c", typeof_v(file));
    return;
  }
  extern FILE * yyin;
	if((strcmp(file->string_val+strlen(file->string_val)-3,".pa")!=0)){
    yyerror("Insert a .pa file");
    return;
  }else{
    file_mod++;
    yyin=fopen(file->string_val, "r");
  }

	if (!yyin){
			yyerror("Error on opening source file");
			return;
	}
	yyrestart(yyin);
	yylineno = 1;
	yyparse();
	fclose(yyin);
  file_mod--;

	yyin=stdin;
	yyrestart(yyin);
	yylineno = temp;
  printf("%s", file_mod ? "" : "> ");
  yyparse();
}

struct val * s2i(struct val * v){
  char * end;
  int number;
  if(typeof_v(v)!='s'){
    yyerror("s2i is defined only for string, found %c", typeof_v(v));
    return NULL;
  } else {
    number=strtol(v->string_val,&end,10);
    if(strlen(end)){
      yyerror("%s is not a valid number", v->string_val);
      return NULL;
    }
    return new_int(number);
  }
}

struct val * s2d(struct val * v){
  char * ip;
  char * port_s;
  int port;

  if(typeof_v(v)!='s'){
    yyerror("s2d is defined only for string, found %c", typeof_v(v));
    return NULL;
  } else {

    ip = strtok(v->string_val, ":");
    port_s = strtok(NULL, ":");
	if(!port_s){
		yyerror("No port number specified");
		return NULL;
	}
    port = atoi(port_s);
	if(!ip || !port){
		yyerror("Cannot convert %s to a device", v->string_val);
		return NULL;
	}

    struct val * result=malloc(sizeof(struct val));
  	result->next = NULL;
    result->aliases=0;
    result->type='d';
    result->string_val=strdup(ip);
    if(port>0 && port<((2<<16)-1)){
      result->port_val = port;
    } else {
      yyerror("invalid port number, found %d", port);
    }
  	result->int_val = 0;
    return result;

  }
}


struct val * s2r(struct val * v){
  char * end;
  double number;
  if(typeof_v(v)!='s'){
    yyerror("s2r is defined only for string, found %c", typeof_v(v));
    return NULL;
  } else {
    number=strtod(v->string_val,&end);
    if(strlen(end)){
      yyerror("%s is not a valid number", v->string_val);
      return NULL;
    }
    return new_real(number);
  }
}

struct val * s2a(struct val * v){
  if(typeof_v(v)!='s'){
    yyerror("s2a is defined only for string, found %c", typeof_v(v));
    return NULL;
  } else {
    return new_address(v->string_val);
  }
}

struct val * get_port(struct val * v){
  if(typeof_v(v)!='d'){
    yyerror("Attribute port is only for type device, found %c", typeof_v(v));
    return NULL;
  } else {
    return new_int(v->port_val);
  }
}

struct val * get_address(struct val * v){
  if(typeof_v(v)!='d'){
    yyerror("Attribute address is only for type device, found %c", typeof_v(v));
    return NULL;
  } else {
    return new_address(v->string_val);
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

struct val * receive_from_connection(struct val * device){
  struct val * result;
  if(typeof_v(device) != 'd'){
		yyerror("Cannot receive from something that is not a device!");
		return NULL;
	}
	if(device->int_val == 0){
		yyerror("There is no active connection to the selected device");
		return NULL;
	}
  char * string;
	if((string = calloc(1024,sizeof(char)))){
		recv(device->int_val, string, sizeof(char)*1024, 0);
    result = new_string(string);
    free(string);
    return result;
	} else {
		yyerror("Out of space");
		exit(1);
	}
  return NULL;
}

void list_insert(struct val * list, struct val * value, struct val * index){
  struct val * v=valuedup(value);
  struct val * temp;
  if(typeof_v(list)!='l'){
    yyerror("Function insert is defined only for list, not for %c", typeof_v(list));
  } else if(typeof_v(value)=='l'){
    yyerror("Cannot have nested lists");
  } else if(typeof_v(index)!='i' || index->int_val<0){
    yyerror("index is not a positive integer, found %c", typeof_v(index));
  } else if(list && index->int_val>length(list)){
    yyerror("index out of bounds, found index=%d length=%d", index->int_val, length(list));
  } else {
    if(index->int_val==0){
      v->next=list->next;
      list->next=v;
      v->aliases++;
    } else {
      temp=get_element(list,index->int_val-1);
      v->aliases++;
      v->next=temp->next;
      temp->next=v;
    }

  }
}

void list_remove(struct val * list, struct val * index){
  struct val * temp;
  struct val * temp2;
  if(typeof_v(list)!='l'){
    yyerror("Function remove is defined only for list, not for %c", typeof_v(list));
  } else if(typeof_v(index)!='i' || index->int_val<0){
    yyerror("index is not a positive integer, found %c", typeof_v(index));
  } else if(list && index->int_val>=length(list)){
    yyerror("index out of bounds, found index=%d length=%d", index->int_val, length(list));
  } else {
    if(index->int_val==0){
      temp=list->next->next;
      list->next->next=NULL;
      list->next->aliases--;
      free_lost(list->next);
      list->next=temp;
    } else {
      temp=get_element(list,index->int_val-1);
      temp2=get_element(list,index->int_val-1)->next;
      temp->next=temp2->next;
      temp2->next=NULL;
      temp2->aliases--;
      free_lost(temp2);
    }
  }
}
