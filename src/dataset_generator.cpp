#include "dataset_generator.hpp"

// bp译码算法数据集的采集
// 当前数据集只在一个信噪比下采集
void generate_bp_datasets(int max_count, double snr) {
    // 冻结位的数据读取
    Eigen::VectorXi frozen_bits = Eigen::VectorXi::Zero(config::N);
    read_frozen_bits(frozen_bits, config::frozen_bits_file_name);

    // 信噪比转换为线性信噪比
    snr = get_snr_to_sigma(snr, 0.5);

    // 当前数据集的索引
    int count = 0;
    std::ifstream read_file(config::output_datasets_file_name);
    // 检查文件是否成功打开
    if (!read_file.is_open()) {
        std::cerr << "无法打开文件进行读取！count赋值为0" << std::endl;
    } else {
        count = get_numbers_of_datasets(read_file) + 1;
        std::cout << "当前共有 " << count - 1 << "条数据" << std::endl;
    }
    read_file.close();

    // 打开输出文件
    std::ofstream out_file(config::output_datasets_file_name, std::ios::app);
    if (!out_file) {
        std::cerr << "无法打开数据集输出文件！" << std::endl;
        return;
    }
    // 数据集采集
    std::cout << "数据开始采集" << std::endl;
    // 用于控制while的循环变量
    int curr = 0;
    while (curr < max_count) {
        // 完成单次bp译码仿真
        // 单次码字仿真

        // 生成随机码字
        Eigen::VectorXi init_codeword = Eigen::VectorXi::Zero(config::N);
        generate_codeword(init_codeword, frozen_bits);
        // 存留生成后的随机码字
        Eigen::VectorXi int_encode_codeword = init_codeword;
        // 编码
        polar_encode(int_encode_codeword);
        // 码字向量转换为double元素类型
        Eigen::VectorXd encode_codeword = int_encode_codeword.cast<double>();

        // 添加噪声
        add_awgn_noise(encode_codeword, snr);
        // 接受向量别名，方便阅读
        const auto &received_codeword = encode_codeword;

        // 译码
        // 声明左右矩阵
        Eigen::MatrixXd left_info(config::N, config::LAYER);
        Eigen::MatrixXd right_info(config::N, config::LAYER);
        // 初始化左右矩阵
        init_left_right_info(left_info, right_info, received_codeword,
                             frozen_bits);
        // 码字判决后的码字
        Eigen::VectorXi decode_codeword = Eigen::VectorXi::Zero(config::N);
        // 记录当前译码成功标志
        bool success_decode = false;
        // 左右信息迭代计算
        for (size_t iter = 0; iter < config::MAX_ITER; iter++) {
            success_decode = false;
            // 向左计算
            left_cacl(left_info, right_info, config::N);
            // 向右计算
            right_cacl(left_info, right_info, config::N);

            // 码字判决
            get_decode_codeword(left_info, right_info, decode_codeword);
            // 检验是否译码成功
            if (check_encode_success(init_codeword, decode_codeword)) {
                // 译码成功，无需进行后续迭代
                success_decode = true;
                break;
            }
        }

        // 结束该次译码，对于译码失败的情况进行数据采集
        if (!success_decode) {
            // 生成错误比特向量
            Eigen::VectorXi error_bits_vector = init_codeword, decode_codeword;
            // 译码失败，进行数据集的采集
            output_datasets_csv(out_file, left_info, error_bits_vector, count);
            // std::cout << "采集第 " << curr << " 条数据" << '\n';
            count++;
            curr++;
        }
    }

    out_file.close();
}