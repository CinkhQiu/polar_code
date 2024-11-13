#include "bp_flip_func.h"
#include "bp_unit_cacl.h"
#include "file_in_out.h"
#include "func.h"
#include "polar_bp_decode.h"
#include "simulate.h"
#include <Eigen/Dense>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

// bug记录，码长为128的情况下仿真bp-flip译码算法
// 要以3.5信噪比为分界点，分两次仿真
// 例如1.0-3.0，3.5-5.0
// 可能是多线程的问题，暂时我排查不出来

constexpr int N = 8; // 码长
constexpr double RATE = 0.5;
constexpr double START_SNR = 1.0;
constexpr double END_SNR = 5.0;
constexpr double STEP = 0.5;
const int TOTAL_STEP_NUMS =
    static_cast<int>(std::round((END_SNR - START_SNR) / STEP) + 1);
constexpr int MAX_FRAME = static_cast<int>(1e6);
constexpr int MAX_ERROR_FRAME = 200;
const int LAYER = static_cast<int>(std::log2(N)) + 1;
const int LOG_N = LAYER - 1;
constexpr int MAX_ITER = 10;

namespace fs = std::filesystem;
const std::string file_name =
    (fs::path("resources") / "frozen_bits_file" /
     ("2^" + std::to_string(LOG_N) + "_by2_5_dB_GA.txt"))
        .string();

// 创建bp译码算法仿真的文件名
std::string output_file_name =
    (fs::path("resources") / "output_file" /
     ("_output_2^" + std::to_string(LOG_N) + "_by2_5_dB_GA.txt"))
        .string();

// 创建基于cs翻转集合的bp-flip译码算法仿真结果保存文件名
std::string output_file_bp_flip_cs_name =
    (fs::path("resources") / "" /
     ("_output_bp_flip_cs_2^" + std::to_string(LOG_N) + "_by2_5_dB_GA.txt"))
        .string();

// 创建bp译码数据采集的文件名
std::string output_datasets_file_name =
    (fs::path("resources") / "datasets" /
     ("_output_bp_datasets_2^" + std::to_string(LOG_N) + "_by2_5_dB_GA.csv"))
        .string();

struct output_info {
    double curr_snr;
    int error_bits = 0;
    int error_frames = 0;
    int total_frames = 0;
    int total_iter = 0;
    int flip_success = 0;
    double error_frames_rate;
    double error_bits_rate;
    double sim_minutes = 0.0;
    double sim_seconds = 0.0;
};

// 全局随机数生成器
std::mt19937
    generator(std::chrono::steady_clock::now().time_since_epoch().count());
std::uniform_int_distribution<int> dist(0, 1);

// 生成随机码字
void generate_codeword(Eigen::VectorXi &codeword, Eigen::VectorXi &frozen_bits);

// BPSK 调制并添加 AWGN 噪声
void add_awgn_noise(Eigen::VectorXd &encode_code, double sigma);

// 完成一次信噪比下的仿真
// 输入参数：冻结位向量，信噪比，output_info
void run(Eigen::VectorXi &frozen_bits, double snr,
         output_info &thread_output_info);

// 多线程信噪比仿真测试
void threads_run();

// 关键集生成运行测试
void critical_sets_run();

// 完成一次信噪比下的仿真
// 输入参数：冻结位向量，信噪比，output_info
void cs_bp_flip_run(Eigen::VectorXi &frozen_bits, double snr,
                    output_info &thread_output_info);

// 基于关键集的比特翻转译码算法性能仿真测试
void bit_flip_simu_test();

// bp译码算法数据集的采集
// 当前数据集只在一个信噪比下采集
void generate_bp_datasets(int max_count, double snr);

int main() {
    Eigen::VectorXi frozen_bits = Eigen::VectorXi::Zero(N);
    read_frozen_bits(frozen_bits, file_name);

    // critical_sets_run();
    // threads_run();
    // bit_flip_simu_test();
    generate_bp_datasets(100, 1.0);
    return 0;
}

// 生成随机码字
void generate_codeword(Eigen::VectorXi &codeword,
                       Eigen::VectorXi &frozen_bits) {
    for (int i = 0; i < frozen_bits.size(); ++i) {
        if (frozen_bits[i] == 0) {         // 如果当前位置为信息位
            codeword[i] = dist(generator); // 随机生成 0 或 1
        }
    }
}

