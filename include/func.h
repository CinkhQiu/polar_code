#pragma once

#include <Eigen/Dense>
#include <ctime>

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