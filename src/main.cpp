#include "bp_unit_cacl.h"
#include "simulate.h"
#include <Eigen/Dense>
#include <iostream>

int main() {
    int N = 16; // 假设长度为 8 的码字
    Eigen::VectorXi codeword(N);
    codeword << 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1;

    polar_encode(codeword);
    std::cout << "编码后的码字:\n" << codeword << std::endl;
    polar_decode(codeword);
    std::cout << "解码后的码字:\n" << codeword << std::endl;

    Eigen::VectorXd double_codeword = codeword.cast<double>();

    return 0;
}