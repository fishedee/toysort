CMAKE_MINIMUM_REQUIRED(VERSION 3.5)
PROJECT (HELLO)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)

SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -g -ggdb ")

SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall")

AUX_SOURCE_DIRECTORY(src SRC_LIST)

MESSAGE(STATUS "This is BINARY dir " ${HELLO_BINARY_DIR})

MESSAGE(STATUS "This is SOURCE dir " ${HELLO_SOURCE_DIR})

ADD_EXECUTABLE(toysort ${SRC_LIST})

SET(FASTSORT_INSERTION_THRESHOLD 32 CACHE STRING "FastSort insertion-sort threshold")
SET(FASTSORT_BLOCK_SIZE 32 CACHE STRING "FastSort partition block size")
OPTION(FASTSORT_SANITIZERS "Enable address/undefined sanitizers for FastSort targets" OFF)

ADD_LIBRARY(fastsort_core STATIC src/fastsort.cpp)
TARGET_INCLUDE_DIRECTORIES(fastsort_core PUBLIC src)
TARGET_COMPILE_DEFINITIONS(fastsort_core PRIVATE
    FASTSORT_INSERTION_THRESHOLD=${FASTSORT_INSERTION_THRESHOLD}
    FASTSORT_BLOCK_SIZE=${FASTSORT_BLOCK_SIZE})
TARGET_COMPILE_DEFINITIONS(toysort PRIVATE
    FASTSORT_INSERTION_THRESHOLD=${FASTSORT_INSERTION_THRESHOLD}
    FASTSORT_BLOCK_SIZE=${FASTSORT_BLOCK_SIZE})

SET(FASTSORT2_INSERTION_THRESHOLD 24 CACHE STRING "FastSort2 insertion-sort threshold")
SET(FASTSORT2_BLOCK_SIZE 64 CACHE STRING "FastSort2 partition block size")
ADD_LIBRARY(fastsort2_core STATIC src/fastsort2.cpp)
TARGET_INCLUDE_DIRECTORIES(fastsort2_core PUBLIC src)
FOREACH(target fastsort2_core toysort)
    TARGET_COMPILE_DEFINITIONS(${target} PRIVATE
        FASTSORT2_INSERTION_THRESHOLD=${FASTSORT2_INSERTION_THRESHOLD}
        FASTSORT2_BLOCK_SIZE=${FASTSORT2_BLOCK_SIZE})
ENDFOREACH()

ADD_EXECUTABLE(sort_benchmark benchmarks/sort_benchmark.cpp src/stdsort.cpp src/stdstablesort.cpp)
TARGET_INCLUDE_DIRECTORIES(sort_benchmark PRIVATE tests)
TARGET_LINK_LIBRARIES(sort_benchmark fastsort_core fastsort2_core)

INCLUDE(CTest)
IF(BUILD_TESTING)
    ADD_EXECUTABLE(fastsort2_test tests/fastsort_test.cpp)
    TARGET_COMPILE_DEFINITIONS(fastsort2_test PRIVATE TEST_FASTSORT2)
    TARGET_LINK_LIBRARIES(fastsort2_test fastsort2_core)
    ADD_TEST(NAME fastsort2_correctness COMMAND fastsort2_test)

    ADD_EXECUTABLE(fastsort2_heap_test tests/fastsort_test.cpp src/fastsort2.cpp)
    TARGET_INCLUDE_DIRECTORIES(fastsort2_heap_test PRIVATE src)
    TARGET_COMPILE_DEFINITIONS(fastsort2_heap_test PRIVATE TEST_FASTSORT2 FASTSORT2_FORCE_HEAP
        FASTSORT2_INSERTION_THRESHOLD=${FASTSORT2_INSERTION_THRESHOLD}
        FASTSORT2_BLOCK_SIZE=${FASTSORT2_BLOCK_SIZE})
    ADD_TEST(NAME fastsort2_heap_fallback COMMAND fastsort2_heap_test)

    ADD_EXECUTABLE(fastsort_test tests/fastsort_test.cpp)
    TARGET_LINK_LIBRARIES(fastsort_test fastsort_core)
    ADD_TEST(NAME fastsort_correctness COMMAND fastsort_test)

    ADD_EXECUTABLE(fastsort_heap_test tests/fastsort_test.cpp src/fastsort.cpp)
    TARGET_INCLUDE_DIRECTORIES(fastsort_heap_test PRIVATE src)
    TARGET_COMPILE_DEFINITIONS(fastsort_heap_test PRIVATE FASTSORT_FORCE_HEAP
        FASTSORT_INSERTION_THRESHOLD=${FASTSORT_INSERTION_THRESHOLD}
        FASTSORT_BLOCK_SIZE=${FASTSORT_BLOCK_SIZE})
    ADD_TEST(NAME fastsort_heap_fallback COMMAND fastsort_heap_test)
ENDIF()

IF(FASTSORT_SANITIZERS)
    IF(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        MESSAGE(FATAL_ERROR "FASTSORT_SANITIZERS requires Clang or GCC")
    ENDIF()
    FOREACH(target fastsort_core fastsort2_core sort_benchmark fastsort_test fastsort_heap_test fastsort2_test fastsort2_heap_test)
        IF(TARGET ${target})
            TARGET_COMPILE_OPTIONS(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
            SET_PROPERTY(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " -fsanitize=address,undefined")
        ENDIF()
    ENDFOREACH()
ENDIF()
