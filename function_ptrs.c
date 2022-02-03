// Copyright 2022 Eliot Roxbergh. Licensed under AGPLv3 as per separate LICENSE file
/*
 * Simple example of function pointers, idea from http://www.simplyembedded.org/archives/battle-of-the-standards-round-2-designated-initializers/
 */

#include <stdio.h>
#include <string.h>


//I assume this is equivalent as to make these two defines separately
enum data_type {
    READ, WRITE
};

//typedef init_t type (which is a function ptr with these characteristics)
typedef int (*init_t)(int);


//two examples of functions which falls under init_t type
int init_read(int a){
    return a+1;
}
int init_write(int a){
    return a-1;
}


//array of init_t, returns the approriate function pointer depending on data type (enum)
//... not to be confused, as this is just a regular array with each index specified explictly according to C99 syntax
static init_t init[] =
{
    [READ]  = init_read,
    [WRITE] = init_write

};

int main(){
    int a = init[READ](5); //call first function in array with argument 5
    int b = init[WRITE](10); //call second function in array with argument 10
    printf("%d %d\n",a,b);
    return 0;
}