// BPSK 调制并添加 AWGN 噪声
void add_awgn_noise(Eigen::VectorXd &encode_code, double sigma) {
    // 每次调用时创建新的高斯分布对象，使用当前的 sigma 值
    std::normal_distribution<double> distribution(0.0, sigma);

    for (int i = 0; i < encode_code.size(); ++i) {
        // BPSK 调制：0 映射为 +1，1 映射为 -1
        encode_code(i) = (encode_code(i) == 0) ? 1.0 : -1.0;

        // 添加 AWGN 噪声
        encode_code(i) += distribution(generator);
    }
}

// 完成一次信噪比下的仿真
// 输入参数：冻结位向量，信噪比，output_info
void run(Eigen::VectorXi &frozen_bits, double snr,
         output_info &thread_output_info) {
    // 程序开始计时
    auto start_time = std::chrono::high_resolution_clock::now();

    while (thread_output_info.total_frames < MAX_FRAME &&
           thread_output_info.error_frames < MAX_ERROR_FRAME) {
        // 单次码字仿真

        // 生成随机码字
        Eigen::VectorXi init_codeword = Eigen::VectorXi::Zero(N);
        generate_codeword(init_codeword, frozen_bits);
        // 存留生成后的随机码字
        Eigen::VectorXi int_encode_codeword = init_codeword;
        // 编码
        polar_encode(int_encode_codeword);
        // 码字向量转换为double元素类型
        Eigen::VectorXd encode_codeword = int_encode_codeword.cast<double>();

        // 添加噪声
        add_awgn_noise(encode_codeword, snr);
        // 接受向量别名，方便阅读
        const auto &received_codeword = encode_codeword;

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
                thread_output_info.total_iter += iter;
                break;
            }
        }
        // 结束该次译码，进行数据统计
        thread_output_info.total_frames++;
        if (!success_decode) {
            // 译码失败，进行错误译码信息统计
            thread_output_info.total_iter += MAX_ITER;
            error_data_stastics(init_codeword, decode_codeword,
                                thread_output_info.error_bits,
                                thread_output_info.error_frames);
        }
    }

    // 获取结束时间点
    auto end_time = std::chrono::high_resolution_clock::now();
    // 计算时间差，单位为秒
    std::chrono::duration<double> elapsed_seconds = end_time - start_time;

    // 将时间转换为分钟和秒
    thread_output_info.sim_minutes =
        static_cast<int>(elapsed_seconds.count()) / 60;
    thread_output_info.sim_seconds =
        elapsed_seconds.count() - (thread_output_info.sim_minutes * 60);

    // 记录总的误帧率
    thread_output_info.error_frames_rate =
        (thread_output_info.error_frames * 1.0) /
        thread_output_info.total_frames;

    // 记录总的误比特率
    thread_output_info.error_bits_rate = (thread_output_info.error_bits * 1.0) /
                                         (thread_output_info.total_frames * N);

    std::cout << "sigma " << snr << " 仿真结束"
              << "，误码率为：" << thread_output_info.error_frames_rate
              << std::endl;
}

