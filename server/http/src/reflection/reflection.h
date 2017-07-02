#pragma once
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <cassert>
#include <reflection/member.h>

template <typename obj_t>
class reflection_t {

public:

  using obj_member_t = std::shared_ptr<any_member_of_obj_t<obj_t>>;

  using member_list_t = std::vector<obj_member_t>;

  reflection_t() {
    validate_reflection();
  }

  reflection_t(member_list_t members) : member_ptrs(members) {
    validate_reflection();
  }

  template <typename val_t>
  void add_attribute(const std::string &name, val_t (obj_t::*ptr)) {
    member_ptrs.push_back(make_attr(name, ptr));
  }

  member_list_t get_members() {
    return member_ptrs;
  }

  void for_each(const std::function<void(obj_member_t member)> &fn) {
    for (auto &member: member_ptrs) {
      fn(member);
    }
  }

  template <typename from_t>
  void write(const obj_t *obj, from_t &from) {
    for_each([&](auto member) {
      member->write(obj, from);
    });
  }

  template <typename from_t>
  void read(obj_t *obj, const from_t &from) {
    for_each([&](auto member) {
      member->read(obj, from);
    });
  }

private:

  member_list_t member_ptrs;

  void validate_reflection() {
    static_assert(
      std::is_same<decltype(obj_t::name), std::string>::value,
      "reflected object must have a name who's type is std::string"
    );
  }

};
