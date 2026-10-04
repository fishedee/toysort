CMAKE_MINIMUM_REQUIRED(VERSION 3.16)
PROJECT (HELLO)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)

SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -g -ggdb ")

SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall")

AUX_SOURCE_DIRECTORY(src SRC_LIST)
LIST(REMOVE_ITEM SRC_LIST src/fastsort3.cpp src/fastsort4.cpp)

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

SET(FASTSORT3_INSERTION_THRESHOLD 24 CACHE STRING "FastSort3 insertion-sort threshold")
SET(FASTSORT3_BLOCK_SIZE 64 CACHE STRING "FastSort3 partition block size")
OPTION(FASTSORT3_ENABLE_SIMD "Enable FastSort3 NEON / runtime-dispatched AVX2" ON)
ADD_LIBRARY(fastsort3_core STATIC src/fastsort3.cpp)
TARGET_INCLUDE_DIRECTORIES(fastsort3_core PUBLIC src)
FOREACH(target fastsort3_core toysort)
    TARGET_COMPILE_DEFINITIONS(${target} PRIVATE
        FASTSORT3_INSERTION_THRESHOLD=${FASTSORT3_INSERTION_THRESHOLD}
        FASTSORT3_BLOCK_SIZE=${FASTSORT3_BLOCK_SIZE}
        FASTSORT3_ENABLE_SIMD=$<BOOL:${FASTSORT3_ENABLE_SIMD}>)
ENDFOREACH()

OPTION(FASTSORT4_ENABLE_METAL "Enable FastSort4 Metal GPU backend on macOS" ${APPLE})
SET(FASTSORT4_GPU_THRESHOLD 100000 CACHE STRING "FastSort4 minimum GPU input size")
SET(FASTSORT4_BLOCK_SIZE 128 CACHE STRING "FastSort4 Metal block size: 128/256/512")
SET(FASTSORT4_MERGE_ITEMS 8 CACHE STRING "FastSort4 outputs per merge thread: 4/8/16")
ADD_LIBRARY(fastsort4_core STATIC src/fastsort4.cpp)
TARGET_INCLUDE_DIRECTORIES(fastsort4_core PUBLIC src)
TARGET_LINK_LIBRARIES(fastsort4_core PUBLIC fastsort3_core)
TARGET_COMPILE_DEFINITIONS(fastsort4_core PRIVATE
    FASTSORT4_GPU_THRESHOLD=${FASTSORT4_GPU_THRESHOLD}
    FASTSORT4_BLOCK_SIZE=${FASTSORT4_BLOCK_SIZE}
    FASTSORT4_MERGE_ITEMS=${FASTSORT4_MERGE_ITEMS})
