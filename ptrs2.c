// Copyright 2022 Eliot Roxbergh. Licensed under AGPLv3 as per separate LICENSE file
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* test0 */
void set_var(int **var)
{
   **var = 4;
   *var = NULL;
}
void test0()
{
   printf("\n\ntest0\n");
   int test = 0;
   int *test_ptr = &test;

   // change test_ptr to be NULL, but first change the value it points to
   set_var(&test_ptr); //NOTE: you may NOT take &&test, this is not a valid operator for this purpose

   // -> &test is valid ptrs, test = 4, but "test_ptr" is NULL
   printf("%p -> %d and %p -> Seg fault \n", (void*)&test, test, (void*)test_ptr);
}

/* test1 */
const int my_secret = 42;
int get_var(int **ret) //this works
{
   int *a_ptr;
   int sanity;
   /*
   //will not be persistent ofc, variable will be set but value will be "freed"
   int a = my_secret;
   *ret = &a;
   */

   /*
    * Any memory we create on the stack will be gone after this function's scope, naturally.
    * But we may return addresses that point to data we did not initialize (not local to this scope).
    * my_secret is local to this file and its address will be valid for longer (local to this file)
    */
   a_ptr = (int*) &my_secret;

   *ret = a_ptr; //OK, pass address (to "global" variable)
   //*ret = (int*) &my_secret; //OK (this is basically the same as above)
   //*ret = &a_ptr;  //(this function then needs to take int ***ret as input), Not OK, we don't want to return address to local stack data (in this case the data is another valid address, but it doesn't matter, it is stored in soon-to-be-freed stack memory)
   sanity = *a_ptr;
   /*
    * We don't really do too much on the stack here.
    * We just copy the address we want to local variable a_ptr,
    * then in-turn copy that value to memory provided by caller.
    *
    * We'd only have a problem if we did something like:
    * ret = &a_ptr;
    *
    */
   return sanity;
}
void test1()
{
   printf("\n\ntest1\n");
   int a = 0;
   int *a_ptr = &a;
   int a_backup = 0;

   a_backup = get_var(&a_ptr);
   printf("%p -> %d (expected %d)\n",(void*) a_ptr,*a_ptr,a_backup);
}



int main()
{
   test0();
   test1();
}