// bp译码算法的多线程仿真
void threads_run() {
    // 读取冻结位信息
    Eigen::VectorXi frozen_bits = Eigen::VectorXi::Zero(N);
    read_frozen_bits(frozen_bits, file_name);

    // 创建固定数量的线程
    std::vector<std::thread> threads;
    // 创建用于存储仿真信息的vector
    std::vector<output_info> all_output_info(TOTAL_STEP_NUMS);

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
        output_file << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++"
                       "+++++++++++"
                    << "\n";
        output_file << "++++++++++++++++++++++" << get_current_date() << "\n";
        output_file << "---------------------"
                    << "多线程版本：当前仿真码长为：" << N
                    << "---------------------"
                    << "\n";
    } else {
        std::cerr << "打开输出文件失败" << std::endl;
    }

    // 记录多线程仿真总时间
    auto start_time = std::chrono::high_resolution_clock::now();

    // 创建多线程
    for (size_t i = 0; i < TOTAL_STEP_NUMS; i++) {
        all_output_info[i].curr_snr = START_SNR + STEP * i;
        threads.emplace_back(run, std::ref(frozen_bits), snr_to_sigma[i],
                             std::ref(all_output_info[i]));
    }

    // 等待所有线程完成
    for (auto &thread : threads) {
        thread.join();
    }

    // 获取结束时间点
    auto end_time = std::chrono::high_resolution_clock::now();
    // 计算时间差，单位为秒
    std::chrono::duration<double> elapsed_seconds = end_time - start_time;

    // 将时间转换为分钟和秒
    int minutes = static_cast<int>(elapsed_seconds.count()) / 60;
    double seconds = elapsed_seconds.count() - (minutes * 60);

    if (output_file.is_open()) {
        // 朝输出文件中写入多线程仿真总耗时
        output_file << "****当前多线程信噪比仿真总耗时 " << minutes << " 分 "
                    << seconds << " 秒****"
                    << "\n";
    } else {
        std::cerr << "打开输出文件失败" << std::endl;
    }

    // 向文件中输出结果
    for (size_t i = 0; i < TOTAL_STEP_NUMS; i++) {
        // 记录当前仿真信噪比
        output_file << "****************"
                    << "当前仿真信噪比为：" << std::fixed
                    << std::setprecision(2) << START_SNR + STEP * i
                    << "****************"
                    << "\n";
        // 记录当前信噪比仿真耗时
        output_file << "当前信噪比仿真耗时 " << all_output_info[i].sim_minutes
                    << " 分 " << all_output_info[i].sim_seconds << " 秒"
                    << "\n";
        // 当前仿真允许最大帧数
        output_file << "当前仿真允许最大帧数：" << MAX_FRAME << "\n";
        // 当前仿真允许最大错误帧数
        output_file << "当前仿真允许最大错误帧数：" << MAX_ERROR_FRAME << "\n";
        // 记录总的仿真次数
        output_file << "当前仿真次数为：" << all_output_info[i].total_frames
                    << "\n";
        // 记录当前错误帧数
        output_file << "当前错误帧数为：" << all_output_info[i].error_frames
                    << "\n";
        // 记录总的误帧率
        output_file << "当前误帧率为" << std::fixed << std::setprecision(8)
                    << all_output_info[i].error_frames_rate << "\n";
        // 记录总的误比特率
        output_file << "当前误比特率为" << std::fixed << std::setprecision(8)
                    << all_output_info[i].error_bits_rate << "\n";
        // 记录总的迭代次数
        output_file << "BP译码总迭代次数为：" << all_output_info[i].total_iter
                    << "\n";
    }

    output_file << "-----------------------------------------------------------"
                   "-----------"
                << "\n";

    output_file.close(); // 关闭文件
}

// 关键集生成运行测试
void critical_sets_run() {
    // 读取冻结位信息
    Eigen::VectorXi frozen_bits = Eigen::VectorXi::Zero(N);
    read_frozen_bits(frozen_bits, file_name);
    std::vector<int> critical_sets;

    generate_critical_sets(frozen_bits, critical_sets);
}

