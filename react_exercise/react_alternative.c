// Copyright 2022 Eliot Roxbergh. Licensed under AGPLv3 as per separate LICENSE file
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "react.h"

/*
 * Alternative version with another iterate_over_all_children()
 * Instead of the other function pointer implementation,
 *  not too exciting really :)
 */

/*
 * Define no debug to enable asserts.
 * The asserts add some checks for programming errors
 *  that should not depend on user input, for testing only.
 */
//#define NDEBUG //uncomment this line to disable asserts
#include <assert.h>

/* for internal function iterate_over_all_children */
enum iterate_action { COMPUTE_VALUE, INVOKE_CALLBACKS, DELETE_ALL };
static void iterate_over_all_children(cell *c, enum iterate_action action);
/* rest of internal functions */
static void run_callbacks(callback_st *cb_st, int new_value);
static void destroy_cell_callbacks(cell *c);
static cell *compute_cell_add_child(cell *c, cell *child);

/* --- EXPOSED FUNCTIONS --- */

reactor *create_reactor()
{
    // I assume zero initialization of memory should result in integer=0 and points=NULL, always? ...
    reactor *r = calloc(1, sizeof(reactor));
    return r;
}

// delete everything on reactor
void destroy_reactor(reactor *r)
{
    if (!r) {
        fprintf(stderr, "Invalid input given\n");
        exit(1);
    }

    // free all cells and callbacks
    cell *top_parent = r->first_parent;
    cell *next_top_parent;
    while (top_parent) {
        next_top_parent = top_parent->next_parent;
        iterate_over_all_children(top_parent, DELETE_ALL);
        top_parent = next_top_parent;
    }
    // free reactor
    free(r);
}

// add top-level parent
cell *create_input_cell(reactor *r, int initial_value)
{
    if (!r) {
        fprintf(stderr, "Invalid input given\n");
        exit(1);
    }

    cell *c = calloc(1, sizeof(cell));
    c->reactor = r;
    c->value = initial_value;
    c->new_value = c->value;
    c->nr_of_children = 0;

    if (r->first_parent == NULL) {
        r->first_parent = c;
    } else {
        // old last parent point to us
        r->last_parent->next_parent = c;
    }
    r->last_parent = c;

    return c;
}

// add new child to a parent
cell *create_compute1_cell(reactor *r, cell *c, compute1 compute1)
{
    if (!r || !c || !compute1 || r != c->reactor) {
        fprintf(stderr, "Invalid input given\n");
        exit(1);
    }

    cell *child;
    child = compute_cell_add_child(c, NULL);
    if (!child) {
        return NULL;
    }  // if this happens we are in trouble

    child->reactor = r;
    child->parents[0] = c;
    child->compute1 = compute1;
    child->value = child->compute1(c->value);
    child->new_value = child->value;

    return child;
}

// add the same new child to two parents
cell *create_compute2_cell(reactor *r, cell *c1, cell *c2, compute2 compute2)
{
    if (!r || !c1 || !c2 || !compute2 || r != c1->reactor || r != c2->reactor) {
        fprintf(stderr, "Invalid input given\n");
        exit(1);
    }

    // allocate child and add its pointer to (its first) parent
    cell *child;
    child = compute_cell_add_child(c1, NULL);
    if (!child) {
        return NULL;
    }  // if this happens we are in trouble

    // add the same child pointer to its other parent (which points to same child in memory)
    child = compute_cell_add_child(c2, child);

    child->reactor = r;
    child->parents[0] = c1;
    child->parents[1] = c2;
    child->compute2 = compute2;
    child->value = child->compute2(c1->value, c2->value);
    child->new_value = child->value;

    return child;
}

int get_cell_value(cell *c)
{
    if (!c) {
        fprintf(stderr, "Invalid input given\n");
        exit(1);
    }
    return c->value;
}

void set_cell_value(cell *c, int new_value)
{
    if (!c) {
        fprintf(stderr, "Invalid input given\n");
        exit(1);
    }
    if (c->value == new_value) {
        return;  // done, no children have changed value either
    }

    c->new_value = new_value;

    // compute 'new_value' and propagate change
    iterate_over_all_children(c, COMPUTE_VALUE);

    // only once all values (new_value) have been propagated, we finalize by;
    //  invoke callbacks and write 'new_value' to 'value'
    iterate_over_all_children(c, INVOKE_CALLBACKS);
}

