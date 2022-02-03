/* Copyright (c) 2021 Exercism, MIT License
 * Copyright (c) 2022 Eliot Roxbergh, Licensed under AGPLv3 as per separate LICENSE file.
 */
#ifndef REACT_H
#define REACT_H
#include <stdbool.h>

struct cell;
struct reactor;

typedef int (*compute1) (int);
typedef int (*compute2) (int, int);

struct reactor *create_reactor();
// destroy_reactor should free all cells created under that reactor.
void destroy_reactor(struct reactor *);

struct cell *create_input_cell(struct reactor *, int initial_value);
struct cell *create_compute1_cell(struct reactor *, struct cell *, compute1);
struct cell *create_compute2_cell(struct reactor *, struct cell *,
                                  struct cell *, compute2);

int get_cell_value(struct cell *);
void set_cell_value(struct cell *, int new_value);

typedef void (*callback) (void *, int);
typedef int callback_id;

// The callback should be called with the same void * given in add_callback.
callback_id add_callback(struct cell *, void *, callback);
void remove_callback(struct cell *, callback_id);


/* My additions */
struct callback_st;

typedef struct callback_st {
    callback func;
    void *data;
    callback_id id;
    struct callback_st *next_cb_st;
} callback_st;

typedef struct reactor {
    struct cell *first_parent;
    struct cell *last_parent;
    callback_id next_cb_id;
} reactor;

// cell can be either:
//   - input cell = top level parent
//   - compute cell = child to an input or compute cell
typedef struct cell {
    /* shared fields */
    struct reactor *reactor;
    struct cell **children;
    unsigned int nr_of_children;
    struct callback_st *cb_st; //one cell may hold multiple callbacks
    int value; //(old value is temporarily cached as to not invoke callback multiple times for one change)
    int new_value;

    /* input cell fields */
    struct cell *next_parent; // next top-level parent (so we can free)

    /* compute cell fields */
    struct cell *parents[2];
    compute1 compute1;
    compute2 compute2;
} cell;


#endif
