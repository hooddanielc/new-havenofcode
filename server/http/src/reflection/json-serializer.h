#pragma once
#include <string>
#include <reflection/serializer.h>
#include <hoc/json.h>

// default json serializer
template<typename val_t>
struct serialize_member_t<val_t, hoc::json> {
  void operator()(const std::string &name, const val_t &val, hoc::json &from) {
    from[name] = val;
  }
};  // serialize_member_t<val_t, hoc::json>

// default json deserializer
template <typename val_t>
struct deserialize_member_t<val_t, hoc::json> {
  void operator()(const std::string &name, val_t &val, const hoc::json &from) {
    if (from.count(name)) {
      val = from[name].template get<val_t>();
    }
  }
};  // deserialize_member_t<val_t, hoc::json>

// std::experimental::optional
template <typename val_t>
struct deserialize_member_t<std::experimental::optional<val_t>, hoc::json> {
  void operator()(const std::string &name, std::experimental::optional<val_t> &val, const hoc::json &from) {
    if (from.count(name)) {
      val = from[name].template get<val_t>();
    }
  }
};  // deserialize_member_t<val_t, hoc::json>

// std::experimental::optional
template<typename val_t>
struct serialize_member_t<std::experimental::optional<val_t>, hoc::json> {
  void operator()(const std::string &name, const std::experimental::optional<val_t> &val, hoc::json &from) {
    if (val) {
      from[name] = val.value();
    }
  }
};  // serialize_member_t<val_t, hoc::json>

template <typename obj_t, typename val_t>
class json_serializer_t: public member_serializer_t<obj_t, val_t, hoc::json> {

public:

  using member_serializer_t<obj_t, val_t, hoc::json>::member_serializer_t;

};  // json_serializer_t