// cs-bp-flip译码算法的多线程性能仿真
// 基于关键集的比特翻转译码算法性能仿真测试
void bit_flip_simu_test() {
    // 读取冻结位信息
    Eigen::VectorXi frozen_bits = Eigen::VectorXi::Zero(N);
    read_frozen_bits(frozen_bits, file_name);

    // 创建固定数量的线程
    std::vector<std::thread> threads;
    // 创建用于存储仿真信息的vector
    std::vector<output_info> all_output_info(TOTAL_STEP_NUMS);

    // 生成信噪比标准差向量
    std::vector<double> snr_to_sigma(TOTAL_STEP_NUMS);
    double curr_snr = START_SNR;
    for (size_t i = 0; i < TOTAL_STEP_NUMS; i++) {
        // 生成当前信噪比下的标准差
        snr_to_sigma[i] = get_snr_to_sigma(curr_snr, RATE);
        curr_snr += STEP;
    }

    // 打开文件
    std::ofstream output_file(output_file_bp_flip_cs_name, std::ios::app);
    if (output_file.is_open()) {
        // 朝输出文件中添加日期
        output_file << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++"
                       "+++++++++++"
                    << "\n";
        output_file << "++++++++++++++++++++++" << get_current_date() << "\n";
        output_file << "---------------------"
                    << "CS-BP-Flip译码算法：多线程版本：当前仿真码长为：" << N
                    << "---------------------"
                    << "\n";
    } else {
        std::cerr << "打开输出文件失败" << std::endl;
    }

    // 记录多线程仿真总时间
    auto start_time = std::chrono::high_resolution_clock::now();

    // 创建多线程
    for (size_t i = 0; i < TOTAL_STEP_NUMS; i++) {
        all_output_info[i].curr_snr = START_SNR + STEP * i;
        threads.emplace_back(cs_bp_flip_run, std::ref(frozen_bits),
                             snr_to_sigma[i], std::ref(all_output_info[i]));
    }

    // 等待所有线程完成
    for (auto &thread : threads) {
        thread.join();
    }

    // 获取结束时间点
    auto end_time = std::chrono::high_resolution_clock::now();
    // 计算时间差，单位为秒
    std::chrono::duration<double> elapsed_seconds = end_time - start_time;

    // 将时间转换为分钟和秒
    int minutes = static_cast<int>(elapsed_seconds.count()) / 60;
    double seconds = elapsed_seconds.count() - (minutes * 60);

    if (output_file.is_open()) {
        // 朝输出文件中写入多线程仿真总耗时
        output_file << "****当前多线程信噪比仿真总耗时 " << minutes << " 分 "
                    << seconds << " 秒****"
                    << "\n";
    } else {
        std::cerr << "打开输出文件失败" << std::endl;
    }

    // 向文件中输出结果
    for (size_t i = 0; i < TOTAL_STEP_NUMS; i++) {
        // 记录当前仿真信噪比
        output_file << "****************"
                    << "当前仿真信噪比为：" << std::fixed
                    << std::setprecision(2) << START_SNR + STEP * i
                    << "****************"
                    << "\n";
        // 记录当前信噪比仿真耗时
        output_file << "当前信噪比仿真耗时 " << all_output_info[i].sim_minutes
                    << " 分 " << all_output_info[i].sim_seconds << " 秒"
                    << "\n";
        // 当前仿真允许最大帧数
        output_file << "当前仿真允许最大帧数：" << MAX_FRAME << "\n";
        // 当前仿真允许最大错误帧数
        output_file << "当前仿真允许最大错误帧数：" << MAX_ERROR_FRAME << "\n";
        // 记录总的仿真次数
        output_file << "当前仿真次数为：" << all_output_info[i].total_frames
                    << "\n";
        // 记录当前错误帧数
        output_file << "当前错误帧数为：" << all_output_info[i].error_frames
                    << "\n";
        // 记录总的误帧率
        output_file << "当前误帧率为" << std::fixed << std::setprecision(8)
                    << all_output_info[i].error_frames_rate << "\n";
        // 记录总的误比特率
        output_file << "当前误比特率为" << std::fixed << std::setprecision(8)
                    << all_output_info[i].error_bits_rate << "\n";
        // 记录总的翻转成功帧数
        output_file << "翻转成功帧数为" << all_output_info[i].flip_success
                    << "\n";
        // 记录总的迭代次数
        output_file << "BP译码总迭代次数为：" << all_output_info[i].total_iter
                    << "\n";
    }

    output_file << "-----------------------------------------------------------"
                   "-----------"
                << "\n";

    output_file.close(); // 关闭文件
}

// cs-bp-flip译码算法一次信噪比下的仿真
// 输入参数：冻结位向量，信噪比，output_info
void cs_bp_flip_run(Eigen::VectorXi &frozen_bits, double snr,
                    output_info &thread_output_info) {
    // 程序开始计时
    auto start_time = std::chrono::high_resolution_clock::now();

    while (thread_output_info.total_frames < MAX_FRAME &&
           thread_output_info.error_frames < MAX_ERROR_FRAME) {
        // 单次码字仿真

        // 生成随机码字
        Eigen::VectorXi init_codeword = Eigen::VectorXi::Zero(N);
        generate_codeword(init_codeword, frozen_bits);
        // 存留生成后的随机码字
        Eigen::VectorXi int_encode_codeword = init_codeword;
        // 编码
        polar_encode(int_encode_codeword);
        // 码字向量转换为double元素类型
        Eigen::VectorXd encode_codeword = int_encode_codeword.cast<double>();

        // 添加噪声
        add_awgn_noise(encode_codeword, snr);
        // 接受向量别名，方便阅读
        const auto &received_codeword = encode_codeword;

        // 获取翻转集
        std::vector<int> critical_sets;
        generate_critical_sets(frozen_bits, critical_sets);

        // 获取翻转集大小
        int cs_length = critical_sets.size();

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
                thread_output_info.total_iter += iter;
                break;
            }
        }

