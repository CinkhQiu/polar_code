#include "bp_unit_cacl.h"
#include <Eigen/Dense>
#include <iostream>

int main() {
    // 示例：创建并初始化一个 3x3 的 Eigen 矩阵
    Eigen::Matrix3d matrix;
    matrix << 1, 2, 3,
              4, 5, 6,
              7, 8, 9;

    // 计算矩阵的转置并输出结果
    Eigen::Matrix3d transpose = matrix.transpose();
    std::cout << "Matrix:" << std::endl << matrix << std::endl;
    std::cout << "Transpose:" << std::endl << transpose << std::endl;

    // 主程序其他逻辑
    // 比如调用项目中的函数
    // double result = some_project_function(matrix);

    return 0;
}