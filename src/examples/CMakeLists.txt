cmake_minimum_required(VERSION 3.17)
project(swamp_typeinfo_example C)

set(CMAKE_C_STANDARD 99)

set(deps ../../deps/)

add_executable(swamp_typeinfo_example
    main.c
)

target_include_directories(swamp_typeinfo_example PRIVATE ${deps}clog/src/include)
target_include_directories(swamp_typeinfo_example PRIVATE ${deps}tiny-libc/src/include)
target_include_directories(swamp_typeinfo_example PRIVATE ${deps}flood-c/src/include)

target_link_libraries(swamp_typeinfo_example swamp_typeinfo m)
