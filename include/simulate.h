#pragma once
#include <Eigen/Dense>
#include <random>
#include "bp_unit_cacl.h"

// 递归编码
void polar_encode(Eigen::VectorXi &codeword, int N, int start_index = 0) {
    if (N == 1) {
        return; // 基准条件，N 为 1 时不再分割
    }

    // 使用 XOR 运算实现极化编码的蝶形结构
    for (int i = 0; i < N / 2; ++i) {
        codeword(start_index + i) ^= codeword(start_index + i + N / 2);
    }

    // 递归处理上半部分和下半部分
    polar_encode(codeword, N / 2, start_index);
    polar_encode(codeword, N / 2, start_index + N / 2);
}

// 使用循环实现蝶形编码
void polar_encode(Eigen::VectorXi &codeword) {
    int N = codeword.size();

    int max_layer = std::log2(N);

    // 遍历每一层的操作
    for (int layer = 1; layer <= max_layer; ++layer) {
        int step = 1 << layer;     // 本层的跨度
        int half_step = step >> 1; // 每组操作的步长

        // 遍历该层中的每一组
        for (int start = 0; start < N; start += step) {
            for (int i = 0; i < half_step; ++i) {
                codeword(start + i) ^= codeword(start + i + half_step);
            }
        }
    }
}

// 蝶形结构解码
void polar_decode(Eigen::VectorXi &codeword) {
    int N = codeword.size();
    int max_layer = static_cast<int>(std::log2(N)); // 计算并存储最大层数

    // 逆序遍历每一层，从 max_layer 开始递减
    for (int layer = max_layer; layer >= 1; --layer) {
        int step = 1 << layer;     // 本层的跨度
        int half_step = step >> 1; // 每组操作的步长

        // 遍历该层中的每一组
        for (int start = 0; start < N; start += step) {
            for (int i = 0; i < half_step; ++i) {
                codeword(start + i) ^= codeword(start + i + half_step);
            }
        }
    }
}

// 初始化左右信息矩阵
void init_left_right_info(Eigen::MatrixXd &left_info,
                          Eigen::MatrixXd &right_info,
                          Eigen::VectorXd &received_codeword,
                          Eigen::VectorXi &frozen_bits) {
    // 初始化右信息矩阵的第 0 列
    for (int i = 0; i < frozen_bits.size(); ++i) {
        right_info(i, 0) = (frozen_bits(i) == 1) ? MAX_INF : 0.0;
    }

    // 初始化左信息矩阵的最后一列
    int last_col = left_info.cols() - 1;
    for (int i = 0; i < received_codeword.size(); ++i) {
        left_info(i, last_col) = received_codeword(i);
    }
}