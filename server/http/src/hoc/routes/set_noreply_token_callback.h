#pragma once

#include <stdexcept>
#include <hoc/app.h>
#include <hoc/route.h>
#include <hoc/json.h>
#include <hoc/request.h>
#include <hoc/db/connection.h>

namespace hoc {

template<typename T>
class set_noreply_token_callback_route_t : public route_t<T> {
  public:
    set_noreply_token_callback_route_t() : route_t<T>("/api/set-noreply-callback") {}

    void get(T &req, const url_match_result_t &) override {
      req.on_end([&]() {
        req.send_header("Content-Type", "text/html");
        std::string args = req.args();

        if (args.size() < 5) {
          return route_t<T>::fail_with_error(req, "invalid arguments");
        }

        if (args == "error=access_denied") {
          return route_t<T>::fail_with_error(req, "user failed to accept");
        }

        if (args.substr(0, 5) != "code=") {
          return route_t<T>::fail_with_error(req, "code not provided");
        }

        std::string authorization_code = args.substr(5, args.size());
        std::string get_token_url("https://www.googleapis.com/oauth2/v4/token");
        std::string get_token_args("code=");

        get_token_args.append(authorization_code)
          .append("&client_id=").append(app_t::get().google_api_client_id)
          .append("&client_secret=").append(app_t::get().google_api_client_secret)
          .append("&redirect_uri=http://")
          .append(url_encode(app_t::get().host)).append("/api/set-noreply-callback")
          .append("&grant_type=authorization_code");

        request_t get_token_request;
        get_token_request.set_url(get_token_url.c_str());
        get_token_request.add_header("Expect:");
        get_token_request.add_header("Transfer-Encoding: chunked");
        std::string token_response_data;

        get_token_request.on_data([&token_response_data](const char *data, size_t len) {
          token_response_data.append(std::string{ data, len });
        });

        get_token_request.send(get_token_args);
        auto json = dj::json_t::from_string(token_response_data.c_str());

        if (!json.contains("refresh_token")) {
          return route_t<T>::fail_with_error(req, "missing refresh token");
        } else {
          std::string get_profile_url("https://www.googleapis.com/plus/v1/people/me?");
          get_profile_url.append("access_token=").append(json["access_token"].as<std::string>());
          request_t get_profile_request;
          get_profile_request.set_url(get_profile_url.c_str());
          std::string get_profile_response;

          get_profile_request.on_data([&get_profile_response](const char *data, size_t len) {
            get_profile_response.append(std::string{ data, len });
          });

          get_profile_request.send();
          auto profile_json = dj::json_t::from_string(get_profile_response.c_str());

          if (
            !profile_json.contains("emails") ||
            profile_json["emails"].get_size() == 0 ||
            !profile_json["emails"][0].contains("value") ||
            profile_json["emails"][0]["value"].as<std::string>() != app_t::get().no_reply_email
          ) {
            req.set_status(420);
            std::string body("seriously?");
            req.set_content_length(body.size());
            req.send_body(body);
          } else {
            auto c = db::super_user_connection();
            pqxx::work w(*c);
            std::stringstream ss;
            ss << "update app_token set refresh_token = "
               << w.quote(json["refresh_token"].as<std::string>()) << " "
               << "where id = 'no_reply_email'";
            w.exec(ss);
            w.commit();

            req.set_status(200);
            auto body = profile_json.to_string();
            req.set_content_length(body.size());
            req.send_body(body);
          }
        }
      });
    }
};

} // hoc