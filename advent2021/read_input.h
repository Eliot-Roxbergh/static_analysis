// Copyright 2022 Eliot Roxbergh. Licensed under AGPLv3 as per separate LICENSE file
int read_ints(const char *, unsigned int *, int **);
int read_str_int(const char *, unsigned int *, char ***, int **);
int read_strs(const char *, unsigned int *, char ***);

typedef struct {
    unsigned int nr_elems;
    int *elems;
} line_entry;
int read_ints_per_line(const char *, unsigned int *, line_entry **);
