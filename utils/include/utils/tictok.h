/** @file    tictok.h
 *  @time    2023/2/27 ~ 下午10:05
 *  @author  Leon
 *
 *  @note    Timing a section of codes
 *
 */

#pragma once

#include <chrono>
#include <iostream>

namespace utils {

/*!
 * Macros for timing a section of codes in ms (roughly)
 * @note Copy from Internet:
 */
#define TIC(x) auto bench_##x = std::chrono::steady_clock::now();
#define TOK(x)                                                                                                                             \
  std::cout << "<-- Time of \"" #x "\": "                                                                                                  \
            << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(std::chrono::steady_clock::now() - bench_##x).count() \
            << "(ms) -->\n";

}  // namespace utils
