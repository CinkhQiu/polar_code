#pragma once
#include "bp_unit_cacl.h"
#include <Eigen/Dense>

void left_cacl(Eigen::MatrixXd &left_info, Eigen::MatrixXd &right_info, int N) {
    // 蝶形，从右向左迭代，最右边一列不进入for循环
    for (int col = left_info.cols() - 2; col > -1; col--) {
        // 每列的计算组数
        int groups = N >> (col + 1);
        // 计算步长
        int step = N / groups;
        // 下方行索引偏移值
        int bias = step >> 1;
        // 每组的循环最大数量
        int limit = step >> 1;

        // 起始行索引
        int init_row = 0;
        // 对列进行分组计算
        for (int i = 0; i < groups; i++) {
            int row = init_row; // 起始行索引
            // 组内的行进行for循环计算
            for (row; row < limit; row++) {
                // 更新左上，左信息
                left_info(row, col) = left_up_left_info_cacl(
                    right_info(row + bias, col), left_info(row + bias, col + 1),
                    left_info(row, col + 1));
                // 更新左下，左信息
                left_info(row + bias, col) = left_down_left_info_cacl(
                    right_info(row, col), left_info(row, col + 1),
                    left_info(row + bias, col + 1));
            }
            init_row += step;
            limit += step;
        }
    }
}

void right_cacl(Eigen::MatrixXd &left_info, Eigen::MatrixXd &right_info,
                int N) {
    // 蝶形，从左向右迭代，最左边一列不进入for循环
    for (int col = 1; col < right_info.cols(); col++) {
        // 每列的计算组数
        int groups = N >> (col);
        // 计算步长
        int step = N / groups;
        // 下方行索引偏移值
        int bias = step >> 1;
        // 每组的循环最大数量
        int limit = step >> 1;

        // 起始行索引
        int init_row = 0;
        // 对列进行分组计算
        for (int i = 0; i < groups; i++) {
            int row = init_row; // 起始行索引
            // 组内的行进行forightright
            for (row; row < limit; row++) {
                // 更新右上，右信息
                right_info(row, col) = right_up_right_info_cacl(
                    right_info(row + bias, col - 1), left_info(row + bias, col),
                    right_info(row, col - 1));
                // 更新右下，右信息
                right_info(row + bias, col) = right_down_right_info_cacl(
                    right_info(row, col - 1), left_info(row, col),
                    right_info(row + bias, col - 1));
            }
            init_row += step;
            limit += step;
        }
    }
}