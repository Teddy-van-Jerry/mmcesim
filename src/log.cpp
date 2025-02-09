/**
 * @file log.cpp
 * @author Wuqiong Zhao (wqzhao@seu.edu.cn)
 * @brief Implementation of Log Class
 * @version 0.2.1
 * @date 2023-03-10
 *
 * @copyright Copyright (c) 2023 Wuqiong Zhao (Teddy van Jerry)
 *
 */

#include "log.h"

Log::Log() : _f(appDir() + "/mmcesim.log"), _open(_f.is_open()) {
    if (_open) {
        // get the current time
        std::time_t curr_time   = std::time(nullptr);
        std::tm curr_tm         = *std::localtime(&curr_time);
        const char* time_format = "%F %T (UTC %z)";
        info() << "* Time     : " << std::put_time(&curr_tm, time_format) << "\n";
        info() << "* Version  : " << _MMCESIM_VER_STR << "\n";
        info() << "* System   : " << spy::operating_system << "-" << spy::architecture << "\n";
        info() << "* Compiler : " << spy::compiler << std::endl;
    } else {
        std::cerr << "[WARNING] Cannot open the log file to write." << std::endl;
    }
}

Log::~Log() {
    if (_open) _f.close();
}

void Log::writeArg(int argc, char* argv[]) {
    if (!_open) return;
    _f << "[INFO] * CLI Args : " << argv[0];
    for (int i = 1; i < argc; ++i) _f << " \"" << argv[i] << '"';
    _f << std::endl;
}

Log _log;