IF(APPLE AND FASTSORT4_ENABLE_METAL)
    ENABLE_LANGUAGE(OBJCXX)
    SET(CMAKE_OBJCXX_STANDARD 14)
    SET(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
    TARGET_SOURCES(fastsort4_core PRIVATE src/metal/fastsort4_backend.mm)
    SET_SOURCE_FILES_PROPERTIES(src/metal/fastsort4_backend.mm PROPERTIES COMPILE_FLAGS "-fobjc-arc")
    TARGET_COMPILE_DEFINITIONS(fastsort4_core PRIVATE FASTSORT4_HAS_METAL)
    TARGET_LINK_LIBRARIES(fastsort4_core PUBLIC "-framework Foundation" "-framework Metal")
ENDIF()
TARGET_LINK_LIBRARIES(toysort fastsort4_core)

ADD_EXECUTABLE(sort_benchmark benchmarks/sort_benchmark.cpp src/stdsort.cpp src/stdstablesort.cpp)
TARGET_INCLUDE_DIRECTORIES(sort_benchmark PRIVATE tests)
TARGET_LINK_LIBRARIES(sort_benchmark fastsort_core fastsort2_core fastsort4_core)

INCLUDE(CTest)
IF(BUILD_TESTING)
    ADD_EXECUTABLE(fastsort4_test tests/fastsort_test.cpp)
    TARGET_COMPILE_DEFINITIONS(fastsort4_test PRIVATE TEST_FASTSORT4)
    TARGET_LINK_LIBRARIES(fastsort4_test fastsort4_core)
    ADD_TEST(NAME fastsort4_correctness COMMAND fastsort4_test)

    ADD_EXECUTABLE(fastsort4_gpu_test tests/fastsort4_gpu_test.cpp)
    TARGET_LINK_LIBRARIES(fastsort4_gpu_test fastsort4_core)
    ADD_TEST(NAME fastsort4_gpu COMMAND fastsort4_gpu_test)
    SET_TESTS_PROPERTIES(fastsort4_gpu PROPERTIES SKIP_RETURN_CODE 77)

    ADD_EXECUTABLE(fastsort4_failure_test tests/fastsort4_failure_test.cpp src/fastsort4.cpp)
    TARGET_COMPILE_DEFINITIONS(fastsort4_failure_test PRIVATE FASTSORT4_HAS_METAL FASTSORT4_GPU_THRESHOLD=2)
    TARGET_LINK_LIBRARIES(fastsort4_failure_test fastsort3_core)
    ADD_TEST(NAME fastsort4_failure COMMAND fastsort4_failure_test)

    ADD_EXECUTABLE(fastsort3_test tests/fastsort_test.cpp)
    TARGET_COMPILE_DEFINITIONS(fastsort3_test PRIVATE TEST_FASTSORT3)
    TARGET_LINK_LIBRARIES(fastsort3_test fastsort3_core)
    ADD_TEST(NAME fastsort3_correctness COMMAND fastsort3_test)

    FOREACH(variant scalar heap)
        ADD_EXECUTABLE(fastsort3_${variant}_test tests/fastsort_test.cpp src/fastsort3.cpp)
        TARGET_INCLUDE_DIRECTORIES(fastsort3_${variant}_test PRIVATE src)
        TARGET_COMPILE_DEFINITIONS(fastsort3_${variant}_test PRIVATE TEST_FASTSORT3
            FASTSORT3_INSERTION_THRESHOLD=${FASTSORT3_INSERTION_THRESHOLD}
            FASTSORT3_BLOCK_SIZE=${FASTSORT3_BLOCK_SIZE})
        ADD_TEST(NAME fastsort3_${variant}_fallback COMMAND fastsort3_${variant}_test)
    ENDFOREACH()
    TARGET_COMPILE_DEFINITIONS(fastsort3_scalar_test PRIVATE FASTSORT3_ENABLE_SIMD=0)
    TARGET_COMPILE_DEFINITIONS(fastsort3_heap_test PRIVATE FASTSORT3_FORCE_HEAP
        FASTSORT3_ENABLE_SIMD=$<BOOL:${FASTSORT3_ENABLE_SIMD}>)

    ADD_EXECUTABLE(fastsort3_masks_test tests/fastsort3_masks_test.cpp)
    TARGET_INCLUDE_DIRECTORIES(fastsort3_masks_test PRIVATE src)
    TARGET_COMPILE_DEFINITIONS(fastsort3_masks_test PRIVATE
        FASTSORT3_ENABLE_SIMD=$<BOOL:${FASTSORT3_ENABLE_SIMD}>)
    ADD_TEST(NAME fastsort3_masks COMMAND fastsort3_masks_test)

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
    # toysort links instrumented FastSort3/4 libraries; leave legacy sources uninstrumented.
    SET_PROPERTY(TARGET toysort APPEND_STRING PROPERTY LINK_FLAGS " -fsanitize=address,undefined")
    FOREACH(target fastsort4_core fastsort4_test fastsort4_gpu_test fastsort4_failure_test fastsort3_core fastsort3_test fastsort3_scalar_test fastsort3_heap_test fastsort3_masks_test fastsort_core fastsort2_core sort_benchmark fastsort_test fastsort_heap_test fastsort2_test fastsort2_heap_test)
        IF(TARGET ${target})
            TARGET_COMPILE_OPTIONS(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
            SET_PROPERTY(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " -fsanitize=address,undefined")
        ENDIF()
    ENDFOREACH()
ENDIF()
