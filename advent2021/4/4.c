#include "../read_input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

typedef struct {
    int numbers[5][5];
    unsigned int draws_to_win;
    int score;
    bool got_bingo; //req. for part 2
} board;
static bool number_is_drawn(int, int *, unsigned int);
static bool check_bingo_rows(board *, int *, unsigned int);
static bool check_bingo_cols(board *, int *, unsigned int);
static int  get_score(board *, int *, unsigned int);

static bool check_bingo_cols(board *board, int *draws, unsigned int draw_count)
{
    int number;
    int winner[5];
    for (int x=0; x<=4; x++) {
        for (int y=0; y<=4; y++) {
            number = board->numbers[y][x];

            if (!number_is_drawn(number, draws, draw_count)) {
                break; //no bingo on column
            }
            winner[y] = number;
            if (y == 4) {
                printf("\n\n\nBingo for column: \n  ");
                for(int i=0; i<5; i++) {
                    printf("%d ",winner[i]);
                }
                return true; //all matched
            }
        }
    }
    return false;
}

static bool check_bingo_rows(board *board, int *draws, unsigned int draw_count)
{
    int number;
    int winner[5];
    for (int y=0; y<=4; y++) {
        for (int x=0; x<=4; x++) {
            number = board->numbers[y][x];

            if (!number_is_drawn(number, draws, draw_count)) {
                break; //no bingo on row
            }
            winner[x] = number;
            if (x == 4) {
                printf("\n\n\nBingo for row: \n  ");
                for(int i=0; i<5; i++) {
                    printf("%d ",winner[i]);
                }
                return true; //all matched
            }
        }
    }
    return false;
}

//check if number is in given list
static bool number_is_drawn(int number, int *draws, unsigned int draw_count) {
    for (unsigned int i=0; i < draw_count; i++) {
        if (number == draws[i]) {
            return true;
        }
    }
    return false;
}

//get score for a board which got bingo.
static int get_score(board *board, int *draws, unsigned int draw_count)
{
    //assert(board->got_bingo);
    printf("\nThis board got bingo at round %d\n", draw_count);
    bool exclude_number;
    int number;

    //add number to score only if it's not drawn (i.e. not in excluded numbers list)
    int sum_of_unmarked_numbers = 0;
    for (unsigned int y=0;y<5;y++) {
        printf("  ");
        for (unsigned int x=0;x<5;x++) {
            number = board->numbers[y][x];
            exclude_number = number_is_drawn(number, draws, draw_count);
            if (!exclude_number) {
                sum_of_unmarked_numbers += number;
            }
            printf("%d ",number);
        }
        printf("\n");
    }
    // score = sum of unmarked numbers * last draw
    return sum_of_unmarked_numbers*draws[draw_count-1];
}


int main() {
    unsigned int lines_in_raw_input;
    line_entry *input_ints;
    if (read_ints_per_line("input", &lines_in_raw_input, &input_ints) != 0) {
        goto error;
    }
    if (!input_ints) {
        goto error;
    }

    /* part 1 & 2 */
    board *winner = NULL; //part 1
    int last_winner_score = 0; //part 2

    int *drawn_numbers = input_ints[0].elems; //first line must be drawn numbers
    unsigned int drawn_numbers_count = input_ints[0].nr_elems;

    // Find and assign boards,
    //  a board is 5 consecutive rows with exactly 5 integers in each.
    unsigned int nr_of_boards = lines_in_raw_input / 5; //theoretical max nr of boards
    board *boards = calloc(nr_of_boards,sizeof(board));
    unsigned int rows_found = 1, nr_of_boards_found = 0;
    for (unsigned int cur_line=1; cur_line < lines_in_raw_input; cur_line++) {
        //invalid row, reset board
        if (input_ints[cur_line].nr_elems != 5) {
            rows_found = 1;
        //board completed, fill board
        } else if (rows_found == 5) {
            for (unsigned int row=0; row<5; row++) {
                for (unsigned int col=0; col<5; col++) {
                    unsigned int start_of_board = cur_line-4;
                    int current_num = input_ints[start_of_board+row].elems[col];

                    boards[nr_of_boards_found].numbers[row][col] = current_num;
                }
            }
            nr_of_boards_found++;
            rows_found = 1;
        //row ok, continue this board
        } else {
            rows_found++;
        }
    }
    nr_of_boards = nr_of_boards_found;


    //check each board if bingo at current draw (i.e. round)
    //  if bingo is found, still check all boards, since another board could get bingo at the same time
    board *board;
    bool is_bingo = false;
    for (unsigned int draws=1; draws <= drawn_numbers_count; draws++) {
        for (unsigned int board_nr=0; board_nr<nr_of_boards; board_nr++) {
            board = &boards[board_nr];

            //a board can only get bingo once
            if (board->got_bingo) {
                continue;
            }

            is_bingo = check_bingo_rows(board, drawn_numbers, draws) ||
                        check_bingo_cols(board, drawn_numbers, draws);

            if (is_bingo) {
                board->got_bingo = true;
                board->score = get_score(board, drawn_numbers, draws);
                board->draws_to_win = draws;
                //TODO should also check that winner (on same draw) has more points than prev winner?
                //      (winner->draws_to_win == draws && board->score > winner->score)
                if (!winner || winner->draws_to_win == draws) {
                    winner = board;
                }
                last_winner_score = board->score; //part 2
                printf("Board %d got bingo after %d draws, with a score of %d\n", board_nr+1, board->draws_to_win, board->score);
            }
        }
        //if (winner) break; //for part 1 we can break here, and not check every draw
    }

    if (!winner) {
        printf("No winner found\n");
    } else {
        printf("\n\n\n");
        printf("part1: score is %d    (highest score after first draw)\n", winner->score);
        printf("part2: score is %d    (score for board which got bingo last)\n", last_winner_score);
    }

error:
    if (boards) {
        free(boards);
    }
    if (input_ints) {
        for (unsigned int i=0; i<lines_in_raw_input; i++) {
            free(input_ints[i].elems);
        }
        free(input_ints);
    }
    return 0;
}

