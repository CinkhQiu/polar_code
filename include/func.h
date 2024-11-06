#pragma once

#include <Eigen/Dense>

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