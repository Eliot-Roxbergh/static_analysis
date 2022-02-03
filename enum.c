// Copyright 2022 Eliot Roxbergh. Licensed under AGPLv3 as per separate LICENSE file
#include <stdio.h>
#include <limits.h>


enum codes {
    DOG,    //0
    COW,    //1
    MONKEY  //2
};

// enum is standard integer type so may be used as such,
//  and any integer may actually be passed to this function
void func (enum codes e)
{
    printf("%d\n", e);
}

int main()
{
    int secret_variable = 0;
    func(DOG);
    func(COW);
    func(MONKEY);
    func(1);

    /* NOTE "invalid" numbers may also be passed */
    func(10); // 10
    func(-1); // -1
    func(3.3); // 3 .. will be cast to integer (floored), regular implicit cast
    func(UINT_MAX); //overflows to -1 (or maybe something else ... signed overflow is undefined behavior in C but don't worry about it ;) )
    func(UINT_MAX+1 == 0); // 0==0 => 1 (true)
    func(secret_variable = (DOG)); //assignment is allowed here of course (argument secret_variable is used to the function)

    printf("\n\nDOG %d\n", secret_variable); //0

    return 0;
}
