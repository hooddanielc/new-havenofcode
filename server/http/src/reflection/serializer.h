#pragma once

template<typename val_t, typename from_t>
struct serialize_member_t {
  void operator()(const std::string &name, const val_t &val, from_t &from);
};  // serialize_member_t<val_t, from_t>

template <typename val_t, typename from_t>
struct deserialize_member_t {
  void operator()(const std::string &name, val_t &val, const from_t &from);
};  // deserialize_member_t<val_t, from_t>

template <typename obj_t, typename from_t>
class any_serializer_of_obj_t {};

template <typename obj_t, typename val_t, typename from_t>
class member_serializer_t: public any_serializer_of_obj_t<obj_t, from_t> {

public:

  using p2m_t = val_t (obj_t::*);

  member_serializer_t(const std::string &name_, p2m_t ptr_) : name(name_), ptr(ptr_) {}

  virtual void write(const obj_t *obj, from_t &from) {
    serialize(this->name, obj->*(this->ptr), from);
  }

  virtual void read(obj_t *obj, const from_t &from) {
    deserialize(this->name, obj->*(this->ptr), from);
  }

protected:

  serialize_member_t<val_t, from_t> serialize;

  deserialize_member_t<val_t, from_t> deserialize;

  const std::string &name;

  p2m_t ptr;

};  // member_serializer_t<typename obj_t, typename val_t, typename from_t>
