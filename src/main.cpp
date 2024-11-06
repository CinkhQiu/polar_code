#include "bp_unit_cacl.h"
#include "file_in_out.h"
#include "polar_bp_decode.h"
#include "simulate.h"
#include "func.h"
#include <Eigen/Dense>
#include <iostream>

constexpr int N = 8; // 码长
constexpr double RATE = 0.5;
constexpr double START_SNR = 2.0;
constexpr double END_SNR = 4.0;
constexpr double STEP = 0.5;
constexpr int MAX_FRAME = static_cast<int>(1e6);
constexpr int MAX_ERROR_FRAME = 200;
const int LAYER = static_cast<int>(std::log2(N)) + 1;
constexpr int MAX_ITER = 10;

int error_bits = 0;
int error_frames = 0;

double snr_to_sigma(double SNR, double rate) {
    // 计算方差
    double variance = 1.0 / (2.0 * rate * SNR);
    // 计算标准差 sigma
    double sigma = std::sqrt(variance);
    return sigma;
}

void run();

int main() {
    Eigen::VectorXi frozen_bits = Eigen::VectorXi::Zero(N);

    frozen_bits.coeffRef(0) = 1;
    frozen_bits.coeffRef(1) = 1;
    frozen_bits.coeffRef(2) = 1;
    frozen_bits.coeffRef(4) = 1;

    Eigen::VectorXi init_codeword = Eigen::VectorXi::Zero(N);
    generate_codeword(init_codeword, frozen_bits);

    Eigen::VectorXi int_encode_codeword = init_codeword;

    polar_encode(int_encode_codeword);

    Eigen::VectorXd encode_codeword = int_encode_codeword.cast<double>();
    ;

    // 生成当前信噪比下的标准差
    double sigma = snr_to_sigma(1.0, 0.5);

    // bpsk + awgn噪声
    add_awgn_noise(encode_codeword, sigma);

    // 接受向量
    auto &received_codeword = encode_codeword;

    // 声明左右矩阵
    Eigen::MatrixXd left_info(N, LAYER);
    Eigen::MatrixXd right_info(N, LAYER);

    // 初始化左右矩阵
    init_left_right_info(left_info, right_info, received_codeword, frozen_bits);
    

    for (size_t i = 0; i < MAX_ITER; i++) {
        // 向左计算
        left_cacl(left_info, right_info, N);

        // 向右计算
        right_cacl(left_info, right_info, N);
    }

    // 码字判决
    Eigen::VectorXi decode_codeword = Eigen::VectorXi::Zero(N);
    get_decode_codeword(left_info, right_info, decode_codeword);

    std::cout << "init_codeword: " << init_codeword.transpose() << std::endl;
    std::cout << "decode_codeword: " << decode_codeword.transpose() << std::endl;

    // 统计错误信息
    error_data_stastics(init_codeword, decode_codeword, error_bits, error_frames);

    std::cout << "错误比特数为：" << error_bits << std::endl;
    std::cout << "错误帧数为：" << error_frames << std::endl;

    return 0;
}

void run() {}