#include "bp_unit_cacl.h"
#include "file_in_out.h"
#include "func.h"
#include "polar_bp_decode.h"
#include "simulate.h"
#include <Eigen/Dense>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

constexpr int N = 64; // 码长
constexpr double RATE = 0.5;
constexpr double START_SNR = 2.0;
constexpr double END_SNR = 5.5; // 结束区间为开区间
constexpr double STEP = 0.5;
const int TOTAL_STEP_NUMS =
    static_cast<int>(std::round((END_SNR - START_SNR) / STEP));
constexpr int MAX_FRAME = static_cast<int>(1e5);
constexpr int MAX_ERROR_FRAME = 100;
const int LAYER = static_cast<int>(std::log2(N)) + 1;
const int LOG_N = LAYER - 1;
constexpr int MAX_ITER = 10;
const std::string file_name = "resources/frozen_bits_file/2^" +
                              std::to_string(LOG_N) + "_by2_5_dB_GA.txt";

// 创建文件名
std::string output_file_name = "./resources/output_file/_output_2^" +
                               std::to_string(LOG_N) + "_by2_5_dB_GA.txt";

int error_bits = 0;
int error_frames = 0;
int total_frames = 0;
int total_iter = 0;

double get_snr_to_sigma(double SNR, double rate) {
    // 计算方差
    double variance = 1.0 / (2.0 * rate * SNR);
    // 计算标准差 sigma
    double sigma = std::sqrt(variance);
    return sigma;
}

void run();

int main() {
    // 读取冻结位信息
    Eigen::VectorXi frozen_bits = Eigen::VectorXi::Zero(N);
    read_frozen_bits(frozen_bits, file_name);

    // 生成信噪比标准差向量
    std::vector<double> snr_to_sigma(TOTAL_STEP_NUMS);
    double curr_snr = START_SNR;
    for (size_t i = 0; i < TOTAL_STEP_NUMS; i++) {
        // 生成当前信噪比下的标准差
        snr_to_sigma[i] = get_snr_to_sigma(curr_snr, RATE);
        curr_snr += STEP;
    }

    // 打开文件
    std::ofstream output_file(output_file_name, std::ios::app);
    if (output_file.is_open()) {
        // 朝输出文件中添加日期
        output_file << get_current_date() << "\n";
        output_file << "---------------------"
                    << "当前仿真码长为：" << N << "---------------------"
                    << "\n";
    } else {
        std::cerr << "打开输出文件失败" << std::endl;
    }

    // 遍历所有信噪比下的情况
    for (size_t i = 0; i < TOTAL_STEP_NUMS; i++) {
        // 程序开始计时
        auto start_time = std::chrono::high_resolution_clock::now();

        while (total_frames < MAX_FRAME && error_frames < MAX_ERROR_FRAME) {
            // 单次码字仿真

            // 生成随机码字
            Eigen::VectorXi init_codeword = Eigen::VectorXi::Zero(N);
            generate_codeword(init_codeword, frozen_bits);
            // 存留生成后的随机码字
            Eigen::VectorXi int_encode_codeword = init_codeword;
            // 编码
            polar_encode(int_encode_codeword);
            // 码字向量转换为double元素类型
            Eigen::VectorXd encode_codeword =
                int_encode_codeword.cast<double>();

            // 添加噪声
            add_awgn_noise(encode_codeword, snr_to_sigma[i]);
            // 接受向量别名，方便阅读
            auto &received_codeword = encode_codeword;

            // 译码
            // 声明左右矩阵
            Eigen::MatrixXd left_info(N, LAYER);
            Eigen::MatrixXd right_info(N, LAYER);
            // 初始化左右矩阵
            init_left_right_info(left_info, right_info, received_codeword,
                                 frozen_bits);
            // 码字判决后的码字
            Eigen::VectorXi decode_codeword = Eigen::VectorXi::Zero(N);
            // 记录当前译码成功标志
            bool success_decode = false;
            // 左右信息迭代计算
            for (size_t iter = 0; iter < MAX_ITER; iter++) {
                success_decode = false;
                // 向左计算
                left_cacl(left_info, right_info, N);
                // 向右计算
                right_cacl(left_info, right_info, N);

                // 码字判决
                get_decode_codeword(left_info, right_info, decode_codeword);
                // 检验是否译码成功
                if (check_encode_success(init_codeword, decode_codeword)) {
                    // 译码成功，无需进行后续迭代
                    success_decode = true;
                    total_iter += iter;
                    break;
                }
            }
            // 结束该次译码，进行数据统计
            total_frames++;
            if (!success_decode) {
                // 译码失败，进行错误译码信息统计
                error_data_stastics(init_codeword, decode_codeword, error_bits,
                                    error_frames);
            }
        }

        // 获取结束时间点
        auto end_time = std::chrono::high_resolution_clock::now();
        // 计算时间差，单位为秒
        std::chrono::duration<double> elapsed_seconds = end_time - start_time;

        // 将时间转换为分钟和秒
        int minutes = static_cast<int>(elapsed_seconds.count()) / 60;
        double seconds = elapsed_seconds.count() - (minutes * 60);

        // 结束while循环，当前信噪比下译码结束
        // 统计全部信息，输出统计信息

        // 记录当前仿真信噪比
        output_file << "****************"
                    << "当前仿真信噪比为：" << std::fixed
                    << std::setprecision(2) << START_SNR + STEP * i
                    << "****************"
                    << "\n";
        // 记录当前信噪比仿真耗时
        output_file << "当前信噪比仿真耗时 " << minutes << " 分 " << seconds
                    << " 秒"
                    << "\n";
        // 当前仿真允许最大帧数
        output_file << "当前仿真允许最大帧数：" << MAX_FRAME << "\n";
        // 当前仿真允许最大错误帧数
        output_file << "当前仿真允许最大错误帧数：" << MAX_ERROR_FRAME << "\n";
        // 记录总的仿真次数
        output_file << "当前仿真次数为：" << total_frames << "\n";
        // 记录当前错误帧数
        output_file << "当前错误帧数为：" << error_frames << "\n";
        // 记录总的误帧率
        output_file << "当前误帧率为" << std::fixed << std::setprecision(8)
                    << (error_frames * 1.0) / total_frames << "\n";
        // 记录总的误比特率
        output_file << "当前误比特率为" << std::fixed << std::setprecision(8)
                    << (error_bits * 1.0) / (total_frames * N) << "\n";
        // 记录总的迭代次数
        output_file << "BP译码总迭代次数为：" << total_iter << "\n";

        // 相关数据重置为0
        total_frames = 0;
        total_iter = 0;
        error_bits = 0;
        error_frames = 0;
    }

    output_file.close(); // 关闭文件

    return 0;
}

void run() {}