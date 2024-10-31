#pragma once
#include "bp_unit_cacl.h"
#include <Eigen/Dense>

void left_cacl(Eigen::MatrixXd &left_info, Eigen::MatrixXd &right_info, int N) {
    // 蝶形，从左向右迭代，最左边一列不进入for循环

    // 最大层数
    int max_layer = std::log(N);
    int step = N;
    // 内层循环步数
    int max_row = 1 << N;

    // 从右向左遍历
    for (int i = max_layer - 1; i > -1; i--)
    {
        // 该层步长
        step /= 2;
        

    }
    
}

void right_cacl(Eigen::MatrixXd &left_info, Eigen::MatrixXd &right_info,
                int N) {
    // 蝶形，从右向左迭代，最右边一列不进入for循环
}