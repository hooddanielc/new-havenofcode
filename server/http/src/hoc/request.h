#pragma once

#include <iostream>
#include <stdexcept>
#include <functional>
#include <vector>
#include <curl/curl.h>

namespace hoc {
  class request_t {
    using cb_data_t = std::function<void(char *data, size_t len)>;

    public:
      request_t();
      ~request_t();

      void add_header(const char *value);
      void set_url(const char *url);
      void set_method(const char *method);
      void on_data(const cb_data_t &fn);
      void send();
      void send(const std::string &data);

    private:
      // prevent copy
      request_t(request_t &&) = delete;
      request_t(const request_t &) = delete;
      request_t &operator=(request_t &&) = delete;
      request_t &operator=(const request_t &) = delete;

      // hold curl c stuff
      CURL *curl;
      struct curl_slist *slist_headers;

      struct write_t {
        const char *readptr;
        size_t sizeleft;
      };

      // callbacks
      static size_t write_func(void *ptr, size_t size, size_t nmemb, request_t *cake);
      static size_t string_read_func(void *ptr, size_t size, size_t nmemb, write_t *cake);

      // hold data callbacks
      std::vector<cb_data_t> data_callbacks;
  };
}
