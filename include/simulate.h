#pragma once
#include <Eigen/Dense>
#include <random>
#include <thread>
#include "bp_flip_func.h"
#include "bp_unit_cacl.h"
#include "file_in_out.h"
#include "func.h"
#include "polar_bp_decode.h"
#include "config.hpp"
#include "output_info.hpp"
#include "dataset_generator.hpp"
#include "random_utils.hpp"

// 递归编码
inline void polar_encode(Eigen::VectorXi &codeword, int N, int start_index = 0) {
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
inline void polar_encode(Eigen::VectorXi &codeword) {
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
inline void polar_decode(Eigen::VectorXi &codeword) {
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
inline void init_left_right_info(Eigen::MatrixXd &left_info,
                          Eigen::MatrixXd &right_info,
                          const Eigen::VectorXd &received_codeword,
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

// 信噪比从对数值转换为线性值
inline double get_snr_to_sigma(double SNR, double rate) {
    // 将 SNR 从 dB 转换为线性值
    double snr_linear = std::pow(10.0, SNR / 10.0);

    // 计算方差
    double variance = 1.0 / (2.0 * rate * snr_linear);

    // 计算标准差 sigma
    double sigma = std::sqrt(variance);
    return sigma;
}

// 生成随机码字
inline void generate_codeword(Eigen::VectorXi &codeword, Eigen::VectorXi &frozen_bits) {
    for (int i = 0; i < frozen_bits.size(); ++i) {
        if (frozen_bits[i] == 0) {         // 如果当前位置为信息位
            codeword[i] = rand_bit();  // ✅ 使用线程安全随机比特生成器
        }
    }
}

// BPSK 调制并添加 AWGN 噪声
inline void add_awgn_noise(Eigen::VectorXd &encode_code, double sigma) {
    std::mt19937 &rng = get_thread_rng();  // ✅ 每个线程自己的 RNG
    std::normal_distribution<double> distribution(0.0, sigma);  // ✅ 局部分布对象

    for (int i = 0; i < encode_code.size(); ++i) {
        encode_code(i) = (encode_code(i) == 0) ? 1.0 : -1.0;  // BPSK
        encode_code(i) += distribution(rng);  // 添加高斯噪声
    }
}

// 完成一次信噪比下的仿真
// 输入参数：冻结位向量，信噪比，output_info
void run(Eigen::VectorXi &frozen_bits, double snr,
         output_info &thread_output_info);

// 多线程信噪比仿真测试
void threads_run();

// 关键集生成运行测试
void critical_sets_run();

// 基于关键集的比特翻转译码算法性能仿真测试
void bit_flip_simu_test();

// 完成一次信噪比下的bit flip仿真
// 输入参数：冻结位向量，信噪比，output_info
void cs_bp_flip_run(Eigen::VectorXi &frozen_bits, double snr,
                    output_info &thread_output_info);