#pragma once

#include <iostream>
#include <cstdlib>
#include <map>
#include <vector>
#include <functional>
#include <string>
#include <cstdio>
#include <fstream>
#include <hoc/req.h>

namespace hoc {

using cb_request_t = std::function<void(req_t &request)>;
using cb_void_t = std::function<void()>;

using cb_request_list_t = std::vector<cb_request_t>;
using cb_void_list_t = std::vector<cb_void_t>;

std::string url_encode(const std::string &str);

class app_t final {
  public:
    static app_t &get() {
      static app_t singleton;
      return singleton;
    }

    void on_request(const cb_request_t &fn);
    void on_start(const cb_void_t &fn);
    void on_exit(const cb_void_t &fn);

    void emit_request(req_t &request);
    void emit_start();
    void emit_exit();
    void log(const std::string &str);

    // defined in <hoc-mail/mail.cc>
    void send_registration_email(const std::string &email, const std::string &secret);

    // defined in <hoc/main.cc>
    void main();

    cb_request_list_t get_request_events();
    cb_void_list_t get_start_events();
    cb_void_list_t get_exit_events();
    int status;

    app_t &clear();

    const char *host;
    const char *db_name;
    const char *db_user;
    const char *db_pass;
    const char *db_host;
    const char *google_api_client_id;
    const char *google_api_client_secret;
    const char *no_reply_email;

  private:
    cb_request_list_t request_events;
    cb_void_list_t start_events;
    cb_void_list_t exit_events;

    template<typename T>
    void emit_void(T &list);

    app_t():
      status(200),
      host(std::getenv("HOC_DOMAIN")),
      db_name(std::getenv("HOC_DB_NAME")),
      db_user(std::getenv("HOC_DB_USER")),
      db_pass(std::getenv("HOC_DB_PASSWORD")),
      db_host(std::getenv("HOC_DB_HOST")),
      google_api_client_id(std::getenv("HOC_GOOGLE_API_CLIENT_ID")),
      google_api_client_secret(std::getenv("HOC_GOOGLE_API_CLIENT_SECRET")),
      no_reply_email(std::getenv("HOC_NOREPLY_EMAIL")) {};

    app_t(app_t &&) = delete;
    app_t(const app_t &) = delete;
    ~app_t() = default;
    app_t &operator=(app_t &&) = delete;
    app_t &operator=(const app_t &) = delete;

}; // app_t

}; // hoc