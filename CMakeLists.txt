cmake_minimum_required(VERSION 3.10)
project(PolarCodeBPDecode)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 包含头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 主程序配置：创建库和可执行文件
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
add_library(polar_bp_decode_lib STATIC ${SOURCES})
add_executable(polar_bp_decode ${SOURCES})  # 主程序可执行文件
target_link_libraries(polar_bp_decode PRIVATE polar_bp_decode_lib)

# 将主程序可执行文件输出到 bin 目录
set_target_properties(polar_bp_decode PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# 废弃：添加系统中 Google Test
# find_package(GTest REQUIRED)
# include_directories(${GTEST_INCLUDE_DIRS})

add_subdirectory(third_party/googletest)

# 启用测试
enable_testing()

# 添加测试文件
file(GLOB TEST_SOURCES "${PROJECT_SOURCE_DIR}/tests/unit/*.cpp")

foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE gtest gtest_main polar_bp_decode_lib)
    set_target_properties(${TEST_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/tests/unit/bin)
    add_test(NAME ${TEST_NAME} COMMAND ${PROJECT_SOURCE_DIR}/tests/unit/bin/${TEST_NAME})
endforeach()