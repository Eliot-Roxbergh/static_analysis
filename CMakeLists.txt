#Copyright 2022 Eliot Roxbergh. Licensed under AGPLv3 as per separate LICENSE file.
#Example to build this: cmake --clean-first -DTEST_ALL=true . && cmake --build . && make clang-format
#Tested with gcc 7.5.0

cmake_minimum_required(VERSION 3.6)
project(examples C)

set(MEMORYCHECK_COMMAND_OPTIONS "--error-exitcode=1") #fail on leak
include(CTest)

set(C_STD gnu11)
#set(CMAKE_C_COMPILER /usr/bin/gcc)
#TODO we set debug (-g) here, should just use -DCMAKE_BUILD_TYPE=Debug instead .. or similar

#-DCMAKE_BUILD_TYPE=Debug
IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("Debug build")
    set(C_FLAGS_DEBUG    "-g -fasynchronous-unwind-tables -fexceptions")
    set(C_FLAGS_WARNINGS "-Wall -Wextra -pedantic -Werror -Wformat=2  -Wconversion")
#-DCMAKE_BUILD_TYPE=Release
ELSE()
    message("Release build")
    set(C_FLAGS_SECURITY "-D_FORTIFY_SOURCE=2  -D_GLIBCXX_ASSERTIONS -fstack-protector-strong -Wl,-z,noexecstack -Wl,-z,now -Wl,-z,relro -Wl,-z,defs")
    #set(C_FLAGS_SECURITY_EXEC "-fpie -Wl,-pie")
    #set(C_FLAGS_SECURITY_LIB  "-fpic")
ENDIF()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=${C_STD} ${C_FLAGS_DEBUG} ${C_FLAGS_WARNINGS} ${C_FLAGS_SECURITY}")

add_executable(array         array.c)
add_executable(bit_manip     bit_manip.c)
add_executable(enum          enum.c)
add_executable(positives positives.c)
add_executable(function_ptrs function_ptrs.c)
add_executable(ptrs          ptrs.c)
add_executable(ptrs2         ptrs2.c)

add_test(array array)
add_test(bit_manip bit_manip)
add_test(enum enum)
add_test(positives positives)
add_test(function_ptrs function_ptrs)
add_test(ptrs ptrs)
add_test(ptrs2 ptrs2)

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



# Extra, option to use clang (et al) front-end, CodeChecker
# Use clang-7, clang-tidy-7 or later
# pip3 install --user codechecker
add_custom_target(
  codechecker_analyze
  COMMAND CodeChecker analyze "compile_commands.json" -o reports;
  --enable sensitive;
  --clean;
  #--ctu;
  #--stats;
)
add_custom_target(
  codechecker
  DEPENDS codechecker_analyze
  COMMAND CodeChecker parse ./reports;
)
add_custom_target(
  codechecker-html
  DEPENDS codechecker_analyze
  COMMAND CodeChecker parse ./reports -e html -o ./reports_html
)
