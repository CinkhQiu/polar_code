#include <filesystem>
#include <math.h>

namespace config {
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
const std::string frozen_bits_file_name =
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
} // namespace config