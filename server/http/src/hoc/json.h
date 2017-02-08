#pragma once

#include <cassert>
#include <cstdint>
#include <functional>
#include <istream>
#include <map>
#include <ostream>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace dj {

  /* A sum of the value types allowable in JSON. */
  class json_t final {
    public:

    /* Aliases for constructed types from which we may construct. */
    using boolean_t = bool;
    using number_t  = double;
    using array_t   = std::vector<json_t>;
    using object_t  = std::map<std::string, json_t>;
    using string_t  = std::string;

    /* The types of callbacks used by for_each_elem(). */
    using array_const_cb_t
        = std::function<bool (const json_t &)>;
    using object_const_cb_t
        = std::function<bool (const std::string &, const json_t &)>;
    using array_nonconst_cb_t
        = std::function<bool (json_t &)>;
    using object_nonconst_cb_t
        = std::function<bool (const std::string &, json_t &)>;

    /* A visitor which will not modify our state. */
    struct const_visitor_t {

      /* For the convenience of our descendants. */
      using boolean_t = json_t::boolean_t;
      using number_t  = json_t::number_t;
      using array_t   = json_t::array_t;
      using object_t  = json_t::object_t;
      using string_t  = json_t::string_t;

      /* Override to handle the state. */
      virtual void operator()() const = 0;
      virtual void operator()(const boolean_t &) const = 0;
      virtual void operator()(const number_t &) const = 0;
      virtual void operator()(const array_t &) const = 0;
      virtual void operator()(const object_t &) const = 0;
      virtual void operator()(const string_t &) const = 0;

    };  // json_t::const_visitor_t

    /* A visitor which may modify our state. */
    struct nonconst_visitor_t {

      /* For the convenience of our descendants. */
      using boolean_t = json_t::boolean_t;
      using number_t  = json_t::number_t;
      using array_t   = json_t::array_t;
      using object_t  = json_t::object_t;
      using string_t  = json_t::string_t;

      /* Override to handle the state. */
      virtual void operator()() const = 0;
      virtual void operator()(boolean_t &) const = 0;
      virtual void operator()(number_t &) const = 0;
      virtual void operator()(array_t &) const = 0;
      virtual void operator()(object_t &) const = 0;
      virtual void operator()(string_t &) const = 0;

    };  // json_t::nonconst_visitor_t

    /* The kinds of states we can be in. */
    enum kind_t { null, boolean, number, array, object, string };

    /* This is a look-up template used by try_get_state().  */
    template <typename state_t>
    struct for_state;

    /* Thrown by get_elem(). */
    class wrong_key_t final
        : public std::logic_error {
      public:

      /* The object does not contain the requested key. */
      wrong_key_t(const object_t &object, const std::string &key)
          : std::logic_error(write(object, key)) {}

      private:

      /* Write a human-readable error message. */
      static std::string write(
          const object_t &object, const std::string &key);

    };  // json_t::wrong_key_t

    /* Thrown by get_state() and get_size(). */
    class wrong_kind_t final
        : public std::logic_error {
      public:

      /* The JSON object is not of any of the expected kinds. */
      wrong_kind_t(
          kind_t actual_kind, const std::set<kind_t> &expected_kinds)
          : std::logic_error(write(actual_kind, expected_kinds)) {}

      private:

      /* Write a human-readable error message. */
      static std::string write(
          kind_t actual_kind, const std::set<kind_t> &expected_kinds);

    };  // json_t::wrong_kind_t

    /* Thrown by operator[size_t]. */
    class wrong_subscript_t final
        : public std::logic_error {
      public:

      /* The array does not contain the requested key. */
      wrong_subscript_t(const array_t &array, size_t index)
          : std::logic_error(write(array, index)) {}

      private:

      /* Write a human-readable error message. */
      static std::string write(const array_t &array, size_t index);

    };  // json_t::wrong_subscript_t

    /* The base for all errors thrown by read(). */
    class read_error_t
        : public std::runtime_error {
      protected:

      /* Pass the error message through. */
      read_error_t(std::string &&msg)
          : std::runtime_error(std::move(msg)) {}

    };  // json_t::read_error_t

    /* Thrown by read() when we find mangled JSON. */
    class syntax_error_t final
        : public read_error_t {
      public:

      /* We found a character we didn't expect. */
      syntax_error_t(int c, const std::set<int> &expected)
          : read_error_t(write(c, expected)) {}

      private:

      /* Write a human-readable error message. */
      static std::string write(int c, const std::set<int> &expected);

      /* Write a character to the stream, or write EOF. */
      static void write_char(std::ostream &strm, int c);

    };  // json_t::syntax_error_t

    /* Thrown by read() when we find mangled JSON. */
    class bad_escape_t final
        : public read_error_t {
      public:

      /* We found a bad escape sequence. */
      bad_escape_t()
          : read_error_t("bad escape") {}

    };  // json_t::bad_escape_t

    /* Thrown by read() when we find mangled JSON. */
    class nothing_to_read_t final
        : public read_error_t {
      public:

      /* We found a bad escape sequence. */
      nothing_to_read_t()
          : read_error_t("nothing to read") {}

    };  // json_t::nothing_to_read_t

    /* Construct as a default instance of the given kind. */
    json_t(kind_t kind = null) noexcept {
      switch (kind) {
        /* Do nothing. */
        case null:   { break; }
        /* Initialize the built-in. */
        case boolean: { boolean_ = false; break; }
        case number:  { number_  = 0;     break; }
        /* Default-construct the object. */
        case array:  { new (&array_ ) array_t (); break; }
        case object: { new (&object_) object_t(); break; }
        case string: { new (&string_) string_t(); break; }
      }
      kind_ = kind;
    }

    /* The donor is left null. */
    json_t(json_t &&that) noexcept {
      assert(&that);
      switch (that.kind_) {
        /* Do nothing. */
        case null:   { break; }
        /* Initialize the built-in. */
        case boolean: { boolean_ = that.boolean_; break; }
        case number:  { number_  = that.number_;  break; }
        /* Move-construct the object. */
        case array:  { new (&array_ ) array_t (std::move(that.array_ )); that.array_ .~array_t (); break; }
        case object: { new (&object_) object_t(std::move(that.object_)); that.object_.~object_t(); break; }
        case string: { new (&string_) string_t(std::move(that.string_)); that.string_.~string_t(); break; }
      }
      kind_ = that.kind_;
      that.kind_ = null;
    }

    /* Deep-copy. */
    json_t(const json_t &that) {
      assert(&that);
      switch (that.kind_) {
        /* Do nothing. */
        case null:   { break; }
        /* Initialize the built-in. */
        case boolean: { boolean_ = that.boolean_; break; }
        case number:  { number_  = that.number_;  break; }
        /* Copy-construct the object. */
        case array:  { new (&array_ ) array_t (that.array_ ); break; }
        case object: { new (&object_) object_t(that.object_); break; }
        case string: { new (&string_) string_t(that.string_); break; }
      }
      kind_ = that.kind_;
    }

    /* Construct as the Boolean. */
    json_t(bool that) noexcept
        : kind_(boolean), boolean_(that) {}

    /* Construct as the number */
    json_t(int8_t that) noexcept
        : kind_(number), number_(that) {}

    /* Construct as the number */
    json_t(int16_t that) noexcept
        : kind_(number), number_(that) {}

    /* Construct as the number */
    json_t(int32_t that) noexcept
        : kind_(number), number_(that) {}

    /* Construct as the number */
    json_t(int64_t that) noexcept
        : kind_(number), number_(that) {}

    /* Construct as the number */
    json_t(uint8_t that) noexcept
        : kind_(number), number_(that) {}

    /* Construct as the number */
    json_t(uint16_t that) noexcept
        : kind_(number), number_(that) {}

    /* Construct as the number */
    json_t(uint32_t that) noexcept
        : kind_(number), number_(that) {}

    /* Construct as the number */
    json_t(uint64_t that) noexcept
        : kind_(number), number_(that) {}

    /* Construct as the number */
    json_t(float that) noexcept
        : kind_(number), number_(that) {}

    /* Construct as the number */
    json_t(double that) noexcept
        : kind_(number), number_(that) {}

    /* Construct as an array, leaving the donor empty. */
    json_t(array_t &&that) noexcept
        : kind_(array), array_(std::move(that)) {}

    /* Construct as a copy of the array. */
    json_t(const array_t &that)
        : kind_(array), array_(that) {}

    /* Construct as an object, leaving the donor empty. */
    json_t(object_t &&that) noexcept
        : kind_(object), object_(std::move(that)) {}

    /* Construct as a copy of the object. */
    json_t(const object_t &that)
        : kind_(object), object_(that) {}

    /* Construct as a string of a single character. */
    json_t(char that)
        : kind_(string), string_(&that, 1) {}

    /* Construct as a string of a single character. */
    json_t(const char *that)
        : kind_(string), string_(that ? that : "") {}

    /* Construct as a string, leaving the donor empty. */
    json_t(string_t &&that) noexcept
        : kind_(string), string_(std::move(that)) {}

    /* Construct as a copy of the string. */
    json_t(const string_t &that)
        : kind_(string), string_(that) {}

    /* Construct an array of copies of the example. */
    json_t(size_t size, const json_t &example)
        : kind_(array), array_(size, example) {}

    /* Contained elements, if any, are also destroyed. */
    ~json_t() {
      assert(this);
      switch (kind_) {
        /* Do nothing. */
        case null:    { break; }
        case boolean: { break; }
        case number:  { break; }
        /* Destroy the object. */
        case array:  { array_ .~array_t (); break; }
        case object: { object_.~object_t(); break; }
        case string: { string_.~string_t(); break; }
      }
    }

    /* The donor is left null. */
    json_t &operator=(json_t &&that) noexcept {
      assert(this);
      assert(&that);
      this->~json_t();
      new (this) json_t(std::move(that));
      return *this;
    }

    /* Deep-copy. */
    json_t &operator=(const json_t &that) {
      assert(this);
      assert(&that);
      return *this = json_t(that);
    }

    /* True iff. this object and that one are in the same state. */
    bool operator==(const json_t &that) const noexcept {
      assert(this);
      assert(&that);
      bool success = (kind_ == that.kind_);
      if (success) {
        switch (kind_) {
          case null:    { break; }
          case boolean: { success = (boolean_ == that.boolean_); break; }
          case number:  { success = (number_  == that.number_ ); break; }
          case array:   { success = (array_   == that.array_  ); break; }
          case object:  { success = (object_  == that.object_ ); break; }
          case string:  { success = (string_  == that.string_ ); break; }
        }
      }
      return success;
    }

    /* True iff. this object and that one are not in the same state. */
    bool operator!=(const json_t &that) const noexcept {
      assert(this);
      return !(*this == that);
    }

    /* Subscript to an element contained in an array. */
    const json_t &operator[](size_t that) const {
      assert(this);
      const auto &array = get_state<array_t>();
      if (that >= array.size()) {
        throw wrong_subscript_t(array, that);
      }
      return array[that];
    }

    /* Subscript to an element contained in an array. */
    json_t &operator[](size_t that) {
      assert(this);
      auto &array = get_state<array_t>();
      if (that >= array.size()) {
        throw wrong_subscript_t(array, that);
      }
      return array[that];
    }

    /* Find an element contained in an object. */
    const json_t &operator[](const string_t &that) const {
      assert(this);
      return get_elem(that);
    }

    /* Find or create an element contained in an object. */
    json_t &operator[](const string_t &that) {
      assert(this);
      auto &object = get_state<object_t>();
      return object[that];
    }

    /* Find or create an element contained in an object. */
    json_t &operator[](string_t &&that) {
      assert(this);
      auto &object = get_state<object_t>();
      return object[std::move(that)];
    }

    /* Accept the visitor. */
    void accept(const const_visitor_t &visitor) const {
      assert(this);
      assert(&visitor);
      switch (kind_) {
        case null:    { visitor(        ); break; }
        case boolean: { visitor(boolean_); break; }
        case number:  { visitor(number_ ); break; }
        case array:   { visitor(array_  ); break; }
        case object:  { visitor(object_ ); break; }
        case string:  { visitor(string_ ); break; }
      }
    }

    /* Accept the visitor. */
    void accept(const nonconst_visitor_t &visitor) {
      assert(this);
      assert(&visitor);
      switch (kind_) {
        case null:    { visitor(        ); break; }
        case boolean: { visitor(boolean_); break; }
        case number:  { visitor(number_ ); break; }
        case array:   { visitor(array_  ); break; }
        case object:  { visitor(object_ ); break; }
        case string:  { visitor(string_ ); break; }
      }
    }

    /* Returns true if the object contains the given key */
    bool contains(const string_t &that) const {
      assert(this);
      return try_get_elem(that) != nullptr;
    }

    /* Call back for each element contained in an array. */
    bool for_each_elem(const array_const_cb_t &cb) const {
      assert(this);
      assert(&cb);
      assert(cb);
      assert(kind_ == array);
      for (const auto &elem: array_) {
        if (!cb(elem)) {
          return false;
        }
      }
      return true;
    }

    /* Call back for each element contained in an object. */
    bool for_each_elem(const object_const_cb_t &cb) const {
      assert(this);
      assert(&cb);
      assert(cb);
      assert(kind_ == object);
      for (const auto &elem: object_) {
        if (!cb(elem.first, elem.second)) {
          return false;
        }
      }
      return true;
    }

    /* Call back for each element contained in an array. */
    bool for_each_elem(const array_nonconst_cb_t &cb) {
      assert(this);
      assert(&cb);
      assert(cb);
      assert(kind_ == array);
      for (auto &elem: array_) {
        if (!cb(elem)) {
          return false;
        }
      }
      return true;
    }

    /* Call back for each element contained in an object. */
    bool for_each_elem(const object_nonconst_cb_t &cb) {
      assert(this);
      assert(&cb);
      assert(cb);
      assert(kind_ == object);
      for (auto &elem: object_) {
        if (!cb(elem.first, elem.second)) {
          return false;
        }
      }
      return true;
    }

    /* The named element in the object. */
    const json_t &get_elem(const std::string &key) const {
      assert(this);
      const auto &object = get_state<object_t>();
      auto iter = object.find(key);
      if (iter == object.end()) {
        throw wrong_key_t(object, key);
      }
      return iter->second;
    }

    /* The named element in the object. */
    json_t &get_elem(const std::string &key) {
      assert(this);
      auto &object = get_state<object_t>();
      auto iter = object.find(key);
      if (iter == object.end()) {
        throw wrong_key_t(object, key);
      }
      return iter->second;
    }

    /* The kind of state we're in. */
    kind_t get_kind() const noexcept {
      assert(this);
      return kind_;
    }

    /* The number of elements in an array or object or the number of
       characters in a string. */
    size_t get_size() const noexcept {
      assert(this);
      switch (kind_) {
        case array:  { return array_ .size(); }
        case object: { return object_.size(); }
        case string: { return string_.size(); }
        default: {
          throw wrong_kind_t(kind_, { array, object, string });
        }
      }
    }

    /* Our state object if we are of the requested kind; otherwise, we
       throw. */
    template <typename state_t>
    const state_t &get_state() const {
      assert(this);
      auto *state = try_get_state<state_t>();
      if (!state) {
        throw wrong_kind_t(kind_, { for_state<state_t>::kind });
      }
      return *state;
    }

    /* Our state object if we are of the requested kind; otherwise, we
       throw. */
    template <typename state_t>
    state_t &get_state() {
      assert(this);
      auto *state = try_get_state<state_t>();
      if (!state) {
        throw wrong_kind_t(kind_, { for_state<state_t>::kind });
      }
      return *state;
    }

    /* True iff. we are in the default-constructed (null) state. */
    bool is_null() const noexcept {
      assert(this);
      return kind_ == null;
    }

    /* Return to the default-constructed (null) state. */
    json_t &reset() noexcept {
      assert(this);
      this->~json_t();
      kind_ = null;
      return *this;
    }

    /* Update this object by reading a new state from the stream. */
    void read(std::istream &strm);

    /* Convert to a string. */
    std::string to_string() const;

    /* A pointer to the named element in the object, or null if we have no
       such element. */
    const json_t *try_get_elem(const std::string &key) const {
      assert(this);
      const auto &object = get_state<object_t>();
      auto iter = object.find(key);
      return (iter != object.end()) ? &(iter->second) : nullptr;
      return this;
    }

    /* A pointer to the named element in the object, or null if we have no
       such element. */
    json_t *try_get_elem(const std::string &key) {
      assert(this);
      auto &object = get_state<object_t>();
      auto iter = object.find(key);
      return (iter != object.end()) ? &(iter->second) : nullptr;
    }

    /* A pointer to our state object if we are of the requested kind;
       otherwise, a null pointer.  This uses SFINAE to remove definitions
       for any but our supported state types. */
    template <
        typename state_t,
        kind_t kind = for_state<state_t>::kind>
    const state_t *try_get_state() const {
      assert(this);
      return (this->kind_ == kind)
          ? &(this->*(for_state<state_t>::member))
          : nullptr;
    }

    /* A pointer to our state object if we are of the requested kind;
       otherwise, a null pointer.  This uses SFINAE to remove definitions
       for any but our supported state types. */
    template <
        typename state_t,
        kind_t kind = for_state<state_t>::kind>
    state_t *try_get_state() {
      assert(this);
      return (this->kind_ == kind)
          ? &(this->*(for_state<state_t>::member))
          : nullptr;
    }

    /* Write our state to the stream. */
    void write(std::ostream &strm) const;

    /* Handy instances. */
    static const json_t empty_array, empty_object;

    /* Create a new JSON object from the given string. */
    static json_t from_string(std::string &&that);

    /* A human-readable string naming the given kind. */
    static const char *get_kind_name(kind_t kind) noexcept;

    private:

    /* Read the given punctuation character from the stream or throw a
       syntax error. */
    static void read_punct(std::istream &strm, char punct);

    static std::string read_quoted_string(std::istream &strm);
    static std::string read_string(std::istream &strm);
    static std::string read_unquoted_string(std::istream &strm);

    /* Skip whitespace, then see if a comma-separated list is going to
       continue or not.  We're looking for a closing mark or, if we're not
       at the start of the list, a comma. */
    static bool try_read_comma(
        std::istream &strm, char close_mark, bool at_start);

    /* Write a quoted string with escape sequences as needed. */
    static void write_quoted_string(
        std::ostream &strm, const std::string &that);

    /* See accessor. */
    kind_t kind_;

    /* The union of our potential states.  At most one of these is valid
       at a time.  The 'kind_', above, determines which, if any, is valid. */
    union {
      boolean_t boolean_;
      number_t  number_;
      object_t  object_;
      array_t   array_;
      string_t  string_;
    };

  };  // json_t

  /* These explicit specializations map types to kinds. */
  template <> struct json_t::for_state<json_t::boolean_t> { static constexpr auto kind = boolean; static constexpr auto member = &json_t::boolean_; };
  template <> struct json_t::for_state<json_t::number_t>  { static constexpr auto kind = number;  static constexpr auto member = &json_t::number_;  };
  template <> struct json_t::for_state<json_t::object_t>  { static constexpr auto kind = object;  static constexpr auto member = &json_t::object_;  };
  template <> struct json_t::for_state<json_t::array_t >  { static constexpr auto kind = array;   static constexpr auto member = &json_t::array_;   };
  template <> struct json_t::for_state<json_t::string_t>  { static constexpr auto kind = string;  static constexpr auto member = &json_t::string_;  };

}  // dj

/* Standard inserter for json_t. */
inline std::ostream &operator<<(std::ostream &strm, const dj::json_t &that) {
  assert(&that);
  that.write(strm);
  return strm;
}

/* Standard extractor for json_t. */
inline std::istream &operator>>(std::istream &strm, dj::json_t &that) {
  assert(&that);
  that.read(strm);
  return strm;
}

/* Standard inserter for json_t::kind_t. */
inline std::ostream &operator<<(
    std::ostream &strm, dj::json_t::kind_t that) {
  assert(&strm);
  return strm << dj::json_t::get_kind_name(that);
}

namespace std {

  /* Standard swapper. */
  template <>
  inline void swap<dj::json_t>(dj::json_t &lhs, dj::json_t &rhs) noexcept {
    assert(&lhs);
    assert(&rhs);
    dj::json_t tmp = std::move(lhs);
    new (&lhs) dj::json_t(std::move(rhs));
    new (&rhs) dj::json_t(std::move(tmp));
  }

}