/**
 * @file shared_info.h
 * @author Wuqiong Zhao (wqzhao@seu.edu.cn)
 * @brief Shared Information Between 'Export' and 'Simulate'
 * @version 0.1.0
 * @date 2022-10-12
 * 
 * @copyright Copyright (c) 2022 Wuqiong Zhao (Teddy van Jerry)
 * 
 */

#ifndef _EXPORT_SHARED_INFO_H_
#define _EXPORT_SHARED_INFO_H_

#include <string>

struct Shared_Info {
    std::string backend = "cpp";
    std::string src_compile_cmd;
    std::string tex_compile_cmd;
};

#endif