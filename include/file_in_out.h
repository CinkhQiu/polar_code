#pragma once
#include <Eigen/Dense>
#include <fstream>
#include <iostream>
#include <string>

// 从文件中读取冻结位信息至frozen_bits中
void read_frozen_bits(Eigen::VectorXi &frozen_bits, std::string &file_name) {
    std::ifstream file(file_name);

    if (!file.is_open()) {
        std::cerr << "Unable to open file: " << file_name << std::endl;
        return;
    }

    std::string line;
    if (std::getline(file, line)) {
        std::istringstream iss(line);
        int index;

        // 读取文件中以 \t 分割的每个数字
        while (iss >> index) {
            if (index >= 0 && index < frozen_bits.size()) {
                frozen_bits(index) = 1; // 将对应的下标置为 1
            }

            if (iss.peek() == '\t') { // 跳过 \t 分隔符
                iss.ignore();
            }
        }
    }

    file.close();
}