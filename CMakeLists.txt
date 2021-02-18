cmake_minimum_required(VERSION 3.12)

project(crefl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# llvm-config --cxxflags --ldflags --libs
find_program(LLVM_CONFIG NAMES llvm-config REQUIRED PATHS /usr/bin)
exec_program(${LLVM_CONFIG} ARGS --cxxflags OUTPUT_VARIABLE LLVM_CXXFLAGS)
exec_program(${LLVM_CONFIG} ARGS --ldflags OUTPUT_VARIABLE LLVM_LDFLAGS)
exec_program(${LLVM_CONFIG} ARGS --libs OUTPUT_VARIABLE LLVM_LIBS)

separate_arguments(LLVM_CXXFLAGS UNIX_COMMAND "${LLVM_CXXFLAGS}")
separate_arguments(LLVM_LDFLAGS UNIX_COMMAND "${LLVM_LDFLAGS}")
separate_arguments(LLVM_LIBS UNIX_COMMAND "${LLVM_LIBS}")

include_directories(src)

add_library(cmodel STATIC src/cmodel.cc src/ctypes.cc src/cdump.cc src/cfileio.cc)

add_library(crefl SHARED src/creflect.cc)
target_compile_options(crefl PRIVATE ${LLVM_CXXFLAGS})
target_link_options(crefl PRIVATE ${LLVM_LDFLAGS})
target_link_libraries(crefl -lclang cmodel ${LLVM_LIBS})

add_executable(crefltool tool/crefltool.cc)
target_link_libraries(crefltool cmodel)

enable_testing()

foreach(prog IN ITEMS t1 t2 t3)
	add_executable(${prog} test/${prog}.c)
	target_link_libraries(${prog} cmodel)
	add_test(test_${prog} ${prog})
endforeach()
