#include "bp_flip_func.hpp"
#include "bp_unit_cacl.hpp"
#include "file_in_out.hpp"
#include "func.hpp"
#include "polar_bp_decode.hpp"
#include "simulate.hpp"
#include "config.hpp"
#include "output_info.hpp"
#include "dataset_generator.hpp"
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

int main() {
    Eigen::VectorXi frozen_bits = Eigen::VectorXi::Zero(config::N);
    read_frozen_bits(frozen_bits, config::frozen_bits_file_name);

    // critical_sets_run();
    // threads_run();
    // bit_flip_simu_test();
    generate_bp_datasets(100, 1.0);
    return 0;
}