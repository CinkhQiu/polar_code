#pragma once

#include <Eigen/Dense>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

void error_data_stastics(Eigen::VectorXi &init_codeword,
                         Eigen::VectorXi &decode_codeword, int &error_bits,
                         int &error_frames) {
    bool frame_has_error = false; // 标记当前帧是否有错误
    for (int i = 0; i < init_codeword.size(); ++i) {
        if (init_codeword[i] != decode_codeword[i]) {
            ++error_bits;           // 有位不同时增加 error_bits
            frame_has_error = true; // 只要有一位不同，就标记当前帧有错误
        }
    }

    // 如果当前帧有错误，则增加 error_frames
    if (frame_has_error) {
        ++error_frames;
    }
}

bool check_encode_success(Eigen::VectorXi &init_codeword,
                          Eigen::VectorXi &decode_codeword) {
    return init_codeword == decode_codeword;
}

std::string get_current_date() {
    std::time_t now = std::time(nullptr); // 获取当前时间
    std::tm tm_struct;
    // localtime_s(&tm_struct, &now); // 使用线程安全的 localtime_s
    localtime_r(&now, &tm_struct); // 使用线程安全的 localtime_r

    char buffer[20]; // "YYYY-MM-DD HH:MM:SS" 格式，需要 19 个字符 + '\0'
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm_struct);

    return std::string(buffer);
}

void output_datasets_csv(std::ofstream &file, const Eigen::MatrixXd &left_info,
                         const Eigen::VectorXi &error_bits_vector,
                         const int count) {
    // 检查文件流是否有效
    if (!file.is_open()) {
        std::cerr << "File stream is not open!" << std::endl;
        return;
    }

    int rows = error_bits_vector.rows();
    int cols = left_info.cols();

    // 输出数据csv文件中
    for (size_t i = 0; i < rows; i++) {
        file << count << ",";
        for (size_t j = 0; j < cols; j++) {
            file << left_info(i, j) << ",";
        }
        file << error_bits_vector(i) << '\n';
    }
}

// 读取csv文件的最后一行的第一列，用以标示当前已有多少条数据
int get_numbers_of_datasets(std::ifstream &file) {
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << std::endl;
        return -1;
    }

    std::string line;
    std::string last_first_data = "0";

    // 按行读取文件
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string first_data;

        // 读取当前行的第一个数据（通常是逗号分隔的第一个字段）
        if (std::getline(ss, first_data, ',')) {
            last_first_data = first_data; // 每次更新为最后一行的第一个数据
        }
    }

    return std::stoi(last_first_data);
}