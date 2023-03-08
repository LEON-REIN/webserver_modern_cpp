/** @file    printer.h
 *  @time    2023/2/27 ~ 下午10:06
 *  @author  Leon
 *
 *  @note    Utils for simple & easy printing
 *  @attention May not correct!!
 *
 */

#pragma once

#include <fmt/core.h>
#include <tuple>
#include <map>
#include <vector>
#include <unordered_map>
#include <set>
#include <unordered_set>


namespace utils {

/*!
 * Just print a dividing line
 */
#define DividingLine(tag) std::cout << "\n############### "#tag" ###############\n";


/*!
 * print a tuple
 * @tparam ...Ts
 * @param tup
 * @return None
 *
 * @note May need C++20 or gcc extension
 */
template <typename... Ts>
constexpr void print(const std::tuple<Ts...>& tup) {
  constexpr std::size_t num_args{sizeof...(Ts) - 1};
  [&]<std::size_t... Idx>(std::index_sequence<Idx...>) {
    (fmt::print("{}{}", std::get<Idx>(tup), num_args != Idx ? "; " : "\n"), ...);
    //  ((std::cout << std::get<Idx>(tup) << (num_args != Idx ? "; " : "\n")), ...);
  }(std::index_sequence_for<Ts...>{});
  // std::make_index_sequence<sizeof...(Ts)>{}
}

/*!
 * print a sequential container
 * @tparam Container
 * @param container
 * @return None
 */
template <typename Container, typename = std::void_t<decltype(std::begin(std::declval<Container>()))>>
constexpr void print(const Container& container) {
  for (const auto& item : container) {
    fmt::print("{},\t", item);
  }
  fmt::print("\n");
}

/*!
 * print a set-like container
 * @tparam Set
 * @tparam Key
 * @param set
 * @return
 */
template <template <typename> typename Set, typename Key,
          typename = std::void_t<decltype(std::declval<Set<Key>>().insert(std::declval<Key>()))>>
constexpr void print(const Set<Key>& set) {
  fmt::print("[");
  for (const auto& key : set) {
    fmt::print("{},\t", key);
  }
  fmt::print("]\n");
}

/*!
 * print a map-like container
 * @tparam Map
 * @tparam Key
 * @tparam Value
 * @param map
 * @return None
 */
template <template <typename, typename> typename Map, typename Key, typename Value,
          typename = std::void_t<decltype(std::declval<Map<Key, Value>>().insert(std::declval<std::pair<Key, Value>>()))>>
constexpr void print(const Map<Key, Value>& map) {
  for (const auto& [key, value] : map) {
    fmt::print("[{}, {}],\t", key, value);
  }
  fmt::print("\n");
}

/*!
 * print a raw array
 * @tparam T
 * @tparam N
 * @param T array[N]
 * @return
 */
template <typename T, std::size_t N>
constexpr void print(const T (&array)[N]) {
  for (std::size_t i{0}; i < N; ++i) {
    fmt::print("{},\t", array[i]);
  }
  fmt::print("\n");
}


template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>, void>>
constexpr void print(T t) {
  fmt::print("{}\n", t);
}

/*!
 * print for const char *
 * @param str
 */
void print(const char * str){
  fmt::print("{}\n", str);
}

/*!
 * print for string
 * @param string
 */
void print(const std::string& string){
  fmt::print("{}\n", string);
}

/*!
 * print for string_view
 * @param string_view
 */
void print(std::string_view string_v){
  fmt::print("{}\n", string_v);
}

/*!
 * print many
 * @tparam Args
 * @param arg
 * @return
 */
template<typename ... Args>
[[maybe_unused]] constexpr void prints(Args ... arg){
  (print(arg), ...);
}

}  // namespace utils

/*  Some tests
utils::print(std::tuple(1, 2.0f, 3.0, "4.0"sv));
utils::print(std::map<int, int> {{1, 10}, {4, 15}, {3, 20}});
utils::print(std::set {"CPU"s, "Hello"s, "GPU"s});
utils::print(std::vector {0, 1, 2});
utils::print(std::array {4, 5, 6});
utils::print({0, 5, 6});
auto aa = {0, 5, 6};
utils::print(aa);
int bb[] {8, 9, 55};
utils::print(bb);
utils::print(667);
utils::print("Test OK (maybe...)"sv);

DividingLine(hello)
utils::prints(aa, std::set {"CPU"s, "Hello"s, "GPU"s}, "Hello!"s, 666);
*/
