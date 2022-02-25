cmake_minimum_required(VERSION 3.12)

project(crefl)

include(cmake/crefl_macro.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(CheckCXXCompilerFlag)

# llvm-config --cxxflags --ldflags --libs
find_program(LLVM_CONFIG NAMES llvm-config REQUIRED PATHS /usr/bin /opt/llvm/bin)
exec_program(${LLVM_CONFIG} ARGS --cxxflags OUTPUT_VARIABLE LLVM_CXXFLAGS)
exec_program(${LLVM_CONFIG} ARGS --ldflags OUTPUT_VARIABLE LLVM_LDFLAGS)
exec_program(${LLVM_CONFIG} ARGS --libs OUTPUT_VARIABLE LLVM_LIBS)

separate_arguments(LLVM_CXXFLAGS UNIX_COMMAND "${LLVM_CXXFLAGS}")
separate_arguments(LLVM_LDFLAGS UNIX_COMMAND "${LLVM_LDFLAGS}")
separate_arguments(LLVM_LIBS UNIX_COMMAND "${LLVM_LIBS}")

include_directories(include)

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

add_library(crefl SHARED src/reflect.cc)
target_compile_options(crefl PRIVATE ${LLVM_CXXFLAGS})
target_link_options(crefl PRIVATE ${LLVM_LDFLAGS})
target_link_libraries(crefl clang-cpp cmodel ${LLVM_LIBS})
if(APPLE)
target_link_libraries(crefl curses)
endif()

add_executable(crefltool tool/crefltool.cc)
target_link_libraries(crefltool cmodel)

add_executable(asn1tool tool/asn1tool.cc)
target_link_libraries(asn1tool cmodel)

# example using the crefl reflection file api
add_executable(example1_file samples/example1_file/main.c)
target_link_libraries(example1_file cmodel)

# example using the crefl reflection embedding
add_executable(example2_embed samples/example2_embed/main.c)
crefl_target_reflect(example2_embed example2_embed_refl)
target_link_libraries(example2_embed example2_embed_refl cmodel)

# example using the crefl embedding and printers
add_executable(example3_printer samples/example3_printer/main.c samples/example3_printer/printer.c)
crefl_target_reflect(example3_printer example3_printer_refl)
target_link_libraries(example3_printer example3_printer_refl cmodel)

add_executable(bench_asn1 test/bench_asn1.cc)
target_link_libraries(bench_asn1 cmodel)

add_executable(rand_vf128 test/rand_vf128.cc)
target_link_libraries(rand_vf128 cmodel)

enable_testing()

foreach(prog IN ITEMS t1 t2 t3 t4 t5 t6 t7 t8)
	add_executable(${prog} test/${prog}.c)
	target_link_libraries(${prog} cmodel)
	add_test(test_${prog} ${prog})
endforeach()
