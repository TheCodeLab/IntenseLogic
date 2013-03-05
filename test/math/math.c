#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "math/matrix.h"
#include "math/quaternion.h"
#include "math/vector.h"

void print_matrix(il_mat m)
{
    printf("{ ");
    int y, x;
    for (y = 0; y < 4; y++) {
        for (x = 0; x < 4; x++) {
            printf("% .6f ", m[y*4 + x]);
        }
        if (y != 3) {
            printf("\n  ");
        }
    }
    printf("}\n");
}

int inversionTest()
{
    il_mat mat = il_mat_new();
    int i;
    for (i = 0; i < 16; i++) {
        mat[i] = (float)rand() / RAND_MAX;
    }
    printf("Input matrix:\n");
    print_matrix(mat);
    il_mat inverted = il_mat_invert(mat, NULL);
    printf("Inverted matrix:\n");
    print_matrix(inverted);
    il_mat final = il_mat_mul(mat, inverted, inverted);
    printf("Input * Inverted:\n");
    print_matrix(final);
    int success = 1;
    for (i = 0; i < 16; i++) {
        float diff = mat[i] - final[i];
        final[i] = diff;
        if (diff > 0.001 || diff < -0.001) {
            success = 0;
        }
    }
    printf("Matrix diff:\n");
    print_matrix(final);
    return success;
}

struct {
    int (*fn)();
    const char *name;
} tests[] = {
    {inversionTest, "inversion"},
    {NULL, NULL}
};

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: %s testname\n", argv[0]);
        return 0;
    }
    int i;
    for (i = 0; tests[i].name != NULL; i++) {
        if (strcmp(tests[i].name, argv[1]) == 0 || strcmp("all", argv[1]) == 0) {
            int res = tests[i].fn();
            if (res) {
                printf("Test successful\n");
            } else {
                printf("Test failed\n");
                return EXIT_FAILURE;
            }
        }
    }
    if (strcmp("all", argv[1]) == 0) {
        return EXIT_SUCCESS;
    }
    printf("Unknown test\n");
    return EXIT_FAILURE;
}

