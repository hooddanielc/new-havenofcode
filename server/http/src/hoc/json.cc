#include <sstream>

#include "json.h"

using namespace std;
using namespace dj;

string json_t::wrong_key_t::write(
    const object_t &object, const std::string &key) {
  ostringstream strm;
  strm << "wrong key; ";
  write_quoted_string(strm, key);
  const char *sep = " is not ";
  for (const auto &elem: object) {
    strm << sep;
    write_quoted_string(strm, elem.first);
    sep = " or ";
  }
  return strm.str();
}

string json_t::wrong_kind_t::write(
    kind_t actual_kind, const std::set<kind_t> &expected_kinds) {
  ostringstream strm;
  strm << "wrong kind; " << actual_kind;
  const char *sep = " is not ";
  for (auto kind: expected_kinds) {
    strm << sep << kind;
    sep = " or ";
  }
  return strm.str();
}

string json_t::wrong_subscript_t::write(const array_t &array, size_t index) {
  ostringstream strm;
  strm << "wrong subscript; " << index << " >= " << array.size();
  return strm.str();
}

string json_t::syntax_error_t::write(int c, const set<int> &expected) {
  ostringstream strm;
  strm << "syntax error; ";
  write_char(strm, c);
  const char *sep = " is not ";
  for (auto c: expected) {
    strm << sep;
    write_char(strm, c);
    sep = " or ";
  }
  return strm.str();
}

void json_t::syntax_error_t::write_char(ostream &strm, int c) {
  if (c >= 0) {
    strm.put(c);
  } else {
    strm << "eof";
  }
}

void json_t::read(istream &strm) {
  assert(this);
  assert(&strm);
  auto c = ws(strm).peek();
  switch (c) {
    case '[': {
      strm.ignore();
      json_t elem;
      array_t temp;
      for (;;) {
        if (!try_read_comma(strm, ']', temp.empty())) {
          break;
        }
        elem.read(strm);
        temp.emplace_back(std::move(elem));
      }
      *this = json_t(std::move(temp));
      break;
    }
    case '{': {
      strm.ignore();
      std::string key;
      json_t val;
      object_t temp;
      for (;;) {
        if (!try_read_comma(strm, '}', temp.empty())) {
          break;
        }
        key = read_string(strm);
        read_punct(strm, ':');
        val.read(strm);
        temp[std::move(key)] = std::move(val);
      }
      *this = json_t(std::move(temp));
      break;
    }
    case '"': {
      *this = json_t(read_quoted_string(strm));
      break;
    }
    default: {
      if (c < 0) {
        throw nothing_to_read_t();
      }
      if (c == '+' || c == '-' || isdigit(c)) {
        number_t temp;
        strm >> temp;
        *this = json_t(std::move(temp));
      } else {
        auto text = read_unquoted_string(strm);
        if (text == "null") {
          reset();
        } else if (text == "true") {
          *this = true;
        } else if (text == "false") {
          *this = false;
        } else {
          *this = json_t(move(text));
        }
      }
    }
  }  // switch
}

string json_t::to_string() const {
  assert(this);
  ostringstream strm;
  strm << *this;
  return strm.str();
}

void json_t::write(ostream &strm) const {
  assert(this);
  assert(&strm);
  switch (kind_) {
    case null: {
      strm << "null";
      break;
    }
    case boolean: {
      strm << (boolean_ ? "true" : "false");
      break;
    }
    case number: {
      strm << number_;
      break;
    }
    case array: {
      strm << '[';
      bool sep = false;
      for (const auto &elem: array_) {
        if (sep) {
          strm << ',';
        } else {
          sep = true;
        }
        elem.write(strm);
      }
      strm << ']';
      break;
    }
    case object: {
      strm << '{';
      bool sep = false;
      for (const auto &elem: object_) {
        if (sep) {
          strm << ',';
        } else {
          sep = true;
        }
        write_quoted_string(strm, elem.first);
        strm << ':';
        elem.second.write(strm);
      }
      strm << '}';
      break;
    }
    case string: {
      write_quoted_string(strm, string_);
      break;
    }
  }
}

const json_t
    json_t::empty_array  = json_t::array_t(),
    json_t::empty_object = json_t::object_t();

json_t json_t::from_string(std::string &&that) {
  istringstream strm(move(that));
  json_t result;
  strm >> result;
  return move(result);
}

const char *json_t::get_kind_name(kind_t kind) noexcept {
  const char *result;
  switch (kind) {
    case null:    { result = "null";    break; }
    case boolean: { result = "boolean"; break; }
    case number:  { result = "number";  break; }
    case array:   { result = "array";   break; }
    case object:  { result = "object";  break; }
    case string:  { result = "string";  break; }
  }
  return result;
}

