#pragma once

#include <hoc/route.h>
#include <hoc/json.h>

namespace hoc {
  template<typename T>
  class logout_route_t : public route_t<T> {
    public:
      logout_route_t() : route_t<T>("/api/logout") {}

      void get(T &req, const url_match_result_t &) override {
        req.on_end([&]() {
          try {
            auto session = req.get_identity();
            session.invalidate(req);
            auto result = dj::json_t::empty_object;
            result["success"] = true;
            route_t<T>::send_json(req, result, 200);
          } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            return route_t<T>::fail_with_error(req, "session error");
          }
        });
      }
  };
}
