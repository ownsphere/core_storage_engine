cmake_minimum_required(VERSION 3.10)
project(OwnSphereEngine)

set(CMAKE_CXX_STANDARD 17)

option(ENABLE_COVERAGE "Enable coverage" ON)

include_directories(include)

# ---------------- CORE LIBRARY ----------------
add_library(engine_lib
    src/engine/storage_engine.cpp
    src/storage/chunk_manager.cpp
    src/storage/metadata_manager.cpp
    src/crypto/checksum.cpp
    src/crypto/encryption.cpp
    src/utils/logger.cpp
)

# ---------------- MAIN EXECUTABLE ----------------
add_executable(engine
    src/main.cpp
)

target_link_libraries(engine engine_lib)

# ---------------- TESTING ----------------
#enable_testing()

#find_package(GTest REQUIRED)

#add_executable(runTests
 #   tests/test_storage_engine.cpp
  #  tests/test_checksum.cpp
#)

#target_link_libraries(runTests
 #   PRIVATE
  #  engine_lib
   # GTest::gtest
    #GTest::gtest_main
#)

#include(GoogleTest)
#gtest_discover_tests(runTests)

# ---------------- COVERAGE (MOVE HERE) ----------------
if(ENABLE_COVERAGE)
    message(STATUS "Coverage enabled")

    # engine_lib
    target_compile_options(engine_lib PRIVATE --coverage -O0 -g)
    target_link_options(engine_lib PRIVATE --coverage)

    # runTests
    target_compile_options(runTests PRIVATE --coverage -O0 -g)
    target_link_options(runTests PRIVATE --coverage)

    # 🔥 FIX: engine also needs coverage
    target_compile_options(engine PRIVATE --coverage -O0 -g)
    target_link_options(engine PRIVATE --coverage)
endif()