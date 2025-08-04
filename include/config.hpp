#pragma once

#include <filesystem>
#include <string>
#include <cmath>

namespace config {

// 编码参数
constexpr int N = 8;
constexpr double RATE = 0.5;
constexpr double START_SNR = 1.0;
constexpr double END_SNR = 5.0;
constexpr double STEP = 0.5;
constexpr int MAX_FRAME = static_cast<int>(1e6);
constexpr int MAX_ERROR_FRAME = 200;
constexpr int MAX_ITER = 10;

// 推导参数
constexpr int TOTAL_STEP_NUMS =
    static_cast<int>((END_SNR - START_SNR) / STEP + 1);
extern const int LAYER;
extern const int LOG_N;

// 文件路径（声明，不定义）
extern const std::string frozen_bits_file_name;
extern const std::string output_file_name;
extern const std::string output_file_bp_flip_cs_name;
extern const std::string output_datasets_file_name;

}  // namespace config