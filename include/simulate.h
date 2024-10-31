#pragma once
#include <Eigen/Dense>
#include <random>

// 生成随机码字
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
        int step = 1 << layer;           // 本层的跨度
        int half_step = step >> 1;       // 每组操作的步长

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
        int step = 1 << layer;           // 本层的跨度
        int half_step = step >> 1;       // 每组操作的步长

        // 遍历该层中的每一组
        for (int start = 0; start < N; start += step) {
            for (int i = 0; i < half_step; ++i) {
                codeword(start + i) ^= codeword(start + i + half_step);
            }
        }
    }
}

// bpsk调制，添加awgn噪声
void add_awgn_noise(Eigen::VectorXd &encode_code, double snr_db) {

}