#include <Eigen/Dense>
#include <cmath>
#include <iostream>
#include <vector>

// 递归函数，用于更新所有的非叶子节点的子节点为 1
inline void update_non_leaf_nodes(Eigen::MatrixXi &matrix, int i, int j, int depth) {
    // 如果已经到达最后一层，则返回
    if (i >= depth - 2) {
        return;
    }

    // 更新当前节点的子节点为 -1
    matrix(i + 1, 2 * j) = -1;     // 左子节点
    matrix(i + 1, 2 * j + 1) = -1; // 右子节点

    // 递归更新左子节点和右子节点
    update_non_leaf_nodes(matrix, i + 1, 2 * j, depth);
    update_non_leaf_nodes(matrix, i + 1, 2 * j + 1, depth);
}

// 更新当前节点所对应的所有的叶子节点的值，更新值的操作在flag_cs上进行
inline void update_leaf_nodes(int i, int j, int depth, Eigen::VectorXi &flag_cs) {
    // 再计算（i， 0）节点的最左索引
    int i_0_left_index = 0;
    // 先计算（i， 0）节点的最右索引
    int i_0_right_index = static_cast<int>(std::pow(2, depth - i - 1)) - 1;
    // 计算（i， 0）与（i， j）之间的横向坐标差值
    int delta = static_cast<int>(std::pow(2, depth - i - 1));
    // flag_cs的最左索引
    int left = i_0_left_index + delta * j;
    // flag_cs的最右索引
    int right = i_0_right_index + delta * j;

    // 更新操作
    flag_cs(left) = 1;
    for (size_t i = left + 1; i < right + 1; i++) {
        flag_cs(i) = 0;
    }
}

// 基于码率为1的码树生成的关键集
inline void generate_critical_sets(Eigen::VectorXi &frozen_bits,
                            std::vector<int> &critical_sets) {
    // 创建一个N × (logN + 1)的空矩阵
    int cols = frozen_bits.size();
    int rows = static_cast<int>(std::log2(cols)) + 1;
    Eigen::MatrixXi matrix(rows, cols);
    matrix.setZero();

    // 创建一个关键集标志位向量
    Eigen::VectorXi flag_cs(cols);
    flag_cs.setConstant(-1);

    // 初始化矩阵底层行
    // 遍历赋值，frozen_bits 为 1 的位置赋值为 0，反之赋值为 1
    for (int i = 0; i < frozen_bits.size(); ++i) {
        matrix(rows - 1, i) = (frozen_bits(i) == 1) ? 0 : 1;
    }

    // 每行要遍历的信息节点数
    int num_cols = cols;
    // 自底向上遍历矩阵，两两遍历给上一层节点赋值
    for (size_t i = rows - 1; i > 0; i--) {
        // 内层行遍历
        for (size_t cols_index = 0; cols_index < num_cols; cols_index += 2) {
            // 计算，将结果赋值给上层
            int res = matrix(i, cols_index) + matrix(i, cols_index + 1);
            if (res == 2) {
                // 先计算上层索引，后赋值
                int pre_layer_index = cols_index / 2;
                matrix(i - 1, pre_layer_index) = 1;
            }
        }
        num_cols /= 2;
    }

    int depth = rows;
    // 遍历从第 0 层至倒数第 2 层（不遍历最后一层）
    for (int i = 0; i < depth - 1; ++i) {
        for (int j = 0; j < (1 << i); ++j) {
            // 当前节点的值为 1，执行更新操作
            if (matrix(i, j) == 1) {
                // 对其所有的非叶子节点的子节点递归置为 -1
                update_non_leaf_nodes(matrix, i, j, depth);
                // 其叶子节点，最左边的索引赋值为1，其他节点赋值为0
                update_leaf_nodes(i, j, depth, flag_cs);
            }
        }
    }

    // 最后一层单独处理
    for (size_t i = 0; i < cols; i++) {
        if (flag_cs(i) == -1) {
            // 如果是冻结位，置为0，信息位则置为1
            if (frozen_bits(i) == 1) {
                flag_cs(i) = 0;
            } else {
                flag_cs(i) = 1;
            }
        }
    }

    // 遍历flag_cs向量，将值为1的索引赋值给critical_sets
    for (size_t i = 0; i < cols; i++) {
        if (flag_cs(i) == 1) {
            critical_sets.push_back(i);
        }
    }

    // for (size_t i = 0; i < cols; i++) {
    //     if (flag_cs(i) == 1) {
    //         std::cout << i << "\t";
    //     }
    // }
    // std::cout << std::endl;
}