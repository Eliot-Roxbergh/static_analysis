// Copyright 2022 Eliot Roxbergh. Licensed under AGPLv3 as per separate LICENSE file
#include <assert.h>
#include <math.h>  //add -lm to build
#include <stdio.h>

/* Question:
 *  - why does shift (<<) and bit inverse (~) return int?
 *      Is it also not more reasonable to use unsigned char
 *      for bit manipulation anyway?
 *  - Werror=conversion, seems quite odd? See examples below.
 */
int main()
{
    unsigned char bits = 5;
    unsigned char bits_inverse = 0;
    bits_inverse = (unsigned char)~bits;                // OK
    bits_inverse = ((unsigned int)~bits) & (0xFF - 1);  // OK
    bits_inverse = ~bits & 0xF;                         // OK

    // BUT!
    // bits_inverse = ~bits;             //NOT OK -Werror=conversion
    // bits_inverse = ~bits & (0xFF-1);  //NOT OK -Werror=conversion
    printf("%u\n", bits_inverse);

    /*
    //longer example:
    printf("%d\n", bits);
    for (int i=1; i<8; i++) {
        // without cast; error conversion to ‘unsigned char’ from ‘int’ may alter its value [-Werror=conversion]
        bits = (unsigned char) (bits << 1);
        assert(bits == pow(2, i)); // although pow returns double, 'bits' can be promoted in comparison (unsigned char
    == double => double == double)

        bits_inverse = (unsigned char) ~bits;           //OK
        //bits_inverse = ((unsigned int) ~bits) & 255;  //OK
        //bits_inverse = ((unsigned int) ~bits) & 256;  //NOT OK
        //bits_inverse = (~bits) & 255;                 //NOT OK
        //bits_inverse = ~bits & 0xF ;                  //OK
        //bits_inverse = ~bits & 0xFF ;                 //NOT OK

        //unsigned char is promoted to int (%d)
        printf("%u (one complement is %u)\n", bits, bits_inverse);
    }
    */

    return 0;
}
