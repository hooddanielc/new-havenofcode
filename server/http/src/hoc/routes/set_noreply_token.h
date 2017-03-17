#pragma once

#include <hoc/app.h>
#include <hoc/route.h>
#include <hoc/app.h>

namespace hoc {
  template<typename T>
  class set_noreply_token_route_t : public route_t<T> {
    public:
      set_noreply_token_route_t() : route_t<T>("/api/set-noreply-token") {}

      void get(T &req, const url_match_result_t &) override {
        req.on_end([&]() {
          std::string redirect_uri("https://accounts.google.com/o/oauth2/v2/auth?");

          redirect_uri
            .append("redirect_uri=http%3A%2F%2F").append(url_encode(app_t::get().host)).append("%2Fapi%2Fset-noreply-callback&")
            .append("prompt=consent&")
            .append("response_type=code&")
            .append("client_id=").append(app_t::get().google_api_client_id).append("&")
            .append("scope=https%3A%2F%2Fmail.google.com%2F+")
            .append("https%3A%2F%2Fwww.googleapis.com%2Fauth%2Fgmail.send+")
            .append("https%3A%2F%2Fwww.googleapis.com%2Fauth%2Fplus.profile.emails.read+")
            .append("https%3A%2F%2Fwww.googleapis.com%2Fauth%2Fplus.login&")
            .append("access_type=offline");

          // redirect to google login
          req.set_status(301);

          req.send_header(
            "Location",
            redirect_uri
          );

          req.set_content_length(redirect_uri.size());
          req.send_body(redirect_uri);
        });
      }
  };
}
