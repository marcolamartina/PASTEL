#ifndef OPERATORS_H
#define OPERATORS_H

struct val * sum(struct val * a, struct val * b);
struct val * sub(struct val * a, struct val * b);
struct val * mul(struct val * a, struct val * b);
struct val * division(struct val * a, struct val * b);
struct val * change_sign(struct val * a);
struct val * and_logic(struct val * a, struct val * b);
struct val * or_logic(struct val * a, struct val * b);
double compare(struct val * a, struct val * b);

#endif /* OPERATORS_H */
