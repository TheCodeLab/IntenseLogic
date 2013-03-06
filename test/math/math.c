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

void print_vector(il_vec4 v)
{
    printf("(% .2f, % .2f, % .2f, % .2f)", v[0], v[1], v[2], v[3]);
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

int dotTest()
{
    il_vec4 vec1 = il_vec4_set(NULL, 1, 2, 3, 4);
    il_vec4 vec2 = il_vec4_set(NULL, 4, 3, 2, 1);
    printf("Input vectors: ");
    print_vector(vec1);
    printf(" ");
    print_vector(vec2);
    printf("\n");
    float res = il_vec4_dot(vec1, vec2);
    il_vec4_free(vec1);
    il_vec4_free(vec2);
    printf("Result: %.6f\n", res);
    return res == 20.f;
}

struct {
    int (*fn)();
    const char *name;
} tests[] = {
    {inversionTest, "inversion"},
    {dotTest, "dot"},
    {NULL, NULL}
};

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: %s testname\n", argv[0]);
        return 0;
    }
    int run_all = strcmp("all", argv[1]) == 0;
    if (run_all) {
        printf("Running all tests.\n");
    }
    int i, passed = 0, failed = 0;
    for (i = 0; tests[i].name != NULL; i++) {
        if (strcmp(tests[i].name, argv[1]) == 0 || run_all) {
            printf("Test: %s\n", tests[i].name);
            int res = tests[i].fn();
            if (res) {
                printf("Test successful\n");
                passed++;
                if (!run_all) {
                    return EXIT_SUCCESS;
                }
            } else {
                printf("Test failed\n");
                failed++;
                if (!run_all) {
                    return EXIT_FAILURE;
                }
            }
        }
    }
    if (run_all) {
        printf("Tests passed: %i\nTests failed: %i\n", passed, failed);
        return EXIT_SUCCESS;
    }
    printf("Unknown test\n");
    return EXIT_FAILURE;
}

