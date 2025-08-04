#include "bp_unit_cacl.hpp"
#include <gtest/gtest.h>
#include <iostream>

#define TEST_OPEN 0

// 运行 Google Test 测试的 main 函数
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// 测试 (1, 1) 左上左信息计算
TEST(BpUnitCaclTest, LeftUpLeftInfoCacl) {
    double x = 0.0;
    double y = -6.5;
    double z = -2.0;
    double result = 1.8750;

    // 打印调试信息，包括函数名和 result 值
    std::cerr << "[" << __func__ << "] result: " << result << std::endl;

    double f_func_result = left_up_left_info_cacl(x, y, z);

    std::cerr << "[" << __func__ << "] f_func_result: " << f_func_result
              << std::endl;

    EXPECT_NEAR(result, f_func_result, 1e-4);
}

// 测试 (2, 1) 左下左信息计算
TEST(BpUnitCaclTest, LeftDownLeftInfoCacl) {
    double x = 0.0;
    double y = -2.0;
    double z = -6.5;
    double result = -6.5;

    // 打印调试信息，包括函数名和 result 值
    std::cerr << "[" << __func__ << "] result: " << result << std::endl;

    double f_func_result = left_down_left_info_cacl(x, y, z);

    std::cerr << "[" << __func__ << "] f_func_result: " << f_func_result
              << std::endl;

    EXPECT_NEAR(result, f_func_result, 1e-4);
}

// 测试 (1, 2) 右上右信息计算
TEST(BpUnitCaclTest, RightUpRightInfoCacl) {
    double x = 0.0;
    double y = 0.9375;
    double z = MAX_INF;
    double result = 0.8789;

    // 打印调试信息，包括函数名和 result 值
    std::cerr << "[" << __func__ << "] result: " << result << std::endl;

    double f_func_result = right_up_right_info_cacl(x, y, z);

    std::cerr << "[" << __func__ << "] f_func_result: " << f_func_result
              << std::endl;

    EXPECT_NEAR(result, f_func_result, 1e-4);
}

// 测试 (2, 2) 右下右信息计算
TEST(BpUnitCaclTest, RightDownRightInfoCacl) {
    double x = MAX_INF;
    double y = -3.75;
    double z = 0.0;
    double result = -3.515625;

    // 打印调试信息，包括函数名和 result 值
    std::cerr << "[" << __func__ << "] result: " << result << std::endl;

    double f_func_result = right_down_right_info_cacl(x, y, z);

    std::cerr << "[" << __func__ << "] f_func_result: " << f_func_result
              << std::endl;

    EXPECT_NEAR(result, f_func_result, 1e-4);
}