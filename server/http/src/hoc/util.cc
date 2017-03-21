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
}
