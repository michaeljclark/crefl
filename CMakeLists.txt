cmake_minimum_required(VERSION 3.12)

project(crefl)

include(cmake/crefl_macro.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(CheckCXXCompilerFlag)

# We need haswell for TZCNT/LZCNT
check_cxx_compiler_flag("-march=haswell" has_march_haswell "int main() { return 0; }")
if (has_march_haswell)
	list(APPEND CMAKE_C_FLAGS -march=haswell)
	list(APPEND CMAKE_CXX_FLAGS -march=haswell)
endif()

# llvm-config --cxxflags --ldflags --libs
find_program(LLVM_CONFIG NAMES llvm-config REQUIRED PATHS /usr/bin)
exec_program(${LLVM_CONFIG} ARGS --cxxflags OUTPUT_VARIABLE LLVM_CXXFLAGS)
exec_program(${LLVM_CONFIG} ARGS --ldflags OUTPUT_VARIABLE LLVM_LDFLAGS)
exec_program(${LLVM_CONFIG} ARGS --libs OUTPUT_VARIABLE LLVM_LIBS)

separate_arguments(LLVM_CXXFLAGS UNIX_COMMAND "${LLVM_CXXFLAGS}")
separate_arguments(LLVM_LDFLAGS UNIX_COMMAND "${LLVM_LDFLAGS}")
separate_arguments(LLVM_LIBS UNIX_COMMAND "${LLVM_LIBS}")

include_directories(src)

add_library(cmodel STATIC
	src/casn1.cc
	src/cbuf.cc
	src/cdump.cc
	src/cfileio.cc
	src/clink.cc
	src/cmodel.cc
	src/coid.cc
	src/ctypes.cc
	src/sha256.c
)

add_library(crefl SHARED src/creflect.cc)
target_compile_options(crefl PRIVATE ${LLVM_CXXFLAGS})
target_link_options(crefl PRIVATE ${LLVM_LDFLAGS})
target_link_libraries(crefl -lclang cmodel ${LLVM_LIBS})

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

add_executable(bench_asn1 test/bench_asn1.cc)
target_link_libraries(bench_asn1 cmodel)

enable_testing()

foreach(prog IN ITEMS t1 t2 t3 t4 t5 t6)
	add_executable(${prog} test/${prog}.c)
	target_link_libraries(${prog} cmodel)
	add_test(test_${prog} ${prog})
endforeach()
