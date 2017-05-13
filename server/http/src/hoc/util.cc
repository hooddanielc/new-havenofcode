#include <hoc/util.h>

using namespace std;

namespace hoc {
  void hexchar(unsigned char c, unsigned char &hex1, unsigned char &hex2) {
    hex1 = c / 16;
    hex2 = c % 16;
    hex1 += hex1 <= 9 ? '0' : 'a' - 10;
    hex2 += hex2 <= 9 ? '0' : 'a' - 10;
  }

  string url_encode(const string &s) {
    const char *str = s.c_str();
    vector<char> v(s.size());
    v.clear();
    for (size_t i = 0, l = s.size(); i < l; i++) {
      char c = str[i];
      if (
        (c >= '0' && c <= '9') ||
        (c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        c == '-' || c == '_' || c == '.' || c == '!' || c == '~' ||
        c == '*' || c == '\'' || c == '(' || c == ')'
      ) {
        v.push_back(c);
      } else if (c == ' ') {
        v.push_back('+');
      } else {
        v.push_back('%');
        unsigned char d1, d2;
        hexchar(c, d1, d2);
        v.push_back(d1);
        v.push_back(d2);
      }
    }

    return string(v.cbegin(), v.cend());
  }

  string url_decode(const string &s) {
    std::string result;
    auto src = s.c_str();

    char a, b;
    while (*src) {
      if (
        (*src == '%') &&
        ((a = src[1]) && (b = src[2])) &&
        (isxdigit(a) && isxdigit(b))
      ) {
        if (a >= 'a'){
          a -= 'a'-'A';
        }

        if (a >= 'A') {
          a -= ('A' - 10);
        } else {
          a -= '0';
        }

        if (b >= 'a') {
          b -= 'a'-'A';
        }

        if (b >= 'A') {
          b -= ('A' - 10);
        } else{
          b -= '0';
        }

        result += 16*a+b;
        src+=3;
      } else if (*src == '+') {
        result += ' ';
        src++;
      } else {
        result += *src++;
      }
    }

    return result;
  }

  void open_ifstream(std::ifstream &strm, const std::string &path) {
    strm.open(path);
    if (!strm) {
      std::ostringstream msg;
      msg << "could not open " << std::quoted(path) << " for reading";
      throw std::runtime_error { msg.str() };
    }
    strm.exceptions(std::ios::badbit | std::ios::failbit);
  }

  std::string random_characters(size_t size) {
    std::string result;

    while (result.size() != size) {
      char *tmp = static_cast<char *>(alloca(size));
      {
        std::ifstream strm;
        open_ifstream(strm, "/dev/urandom");
        strm.read(tmp, static_cast<std::streamsize>(size));
      }

      for (size_t i = 0; i < size; ++i) {
        if (result.size() == size) {
          break;
        }

        if (isalnum(tmp[i])) {
          result += tmp[i];
        }
      }
    }

    return result;
  }

  string camelify(string &str) {
    for (auto it = str.begin(); it != str.end(); ++it) {
      if (*it == '_') {
        str.erase(it);
        *it = toupper(*(it));
      }
    }
    return str;
  }

  // convert postgres result to json array
  dj::json_t::array_t to_json(const pqxx::result &result) {
    dj::json_t::array_t json;

    for (pqxx::tuple::size_type y = 0; y < result.size(); ++y) {
      auto obj = dj::json_t::empty_object;

      for (pqxx::tuple::size_type x = 0; x < result.columns(); ++x) {
        string name(result.column_name(x));
        camelify(name);

        if (result[y][x].is_null()) {
          obj[name] = dj::json_t::null;
          continue;
        }

        switch (result[y][x].type()) {
          case 16:
          case 1000:
            obj[name] = result[y][x].as<bool>();
            break;
          case 21: // int2
          case 23: // int4
            obj[name] = result[y][x].as<int>();
            break;
          case 700: // float4
          case 701: // float8
            obj[name] = result[y][x].as<double>();
            break;
          default:
            obj[name] = result[y][x].as<string>();
        }
      }

      json.emplace_back(obj);
    }

    return json;
  }

  std::string random_tmp_path() {
    std::string p(env_t::get().upload_tmp_path);
    p += '/';
    p.append(random_characters(50));
    return p;
  }

  std::string get_s3_url(const dj::json_t &file) {
    Aws::Client::ClientConfiguration config;
    config.region = file["awsRegion"].as<std::string>().c_str();

    Aws::S3::S3Client client(Aws::Auth::AWSCredentials(
      env_t::get().aws_key,
      env_t::get().aws_secret
    ), config);

    const char *key = file["awsKey"].as<std::string>().substr(1).c_str();
    const char *bucket = file["awsBucket"].as<std::string>().c_str();

    return client.GeneratePresignedUrl(
      bucket,
      key,
      Aws::Http::HttpMethod::HTTP_GET,
      600000
    ).c_str();
  }
}
