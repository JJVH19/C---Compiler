cmake_minimum_required(VERSION 3.22)
project(Milestone_1 C)

set(CMAKE_C_STANDARD 11)

add_executable(Milestone_1
        parser.c
        scanner.c
        driver.c
        semantics.c
        ast-print.c
)