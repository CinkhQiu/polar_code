#pragma once

#include <Eigen/Dense>
#include <cmath>
#include <iostream>

const double ALPHA = 0.9375;
const double MAX_INF = 1e6;
const double MIN_INF = -1e6;

// 符号函数
inline double sign(double x) { return (x > 0) ? 1.0 : ((x < 0) ? -1.0 : 0.0); }

// 返回绝对值的最小值
inline double min_abs(double x, double y) {
    return (std::abs(x) < std::abs(y)) ? std::abs(x) : std::abs(y);
}

// f函数计算公式
inline double f_func(double x, double y) {
    return ALPHA * sign(x) * sign(y) * min_abs(x, y);
}

// 向左迭代，半次计算

// (1, 1)左上左信息计算
inline double left_up_left_info_cacl(double right_left_down,
                                     double left_right_down,
                                     double left_right_up) {
    return f_func(right_left_down + left_right_down, left_right_up);
}

// (2, 1)左下左信息计算
inline double left_down_left_info_cacl(double right_left_up,
                                       double left_right_up,
                                       double left_right_down) {
    return f_func(right_left_up, left_right_up) + left_right_down;
}

// 向右迭代，半次计算

// (1, 2)右上右信息计算
inline double right_up_right_info_cacl(double right_left_down,
                                       double left_right_down,
                                       double right_left_up) {
    return f_func(right_left_down + left_right_down, right_left_up);
}

// (2, 2)右下右信息计算
inline double right_down_right_info_cacl(double right_left_up,
                                         double left_right_up,
                                         double right_left_down) {
    return f_func(right_left_up, left_right_up) + right_left_down;
}