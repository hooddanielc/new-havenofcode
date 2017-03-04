#pragma once

#include <iostream>
#include <functional>
#include <hoc/app.h>

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
    private:
      route_t(route_t &&) = delete;
      route_t(const route_t &) = delete;
      route_t &operator=(route_t &&) = delete;
      route_t &operator=(const route_t &) = delete;

    protected:
      route_t(std::string pat) : pattern(pat) {}

    public:
      virtual void get(const T &, const url_match_result_t &) {};
      virtual void post(const T &, const url_match_result_t &) {};
      virtual void put(const T &, const url_match_result_t &) {};
      virtual void del(const T &, const url_match_result_t &) {};

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
        std::string tmpp;
        std::vector<std::string> subject_tokens;

        for (size_t i = 0; i < subject.size(); ++i) {
          char c = subject.at(i);

          if (c == '/') {
            if (tmpp.size() > 0) {
              subject_tokens.push_back(tmpp);
              tmpp.clear();
            }
          } else if (c != '\0') {
            tmpp += c;
          }
        }

        if (tmpp.size() > 0) {
          subject_tokens.push_back(tmpp);
        }

        if (tokens.size() != subject_tokens.size()) {
          return url_match_result_t();
        }

        for (size_t i = 0; i < tokens.size(); ++i) {
          if (tokens[i].type == url_literal) {
            if (tokens[i].val != subject_tokens[i]) {
              return url_match_result_t();
            }
          } else if (tokens[i].type == url_param) {
            params.push_back(subject_tokens[i]);
          }
        }

        return url_match_result_t(params);
      }
    protected:
      std::string pattern;
  };
}