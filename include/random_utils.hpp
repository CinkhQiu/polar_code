#pragma once
#include <random>
#include <chrono>
#include <thread>

// 获取线程局部的随机数生成器
inline std::mt19937 &get_thread_rng() {
    thread_local std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count()
                                   + std::hash<std::thread::id>{}(std::this_thread::get_id()));
    return rng;
}

// 获取 0 或 1 的二元分布（Bernoulli-like）
inline int rand_bit() {
    thread_local std::uniform_int_distribution<int> dist(0, 1);
    return dist(get_thread_rng());
}

// 获取 N(0, sigma^2) 的高斯分布
inline double rand_normal(double sigma) {
    thread_local std::normal_distribution<double> dist(0.0, 1.0);
    return dist(get_thread_rng()) * sigma;
}