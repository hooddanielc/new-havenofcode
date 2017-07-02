#pragma once
#include <string>
#include <memory>

class any_member_t {

public:

  any_member_t(const std::string &name_) : name(name_) {}

  virtual std::string get_name() {
    return name;
  }

  const std::string &name;

};

template <typename obj_t>
class any_member_of_obj_t: public any_member_t {

public:

  any_member_of_obj_t(const std::string &name_) : any_member_t(name_) {};

};

template <typename obj_t, typename val_t, typename... interfaces_t>
class member_t: public any_member_of_obj_t<obj_t>, public interfaces_t... {

public:

  using p2m_t = val_t (obj_t::*);

  member_t(const std::string &name_, p2m_t ptr_) : any_member_of_obj_t<obj_t>(name_), interfaces_t(name_, ptr_)..., ptr(ptr_) {}

private:

  p2m_t ptr;

};

template <typename obj_t, typename val_t, typename... interfaces_t>
inline std::unique_ptr<member_t<obj_t, val_t>> make_attr(const std::string &name, val_t (obj_t::*ptr)) {
  return std::make_unique<member_t<obj_t, val_t, interfaces_t...>>(name, ptr);
}
