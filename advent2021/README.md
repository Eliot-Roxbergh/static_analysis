Advent of code 2021 <https://adventofcode.com/2021>

A few quick working solutions for AoC2021; they return the correct answer and have no memory leaks or memory errors.

Still they are not bug-free or necessarily safe.



Originally built with:

_gcc -std=c11 -Wall -Werror -pedantic -Wconversion -g -fasynchronous-unwind-tables -fexceptions -Wall -Werror -pedantic -Wconversion -Wextra -Wformat=2 -Wformat-truncation -Wunused -Werror=implicit-function-declaration -Werror=format-security_

_valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes_

gcc version 7.5.0

valgrind-3.13.0
