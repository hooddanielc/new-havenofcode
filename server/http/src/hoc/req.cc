#include <hoc/req.h>

using namespace std;

namespace hoc {

void req_t::on_data(const req_t::cb_data_t &fn) {
  data_events.push_back(fn);
}

void req_t::on_end(const req_t::cb_void_t &fn) {
  end_events.push_back(fn);
}

void req_t::emit_data(const vector<uint8_t> &data) {
  for (auto it = data_events.begin(); it != data_events.end(); ++it) {
    (*it)(data);
  }
}

void req_t::emit_end() {
  for (auto it = end_events.begin(); it != end_events.end(); ++it) {
    (*it)();
  }
}

void req_t::send_body(const string &body) {
  const std::string::size_type size = body.size();
  u_char *data = new u_char[size];
  memcpy(data, body.c_str(), size);

  ngx_buf_t *b;
  b = (ngx_buf_t*) ngx_pcalloc(request_context->pool, sizeof(ngx_buf_t));
  out->buf = b;
  out->next = NULL;
  b->pos = data;
  b->last = data + body.size();
  b->memory = 1;
  b->last_buf = 1;
}

void req_t::send_header(const string &key, const string &val) {
  if (key == "Content-Type") {
    // the content type is straight forward
    const std::string::size_type size = val.size();
    u_char *buf = new u_char[size];
    memcpy(buf, val.c_str(), size);

    request_context->headers_out.content_type.len = val.size();
    request_context->headers_out.content_type.data = buf;
  } else {
    // custom header must be inserted in headers_out

    const std::string::size_type key_size = key.size();
    u_char *buf_key = new u_char[key_size];
    memcpy(buf_key, key.c_str(), key_size);

    const std::string::size_type val_size = val.size();
    u_char *buf_val = new u_char[val_size];
    memcpy(buf_val, val.c_str(), val_size);

    ngx_str_t *header_key;
    ngx_str_t *header_val;
    header_key = (ngx_str_t*) ngx_pcalloc(request_context->pool, sizeof(ngx_str_t));
    header_key->data = buf_key;
    header_key->len = key_size;

    header_val = (ngx_str_t*) ngx_pcalloc(request_context->pool, sizeof(ngx_str_t));
    header_val->data = buf_val;
    header_val->len = val_size;


    ngx_table_elt_t *h;
    h = (ngx_table_elt_t *) ngx_list_push(&request_context->headers_out.headers);
    h->key = *header_key;
    h->value = *header_val;
    h->hash = 1;
  }
}

void req_t::set_status(int status) {
  request_context->headers_out.status = status;
}

void req_t::set_content_length(int len) {
  request_context->headers_out.content_length_n = len;
}

string req_t::uri() {
  return string{
    reinterpret_cast<char*>(request_context->uri.data),
    static_cast<long unsigned int>(request_context->uri.len)
  };
}

string req_t::args() {
  return string{
    reinterpret_cast<char*>(request_context->args.data),
    static_cast<long unsigned int>(request_context->args.len)
  };
}

string req_t::request_line() {
  return string{
    reinterpret_cast<char*>(request_context->request_line.data),
    static_cast<long unsigned int>(request_context->request_line.len)
  };
}

string req_t::exten() {
  return string{
    reinterpret_cast<char*>(request_context->exten.data),
    static_cast<long unsigned int>(request_context->exten.len)
  };
}

string req_t::unparsed_uri() {
  return string{
    reinterpret_cast<char*>(request_context->unparsed_uri.data),
    static_cast<long unsigned int>(request_context->unparsed_uri.len)
  };
}

string req_t::method() {
  return string{
    reinterpret_cast<char*>(request_context->method_name.data),
    static_cast<long unsigned int>(request_context->method_name.len)
  };
}

map<string, string> req_t::query() {
  std::map<string, string> result;
  vector<string> pairs;
  split(args(), '&', pairs);

  for (auto const &pair : pairs) {
    vector<string> parts;
    split(pair, '=', parts);

    if (parts.size() == 2) {
      result[url_decode(parts[0])] = url_decode(parts[1]);
    } else if (parts.size() == 1) {
      string empty_string;
      result[url_decode(parts[0])] = empty_string;
    }
  }

  return result;
}

std::string req_t::user_agent() {
  if (request_headers.count("User-Agent")) {
    return request_headers["User-Agent"][0];
  } else {
    return "none";
  }
}

std::string req_t::ip() {
  struct sockaddr_storage addr;
  socklen_t len = sizeof(addr);
  if (getpeername(request_context->connection->fd, (struct sockaddr*) &addr, &len) < 0) {
    throw runtime_error("get peername failed");
  }
  char host[NI_MAXHOST];
  int status = getnameinfo(
    (struct sockaddr*) &addr, len,
    host,
    sizeof(host),
    nullptr,
    0,
    NI_NUMERICHOST
  );
  if (status) {
    throw runtime_error(gai_strerror(status));
  }

  return string(host);
}

map<string, vector<string>> req_t::cookies() {
  map<string, vector<string>> result;

  if (!request_headers.count("Cookie")) {
    return result;
  }

  for (auto &cookie : request_headers["Cookie"]) {
    std::vector<std::string> pairs;
    split(cookie, ';', pairs);

    for (auto &pair : pairs) {
      std::vector<std::string> parts;
      split(pair, '=', parts);

      if (parts.size() == 2) {
        result[parts[0]].push_back(parts[1]);
      } else if (parts.size() == 1) {
        result[parts[0]].push_back("");
      }
    }
  }

  return result;
}

} // hoc