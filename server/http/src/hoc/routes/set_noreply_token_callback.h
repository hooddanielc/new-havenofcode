#pragma once

#include <stdexcept>
#include <curl/curl.h>
#include <hoc/app.h>
#include <hoc/route.h>
#include <hoc/json.h>
#include <hoc-db/db.h>

using namespace std;

namespace hoc {
  struct http_response_buff_t {
    char *memory;
    size_t size;
  };

  static size_t
  write_http_response_cb(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size *nmemb;
    struct http_response_buff_t *mem = (struct http_response_buff_t *) userp;

    mem->memory = (char *) realloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory == NULL) {
      /* out of memory! */ 
      printf("not enough memory (realloc returned NULL)\n");
      return 0;
    }
   
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
  }

  template<typename T>
  class set_noreply_token_callback_route_t : public route_t<T> {
    public:
      set_noreply_token_callback_route_t() : route_t<T>("/api/set-noreply-callback") {}

      void get(const T &req, const url_match_result_t &) override {
        req.on_end([&]() {
          req.send_header("Content-Type", "text/html");
          std::string args = req.args();

          if (args.size() < 5) {
            req.set_status(400);
            string body("bad request");
            req.set_content_length(body.size());
            req.send_body(body);
            return;
          }

          if (args == "error=access_denied") {
            req.set_status(400);
            string body("user failed to accept");
            req.set_content_length(body.size());
            req.send_body(body);
            return;
          }

          if (args.substr(0, 5) != "code=") {
            req.set_status(400);
            string body("user failed to accept");
            req.set_content_length(body.size());
            req.send_body(body);
            return;
          }

          CURL *curl;
          CURL *curl_get_profile;
          CURLcode res;
          curl_global_init(CURL_GLOBAL_ALL);
          curl = curl_easy_init();

          if (!curl) {
            req.set_status(500);
            string body("curl failed to init");
            req.set_content_length(body.size());
            req.send_body(body);
            curl_global_cleanup();
            return;
          }

          string authorization_code = args.substr(5, args.size());
          std::string get_token_url("https://www.googleapis.com/oauth2/v4/token");
          std::string get_token_args("code=");

          get_token_args.append(authorization_code)
            .append("&client_id=").append(app_t::get().google_api_client_id)
            .append("&client_secret=").append(app_t::get().google_api_client_secret)
            .append("&redirect_uri=http://")
            .append(url_encode(app_t::get().host)).append("/api/set-noreply-callback")
            .append("&grant_type=authorization_code");

          struct http_response_buff_t chunk;
          chunk.memory = (char *) malloc(1); 
          chunk.size = 0;

          struct http_response_buff_t chunk_get_profile;
          chunk_get_profile.memory = (char *) malloc(1); 
          chunk_get_profile.size = 0;

          curl_easy_setopt(curl, CURLOPT_URL, get_token_url.c_str());
          curl_easy_setopt(curl, CURLOPT_POSTFIELDS, get_token_args.c_str());
          curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_http_response_cb);
          curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &chunk);
          res = curl_easy_perform(curl);

          if (res != CURLE_OK) {
            req.set_status(500);
            string body("curl post request failed");
            req.set_content_length(body.size());
            req.send_body(body);
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return;
          }

          try {
            auto json = dj::json_t::from_string(chunk.memory);

            if (!json.contains("refresh_token")) {
              req.set_status(400);
              auto body = json.to_string();
              req.set_content_length(body.size());
              req.send_body(body);
            } else {
              string get_profile_url("https://www.googleapis.com/plus/v1/people/me?");
              get_profile_url.append("access_token=").append(json["access_token"].as<string>());
              curl_get_profile = curl_easy_init();
              curl_easy_setopt(curl_get_profile, CURLOPT_URL, get_profile_url.c_str());
              curl_easy_setopt(curl_get_profile, CURLOPT_WRITEFUNCTION, write_http_response_cb);
              curl_easy_setopt(curl_get_profile, CURLOPT_WRITEDATA, (void *) &chunk_get_profile);
              res = curl_easy_perform(curl_get_profile);

              if (res != CURLE_OK) {
                req.set_status(500);
                string body("curl post request failed");
                req.set_content_length(body.size());
                req.send_body(body);
                curl_easy_cleanup(curl);
                free(chunk.memory);
                curl_global_cleanup();
                return;
              } else {
                auto profile_json = dj::json_t::from_string(chunk_get_profile.memory);

                if (
                  !profile_json.contains("emails") ||
                  profile_json["emails"].get_size() == 0 ||
                  !profile_json["emails"][0].contains("value") ||
                  profile_json["emails"][0]["value"].as<string>() != app_t::get().no_reply_email
                ) {
                  req.set_status(420);
                  string body("seriously?");
                  req.set_content_length(body.size());
                  req.send_body(body);
                } else {
                  // update no_reply_email token in db

                  db_t db;
                  db.exec("BEGIN");
                  db.exec(
                    "UPDATE app_token SET refresh_token = $1 "
                    "WHERE id = 'no_reply_email'",
                    vector<db_param_t>({ json["refresh_token"].as<string>() })
                  );
                  db.exec("END");

                  req.set_status(200);
                  auto body = profile_json.to_string();
                  req.set_content_length(body.size());
                  req.send_body(body);
                }
              }
            }
          } catch (exception e) {
            req.set_status(500);
            string body(chunk.memory);
            req.set_content_length(body.size());
            req.send_body(body);
          }

          curl_easy_cleanup(curl);
          curl_easy_cleanup(curl_get_profile);
          free(chunk_get_profile.memory);
          free(chunk.memory);
          curl_global_cleanup();
        });
      }
  };
}
