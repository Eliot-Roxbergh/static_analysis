#include "../read_input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    unsigned int hits;
    int *input_ints;
    char **input_strs;
    if (read_str_int("input", &hits, &input_strs, &input_ints) != 0) {
        goto error;
    }
    if (!input_ints || !input_strs) {
        goto error;
    }

    /* part 1 */
    int pos_x = 0, pos_y = 0;
    char *str;
    int num;
    for (unsigned int i=0; i<hits; i++) {
        str = input_strs[i];
        num = input_ints[i];

        if (strcmp(str, "forward") == 0) {
            pos_x += num;
        } else if (strcmp(str, "down") == 0) {
            pos_y += num;
        } else if (strcmp(str, "up") == 0) {
            pos_y -= num;
        }
    }
    printf("i) %d\n", pos_x*pos_y);

    /* part 2 */
    pos_x = 0, pos_y = 0;
    int velocity_down = 0; //"aim", for every forward tick we will also go down by this much
    for (unsigned int i=0; i<hits; i++) {
        str = input_strs[i];
        num = input_ints[i];

        if (strcmp(str, "forward") == 0) {
            pos_x += num;
            pos_y += num*velocity_down;

        } else if (strcmp(str, "down") == 0) {
            velocity_down += num;
        } else if (strcmp(str, "up") == 0) {
            velocity_down -= num;
        }
    }
    printf("ii) %d\n", pos_x*pos_y);

error:
    if (input_ints) {
        free(input_ints);
    }
    if (input_strs) {
        for (unsigned int i=0; i<hits; i++) {
            free(input_strs[i]);
        }
        free(input_strs);
    }
    return 0;
}
