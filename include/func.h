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
    char buffer[11]; // "YYYY-MM-DD" 格式，需要 10 个字符 + '\0'

    // 使用 C 标准库的 strftime 进行格式化
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", std::localtime(&now));

    return std::string(buffer);
}