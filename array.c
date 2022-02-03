// Copyright 2020 Eliot Roxbergh. Licensed under AGPLv3 as per separate LICENSE file.
#include <alloca.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Passing two-dimensional arrays to functions
 *  http://c-faq.com/aryptr/pass2dary.html
 *      "The rule (see question 6.3) by which arrays decay into pointers is not applied recursively"
 *      "An array of arrays (i.e. a two-dimensional array in C) decays into a pointer to an array, not a pointer to a
 * pointer. Pointers to arrays can be confusing, and must be treated carefully" (also
 * https://stackoverflow.com/questions/9446707/correct-way-of-passing-2-dimensional-array-into-a-function)
 */

/*
 * A two-dimensional array apa[5][3]:
 *     [0 1 2]
 *     [0 1 2]
 *     [0 1 2]
 *     [0 1 2]
 *     [0 1 2]
 */

// apa[5][3]
// 5 pointers to int[3] arrays
void func(int (*apa)[3], unsigned int len)
{
    int my_apa[3];
    // my_apa = apa[1]; //not ok (there's no assignment operator for arrays)
    memcpy(my_apa, apa[0], 3);  // copy data of first array

    int *my_apap = apa[0];  // point to first array of (3) ints

    unsigned int width = 3;
    for (unsigned int y = 0; y < len; y++) {
        printf("%d: ", y + 1);
        for (unsigned int x = 0; x < width; x++) {
            /*
            assert(my_apa[x] == apa[0][x]);
            assert(my_apa[x] == my_apap[x]);
            */
            (void)my_apap;
            (void)my_apa;

            int num = apa[y][x];
            printf("%d ", num);
        }
        printf("\n");
    }
}

// Alternative: can also write it as apa[][3]
void func_alt(int apa[][3], unsigned int len)
{
    unsigned int width = 3;
    for (unsigned int y = 0; y < len; y++) {
        printf("%d: ", y + 1);
        for (unsigned int x = 0; x < width; x++) {
            printf("%d ", apa[y][x]);
        }
        printf("\n");
    }
}

int main()
{
    /* warm up */
    int arr[5] = {1, 2, 3, 4, 6};
    int len = *(&arr + 1) - arr;
    printf("The length of the array is: %i elem\n", len);                       // 5
    printf("The length of the array is: %li Bytes\n", sizeof(arr));             // 20 Bytes
    printf("This is the same as %p - %p\n", (void *)*(&arr + 1), (void *)arr);  // diff is 20 (Bytes)
    printf("\n\n");

    /* two-dimensional array */
    int apa[5][3] = {0};
    func(apa, 5);
    // printf("\n");
    // func_alt(apa, 5);

    printf("\nArray of arrays:\n");
    for (unsigned int y = 0; y < 5; y++) {
        for (unsigned int x = 0; x < 3; x++) {
            printf("%d ", apa[y][x]);
        }
        printf("\n");
    }

    /* similar two-dimensional list with ptr ptr */
    // is this the same memory structure as the array above?
    int **papa = alloca(5 * sizeof(void *));  // allocate memory for five ptrs, which each points to int* ("int[3]")
    for (int x = 0; x < 5; x++) {
        papa[x] = alloca(3 * sizeof(int));  // do five times: allocate memory for 3 ints and store the pointer in ptrs
                                            // allocated above ("int[5]")
    }
    printf("\nPtr ptr:\n");
    for (unsigned int y = 0; y < 5; y++) {
        for (unsigned int x = 0; x < 3; x++) {
            papa[y][x] = 0;
            printf("%d ", papa[y][x]);
        }
        printf("\n");
    }
    return 0;
}
