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

  class url_match_result_t {
    public:
      url_match_result_t(std::vector<std::string> params) : pass(true), params(params) {}
      url_match_result_t() : pass(false) {}
      bool pass;
      std::vector<std::string> params;
  };

  template <typename T>
  class route_t {
    public:
      route_t(std::string pat) : pattern(pat) {}

      void get(const T &req) {};
      void post(const T &req) {};
      void put(const T &req) {};
      void del(const T &req) {};

      std::string get_pattern() {
        return pattern;
      };

      url_match_result_t match(const char *url) {
        // gather a list of tokens

        enum state_t {
          start,
          literal,
          literal_begin,
          param,
          end
        };

        state_t state(start);
        const char *c = get_pattern().c_str();
        std::string tmp;
        std::vector<url_token_t> tokens;

        while (state != end) {
          switch (state) {
            case start:
              if (*c == ':') {
                state = param;
              } else if (c == '\0') {
                state = end;
              } else {
                state = literal_begin;
              }

              break;
            case literal_begin:
              if (*c == '\0') {
                state = end;
              } else if (*c == ':') {
                state = param;
              } else if (*c == '/') {
                state = literal_begin;
              } else {
                state = literal;
                tmp += *c;
              }

              break;
            case literal:
              if (*c == ':') { // found end of literal
                state = param;
                tokens.push_back(url_token_t(url_literal, tmp));
                tmp.clear();
              } else if (*c == '/') {
                state = literal_begin;
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
                state = literal_begin;
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
            case end:
              break;
          }

          c++;
        }

        // now that we have tokens
        // match it against url
        std::vector<std::string> params;
        std::string subject(url);

        for (url_token_t &tok : tokens) {
          if (tok.type == url_literal) {
            if (subject.size() < tok.val.size() + 1) {
              return url_match_result_t();
            } else {
              if (
                subject.substr(1, tok.val.size()) == tok.val &&
                subject.substr(tok.val.size() + 1, 1) == "/"
              ) {
                subject.erase(0, tok.val.size() + 1);
              } else {
                return url_match_result_t();
              }
            }
          } else if (tok.type == url_param) {
            if (subject.size() > 0) {
              size_t idx = subject.find_first_of("/", 1);
              params.push_back(subject.substr(1, idx - 1));

              if (idx + 1 == subject.size()) {
                subject.clear();
              } else {
                subject.erase(0, idx);
              }
            } else {
              return url_match_result_t();
            }
          }
        }

        if (subject.size() == 0) {
          return url_match_result_t(params);
        } else {
          return url_match_result_t();
        }
      }
    protected:
      std::string pattern;
  };
}
