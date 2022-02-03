// Copyright 2022 Eliot Roxbergh. Licensed under AGPLv3 as per separate LICENSE file
#include <stdio.h>
#include <stdlib.h>
/* some buggy code to see what static analysis tools say,
 * gcc does not complain (see CMakeLists.txt) */

#define STR_SIZE 10

static int set_data(char **data_p)
{
    int rv = -1;
    char *tmp_p = NULL;
    tmp_p = malloc(STR_SIZE);

    if (!tmp_p) {
        free(tmp_p);  // bug!
        return rv;
    }

    // with snprintf gcc warns us if we lose text (format-truncation)
    // using sprintf no warning is possible and we could get stack overflow
    sprintf(tmp_p, "Some text here");  // overflow
    snprintf(tmp_p, STR_SIZE, "Some text");
    // we should also check return value of sprintf/snprintf, or at least something to consider...
    //  to detect output error (retval<0), and for snprintf, truncation (retval>STR_SIZE)

    // We dereference data_p without checking it's not null,
    // however as called from main this is not a problem right now.
    // Recommendation should be to check data_p != NULL
    *data_p = tmp_p;
    free(tmp_p);  // bug! (-> double free)

    rv = 0;
    return rv;
}

int main()
{
    int rv = -1;

    // ok
    char *data_1 = NULL;
    rv = set_data(&data_1);
    printf("Got '%s'\n", data_1);
    if (rv != 0) {
        goto error;
    }

    // two problems
    // 1. Not really a problem in this case
    //      but "safer" to initialize pointer to NULL
    // 2. Declaring variable after goto should be undefined behavior
    char *data_2;
    rv = set_data(&data_2);
    if (rv != 0) {
        goto error;
    }
    rv = 0;

error:
    if (data_1) {
        free(data_1);
    }

    // on error could read and free without being declared -> segfault
    if (data_2) {
        free(data_2);
    }
    return rv;
}