// note: one cell can have multiple callbacks
callback_id add_callback(cell *cell, void *cb_data, callback cb)
{
    if (!cell || !cb) {
        fprintf(stderr, "Invalid input given\n");
        exit(1);
    }
    if (cell->reactor->next_cb_id == INT_MAX) {
        fprintf(stderr, "Sorry! Callbacks full, have already %d\n", cell->reactor->next_cb_id);
        return 0;
    }

    callback_st *cb_st = calloc(1, sizeof(callback_st));
    cb_st->data = cb_data;
    cb_st->func = cb;
    cb_st->id = cell->reactor->next_cb_id++;
    cb_st->next_cb_st = NULL;  // we are last in list

    // first callback on cell, cell point to us directly
    if (cell->cb_st == NULL) {
        cell->cb_st = cb_st;
        // otherwise, previous cb in list should point to us, not null
    } else {
        callback_st *tmp = cell->cb_st;
        while (tmp->next_cb_st) {
            tmp = tmp->next_cb_st;
        }
        tmp->next_cb_st = cb_st;
    }
    return cb_st->id;
}

/*
 * Remove a single callback (cb_st) from cell, matching the id.
 *  (cb_st is a linked list of one or multiple cb_st)
 */
void remove_callback(cell *c, callback_id id)
{
    callback_st *cb_st, *cb_st_previous, *matching_cb;
    if (!c) {
        fprintf(stderr, "Invalid input given\n");
        exit(1);
    }
    if (!c->cb_st) return;

    // check first element in cb_st
    if (c->cb_st->id == id) {
        matching_cb = c->cb_st;
        // put second element in list first (element is be NULL if matching_cb was only element in list)
        c->cb_st = matching_cb->next_cb_st;
        free(matching_cb);
        return;
    }

    // otherwise check rest of list
    cb_st_previous = c->cb_st;
    cb_st = c->cb_st->next_cb_st;
    while (cb_st) {
        if (cb_st->id == id) {
            matching_cb = cb_st;
            // point previous cb_st to the one after matching entry (points to NULL if last)
            cb_st_previous->next_cb_st = matching_cb->next_cb_st;
            free(matching_cb);
            return;
        }
        cb_st_previous = cb_st;
        cb_st = cb_st->next_cb_st;
    }
}

/* --- INTERNAL FUNCTIONS --- */

// delete all callbacks on a single cell
static void destroy_cell_callbacks(cell *c)
{
    assert(c);
    if (!c->cb_st) {
        return;
    }
    if (!c->cb_st->next_cb_st) {
        // delete the only element in list
        free(c->cb_st);
    } else {
        callback_st *cb;
        callback_st *next_cb;

        // delete all elements but the first
        cb = c->cb_st;
        next_cb = cb->next_cb_st;
        while (next_cb) {
            cb = next_cb;
            next_cb = next_cb->next_cb_st;
            free(cb);
        }
        // delete the first element
        free(c->cb_st);
    }
    c->cb_st = NULL;
}

/*
 * Add a child to compute cell c (in c->children), if child is null: allocate and zero initialize that child.
 *  Returns child (same as the given argument 'child' if it wasn't NULL)
 *
 * Each parent have a cell** children, which contains a list to its direct children,
 *  this function (re-)allocates the children memory and points this new area to a new child.
 * Both children and new child (children[nr_of_children-1]) need to be freed.
 */
