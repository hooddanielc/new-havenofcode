#pragma once

#include <iostream>
#include <functional>

namespace hoc {
  enum url_token_type_t {
    url_literal,
    url_param
  };

  class url_token_t {
    public:
      url_token_t(url_token_type_t type, std::string val) : val(val), type(type) {}
      std::string val;
      url_token_type_t type;
  };

  template <typename T>
  class route_t {
    public:
      route_t(std::string pat) : pattern(pat) { std::cout << pattern << std::endl; }

      void get(const T &req) {};
      void post(const T &req) {};
      void put(const T &req) {};
      void del(const T &req) {};

      std::string get_pattern() {
        return pattern;
      };

      bool match(const char *url) {
        // gather a list of tokens

        enum state_t {
          start,
          literal,
          param,
          end
        };

        state_t state(literal);
        const char *c = get_pattern().c_str();
        std::string tmp;
        std::vector<url_token_t> tokens;

        while (state != end) {
          std::cout << *c << std::endl;

          switch (state) {
            case start:
              if (*c == ':') {
                state = param;
              } else if (c == '\0') {
                state = end;
              } else {
                state = literal;
              }

              break;
            case literal:
              if (*c == ':') { // found end of literal
                state = param;
                tokens.push_back(url_token_t(url_literal, tmp));
                tmp.clear();
              } else if (*c == '\0') {
                state = end;
                tokens.push_back(url_token_t(url_literal, tmp));
                tmp.clear();
              } else {
                tmp += *c;
              }

              break;
            case param:
              if (*c == '/') {
                state = literal;
                tokens.push_back(url_token_t(url_param, tmp));
                tmp.clear();
              } else if (*c == '\0') {
                state = end;
                tokens.push_back(url_token_t(url_param, tmp));
                tmp.clear();
              } else {
                tmp += *c;
              }

              break;
          }

          c++;
        }

        // now that we have tokens
        // match it against url

        return true;
      }
    protected:
      std::string pattern;
  };
}
