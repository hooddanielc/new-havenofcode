#pragma once

#include <iostream>
#include <functional>
#include <map>
#include <vector>

namespace hoc {

template <typename iterable_t>
class basic_multipart_parser_t {
private:
  using cb_header_t = std::function<void(const std::map<iterable_t, iterable_t> &)>;
  using cb_body_t = std::function<void(const iterable_t &)>;
  using cb_end_t = std::function<void()>;

  // callbacks
  std::vector<cb_header_t> cb_header_list;
  std::vector<cb_body_t> cb_body_list;
  std::vector<cb_end_t> cb_end_list;

  // anchors and buffers
  iterable_t boundary_buffer;
  typename iterable_t::iterator boundary_anchor;
  iterable_t header_key_buffer;
  iterable_t header_val_buffer;
  std::map<iterable_t, iterable_t> header_buffer;
  typename iterable_t::iterator header_anchor;
  iterable_t body_buffer;

  // state
  enum state_t {
    BEGIN,
    BOUNDARY,
    BOUNDARY_END,
    BOUNDARY_TERM,
    HEADER_KEY,
    HEADER_VALUE,
    HEADER_END,
    BODY_BOUNDARY,
    BODY,
    END,
    END_TERM
  };

  state_t state;

  void emit_end() {
    for (auto it = cb_end_list.begin(); it != cb_end_list.end(); ++it) {
      (*it)();
    }
  }

  void emit_header(const std::map<iterable_t, iterable_t> &headers) {
    for (auto it = cb_header_list.begin(); it != cb_header_list.end(); ++it) {
      (*it)(headers);
    }
  }

  void emit_body(const iterable_t &body) {
    for (auto it = cb_body_list.begin(); it != cb_body_list.end(); ++it) {
      (*it)(body);
    }
  }

public:
  const std::string boundary;
  const iterable_t comparable_boundary;
  const size_t max_body_buffer_size;
  basic_multipart_parser_t(const char *_boundary, size_t _max_body_buffer_size = 16000) :
    state(BEGIN),
    boundary(std::string("--") + _boundary),
    comparable_boundary(boundary.begin(), boundary.end()),
    max_body_buffer_size(_max_body_buffer_size) {}

  void on_header(const cb_header_t &fn) {
    cb_header_list.push_back(fn);
  }

  void on_body(const cb_body_t &fn) {
    cb_body_list.push_back(fn);
  }

  void on_multipart_end(const cb_end_t &fn) {
    cb_end_list.push_back(fn);
  }

  void push_char(char c) {
    if (c == '\r') {
      return;
    }

    switch (state) {
      case BEGIN:
        state = BOUNDARY;
      case BOUNDARY:
        if (isspace(c)) {
          boundary_buffer.clear();
        } else {
          boundary_buffer.push_back(c);
        }

        if (
          boundary_buffer.size() == comparable_boundary.size() &&
          boundary_buffer == comparable_boundary
        ) {
          state = BOUNDARY_END;
        }

        break;
      case BOUNDARY_END:
        if (c == '\n') {
          boundary_buffer.clear();
          state = HEADER_KEY;
        } else if (c == '-') {
          state = BOUNDARY_TERM;
          boundary_buffer.push_back(c);
        } else {
          throw std::runtime_error("found invalid character end of boundary");
        }

        break;
      case BOUNDARY_TERM:
        if (c == '-') {
          state = END;
        }
      case HEADER_KEY:
        if (c == ':') {
          state = HEADER_VALUE;
        } else if (c != '\n') {
          header_key_buffer.push_back(c);
        }

        break;
      case HEADER_VALUE:
        if (c == '\n') {
          state = HEADER_END;
          header_buffer[header_key_buffer] = header_val_buffer;
          header_val_buffer.clear();
          header_key_buffer.clear();
        } else if (!(header_val_buffer.size() == 0 && isspace(c))) {
          header_val_buffer.push_back(c);
        }

        break;
      case HEADER_END:
        if (c == '\n') {
          state = BODY;
          emit_header(header_buffer);
          header_buffer.clear();
        } else {
          header_key_buffer.push_back(c);
          state = HEADER_KEY;
        }

        break;
      case BODY_BOUNDARY:
      case BODY:
        if (c == '\n') {
          boundary_buffer.clear();
        } else {
          boundary_buffer.push_back(c);
        }

        if (
          (boundary_buffer.size() == 0 || boundary_buffer.size() > comparable_boundary.size()) &&
          body_buffer.size() >= max_body_buffer_size
        ) {
          emit_body(body_buffer);
          body_buffer.clear();
        }

        body_buffer.push_back(c);

        if (
          boundary_buffer.size() == comparable_boundary.size() &&
          boundary_buffer == comparable_boundary
        ) {
          if (body_buffer.size() > comparable_boundary.size() + 1) {
            body_buffer.erase(body_buffer.end() - comparable_boundary.size() - 1, body_buffer.end());
          } else {
            body_buffer.clear();
          }

          emit_body(body_buffer);
          body_buffer.clear();
          state = BOUNDARY_END;
        }
        break;
      case END:
        if (c != '\n') {
          throw std::runtime_error("multipart ended");
        }

        emit_end();
        state = END_TERM;
        break;
      case END_TERM:
        throw std::runtime_error("multipart ended");
        break;
    }
  }
};

template <typename iterable_t>
std::ostream& operator<<(std::ostream &os, const basic_multipart_parser_t<iterable_t> &that) {
  // write remaining data to os
  os << &that[0];
  return os;
}

template <typename iterable_t>
std::istream& operator>>(std::istream &is, basic_multipart_parser_t<iterable_t> &that) {
  char c;
  while (is >> c) {
    that.push_char(c);
  }
  return is;
}

template <typename iterable_t>
const char *operator>>(const char *is, basic_multipart_parser_t<iterable_t> &that) {
  for (int i = 0; is[i] != '\0'; ++i) {
    that.push_char(is[i]);
  }
  return is;
}

template <typename iterable_t>
const std::string &operator>>(const std::string &is, basic_multipart_parser_t<iterable_t> &that) {
  for (auto it = is.begin(); it != is.end(); ++it) {
    that.push_char(*it);
  }
  return is;
}

template <typename iterable_t>
const iterable_t &operator>>(const iterable_t &is, basic_multipart_parser_t<iterable_t> &that) {
  for (auto it = is.begin(); it != is.end(); ++it) {
    that.push_char(*it);
  }
  return is;
}

class multipart_parser_t : public basic_multipart_parser_t<std::string> {
  public:
    multipart_parser_t(const char *boundary) : basic_multipart_parser_t<std::string>(boundary) {}
};

class multipart_binary_parser_t : public basic_multipart_parser_t<std::vector<uint8_t>> {
  public:
    multipart_binary_parser_t(const char *boundary) : basic_multipart_parser_t<std::vector<uint8_t>>(boundary) {}
};

} // namespace hoc