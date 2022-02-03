// Copyright 2020 Eliot Roxbergh. Licensed under AGPLv3 as per separate LICENSE file

/*
 * Just some simple examples on C pointers and memory.
 *
 * Tl;dr:
 * For another function to set the values of our struct either;
 *    - We (the caller) initalize memory for it and send the pointer
 *                                           (e.g. on the stack by simple declaration:
 *                                                 struct test obj;
 *                                                 struct test *obj_ptr = &obj;)
 *
 *    - Or we don't need to do anything and the function can store it on the heap with malloc and return the pointer.
 *      By doing so it's not released when that function returns, but (we) caller needs to free the memory manually.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct test {
    int a;
    /* ... */
};

/* Naive (broken) solution: Creating instance in local scope is not persistent,
 * We only change the value locally, the parent has no way of knowing local changes.
 */
void init_test(struct test *obj_ptr)
{
    struct test new_obj = *obj_ptr;  // only copies value, useless if we don't return it somehow
    new_obj.a = 1;

    (void)new_obj;  // compiler know the problem... above lines will otherwise result in warning
                    // (-Werror=unused-but-set-variable)
}

/* Just sets a field in the given memory address.
 * Remember this memory needs to be allocated by the caller.
 * So this function can update its values but the structure needs to already be allocated elsewhere.
 */
void init_test_2(struct test *obj_ptr)
{
    /*
    // This does the same as below
    struct test *new_obj_ptr = obj_ptr;
    new_obj_ptr->a = 1;
    */
    obj_ptr->a = 1;
}

/* Malloc: create presistent struct with malloc, set given pointer to this new object.
 * This is all good, just remember to free it later!
 * Pointer could either be returned via argument (set the new address in the ptr ptr given by caller),
 *    or as a return value (return new_obj;)
 */
void init_test_malloc(struct test **obj)
{
    size_t len = sizeof(struct test);
    struct test *new_obj;
    new_obj = malloc(len);
    new_obj->a = 1;

    *obj = new_obj;
}

int main()
{
    /* Value not persitent */
    struct test obj;  // memset this to be sure what the initial values are
    struct test *obj_ptr = &obj;
    init_test(obj_ptr);
    printf("Got %d \n", obj_ptr->a);  // not initialized or set, random value from memory (or segfault)

    /* Uninitialized ptr is not meant to be deferenced */
    printf("This might segfault, ignoring...\n");
    /*
    struct test *obj_ptr_2; //This pointer is not initialized, we don't know if its value (address) is valid!
    init_test_2(obj_ptr_2); //possible segfault, dereferencing and writing to unknown memory
    printf("Got %d \n", obj_ptr_2->a);
    */

    /* OK, as we create the object first */
    size_t len = sizeof(struct test);
    struct test obj_3;  // allocate memory for struct
    struct test *obj_ptr_3 = &obj_3;
    memset(&obj_3, 0, len);  // initilize struct data to known values
    init_test_2(obj_ptr_3);
    printf("Got %d \n", obj_ptr_3->a);  // Got 1, OK!

    printf("These are the same btw: %lu %lu\n", sizeof(struct test), sizeof obj_3);

    /*
     * MALLOC METHOD
     * By passing ptr ptr, the local function can
     *  create its own persistent object with malloc,
     *  and by changing the "middle pointer" (i.e. struct test*)
     *  with the address of that object.
     *  Now the parent function can read the outer ptr (still intact),
     *  which points to the structure as created by the function.
     */
    struct test *obj_1 = NULL;
    // since malloc returns a new memory address,
    // we need to send a ptr to that address (ptr to struct test*) so the function can return it for us to read
    init_test_malloc(&obj_1);
    printf("Got %d\n", obj_1->a);
    free(obj_1);

    return 0;
}
