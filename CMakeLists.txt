cmake_minimum_required(VERSION 3.12)

project(cinf)

include(cmake/cinf_macro.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Clang REQUIRED CONFIG)

include_directories(include)
include_directories(${CLANG_INCLUDE_DIRS})

set(LLVM_LINK_COMPONENTS upport)

add_library(cmodel STATIC
	src/asn1.cc
	src/buf.cc
	src/dump.cc
	src/db.cc
	src/link.cc
	src/model.cc
	src/oid.cc
	src/types.cc
	src/sha256.cc
)

add_library(cinf SHARED src/reflect.cc)
target_link_libraries(cinf PRIVATE cmodel clangAST clangFrontend)
if(APPLE)
target_link_libraries(cinf curses)
endif()

add_executable(cinftool tool/cinftool.cc)
target_link_libraries(cinftool cmodel)

add_executable(asn1tool tool/asn1tool.cc)
target_link_libraries(asn1tool cmodel)

# example using the cinf reflection file api
add_executable(example1_file samples/example1_file/main.c)
target_link_libraries(example1_file cmodel)

# example using the cinf reflection embedding
add_executable(example2_embed samples/example2_embed/main.c)
cinf_target_reflect(example2_embed example2_embed_inf)
target_link_libraries(example2_embed example2_embed_inf cmodel)

# example using the cinf embedding and printers
add_executable(example3_printer samples/example3_printer/main.c samples/example3_printer/printer.c)
cinf_target_reflect(example3_printer example3_printer_inf)
target_link_libraries(example3_printer example3_printer_inf cmodel)

add_executable(bench_asn1 test/bench_asn1.cc)
target_link_libraries(bench_asn1 cmodel)

enable_testing()

foreach(prog IN ITEMS t1 t2 t3 t4 t5 t6 t7)
	add_executable(${prog} test/${prog}.c)
	target_link_libraries(${prog} cmodel)
	add_test(test_${prog} ${prog})
endforeach()
