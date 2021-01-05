cmake_minimum_required(VERSION 3.17)
project(swamp_typeinfo C)

set(CMAKE_C_STANDARD 99)

set(deps ../deps/)

file(GLOB_RECURSE deps_src FOLLOW_SYMLINKS
        "${deps}*/src/lib/*.c"
        )

file(GLOB_RECURSE lib_src FOLLOW_SYMLINKS
        "lib/*.c"
        )

add_library(swamp_typeinfo
    ${deps_src}
    ${lib_src}
    examples/main.c
)

target_compile_options(swamp_typeinfo PRIVATE -Wall -Wextra -Wshadow -Weffc++ -Wstrict-aliasing -ansi -pedantic -Wno-unused-function -Wno-unused-parameter)


target_include_directories(swamp_typeinfo PRIVATE ${deps}runtime-c/src/include)
target_include_directories(swamp_typeinfo PRIVATE ${deps}tiny-libc/src/include)
target_include_directories(swamp_typeinfo PRIVATE ${deps}flood-c/src/include)
target_include_directories(swamp_typeinfo PRIVATE ${deps}clog/src/include)
target_include_directories(swamp_typeinfo PUBLIC include)


target_link_libraries(swamp_typeinfo m)
