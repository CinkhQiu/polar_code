#include "bp_unit_cacl.h"
#include "file_in_out.h"
#include "polar_bp_decode.h"
#include "simulate.h"
#include <Eigen/Dense>
#include <iostream>

constexpr int N = 1024; // 码长
constexpr double RATE = 0.5;
constexpr double START_SNR = 2.0;
constexpr double END_SNR = 4.0;
constexpr double STEP = 0.5;
constexpr int MAX_FRAME = static_cast<int>(1e6);
constexpr int MAX_ERROR_FRAME = 200;

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
    std::string file_name = "resources/frozen_bits_file/2^10_2.txt";

    read_frozen_bits(frozen_bits, file_name);

    Eigen::VectorXi codeword = Eigen::VectorXi::Zero(N);
    generate_codeword(codeword, frozen_bits);

    polar_encode(codeword);

    Eigen::VectorXd encode_codeword = codeword.cast<double>();;

    double sigma = snr_to_sigma(2.0, 0.5);

    add_awgn_noise(encode_codeword, sigma);

    std::cout << encode_codeword << std::endl;

    return 0;
}

void run() {}