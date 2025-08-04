#include "config.hpp"

namespace fs = std::filesystem;

namespace config {

const int LAYER = static_cast<int>(std::log2(N)) + 1;
const int LOG_N = LAYER - 1;

const std::string frozen_bits_file_name =
    (fs::path("resources") / "frozen_bits_file" /
     ("2^" + std::to_string(LOG_N) + "_by2_5_dB_GA.txt"))
        .string();

const std::string output_file_name =
    (fs::path("resources") / "output_file" /
     ("_output_2^" + std::to_string(LOG_N) + "_by2_5_dB_GA.txt"))
        .string();

const std::string output_file_bp_flip_cs_name =
    (fs::path("resources") /
     ("_output_bp_flip_cs_2^" + std::to_string(LOG_N) + "_by2_5_dB_GA.txt"))
        .string();

const std::string output_datasets_file_name =
    (fs::path("resources") / "datasets" /
     ("_output_bp_datasets_2^" + std::to_string(LOG_N) + "_by2_5_dB_GA.csv"))
        .string();

}  // namespace config
