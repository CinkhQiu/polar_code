#include "simulate.h"

// 完成一次信噪比下的仿真
// 输入参数：冻结位向量，信噪比，output_info
void run(Eigen::VectorXi &frozen_bits, double snr,
         output_info &thread_output_info) {
    // 程序开始计时
    auto start_time = std::chrono::high_resolution_clock::now();

    while (thread_output_info.total_frames < config::MAX_FRAME &&
           thread_output_info.error_frames < config::MAX_ERROR_FRAME) {
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
                thread_output_info.total_iter += iter;
                break;
            }
        }
        // 结束该次译码，进行数据统计
        thread_output_info.total_frames++;
        if (!success_decode) {
            // 译码失败，进行错误译码信息统计
            thread_output_info.total_iter += config::MAX_ITER;
            error_data_stastics(init_codeword, decode_codeword,
                                thread_output_info.error_bits,
                                thread_output_info.error_frames);
        }
    }

    // 获取结束时间点
    auto end_time = std::chrono::high_resolution_clock::now();
    // 计算时间差，单位为秒
    std::chrono::duration<double> elapsed_seconds = end_time - start_time;

    // 将时间转换为分钟和秒
    thread_output_info.sim_minutes =
        static_cast<int>(elapsed_seconds.count()) / 60;
    thread_output_info.sim_seconds =
        elapsed_seconds.count() - (thread_output_info.sim_minutes * 60);

    // 记录总的误帧率
    thread_output_info.error_frames_rate =
        (thread_output_info.error_frames * 1.0) /
        thread_output_info.total_frames;

    // 记录总的误比特率
    thread_output_info.error_bits_rate = (thread_output_info.error_bits * 1.0) /
                                         (thread_output_info.total_frames * config::N);

    std::cout << "sigma " << snr << " 仿真结束"
              << "，误码率为：" << thread_output_info.error_frames_rate
              << std::endl;
}

// bp译码算法的多线程仿真
void threads_run() {
    // 读取冻结位信息
    Eigen::VectorXi frozen_bits = Eigen::VectorXi::Zero(config::N);
    read_frozen_bits(frozen_bits, config::frozen_bits_file_name);

    // 创建固定数量的线程
    std::vector<std::thread> threads;
    // 创建用于存储仿真信息的vector
    std::vector<output_info> all_output_info(config::TOTAL_STEP_NUMS);

    // 生成信噪比标准差向量
    std::vector<double> snr_to_sigma(config::TOTAL_STEP_NUMS);
    double curr_snr = config::START_SNR;
    for (size_t i = 0; i < config::TOTAL_STEP_NUMS; i++) {
        // 生成当前信噪比下的标准差
        snr_to_sigma[i] = get_snr_to_sigma(curr_snr, config::RATE);
        curr_snr += config::STEP;
    }

    // 打开文件
    std::ofstream output_file(config::output_file_name, std::ios::app);
    if (output_file.is_open()) {
        // 朝输出文件中添加日期
        output_file << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++"
                       "+++++++++++"
                    << "\n";
        output_file << "++++++++++++++++++++++" << get_current_date() << "\n";
        output_file << "---------------------"
                    << "多线程版本：当前仿真码长为：" << config::N
                    << "---------------------"
                    << "\n";
    } else {
        std::cerr << "打开输出文件失败" << std::endl;
    }

    // 记录多线程仿真总时间
    auto start_time = std::chrono::high_resolution_clock::now();

    // 创建多线程
    for (size_t i = 0; i < config::TOTAL_STEP_NUMS; i++) {
        all_output_info[i].curr_snr = config::START_SNR + config::STEP * i;
        threads.emplace_back(run, std::ref(frozen_bits), snr_to_sigma[i],
                             std::ref(all_output_info[i]));
    }

    // 等待所有线程完成
    for (auto &thread : threads) {
        thread.join();
    }

    // 获取结束时间点
    auto end_time = std::chrono::high_resolution_clock::now();
    // 计算时间差，单位为秒
    std::chrono::duration<double> elapsed_seconds = end_time - start_time;

    // 将时间转换为分钟和秒
    int minutes = static_cast<int>(elapsed_seconds.count()) / 60;
    double seconds = elapsed_seconds.count() - (minutes * 60);

    if (output_file.is_open()) {
        // 朝输出文件中写入多线程仿真总耗时
        output_file << "****当前多线程信噪比仿真总耗时 " << minutes << " 分 "
                    << seconds << " 秒****"
                    << "\n";
    } else {
        std::cerr << "打开输出文件失败" << std::endl;
    }

    // 向文件中输出结果
    for (size_t i = 0; i < config::TOTAL_STEP_NUMS; i++) {
        // 记录当前仿真信噪比
        output_file << "****************"
                    << "当前仿真信噪比为：" << std::fixed
                    << std::setprecision(2) << config::START_SNR + config::STEP * i
                    << "****************"
                    << "\n";
        // 记录当前信噪比仿真耗时
        output_file << "当前信噪比仿真耗时 " << all_output_info[i].sim_minutes
                    << " 分 " << all_output_info[i].sim_seconds << " 秒"
                    << "\n";
        // 当前仿真允许最大帧数
        output_file << "当前仿真允许最大帧数：" << config::MAX_FRAME << "\n";
        // 当前仿真允许最大错误帧数
        output_file << "当前仿真允许最大错误帧数：" << config::MAX_ERROR_FRAME << "\n";
        // 记录总的仿真次数
        output_file << "当前仿真次数为：" << all_output_info[i].total_frames
                    << "\n";
        // 记录当前错误帧数
        output_file << "当前错误帧数为：" << all_output_info[i].error_frames
                    << "\n";
        // 记录总的误帧率
        output_file << "当前误帧率为" << std::fixed << std::setprecision(8)
                    << all_output_info[i].error_frames_rate << "\n";
        // 记录总的误比特率
        output_file << "当前误比特率为" << std::fixed << std::setprecision(8)
                    << all_output_info[i].error_bits_rate << "\n";
        // 记录总的迭代次数
        output_file << "BP译码总迭代次数为：" << all_output_info[i].total_iter
                    << "\n";
    }

    output_file << "-----------------------------------------------------------"
                   "-----------"
                << "\n";

    output_file.close(); // 关闭文件
}

