/** @file    show_type.h
 *  @time    2023/2/27 ~ 下午9:48
 *  @author  Leon
 *
 *  @note    A tool for showing the type
 *
 */
#pragma once

#include <cstdlib>
#include <iostream>
#include <string>
#if defined(__GNUC__) || defined(__clang__)
#include <cxxabi.h>
#endif

namespace utils {

/*!
 * A function for showing a type. Please use Macro `SHOW_TYPE(...)`
 * @tparam T
 * @return std::string
 */
template<typename T>
std::string cpp_type_name() {
  const char *name = typeid(T).name();
#if defined(__GNUC__) || defined(__clang__)
  int status;
  char *p = abi::__cxa_demangle(name, nullptr, nullptr, &status);
  std::string s{p};
  std::free(p);
#else
  std::string s{name};
#endif
  if (std::is_volatile_v<std::remove_reference_t<T>>)
    s = "volatile " + s;
  if (std::is_const_v<std::remove_reference_t<T>>)
    s = "const " + s;
  if (std::is_lvalue_reference_v<T>)
    s += " &";
  if (std::is_rvalue_reference_v<T>)
    s += " &&";
  return s;
}
#define SHOW_TYPE(T) std::cout << "{" #T "}'s type is --> " << utils::cpp_type_name<T>() << '\n';


/*!
 * An empty function, showing the type from compile errors. Usage: `foo<T>();`
 * @tparam T
 * @return None
 */
template<typename T>
T foo();

}// namespace utils
