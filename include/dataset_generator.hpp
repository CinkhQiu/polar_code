#pragma once
#include "bp_flip_func.hpp"
#include "bp_unit_cacl.hpp"
#include "config.hpp"
#include "file_in_out.hpp"
#include "func.hpp"
#include "output_info.hpp"
#include "polar_bp_decode.hpp"
#include "simulate.hpp"
#include <Eigen/Dense>
#include <iostream>
#include <random>
#include <thread>

// bp译码算法数据集的采集
// 当前数据集只在一个信噪比下采集
void generate_bp_datasets(int max_count, double snr);