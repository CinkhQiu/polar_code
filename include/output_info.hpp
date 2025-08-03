#pragma once

struct output_info {
    double curr_snr;
    int error_bits = 0;
    int error_frames = 0;
    int total_frames = 0;
    int total_iter = 0;
    int flip_success = 0;
    double error_frames_rate;
    double error_bits_rate;
    double sim_minutes = 0.0;
    double sim_seconds = 0.0;
};