// 关键集生成运行测试
void critical_sets_run() {
    // 读取冻结位信息
    Eigen::VectorXi frozen_bits = Eigen::VectorXi::Zero(config::N);
    read_frozen_bits(frozen_bits, config::frozen_bits_file_name);
    std::vector<int> critical_sets;

    generate_critical_sets(frozen_bits, critical_sets);
}

// cs-bp-flip译码算法的多线程性能仿真
// 基于关键集的比特翻转译码算法性能仿真测试
void bit_flip_simu_test() {
    // 读取冻结位信息
    Eigen::VectorXi frozen_bits = Eigen::VectorXi::Zero(config::N);
    read_frozen_bits(frozen_bits, config::frozen_bits_file_name);

    // 创建固定数量的线程
    std::vector<std::thread> threads;
    // 创建用于存储仿真信息的vector
    std::vector<output_info> all_output_info(config::TOTAL_STEP_NUMS);

    // 生成信噪比标准差向量
    std::vector<double> snr_to_sigma(config::TOTAL_STEP_NUMS);
    double curr_snr = config::START_SNR;
    for (size_t i = 0; i < config::TOTAL_STEP_NUMS; i++) {
        // 生成当前信噪比下的标准差
        snr_to_sigma[i] = get_snr_to_sigma(curr_snr, config::RATE);
        curr_snr += config::STEP;
    }

    // 打开文件
    std::ofstream output_file(config::output_file_bp_flip_cs_name, std::ios::app);
    if (output_file.is_open()) {
        // 朝输出文件中添加日期
        output_file << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++"
                       "+++++++++++"
                    << "\n";
        output_file << "++++++++++++++++++++++" << get_current_date() << "\n";
        output_file << "---------------------"
                    << "CS-BP-Flip译码算法：多线程版本：当前仿真码长为：" << config::N
                    << "---------------------"
                    << "\n";
    } else {
        std::cerr << "打开输出文件失败" << std::endl;
    }

    // 记录多线程仿真总时间
    auto start_time = std::chrono::high_resolution_clock::now();

    // 创建多线程
    for (size_t i = 0; i < config::TOTAL_STEP_NUMS; i++) {
        all_output_info[i].curr_snr = config::START_SNR + config::STEP * i;
        threads.emplace_back(cs_bp_flip_run, std::ref(frozen_bits),
                             snr_to_sigma[i], std::ref(all_output_info[i]));
    }

    // 等待所有线程完成
    for (auto &thread : threads) {
        thread.join();
    }

    // 获取结束时间点
    auto end_time = std::chrono::high_resolution_clock::now();
    // 计算时间差，单位为秒
    std::chrono::duration<double> elapsed_seconds = end_time - start_time;

    // 将时间转换为分钟和秒
    int minutes = static_cast<int>(elapsed_seconds.count()) / 60;
    double seconds = elapsed_seconds.count() - (minutes * 60);

    if (output_file.is_open()) {
        // 朝输出文件中写入多线程仿真总耗时
        output_file << "****当前多线程信噪比仿真总耗时 " << minutes << " 分 "
                    << seconds << " 秒****"
                    << "\n";
    } else {
        std::cerr << "打开输出文件失败" << std::endl;
    }

    // 向文件中输出结果
    for (size_t i = 0; i < config::TOTAL_STEP_NUMS; i++) {
        // 记录当前仿真信噪比
        output_file << "****************"
                    << "当前仿真信噪比为：" << std::fixed
                    << std::setprecision(2) << config::START_SNR + config::STEP * i
                    << "****************"
                    << "\n";
        // 记录当前信噪比仿真耗时
        output_file << "当前信噪比仿真耗时 " << all_output_info[i].sim_minutes
                    << " 分 " << all_output_info[i].sim_seconds << " 秒"
                    << "\n";
        // 当前仿真允许最大帧数
        output_file << "当前仿真允许最大帧数：" << config::MAX_FRAME << "\n";
        // 当前仿真允许最大错误帧数
        output_file << "当前仿真允许最大错误帧数：" << config::MAX_ERROR_FRAME << "\n";
        // 记录总的仿真次数
        output_file << "当前仿真次数为：" << all_output_info[i].total_frames
                    << "\n";
        // 记录当前错误帧数
        output_file << "当前错误帧数为：" << all_output_info[i].error_frames
                    << "\n";
        // 记录总的误帧率
        output_file << "当前误帧率为" << std::fixed << std::setprecision(8)
                    << all_output_info[i].error_frames_rate << "\n";
        // 记录总的误比特率
        output_file << "当前误比特率为" << std::fixed << std::setprecision(8)
                    << all_output_info[i].error_bits_rate << "\n";
        // 记录总的翻转成功帧数
        output_file << "翻转成功帧数为" << all_output_info[i].flip_success
                    << "\n";
        // 记录总的迭代次数
        output_file << "BP译码总迭代次数为：" << all_output_info[i].total_iter
                    << "\n";
    }

    output_file << "-----------------------------------------------------------"
                   "-----------"
                << "\n";

    output_file.close(); // 关闭文件
}

