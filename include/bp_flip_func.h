#include <Eigen/Dense>
#include <cmath>

// 基于码率为1的码树生成的关键集
void generate_critical_sets(Eigen::VectorXi &frozen_bits) {
    // 创建一个N × (logN + 1)的空矩阵
    int cols = frozen_bits.size();
    int rows = static_cast<int>(std::log2(cols)) + 1;
    Eigen::MatrixXi matrix(rows, cols);
    matrix.setZero();

    // 创建一个关键集标志位向量
    Eigen::VectorXi flag_cs;
    flag_cs.setConstant(-1);

    // 初始化矩阵底层行
    matrix.row(rows - 1) = frozen_bits.transpose();

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

    // 当前行大小
    num_cols = 1;
    // 矩阵自顶向下遍历，为flag_cs设置标志位
    for (size_t i = 0; i < rows - 1; i++) {
        // 若当前节点的值为1，其叶子节点的首元素设置为1，其他设置为0
        // 行遍历
        for (size_t cols_index = 0; cols_index < num_cols; cols_index++) {
            if (matrix(i, cols_index) == 1) {
                // 计算左右索引
                // 先计算右索引
                int real_cols_index = cols_index = 1;
                int right_index = static_cast<int>(pow(2, cols - i)) * cols_index;
            }
        }
    }
}