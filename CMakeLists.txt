cmake_minimum_required(VERSION 3.10)

project(gitsql)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(SRC_FILES
	src/gitsql.cpp
	src/git.cpp
	src/table.cpp
	src/ldap.cpp)

add_executable(gitsql ${SRC_FILES})

find_package(tdscpp REQUIRED)
find_package(libgit2 REQUIRED)

target_link_libraries(gitsql tdscpp)
target_link_libraries(gitsql libgit2)
target_link_libraries(gitsql wldap32)

if(NOT MSVC)
	target_compile_options(gitsql PUBLIC -Wall -Werror=cast-function-type -Wno-expansion-to-defined -Wunused-parameter -Wtype-limits -Wextra -Wconversion)
endif()