// cs-bp-flip译码算法一次信噪比下的仿真
// 输入参数：冻结位向量，信噪比，output_info
void cs_bp_flip_run(Eigen::VectorXi &frozen_bits, double snr,
                    output_info &thread_output_info) {
    // 程序开始计时
    auto start_time = std::chrono::high_resolution_clock::now();

    while (thread_output_info.total_frames < config::MAX_FRAME &&
           thread_output_info.error_frames < config::MAX_ERROR_FRAME) {
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

        // 获取翻转集
        std::vector<int> critical_sets;
        generate_critical_sets(frozen_bits, critical_sets);

        // 获取翻转集大小
        int cs_length = critical_sets.size();

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
                thread_output_info.total_iter += iter;
                break;
            }
        }

// 若当前译码失败，进入到bit-flip译码算法中
#if 1
        if (!success_decode) {
            // 译码失败，进入到bp-flip译码算法中
            // 遍历翻转集合
            for (size_t i = 0; i < cs_length; i++) {
                double llr = received_codeword(critical_sets[i]);
                // 重新初始化左右矩阵
                init_left_right_info(left_info, right_info, received_codeword,
                                     frozen_bits);
                // 翻转
                if (llr > 0) {
                    right_info(critical_sets[i], 0) = MIN_INF;
                } else {
                    right_info(critical_sets[i], 0) = MAX_INF;
                }

                // 翻转后重新进入到bp译码算法中进行译码
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
                        thread_output_info.total_iter += iter;
                        break;
                    }
                }
                if (success_decode == true) {
                    thread_output_info.flip_success++;
                    break;
                }
            }
        }
#endif

        // 信息统计
        // 结束该次译码，进行数据统计
        thread_output_info.total_frames++;
        if (!success_decode) {
            // 译码失败，进行错误译码信息统计
            thread_output_info.total_iter += config::MAX_ITER;
            error_data_stastics(init_codeword, decode_codeword,
                                thread_output_info.error_bits,
                                thread_output_info.error_frames);
        }
    }

    // 获取结束时间点
    auto end_time = std::chrono::high_resolution_clock::now();
    // 计算时间差，单位为秒
    std::chrono::duration<double> elapsed_seconds = end_time - start_time;

    // 将时间转换为分钟和秒
    thread_output_info.sim_minutes =
        static_cast<int>(elapsed_seconds.count()) / 60;
    thread_output_info.sim_seconds =
        elapsed_seconds.count() - (thread_output_info.sim_minutes * 60);

    // 记录总的误帧率
    thread_output_info.error_frames_rate =
        (thread_output_info.error_frames * 1.0) /
        thread_output_info.total_frames;

    // 记录总的误比特率
    thread_output_info.error_bits_rate = (thread_output_info.error_bits * 1.0) /
                                         (thread_output_info.total_frames * config::N);

    std::cout << "sigma " << snr << " 仿真结束"
              << "，误码率为：" << thread_output_info.error_frames_rate
              << "，翻转成功数为：" << thread_output_info.flip_success
              << "总帧数为：" << thread_output_info.total_frames << std::endl;
}