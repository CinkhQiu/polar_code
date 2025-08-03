#pragma once
#include "bp_flip_func.h"
#include "bp_unit_cacl.h"
#include "config.hpp"
#include "dataset_generator.hpp"
#include "file_in_out.h"
#include "func.h"
#include "output_info.hpp"
#include "polar_bp_decode.h"
#include "simulate.h"
#include <Eigen/Dense>
#include <iostream>
#include <random>
#include <thread>

// bp译码算法数据集的采集
// 当前数据集只在一个信噪比下采集
void generate_bp_datasets(int max_count, double snr);