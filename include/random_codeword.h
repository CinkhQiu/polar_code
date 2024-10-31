#pragma once
#include <Eigen/Dense>
#include <random>

void generate_codeword(Eigen::VectorXi &codeword,
                       Eigen::VectorXi &frozen_bits) {
    // 初始化随机数生成器
    std::random_device rd;                         // 用于生成随机种子
    std::mt19937 generator(rd());                  // 随机数生成器
    std::uniform_int_distribution<int> dist(0, 1); // 生成 0 或 1 的均匀分布

    // 遍历 frozen_bits
    for (int i = 0; i < frozen_bits.size(); ++i) {
        if (frozen_bits[i] == 0) {         // 如果当前位置为信息位
            codeword[i] = dist(generator); // 随机生成 0 或 1
        }
        // 如果 frozen_bits[i] 为 1，codeword[i] 保持原样
    }
}