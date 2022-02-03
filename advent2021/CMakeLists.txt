#Building root dir (../) builds this project and runs code tests here as well.
#   NOTE: so only building this project will not generate any code checks...
#   TODO can we inherit tests from parent? So we don't need to run all tests every time.

cmake_minimum_required(VERSION 3.6)
project(advent2021 C)

#TODO make a loop for these... (see foreach)
add_executable(1.out read_input.c 1/1.c)
add_executable(2.out read_input.c 2/2.c)
add_executable(3.out read_input.c 3/3.c)
add_executable(4.out read_input.c 4/4.c)

#NOTE! A bit messy... the binaries need to be placed in their own directory as setup now
#       This is accomplished by the below lines and the command: make install (cmake --build does not "install")
#   (TODO how to do this with cmake instead of needing 'make install'?)
install(TARGETS 1.out RUNTIME DESTINATION ${CMAKE_CURRENT_LIST_DIR}/1/)
install(TARGETS 2.out RUNTIME DESTINATION ${CMAKE_CURRENT_LIST_DIR}/2/)
install(TARGETS 3.out RUNTIME DESTINATION ${CMAKE_CURRENT_LIST_DIR}/3/)
install(TARGETS 4.out RUNTIME DESTINATION ${CMAKE_CURRENT_LIST_DIR}/4/)

include_directories(.)
