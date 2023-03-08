#include <event2/event.h>
#include <fmt/core.h>
#include <iostream>
#include <tbb/parallel_for.h>
#include <utils/show_type.h>
#include <vector>

int main() {
  std::vector<int> vec(5);
  std::cout << "Hello, World!" << std::endl;

  fmt::print("Hello, {}!\n", "World");

  fmt::print("WITH_TBB: {}\n", WITH_TBB);
  SHOW_TYPE(std::vector<int>);
  auto bbb =3;



  return 0;
}
