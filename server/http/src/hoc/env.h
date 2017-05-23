#pragma once

#include <cstdlib>
#include <string>
#include <hoc/util.h>

namespace hoc {

// A string variable that can set itself from the shell environment.  This is
// safe to construct in the static data segment.  It looks up its value using
// getenv(), but will do so at most once, and only when it is requested.  If
// the shell has no value for the variable's key, the variable will use a
// default you supply at construction time.
//
// For example:
//
//    env_var_t desktop_session {
//      "DESKTOP_SESSION",  // The key used in getenv().
//      "ubuntu"            // The value used if getenv() returns null.
//    };
//    if (desktop_session.get() == "ubuntu") { ... }
//
// You may explicitly set the variable's value by calling set(), or clear the
// variable's value by calling reset().  Setting and clearing a variable
// manually can be useful in unit testing or when parsing the command line or
// reading a config file.  However, in production, it is most common simply to
// allow the variable to set itself from the environment.
class env_var_t final {
public:

  // Caches the key string and the default value string.
  explicit env_var_t(std::string key_, std::string def_val_ = "")
      : key(std::move(key_)), def_val(std::move(def_val_)),
        is_set(false) { get(); }

  // Returns the value string of the variable.  If this is the first call
  // to get() and there has been no call to set(), this will call getenv()
  // to see if the key is set in the shell.  If so, it will cache and return
  // the shell's value string.  If not, it will cache the default value
  // string.  Subsequent calls to get() will return the cached string until
  // set() or reset() is called.
  const std::string &get() const {
    if (!is_set) {
      const char *env_val = getenv(key.c_str());
      if (env_val) {
        val = env_val;
      } else {
        val = std::move(def_val);
      }
      is_set = true;
    }
    return val;
  }

  // The key used in the call to getenv().  This is cached at construction
  // and never changes.
  const std::string &get_key() const noexcept {
    return key;
  }

  bool operator==(const std::string &str) const noexcept {
    return str == get();
  }

  // Clears the value stored in the variable and sets the default string.
  // This puts the variable back into its newly constructed state.  The
  // next call to get() will call getenv() again.
  void reset(std::string def_val_ = "") {
    val.clear();
    def_val = std::move(def_val_);
    is_set = false;
  }

  // Sets the value string cached in the variable.  Subsequent calls to get()
  // will return this string.
  void set(std::string new_val) noexcept {
    is_set = true;
    val = std::move(new_val);
  }

private:

  // See accessor.
  const std::string key;

  // The default value string.  If get() uses this, it moves it to val,
  // leaving this string empty.
  std::string def_val;

  // Initially false, then set bu get() or set().  This flag is cleared by
  // reset().
  mutable bool is_set;

  // If is_set is true, this contains the cached value string; otherwise,
  // this contains junk.
  mutable std::string val;

};  // env_var_t;

inline bool operator==(const std::string &str, const env_var_t &var) {
  return str == var.get();
}

class env_t final {
  public:
    static env_t &get() {
      static env_t singleton;
      return singleton;
    }

    env_var_t host;
    env_var_t db_name;
    env_var_t db_user;
    env_var_t db_pass;
    env_var_t db_host;
    env_var_t upload_tmp_path;
    env_var_t upload_buffer_size;
    env_var_t google_api_client_id;
    env_var_t google_api_client_secret;
    env_var_t no_reply_email;
    env_var_t anonymous_user;
    env_var_t anonymous_pass;
    env_var_t aws_key;
    env_var_t aws_secret;
    env_var_t mock_s3_uploads;

    env_t():
      host("HOC_DOMAIN"),
      db_name("HOC_DB_NAME"),
      db_user("HOC_DB_USER"),
      db_pass("HOC_DB_PASSWORD"),
      db_host("HOC_DB_HOST"),
      upload_tmp_path("HOC_HTTP_UPLOAD_TEMP_PATH"),
      upload_buffer_size("HOC_HTTP_UPLOAD_BUFFER_SIZE"),
      google_api_client_id("HOC_GOOGLE_API_CLIENT_ID"),
      google_api_client_secret("HOC_GOOGLE_API_CLIENT_SECRET"),
      no_reply_email("HOC_NOREPLY_EMAIL"),
      anonymous_user("HOC_ANONYMOUS_USER", "anonymous"),
      anonymous_pass("HOC_ANONYMOUS_USER", "password"),
      aws_key("HOC_AWS_KEY"),
      aws_secret("HOC_AWS_SECRET"),
      mock_s3_uploads("HOC_MOCK_S3_UPLOADS") {};

  private:
    env_t(env_t &&) = delete;
    env_t(const env_t &) = delete;
    ~env_t() = default;
    env_t &operator=(env_t &&) = delete;
    env_t &operator=(const env_t &) = delete;
};
}
