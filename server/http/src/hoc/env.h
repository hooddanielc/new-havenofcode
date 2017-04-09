#pragma once

namespace hoc {
  class env_t final {
    public:
      static env_t &get() {
        static env_t singleton;
        return singleton;
      }

      const char *host;
      const char *db_name;
      const char *db_user;
      const char *db_pass;
      const char *db_host;
      const char *google_api_client_id;
      const char *google_api_client_secret;
      const char *no_reply_email;
      const char *anonymous_user;
      const char *anonymous_pass;
      const char *aws_key;
      const char *aws_secret;

      env_t():
        host(std::getenv("HOC_DOMAIN")),
        db_name(std::getenv("HOC_DB_NAME")),
        db_user(std::getenv("HOC_DB_USER")),
        db_pass(std::getenv("HOC_DB_PASSWORD")),
        db_host(std::getenv("HOC_DB_HOST")),
        google_api_client_id(std::getenv("HOC_GOOGLE_API_CLIENT_ID")),
        google_api_client_secret(std::getenv("HOC_GOOGLE_API_CLIENT_SECRET")),
        no_reply_email(std::getenv("HOC_NOREPLY_EMAIL")),
        anonymous_user("anonymous"),
        anonymous_pass("password"),
        aws_key(std::getenv("HOC_AWS_KEY")),
        aws_secret(std::getenv("HOC_AWS_SECRET")) {};
  
    private:
      env_t(env_t &&) = delete;
      env_t(const env_t &) = delete;
      ~env_t() = default;
      env_t &operator=(env_t &&) = delete;
      env_t &operator=(const env_t &) = delete;
  };
}
