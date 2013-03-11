#ifndef ASSIGN_H
#define ASSIGN_H

void
init_dp();

int
edit_distance(const char *s1, int len1, const char *s2, int len2);

int *
munkres(int **weights, int mat_size);
#endif
