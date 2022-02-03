#Copyright 2022 Eliot Roxbergh. Licensed under AGPLv3 as per separate LICENSE file.
#Example to build this: cmake --clean-first -DTEST_ALL=true . && cmake --build . && make clang-format
#Tested with gcc 7.5.0

cmake_minimum_required(VERSION 3.6)
project(examples C)


set(C_STD c11)
#set(CMAKE_C_COMPILER /usr/bin/gcc)
#TODO we set debug (-g) here, should just use -DCMAKE_BUILD_TYPE=Debug instead .. or similar
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=${C_STD} -g -fasynchronous-unwind-tables -fexceptions -Wall -Werror -pedantic -Wconversion -Wextra -Wformat=2 -Wformat-truncation -Wunused -Werror=implicit-function-declaration -Werror=format-security")

add_executable(array         array.c)
add_executable(bit_manip     bit_manip.c)
add_executable(enum          enum.c)
add_executable(positives positives.c)
add_executable(function_ptrs function_ptrs.c)
add_executable(ptrs          ptrs.c)
add_executable(ptrs2         ptrs2.c)

#Also build these projects
add_subdirectory(advent2021)
add_subdirectory(react_exercise)


###############################################
#Enable static analysis with -DTEST_ALL=true
#   Some inspiration:
#       https://www.kitware.com/static-checks-with-cmake-cdash-iwyu-clang-tidy-lwyu-cpplint-and-cppcheck/
#       (https://www.youtube.com/watch?v=rLopVhns4Zs&t=4633s)
#   Requires: sudo apt install    clang clang-tidy clang-format cppcheck iwyu
#             pip3 install --user cpplint
#   For C++, remember to change _C_ to _CXX_
if (TEST_ALL)
    set(TEST_CLANG true)
    set(TEST_CPPCHECK true)
    set(TEST_IWYU true)
    #set(TEST_EXTRAS true)
endif()
#
if (TEST_CLANG)
    #comment, it is also possible to enable experimental alpha checks:
    #   https://clang-analyzer.llvm.org/alpha_checks.html#security_alpha_checkers
    set(CMAKE_C_CLANG_TIDY
        clang-tidy;
        -header-filter=.*;
        -checks=*;
        -extra-arg=-Wno-error=unknown-warning-option;
       )
    set(CMAKE_LINK_WHAT_YOU_USE TRUE)
endif()
if (TEST_CPPCHECK)
    set(CMAKE_C_CPPCHECK
        cppcheck;
        --enable=warning;
        --inconclusive;
        --force;
        --inline-suppr;
        --std=${C_STD};
        #"--suppressions-list=${CMAKE_SOURCE_DIR}/CppCheckSuppressions.txt";
       )
endif()
if (TEST_IWYU)
    set(CMAKE_C_INCLUDE_WHAT_YOU_USE
        iwyu;
       )
endif()


#TODO it must be possible to list all source files cmake uses?
#file(GLOB_RECURSE SOURCE_FILES RELATIVE ${CMAKE_SOURCE_DIR} "*.c" "*.h")
file(GLOB
    SOURCE_FILES
    LIST_DIRECTORIES true
    ${CMAKE_SOURCE_DIR}/*.[ch]
    ${CMAKE_SOURCE_DIR}/react_exercise/*.[ch]
    ${CMAKE_SOURCE_DIR}/react_exercise/*/*.[ch]
    ${CMAKE_SOURCE_DIR}/advent2021/*.[ch]
    ${CMAKE_SOURCE_DIR}/advent2021/*/*.[ch]
)
#clang-format on all source files, including subdirectories
#Run with, make clang-format (NOTE! This changes the source files directly)
add_custom_target(
    clang-format
    COMMAND clang-format
    -style=file #see file .clang-format
    -i ${SOURCE_FILES}
)
#cpplint mainly gives minor style comments.
# Note unlike clang-format, it does NOT modify any file.
#It is also possible run cpplint with CMAKE_C_CPPLINT
add_custom_target(
    cpplint
    COMMAND cpplint;
    --filter=-build/header_guard,-readability/todo;
    ${SOURCE_FILES};
    #To exclude specific error ids but not the whole category; (is there a good way of doing this?)
    #(The results we want to parse are by default in stderr)
    2>&1 | grep -v -e "whitespace/braces...4" -e "readability/casting...4" ;
)
#splint, note this gives a lot of warnings and requires some thought.
# (there's also -weak for a weaker check)
# For instance, it wants the phrase /*@null@*/ before each function that can return NULL, ...
add_custom_target(
    splint
    COMMAND splint;
    +ptrnegate;
    ${SOURCE_FILES};
)
#TODO infer also looks like an interesting analyzer
#TODO most these tools are supported in CodeChecker, easier to get an overview with that front-end
#       https://github.com/facebook/infer
#CodeQL and Semgrep are run via CI so not added here.