// 若当前译码失败，进入到bit-flip译码算法中
#if 1
        if (!success_decode) {
            // 译码失败，进入到bp-flip译码算法中
            // 遍历翻转集合
            for (size_t i = 0; i < cs_length; i++) {
                double llr = received_codeword(critical_sets[i]);
                // 重新初始化左右矩阵
                init_left_right_info(left_info, right_info, received_codeword,
                                     frozen_bits);
                // 翻转
                if (llr > 0) {
                    right_info(critical_sets[i], 0) = MIN_INF;
                } else {
                    right_info(critical_sets[i], 0) = MAX_INF;
                }

                // 翻转后重新进入到bp译码算法中进行译码
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
                        thread_output_info.total_iter += iter;
                        break;
                    }
                }
                if (success_decode == true) {
                    thread_output_info.flip_success++;
                    break;
                }
            }
        }
#endif

        // 信息统计
        // 结束该次译码，进行数据统计
        thread_output_info.total_frames++;
        if (!success_decode) {
            // 译码失败，进行错误译码信息统计
            thread_output_info.total_iter += MAX_ITER;
            error_data_stastics(init_codeword, decode_codeword,
                                thread_output_info.error_bits,
                                thread_output_info.error_frames);
        }
    }

    // 获取结束时间点
    auto end_time = std::chrono::high_resolution_clock::now();
    // 计算时间差，单位为秒
    std::chrono::duration<double> elapsed_seconds = end_time - start_time;

    // 将时间转换为分钟和秒
    thread_output_info.sim_minutes =
        static_cast<int>(elapsed_seconds.count()) / 60;
    thread_output_info.sim_seconds =
        elapsed_seconds.count() - (thread_output_info.sim_minutes * 60);

    // 记录总的误帧率
    thread_output_info.error_frames_rate =
        (thread_output_info.error_frames * 1.0) /
        thread_output_info.total_frames;

    // 记录总的误比特率
    thread_output_info.error_bits_rate = (thread_output_info.error_bits * 1.0) /
                                         (thread_output_info.total_frames * N);

    std::cout << "sigma " << snr << " 仿真结束"
              << "，误码率为：" << thread_output_info.error_frames_rate
              << "，翻转成功数为：" << thread_output_info.flip_success
              << "总帧数为：" << thread_output_info.total_frames << std::endl;
}

// bp译码算法数据集的采集
// 当前数据集只在一个信噪比下采集
void generate_bp_datasets(int max_count, double snr) {
    // 冻结位的数据读取
    Eigen::VectorXi frozen_bits = Eigen::VectorXi::Zero(N);
    read_frozen_bits(frozen_bits, file_name);

    // 信噪比转换为线性信噪比
    snr = get_snr_to_sigma(snr, 0.5);

    // 当前数据集的索引
    int count = 0;
    std::ifstream read_file(output_datasets_file_name);
    // 检查文件是否成功打开
    if (!read_file.is_open()) {
        std::cerr << "无法打开文件进行读取！count赋值为0" << std::endl;
    } else {
        count = get_numbers_of_datasets(read_file) + 1;
        std::cout << "当前共有 " << count - 1 << "条数据" << std::endl;
    }
    read_file.close();

    // 打开输出文件
    std::ofstream out_file(output_datasets_file_name, std::ios::app);
    if (!out_file) {
        std::cerr << "无法打开数据集输出文件！" << std::endl;
        return;
    }
    // 数据集采集
    std::cout << "数据开始采集" << std::endl;
    // 用于控制while的循环变量
    int curr = 0;
    while (curr < max_count) {
        // 完成单次bp译码仿真
        // 单次码字仿真

        // 生成随机码字
        Eigen::VectorXi init_codeword = Eigen::VectorXi::Zero(N);
        generate_codeword(init_codeword, frozen_bits);
        // 存留生成后的随机码字
        Eigen::VectorXi int_encode_codeword = init_codeword;
        // 编码
        polar_encode(int_encode_codeword);
        // 码字向量转换为double元素类型
        Eigen::VectorXd encode_codeword = int_encode_codeword.cast<double>();

        // 添加噪声
        add_awgn_noise(encode_codeword, snr);
        // 接受向量别名，方便阅读
        const auto &received_codeword = encode_codeword;

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
                break;
            }
        }

        // 结束该次译码，对于译码失败的情况进行数据采集
        if (!success_decode) {
            // 生成错误比特向量
            Eigen::VectorXi error_bits_vector = init_codeword, decode_codeword;
            // 译码失败，进行数据集的采集
            output_datasets_csv(out_file, left_info, error_bits_vector, count);
            // std::cout << "采集第 " << curr << " 条数据" << '\n';
            count++;
            curr++;
        }
    }

    out_file.close();
}