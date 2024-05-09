cmake_minimum_required(VERSION 3.12)

project(gitsql)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(SRC_FILES
	src/gitsql.cpp
	src/git.cpp
	src/table.cpp
	src/ldap.cpp
	src/lex.cpp
	src/parse.cpp
	src/master.cpp
	src/aes.cpp)

add_executable(gitsql ${SRC_FILES})

find_package(tdscpp REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(ZLIB REQUIRED)
find_package(nlohmann_json REQUIRED)

pkg_check_modules(LIBGIT2 REQUIRED IMPORTED_TARGET libgit2)

add_definitions(-DNOMINMAX)

target_link_libraries(gitsql tdscpp)
target_link_libraries(gitsql ZLIB::ZLIB)
target_link_libraries(gitsql nlohmann_json::nlohmann_json)

if(WIN32)
	target_link_libraries(gitsql wldap32)
else()
	pkg_check_modules(LDAP REQUIRED IMPORTED_TARGET ldap)
	target_link_libraries(gitsql PkgConfig::LDAP)
endif()

target_link_libraries(gitsql PkgConfig::LIBGIT2)

if(NOT MSVC)
	target_compile_options(gitsql PUBLIC -Wall -Werror=cast-function-type -Wno-expansion-to-defined -Wunused-parameter -Wtype-limits -Wextra -Wconversion)

	if(WIN32)
		target_link_options(gitsql PUBLIC -municode)
	endif()
endif()

install(TARGETS gitsql DESTINATION "${CMAKE_INSTALL_BINDIR}")

if(MSVC)
	install(FILES $<TARGET_PDB_FILE:gitsql> DESTINATION bin OPTIONAL)
endif()
