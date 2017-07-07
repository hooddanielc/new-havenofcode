#pragma once

#include <string>
#include <memory>

namespace reflex {

template <typename obj_t, typename val_t, typename convertable_t>
struct member_helper_t {

  static constexpr bool is_specialized { false };

  static void read(const std::string &, val_t &, const convertable_t &) {
    static_assert(
      member_helper_t<obj_t, val_t, convertable_t>::is_specialized,
      "member_helper_t<val_t, convertable_t>::read needs specialization"
    );
  }

  static void write(const std::string &, const val_t &, convertable_t &) {
    static_assert(
      member_helper_t<obj_t, val_t, convertable_t>::is_specialized,
      "member_helper_t<val_t, convertable_t>::write needs specialization"
    );
  }

};

template <typename... convertables_t>
class any_member_t {

public:

  any_member_t(const std::string &name_): name(name_) {}

  const std::string name;

};

template <typename obj_t, typename convertable_t>
class undefined_convertable_interface_t {

public:

  virtual void read(obj_t *obj, const convertable_t &from) = 0;

  virtual void write(const obj_t *obj, convertable_t &from) = 0;

};

template <typename obj_t, typename... convertables_t>
class any_member_of_obj_t: public any_member_t<convertables_t...>, virtual public undefined_convertable_interface_t<obj_t, convertables_t>... {

public:

  using any_member_t<convertables_t...>::any_member_t;

  using undefined_convertable_interface_t<obj_t, convertables_t>::read...;

  using undefined_convertable_interface_t<obj_t, convertables_t>::write...;

  virtual ~any_member_of_obj_t() = default;

};

template <typename obj_t, typename val_t, typename convertable_t>
class defined_convertable_t: virtual public undefined_convertable_interface_t<obj_t, convertable_t> {

public:

  using p2m_t = val_t (obj_t::*);

  defined_convertable_t(const std::string &name_, p2m_t ptr_): ptr(ptr_), name(name_) {};

  void read(obj_t *obj, const convertable_t &from) override {
    member_helper_t<obj_t, val_t, convertable_t>::read(this->name, obj->*ptr, from);
  }

  void write(const obj_t *obj, convertable_t &from) override {
    member_helper_t<obj_t, val_t, convertable_t>::write(this->name, obj->*ptr, from);
  }

private:

  p2m_t ptr;

  const std::string name;

};

template <typename obj_t, typename val_t, typename... convertables_t>
class convertable_member_t: public any_member_of_obj_t<obj_t, convertables_t...>, public defined_convertable_t<obj_t, val_t, convertables_t>...  {

public:

  using p2m_t = val_t (obj_t::*);

  convertable_member_t(const std::string &name, p2m_t ptr_): any_member_of_obj_t<obj_t, convertables_t...>(name), defined_convertable_t<obj_t, val_t, convertables_t>(name, ptr_)..., ptr(ptr_) {}

  using defined_convertable_t<obj_t, val_t, convertables_t>::read...;

  using defined_convertable_t<obj_t, val_t, convertables_t>::write...;

  virtual ~convertable_member_t() = default;

private:

  p2m_t ptr;

};

template <typename obj_t, typename val_t, typename... convertables_t>
class member_t: public convertable_member_t<obj_t, val_t, convertables_t...> {

public:

  using p2m_t = val_t (obj_t::*);

  using convertable_member_t<obj_t, val_t, convertables_t...>::read;

  using convertable_member_t<obj_t, val_t, convertables_t...>::write;

  member_t(const std::string &name, p2m_t ptr_): convertable_member_t<obj_t, val_t, convertables_t...>(name, ptr_), ptr(ptr_) {}

  virtual ~member_t() = default;

private:

  p2m_t ptr;

};

template <typename obj_t, typename val_t, typename... interfaces_t>
inline std::unique_ptr<member_t<obj_t, val_t, interfaces_t...>> make_attr(const std::string &name, val_t (obj_t::*ptr)) {
  return std::make_unique<member_t<obj_t, val_t, interfaces_t...>>(name, ptr);
}

}   // reflex