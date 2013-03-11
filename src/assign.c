#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "assign.h"
#include "common.h"

typedef struct Slack_s {
    int x;
    int v;
} Slack;

static int dp[MAX_TITLE_LEN+1][MAX_TITLE_LEN+1];
static int num_assigned;
static int matrix_size;

static int
min_row(int *row);

static Slack *
min_slack(Slack *slacks, int *T, int *y);

static void
path_update(int y, int *T, int *x2y, int *y2x);

static void
find_aug(int **weights, int *X, int *Y, int *x2y, int *y2x, int *S, int * T, Slack *slacks, int x);


void
init_dp() {
    int i, j;
    for(i=1; i<=MAX_TITLE_LEN; ++i) {
        dp[i][0] = i;
    }
    for(j=1; j<=MAX_TITLE_LEN; ++j) {
        dp[0][j] = j;
    }
}


int
edit_distance(const char *s1, int len1, const char *s2, int len2) {
    int i, j, t;
    len1 = len1 < MAX_TITLE_LEN ? len1 : MAX_TITLE_LEN;
    len2 = len2 < MAX_TITLE_LEN ? len2 : MAX_TITLE_LEN;
    
    for(i=1; i<=len1; ++i) {
        for(j=1; j<=len2; ++j) {
            if(*(s1+i-1) == *(s2+j-1)) {
                dp[i][j] = dp[i-1][j-1];
            }
            else {
                t = dp[i-1][j];
                t = t < dp[i][j-1] ? t : dp[i][j-1];
                t = t < dp[i-1][j-1] ? t : dp[i-1][j-1];
                dp[i][j] = t + 1;
            }
        }
    }
    return dp[len1][len2];
}


int *
munkres(int **weights, int mat_size) {
    int i, j, x;
    matrix_size = mat_size;
    int *X = (int *)malloc(mat_size*sizeof(int));
    int *Y = (int *)calloc(mat_size, sizeof(int));
    int *x2y = (int *)malloc(mat_size*sizeof(int));
    int *y2x = (int *)malloc(mat_size*sizeof(int));
    int *S = (int *)malloc(mat_size*sizeof(int));
    int *T = (int *)malloc(mat_size*sizeof(int));
    Slack *slacks = (Slack *)malloc(mat_size*sizeof(Slack));
    for(i=0; i<mat_size; ++i) {
        x2y[i] = -1;
        y2x[i] = -1;
        X[i] = min_row(weights[i]);
    }

    while(num_assigned < mat_size) {
        memset(S, 0, mat_size*sizeof(int));
        for(i=0; i<mat_size; ++i) {
            T[i] = -1;
            if(x2y[i] == -1)//i not assigned
                x = i;
        }
        S[x] = 1;
        find_aug(weights, X, Y, x2y, y2x, S, T, slacks, x);
    }
    free(X);
    free(Y);
    free(x2y);
    free(S);
    free(T);
    free(slacks);
    return y2x;
}

static void
find_aug(int **weights, int *X, int *Y, int *x2y, int *y2x, int *S, int * T, Slack *slacks, int x) {
    int y, i, v;
    for(y=0; y<matrix_size; y++) {
        slacks[y].x = x;
        slacks[y].v = weights[x][y] - X[x] - Y[y];
    }
    Slack *edge;
    while(1) {
        edge = min_slack(slacks, T, &y);
        v = edge->v;
        if(v > 0) {
            for(i=0; i<matrix_size; ++i) {
                if(S[i])
                    X[i] += v;
                if(T[i] != -1) 
                    Y[i] -= v;
                else
                    slacks[i].v -= v;
            }
        }
        T[y] = edge->x;
        if((x=y2x[y])==-1) {
            path_update(y, T, x2y, y2x);
            break;
        }
        else {//y is also in other path reached from x
            S[x] = 1;
            for(i=0; i<matrix_size; ++i) {
                if((T[i]==-1) && (slacks[i].v > weights[x][i]-X[x]-Y[i])) {//update slack
                    slacks[i].v = weights[x][i]-X[x]-Y[i];
                    slacks[i].x = x;
                }
            }
        } 
    }
}

static void
path_update(int y, int *T, int *x2y, int *y2x) {
    int x = T[y];
    if(x2y[x] != -1)
        path_update(x2y[x], T, x2y, y2x);
    else
        ++num_assigned;
    x2y[x] = y;
    y2x[y] = x;
}

static Slack *
min_slack(Slack *slacks, int *T, int *y) {//T[y] indicated whether y is reached
    int i, j=0, min=MAX_TITLE_LEN;
    for(i=0; i<matrix_size; i++) {
        if((T[i]==-1) && (slacks[i].v < min)) {//min slack with y not reached
            j = i;
            min = slacks[i].v;
        }
    }
    *y = j;
    return slacks+j;
}


static int
min_row(int *row) {
    int i, min=MAX_TITLE_LEN;
    for(i=0; i<matrix_size; i++) {
        if(row[i] < min)
            min = row[i];
    }
    return min;
}