static cell *compute_cell_add_child(cell *c, cell *child)
{
    cell **children;
    unsigned int nr_of_children;
    size_t new_size;

    assert(c);
    assert(!(c->children && c->nr_of_children == 0));  // no children but ptr not NULL

    if (c->nr_of_children == UINT_MAX) {
        fprintf(stderr, "Sorry! Parent is full and has already %d children\n", c->nr_of_children);
        return NULL;
    }

    if (!child) {
        child = calloc(1, sizeof(cell));
    }

    nr_of_children = c->nr_of_children + 1;
    new_size = nr_of_children * sizeof(cell *);

    // reallocate children
    //  c->children points to a memory area, increase this to hold one more pointer
    //  or if c->children=NULL this will initialize the memory (to hold one child pointer)
    children = realloc(c->children, new_size);
    if (!children) {
        exit(1);
    }

    // save result
    c->children = children;
    c->nr_of_children = nr_of_children;
    // write address of new child to the expanded memory
    children[nr_of_children - 1] = child;

    return child;
}

// invoke all callbacks on a single cell (this should be called once whenever cell value has changed)
static void run_callbacks(callback_st *cb_st, int new_value)
{
    assert(cb_st);
    cb_st->func(cb_st->data, new_value);

    cb_st = cb_st->next_cb_st;
    while (cb_st) {
        cb_st->func(cb_st->data, new_value);
        cb_st = cb_st->next_cb_st;
    }
}

/*
 * Perform supplied action on a cell, then go deeper (first a parent, then all its children, and then on the children's
 * children, etc.) Both input cell and compute cell can be given as input (cell* c)
 *
 * Using depth-first search,
 *  we may either perform action then go deeper (parent then child),
 *  or go deeper then perform action which results in a kind of reverse order (child then parent)
 *
 * (Comment: The "reverse order" is not globally reversed, but each subtree is taken in reversed order,
 *              e.g. the parent 0 with two children trees 123 45
 *                   would be in order 0 123 45 but reversed 321 54 0 )
 *
 * Comment: recursion in C might blow the stack if we go too deep?
 */
static void iterate_over_all_children(cell *c, enum iterate_action action)
{
    if (!c) {
        return;
    }

    // actions to perform from first to last cell
    switch (action) {
        // update c->new_value
        case COMPUTE_VALUE: {
            if (c->compute1) {
                c->new_value = c->compute1(c->parents[0]->new_value);
            } else if (c->compute2) {
                c->new_value = c->compute2(c->parents[0]->new_value, c->parents[1]->new_value);
            } else {
                // we are a top-level cell (i.e. input cell), go deeper
                break;
            }
            if (c->value == c->new_value) {
                return;
            }
            break;
        }
        // write new_value to value and invoke callbacks
        case INVOKE_CALLBACKS: {
            if (c->value == c->new_value) {
                return;
            }
            c->value = c->new_value;
            if (c->cb_st) {
                run_callbacks(c->cb_st, c->value);
            }
            break;
        }
        // delete cell, callbacks, and parent's reference to cell
        case DELETE_ALL:
            break;  // we will delete later, after recursion call, due to reverse order: from last to first child
        default:
            return;
            break;
    }

    // go deeper
    for (unsigned int i = 0; c->children != NULL && i < c->nr_of_children; i++) {
        iterate_over_all_children(c->children[i], action);
    }

    // actions to perform from last to first cell
    if (action != DELETE_ALL) {
        return;
    } else {
        // so basically, we iterate all the way down and then free each cell upwards
        bool parent_has_no_children;
        unsigned int nr_parents = 0;  // here, c may have 0, 1, or 2 parents
        if (c->parents[0] && c->parents[0]->children) {
            nr_parents++;
        }
        if (c->parents[1] && c->parents[1]->children) {
            nr_parents++;
            assert(nr_parents == 2);
        }

        // delete my parent(s) references to me, then if they have no children left free that memory (cell **children)
        for (unsigned int k = 0; k < nr_parents; k++) {
            parent_has_no_children = true;

            for (unsigned int i = 0; i < c->parents[k]->nr_of_children; i++) {
                if (c == c->parents[k]->children[i]) {
                    // delete reference to me
                    c->parents[k]->children[i] = NULL;
                }
                if (c->parents[k]->children[i] != NULL) {
                    parent_has_no_children = false;
                }
            }

            if (parent_has_no_children) {
                // I was last child, free children
                free(c->parents[k]->children);
                c->parents[k]->children = NULL;
            }
        }
        // delete my callbacks and then finally delete myself
        destroy_cell_callbacks(c);
        free(c);
    }
}
