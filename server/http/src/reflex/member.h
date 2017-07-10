#pragma once

#include <string>
#include <memory>

namespace reflex {

template <typename obj_t, typename val_t, typename convertable_t, typename linked_obj_t = void, typename linked_val_t = void>
struct member_helper_t {

  static constexpr bool is_specialized { false };

  static bool read(const std::string &name, val_t &, const convertable_t &) {
    static_assert(
      member_helper_t<obj_t, val_t, convertable_t>::is_specialized,
      "member_helper_t<val_t, convertable_t>::read needs at least one specialization"
    );

    throw_unimplemented(name);
  }

  static bool write(const std::string &name, const val_t &, convertable_t &) {
    static_assert(
      member_helper_t<obj_t, val_t, convertable_t>::is_specialized,
      "member_helper_t<val_t, convertable_t>::write needs at least one specialization"
    );

    throw_unimplemented(name);
  }

  static void throw_unimplemented(const std::string &name) {
    std::string msg("trying to convert '");
    msg += name + "' failed. need specialization for member_helper_t<obj_t, val_t, convertable_t>::read|write";
    throw std::runtime_error(msg);
  }

};  // member_helper_t<obj_t, val_t, convertable_t, linked_obj_t, linked_val_t>

template <typename... convertables_t>
class any_member_t {

public:

  any_member_t(const std::string &name_): name(name_) {}

  const std::string name;

};  // any_member_t<convertables_t...>

template <typename obj_t, typename convertable_t>
class undefined_convertable_interface_t {

public:

  virtual void read(obj_t *obj, const convertable_t &from) = 0;

  virtual void write(const obj_t *obj, convertable_t &from) = 0;

};  // undefined_convertable_interface_t<obj_t, convertable_t>

template <typename obj_t, typename... convertables_t>
class any_member_of_obj_t: public any_member_t<convertables_t...>, virtual public undefined_convertable_interface_t<obj_t, convertables_t>... {

public:

  using any_member_t<convertables_t...>::any_member_t;

  using undefined_convertable_interface_t<obj_t, convertables_t>::read...;

  using undefined_convertable_interface_t<obj_t, convertables_t>::write...;

  virtual ~any_member_of_obj_t() = default;

};  // any_member_of_obj_t<obj_t, convertables_t...>

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

};  // defined_convertable_t<obj_t, val_t, convertable_t>

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

};  // convertable_member_t<obj_t, val_t, convertable_t>

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

};  // member_t<obj_t, val_t, convertables_t...>

template <typename obj_t, typename val_t, typename linked_obj_t, typename linked_val_t, typename convertable_t>
class defined_linked_convertable_t: virtual public undefined_convertable_interface_t<obj_t, convertable_t> {

public:

  using p2m_t = val_t (obj_t::*);

  using linked_p2m_t = linked_val_t (linked_obj_t::*);

  defined_linked_convertable_t(const std::string &name_, p2m_t ptr_, linked_p2m_t linked_ptr_): ptr(ptr_), linked_ptr(linked_ptr_), name(name_) {};

  void read(obj_t *obj, const convertable_t &from) override {
    member_helper_t<obj_t, val_t, convertable_t, linked_obj_t, linked_val_t>::read(this->name, obj->*ptr, from);
  }

  void write(const obj_t *obj, convertable_t &from) override {
    member_helper_t<obj_t, val_t, convertable_t, linked_obj_t, linked_val_t>::write(this->name, obj->*ptr, from);
  }

private:

  p2m_t ptr;

  linked_p2m_t linked_ptr;

  const std::string name;

};  // defined_linked_convertable_t<obj_t, val_t, linked_obj_t, linked_val_t, convertable_t>

template <typename obj_t, typename val_t, typename linked_obj_t, typename linked_val_t, typename... convertables_t>
class linked_convertable_member_t: public any_member_of_obj_t<obj_t, convertables_t...>, public defined_linked_convertable_t<obj_t, val_t, linked_obj_t, linked_val_t, convertables_t>...  {

public:

  using p2m_t = val_t (obj_t::*);

  using linked_p2m_t = linked_val_t (linked_obj_t::*);

  linked_convertable_member_t(const std::string &name, p2m_t ptr_, linked_p2m_t linked_ptr_): any_member_of_obj_t<obj_t, convertables_t...>(name), defined_linked_convertable_t<obj_t, val_t, linked_obj_t, linked_val_t, convertables_t>(name, ptr_, linked_ptr_)..., ptr(ptr_), linked_ptr(linked_ptr_) {}

  using defined_linked_convertable_t<obj_t, val_t, linked_obj_t, linked_val_t, convertables_t>::read...;

  using defined_linked_convertable_t<obj_t, val_t, linked_obj_t, linked_val_t, convertables_t>::write...;

  virtual ~linked_convertable_member_t() = default;

private:

  p2m_t ptr;

  linked_p2m_t linked_ptr;

};  // linked_convertable_member_t<obj_t, val_t, linked_obj_t, linked_val_t, convertables_t...>

template <typename obj_t, typename val_t, typename linked_obj_t, typename linked_val_t, typename... convertables_t>
class linked_member_t: public linked_convertable_member_t<obj_t, val_t, linked_obj_t, linked_val_t, convertables_t...> {

public:

  using p2m_t = val_t (obj_t::*);

  using linked_p2m_t = linked_val_t (linked_obj_t::*);

  using linked_convertable_member_t<obj_t, val_t, linked_obj_t, linked_val_t, convertables_t...>::read;

  using linked_convertable_member_t<obj_t, val_t, linked_obj_t, linked_val_t, convertables_t...>::write;

  linked_member_t(const std::string &name, p2m_t ptr_, linked_p2m_t linked_ptr_): linked_convertable_member_t<obj_t, val_t, linked_obj_t, linked_val_t, convertables_t...>(name, ptr_, linked_ptr_), ptr(ptr_), linked_ptr(ptr_) {}

  virtual ~linked_member_t() = default;

private:

  p2m_t ptr;

  p2m_t linked_ptr;

};  // linked_member_t<obj_t, val_t, linked_obj_t, linked_val_t, convertables_t...>

template <typename obj_t, typename val_t, typename... interfaces_t>
inline std::unique_ptr<member_t<obj_t, val_t, interfaces_t...>> make_attr(const std::string &name, val_t (obj_t::*ptr)) {
  return std::make_unique<member_t<obj_t, val_t, interfaces_t...>>(name, ptr);
}

template <typename obj_t, typename val_t, typename linked_obj_t, typename linked_val_t, typename... interfaces_t>
inline std::unique_ptr<linked_member_t<obj_t, val_t, linked_obj_t, linked_val_t, interfaces_t...>> make_linked_attr(const std::string &name, val_t (obj_t::*ptr), linked_val_t (linked_obj_t::*linked_ptr)) {
  return std::make_unique<linked_member_t<obj_t, val_t, linked_obj_t, linked_val_t, interfaces_t...>>(name, ptr, linked_ptr);
}

}   // reflex