#include "../read_input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>


int main() {
    unsigned int *input_string_status = NULL;

    unsigned int hits;
    char **input_strs;
    if (read_strs("input", &hits, &input_strs) != 0) {
        goto error;
    }
    if (!input_strs) {
        goto error;
    }

    /* part 1 */
    uint16_t gamma_rate = 0, epsilon_rate = 0;
    uint16_t ones_found_in_column;

    size_t width = strlen(input_strs[0]);
    if (width != 12) {
        goto error;
    }

    //for each column (x from 0 to 11), if more than half chars in row (y) are '1'
    //  set corresponding bit in gamma rate to 1, specifically the bit 2^(11-x).
    //epsilon rate follows the same logic, but looking for '0' instead,
    //  since a value may only be one or zero, we simply bit invert gamma rate to get epsilon rate.
    //NOTE: if a column has the same number of '1' and '0', the result is undefined
    for (uint16_t x=0; x < width; x++) {
        ones_found_in_column = 0;
        for (uint16_t y=0; y<hits; y++) {
            if (input_strs[y][x] == '1') {
                ones_found_in_column++;
            }
            if (ones_found_in_column > hits/2) {
                gamma_rate |= (uint16_t) (2048 >> x); // 2^11 >> x
                break;
            }
        }
    }

    epsilon_rate = gamma_rate^(4096-1); //invert the data bits (12 low bits: 4096-1 = 2^12-1)
    printf("i) %d*%d = %d\n", gamma_rate, epsilon_rate, gamma_rate*epsilon_rate);


    /* part 2 */
    enum status {
        UNSET = 0,
        OXYGEN,
        SCRUBBER,
        NONE
    };
    input_string_status = calloc(hits, sizeof(unsigned int));
    unsigned int oxygen_strs_matching = hits;
    unsigned int scrubber_strs_matching = hits;

    uint16_t zeroes_oxygen = 0, ones_oxygen = 0;
    uint16_t zeroes_scrubber = 0, ones_scrubber = 0;
    uint16_t oxygen_rating = 0, scrubber_rating = 0;
    char *scrubber_str = NULL, *oxygen_str = NULL;
    char oxygen_last_char, scrubber_last_char, actual_last_char;
    for (uint16_t x=0; x < width; x++) {
        zeroes_oxygen = 0; ones_oxygen = 0;
        zeroes_scrubber = 0; ones_scrubber = 0;

        for (uint16_t y=0; y<hits; y++) {
            if (input_string_status[y] == NONE) {
                continue;
            }
            //remove lines not matching previous runs result for oxygen and scrubber
            if (x > 0) {
                //TODO well I guess this might fail for certain input since we don't check the last iteration this way (only x-1)
                actual_last_char = input_strs[y][x-1];

                //line is ok first time, set line status
                if (input_string_status[y] == UNSET) {
                    if (actual_last_char == oxygen_last_char) {
                        input_string_status[y] = OXYGEN;
                        oxygen_str = input_strs[y];
                        scrubber_strs_matching--;
                    } else if (actual_last_char == scrubber_last_char) {
                        input_string_status[y] = SCRUBBER;
                        scrubber_str = input_strs[y];
                        oxygen_strs_matching--;
                    }
                //line is still OK
                } else if (actual_last_char == oxygen_last_char && input_string_status[y] == OXYGEN) {
                    oxygen_str = input_strs[y];
                } else if (actual_last_char == scrubber_last_char && input_string_status[y] == SCRUBBER) {
                    scrubber_str = input_strs[y];
                //remove line
                } else {
                    if (input_string_status[y] == OXYGEN && oxygen_strs_matching > 1) {
                        oxygen_strs_matching--;
                        input_string_status[y] = NONE;
                    } else if (input_string_status[y] == SCRUBBER && scrubber_strs_matching > 1) {
                        scrubber_strs_matching--;
                        input_string_status[y] = NONE;
                    }
                    continue;
                }
            }

            // count zeroes and ones only if still in list
            if (input_string_status[y] == SCRUBBER|| input_string_status[y] == UNSET) {
                if (input_strs[y][x] == '0') {
                    zeroes_scrubber++;
                }
                if (input_strs[y][x] == '1') {
                    ones_scrubber++;
                }
            }
            if (input_string_status[y] == OXYGEN || input_string_status[y] == UNSET) {
                if (input_strs[y][x] == '0') {
                    zeroes_oxygen++;
                }
                if (input_strs[y][x] == '1') {
                    ones_oxygen++;
                }
            }
        }
        if (oxygen_strs_matching == 1 && scrubber_strs_matching == 1) {
            break;
        }

        if (ones_oxygen >= zeroes_oxygen) {
            oxygen_last_char = '1';
        } else if (zeroes_oxygen > ones_oxygen) {
            oxygen_last_char = '0';
        }
        if (ones_scrubber >= zeroes_scrubber) {
            scrubber_last_char = '0';
        } else if (zeroes_scrubber > ones_scrubber) {
            scrubber_last_char = '1';
        }
    }

    //binary string to int
    uint16_t i = 1;
    for (int x=11; x>=0; x--) {
        if (scrubber_str[x] == '1') {
            scrubber_rating |= i;
        }
        if (oxygen_str[x] == '1') {
            oxygen_rating |= i;
        }
        i = (uint16_t) (i << 1);
    }

    printf("ii) %d*%d = %d\n", oxygen_rating, scrubber_rating, oxygen_rating*scrubber_rating);

error:
    if (input_string_status) {
        free(input_string_status);
    }
    if (input_strs) {
        for (unsigned int i=0; i<hits; i++) {
            free(input_strs[i]);
        }
        free(input_strs);
    }
    return 0;
}
