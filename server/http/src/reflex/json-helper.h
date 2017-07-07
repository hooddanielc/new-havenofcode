#pragma once

#include <string>
#include <hoc/json.h>
#include <reflex/member.h>

namespace reflex {

  template <typename obj_t, typename val_t>
  struct member_helper_t<obj_t, val_t, hoc::json> {

    static constexpr bool is_specialized { true };

    static void read(const std::string &name, val_t &val, const hoc::json &from) {
      if (from.count(name)) {
        val = from[name].template get<val_t>();
      }
    }

    static void write(const std::string &name, const val_t &val, hoc::json &from) {
      from[name] = val;
    }

  };  // member_helper_t<obj_t, val_t, hoc::json>

}   // reflex

