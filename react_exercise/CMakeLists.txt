#Building root dir (../) builds this project and runs code tests here as well.
#   NOTE: so only building this project will not generate any code checks...
#   TODO can we inherit tests from parent? So we don't need to run all tests every time.

cmake_minimum_required(VERSION 3.6)
project(react C)

#This exercise does not have main, only run with test suite from Exercism.io ..
#   Let's build as shared ("dynamic") library for now
add_library(react SHARED react.c)
add_library(react_alternative SHARED react_alternative.c)

include_directories(.)
