cmake_minimum_required(VERSION 3.10)
project(PolarCodeBPDecode)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# === 头文件路径 ===
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/Eigen
    ${PROJECT_SOURCE_DIR}/include/fmt
)

# === 指定源文件（不包含 main.cpp） ===
set(SRC_FILES
    src/bit_flip_simulator.cpp
    src/config.cpp
    src/dataset_generator.cpp
    src/simulate.cpp
    # 如有其他源文件，继续添加
)

# === 创建静态库 ===
add_library(polar_bp_decode_lib STATIC ${SRC_FILES})

# === 主程序入口 ===
add_executable(polar_bp_decode src/main.cpp)
target_link_libraries(polar_bp_decode PRIVATE polar_bp_decode_lib)

# === 设置主程序输出目录为 run.sh 所在的根目录 ===
set_target_properties(polar_bp_decode PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
)

# === 添加 Google Test ===
add_subdirectory(third_party/googletest)

# === 启用测试支持 ===
enable_testing()

# === 自动查找所有测试文件 ===
file(GLOB TEST_SOURCES "${PROJECT_SOURCE_DIR}/tests/unit/*.cpp")

foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE gtest gtest_main polar_bp_decode_lib)
    set_target_properties(${TEST_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/tests/unit/bin)
    add_test(NAME ${TEST_NAME} COMMAND ${PROJECT_SOURCE_DIR}/tests/unit/bin/${TEST_NAME})
endforeach()