#pragma once

#include <hoc/route.h>

using namespace std;

namespace hoc {
  template<typename T>
  class login_route_t : public route_t<T> {
    public:
      login_route_t() : route_t<T>("/api/login") {}

      void get(const T &, const url_match_result_t &) override {
        std::cout << "todo, route login" << std::endl;
      }
  };
}
