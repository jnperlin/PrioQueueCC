# -------------------------------------------------------------------------------------------
# priority queue unit tests
# -------------------------------------------------------------------------------------------
# this file is part of "PrioQueueCC" by J.Perlinger.
#
# PrioQueueCC by J.Perlinger is marked CC0 1.0. To view a copy of this mark,
#    visit https://creativecommons.org/publicdomain/zero/1.0/
#
# -------------------------------------------------------------------------------------------
# The code is meant to be integrated by compiling and linking in any way that suits the
# environment.  This is just a little test bed, requiring Google Test to be available.
# -------------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.10.0)
project(PairingHeapCC VERSION 0.1.0 LANGUAGES C CXX)

find_package(GTest REQUIRED)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-fsanitize=address HAS_ASAN)
if (HAS_ASAN)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/inc
    ${GTEST_INCLUDE_DIRS}
)

add_library(PairingHeapCC STATIC
    src/phqueue2.cpp src/phq2check.cpp
    src/phqueue3.cpp src/phq3check.cpp
    src/lhqueue2.cpp src/lhq2check.cpp
    src/mdqueue3.cpp src/mdq3check.cpp
    src/PointerMap.cpp)

add_executable(pq_tests test/test_mindist.cpp test/test_pairing.cpp)
target_link_libraries(pq_tests PRIVATE PairingHeapCC GTest::gtest_main)

include(CTest)
enable_testing()
add_test(pq_tests pq_tests)