void json_t::read_punct(istream &strm, char punct) {
  assert(&strm);
  auto c = ws(strm).peek();
  if (c != punct) {
    throw syntax_error_t(c, { punct });
  }
  strm.ignore();
}

string json_t::read_quoted_string(istream &strm) {
  assert(&strm);
  read_punct(strm, '"');
  std::ostringstream accum;
  bool go = true;
  do {
    auto c = strm.peek();
    switch (c) {
      /* Closing quote. */
      case '"': {
        strm.ignore();
        go = false;
        break;
      }
      /* Escape sequence. */
      case '\\': {
        strm.ignore();
        switch (strm.peek()) {
          case '\\': { accum.put('\\'); break; }
          case '"':  { accum.put('\"'); break; }
          case '/':  { accum.put('/' ); break; }
          case 'b':  { accum.put('\b'); break; }
          case 'f':  { accum.put('\f'); break; }
          case 'n':  { accum.put('\n'); break; }
          case 'r':  { accum.put('\r'); break; }
          case 't':  { accum.put('\t'); break; }
          case 'u': {
            uint32_t val = 0;
            for (size_t i = 0; i < 4; ++i) {
              strm.ignore();
              c = strm.peek();
              if (c >= '0' && c <= '9') {
                c -= '0';
              } else if (c >= 'A' && c <= 'F') {
                c -= 'A' - 10;
              } else if (c >= 'a' && c <= 'f') {
                c -= 'a' - 10;
              } else {
                throw syntax_error_t(
                    c,
                    { '0', '1', '2', '3', '4', '5', '6', '7',
                      '7', '8', '9', 'A', 'B', 'C', 'D', 'E',
                      'a', 'b', 'c', 'd', 'e', 'f' });
              }
              val = val * 16 + c;
            }  // for
            if (val <= 0x7F) {
              accum.put(val);
            } else if (val <= 0x7FF) {
              accum.put(0xC0 | (val >> 6));
              accum.put(0x80 | (val & 0x3F));
            } else if (val <= 0xFFFF) {
              accum.put(0xE0 | (val >> 12));
              accum.put(0x80 | ((val >> 6) & 0x3F));
              accum.put(0x80 | (val & 0x3F));
            } else {
              throw bad_escape_t();
            }
            break;
          }
          default: {
            throw bad_escape_t();
          }
        }  // switch
        strm.ignore();
        break;
      }
      /* Normal character or EOF. */
      default: {
        if (c < 0) {
          throw syntax_error_t(c, { '"' });
        }
        accum.put(c);
        strm.ignore();
      }
    }  // switch
  } while (go);
  return accum.str();
}

string json_t::read_string(istream &strm) {
  assert(&strm);
  return (ws(strm).peek() == '"')
      ? read_quoted_string(strm)
      : read_unquoted_string(strm);
}

string json_t::read_unquoted_string(istream &strm) {
  assert(&strm);
  ostringstream accum;
  for (;;) {
    auto c = strm.peek();
    bool go;
    switch (c) {
      case '[': case ']':
      case '{': case '}':
      case ',': case ':': case '"': {
        go = false;
        break;
      }
      default: {
        go = (c >= 0) && !isspace(c);
      }
    }
    if (!go) {
      break;
    }
    accum.put(c);
    strm.ignore();
  }
  return accum.str();
}

bool json_t::try_read_comma(
    istream &strm, char close_mark, bool at_start) {
  assert(&strm);
  auto c = ws(strm).peek();
  if (c == close_mark) {
    strm.ignore();
    return false;
  }
  if (!at_start) {
    read_punct(strm, ',');
  }
  return true;
}

void json_t::write_quoted_string(ostream &strm, const std::string &that) {
  assert(&strm);
  assert(&that);
  strm << '"';
  for (auto c: that) {
    switch (c) {
      case '\\': {
        strm << R"(\\)";
        break;
      }
      case '\"': {
        strm << R"(\")";
        break;
      }
      case '\b': {
        strm << R"(\b)";
        break;
      }
      case '\f': {
        strm << R"(\f)";
        break;
      }
      case '\n': {
        strm << R"(\n)";
        break;
      }
      case '\r': {
        strm << R"(\r)";
        break;
      }
      case '\t': {
        strm << R"(\t)";
        break;
      }
      default: {
        strm << c;
      }
    }  // switch
  }  // for
  strm << '"';
}
