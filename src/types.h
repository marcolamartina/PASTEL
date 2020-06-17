#ifndef TYPES_H
#define TYPES_H

/* types
'u': undefined
'i': int
'r': real
's': string
'a': address
'l': list
'd': device
*/

struct val {
  char type;
	struct val * next;		/* next element if list */
  char* string_val;
  int int_val;
  double real_val;
  unsigned short port_val;
  size_t aliases;
};


struct val * new_real(double a);
struct val * new_int(int a);
struct val * new_device(struct val * addr, struct val * port);
struct val * new_string(char * a);
struct val * new_address(char * a);
struct ast * new_list(struct ast * list);

#endif /* TYPES_H */
