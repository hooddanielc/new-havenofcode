#include <hoc/request.h>

using namespace hoc;
using namespace std;

void request_t::init() {
  curl_global_init(CURL_GLOBAL_ALL);
}

void request_t::cleanup() {
  curl_global_cleanup();
}

size_t request_t::write_func(void *ptr, size_t size, size_t nmemb, request_t *cake) {
  for (auto it = cake->data_callbacks.begin(); it != cake->data_callbacks.end(); ++it) {
    (*it)(static_cast<char*>(ptr), size * nmemb);
  }

  return size*nmemb;
}

size_t request_t::string_read_func(void *ptr, size_t, size_t, write_t *cake) {
  if (cake->sizeleft) {
    *(char *) ptr = cake->readptr[0];
    cake->readptr++;
    cake->sizeleft--;
    return 1;
  }

  return 0;
}

request_t::request_t() {
  curl = curl_easy_init();
  slist_headers = nullptr;

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
  } else {
    throw runtime_error("curl_easy_init failed");
  }
}

request_t::~request_t() {
  if (slist_headers != nullptr) {
    curl_slist_free_all(slist_headers);    
  }

  curl_easy_cleanup(curl);
}

void request_t::on_data(const cb_data_t &fn) {
  data_callbacks.push_back(std::move(fn));

  if (data_callbacks.size() == 1) {
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, reinterpret_cast<void*>(this));
  }
}

void request_t::add_header(const char *header) {
  slist_headers = curl_slist_append(slist_headers, header);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist_headers);
}

void request_t::set_url(const char *url) {
  curl_easy_setopt(curl, CURLOPT_URL, url);
}

void request_t::set_method(const char *method) {
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
}

void request_t::send() {
  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    throw runtime_error(curl_easy_strerror(res));
  }
}

void request_t::send(const string &str) {
  struct write_t cake;
  cake.readptr = str.data();
  cake.sizeleft = str.size();
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_READFUNCTION, string_read_func);
  curl_easy_setopt(curl, CURLOPT_READDATA, &cake);
  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    throw runtime_error(curl_easy_strerror(res));
  }
}