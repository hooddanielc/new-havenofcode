#pragma once

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <cassert>
#include <reflex/member.h>

namespace reflex {

template <typename obj_t, typename... convertables_t>
class reflection_t {

public:

  using obj_member_t = std::shared_ptr<any_member_of_obj_t<obj_t, convertables_t...>>;

  using member_list_t = std::vector<obj_member_t>;

  const std::string name;

  reflection_t(): name("unknown") {
    validate_reflection();
  }

  reflection_t(member_list_t members) : name("unknown"), member_ptrs(members) {
    validate_reflection();
  }

  reflection_t(const std::string &name_, member_list_t members) : name(name_), member_ptrs(members) {
    validate_reflection();
  }

  template <typename val_t>
  void add_attribute(const std::string &name, val_t (obj_t::*ptr)) {
    member_ptrs.push_back(make_attr<obj_t, val_t, convertables_t...>(name, ptr));
  }

  member_list_t get_members() const {
    return member_ptrs;
  }

  void for_each(const std::function<void(obj_member_t member)> &fn) const {
    for (auto &member: member_ptrs) {
      fn(member);
    }
  }

  template <typename from_t>
  void write(const obj_t *obj, from_t &from) const {
    for_each([&](auto member) {
      member->write(obj, from);
    });
  }

  template <typename from_t>
  void read(obj_t *obj, const from_t &from) const {
    for_each([&](auto member) {
      member->read(obj, from);
    });
  }

private:

  member_list_t member_ptrs;

  void validate_reflection() {
    static_assert(
      std::is_same<decltype(obj_t::reflection), const reflection_t<obj_t, convertables_t...>>::value ||
      std::is_same<decltype(obj_t::reflection), reflection_t<obj_t, convertables_t...>>::value,
      "reflection_t instance must be assigned to static property on reflected object"
    );
  }

};  // reflection_t<obj_t, convertables_t...>

template <typename... convertables_t>
struct reflex_pack {

  template <typename obj_t>
  struct with {

    using ref_t = reflection_t<obj_t, convertables_t...>;

    using attr_t = any_member_of_obj_t<obj_t, convertables_t...>;

    template <typename val_t>
    static std::unique_ptr<attr_t> make_attr(const std::string &name, val_t (obj_t::*ptr)) {
      return std::make_unique<member_t<obj_t, val_t, convertables_t...>>(name, ptr);
    }   // make_attr<val_t>

  };  // with<obj_t>

};  // reflex_pack<convertables_t...>

}   // reflex
