#include <iostream>
#include <string>
#include <cstdio>

#include <hoc/main.h>
#include <hoc-mail/mail.h>

extern "C" {
  #include <ngx_config.h>
  #include <ngx_core.h>
  #include <ngx_http.h>
}

using namespace std;
using namespace hoc;

req_t *current_req = NULL;
ngx_http_request_t *current_request;
ngx_chain_t *out;

static void ngx_http_sample_put_handler(ngx_http_request_t *r) {
  if (current_request->request_body->temp_file == NULL) {
    /*
     * The entire request body is available in the list
     * of buffers pointed by r->request_body->bufs.
     *
     * The list can have a maixmum of two buffers. One
     * buffer contains the request body that was pre-read
     * along with the request headers. The other buffer contains
     * the rest of the request body. The maximum size of the
     * buffer is controlled by 'client_body_buffer_size' directive.
     * If the request body cannot be contained within these two
     * buffers, the entire body  is writtin to the temp file and
     * the buffers are cleared.
     */
    ngx_chain_t *cl = current_request->request_body->bufs;

    while (cl != NULL) {
      ngx_buf_t *buf = cl->buf;

      std::string data{
        reinterpret_cast<char*>(buf->pos),
        static_cast<long unsigned int>(buf->last - buf->pos)
      };

      current_req->emit_data(data);
      cl = cl->next;
    }
  } else {
    // The entire request body is available in the temporary file.
    size_t ret;
    size_t offset = 0;
    unsigned char data[4096];

    while ((ret = ngx_read_file(&current_request->request_body->temp_file->file, data, 4096, offset)) > 0) {
      cout << "Got a file" << endl;
      // if (write(fd, data, ret) < 0) {
      //   ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "Failed to allocate response buffer.");
      //   ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
      //   close(fd);
      //   return;
      // }

      offset = offset + ret;
    }
  }

  // we are done reading request
  current_req->emit_end();
  ngx_http_send_header(current_request);

  // send the body and return the status code of the output filter chain
  ngx_http_finalize_request(r, ngx_http_output_filter(r, out));
  delete current_req;
  ngx_pfree(r->pool, out);
}

void
set_custom_header_in_headers_out(ngx_http_request_t *, ngx_str_t *key, ngx_str_t *value) {
  ngx_table_elt_t *h;
  // allocate the header
  h = (ngx_table_elt_t *) ngx_list_push(&current_request->headers_out.headers);
  // setup the header key
  h->key = *key;
  // set header value
  h->value = *value;
  // mark header as not deleted
  h->hash = 1;
}

void req_t::send_body(const string &body) const {
  const std::string::size_type size = body.size();
  u_char *data = new u_char[size];
  memcpy(data, body.c_str(), size);

  ngx_buf_t *b;
  b = (ngx_buf_t*) ngx_pcalloc(current_request->pool, sizeof(ngx_buf_t));
  out->buf = b;
  out->next = NULL;
  b->pos = data;
  b->last = data + body.size();
  b->memory = 1;
  b->last_buf = 1;
}

void req_t::send_header(const string &key, const string &val) const {
  if (key == "Content-Type") {
    // the content type is straight forward
    const std::string::size_type size = val.size();
    u_char *buf = new u_char[size];
    memcpy(buf, val.c_str(), size);

    current_request->headers_out.content_type.len = val.size();
    current_request->headers_out.content_type.data = buf;
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
    header_key = (ngx_str_t*) ngx_pcalloc(current_request->pool, sizeof(ngx_str_t));
    header_key->data = buf_key;
    header_key->len = key_size;

    header_val = (ngx_str_t*) ngx_pcalloc(current_request->pool, sizeof(ngx_str_t));
    header_val->data = buf_val;
    header_val->len = val_size;

    set_custom_header_in_headers_out(current_request, header_key, header_val);
  }
}

void req_t::set_status(int status) const {
  current_request->headers_out.status = status;
}

void req_t::set_content_length(int len) const {
  current_request->headers_out.content_length_n = len;
}

string req_t::uri() const {
  return string{
    reinterpret_cast<char*>(current_request->uri.data),
    static_cast<long unsigned int>(current_request->uri.len)
  };
}

string req_t::args() const {
  return string{
    reinterpret_cast<char*>(current_request->args.data),
    static_cast<long unsigned int>(current_request->args.len)
  };
}

string req_t::request_line() const {
  return string{
    reinterpret_cast<char*>(current_request->request_line.data),
    static_cast<long unsigned int>(current_request->request_line.len)
  };
}

string req_t::exten() const {
  return string{
    reinterpret_cast<char*>(current_request->exten.data),
    static_cast<long unsigned int>(current_request->exten.len)
  };
}

string req_t::unparsed_uri() const {
  return string{
    reinterpret_cast<char*>(current_request->unparsed_uri.data),
    static_cast<long unsigned int>(current_request->unparsed_uri.len)
  };
}

string req_t::method() const {
  return string{
    reinterpret_cast<char*>(current_request->method_name.data),
    static_cast<long unsigned int>(current_request->method_name.len)
  };
}

static ngx_int_t ngx_hoc_interface_on_http_request(ngx_http_request_t *r) {
  current_request = r;
  out = (ngx_chain_t*) ngx_pcalloc(current_request->pool, sizeof(ngx_chain_t));

  req_t::header_list_t request_headers;
  ngx_list_part_t *part = &current_request->headers_in.headers.part;

  for (;;) {
    ngx_table_elt_t *h = static_cast<ngx_table_elt_t*>(part->elts);

    for (ngx_uint_t i = 0; i < part->nelts; ++i) {
      string header_key{
        reinterpret_cast<char*>(h[i].key.data),
        static_cast<long unsigned int>(h[i].key.len)
      };

      string header_value{
        reinterpret_cast<char*>(h[i].value.data),
        static_cast<long unsigned int>(h[i].value.len)
      };

      request_headers[header_key] = header_value;
    }

    // there are no more parts
    if (part->next == NULL) {
      break;
    } else {
      part = part->next;
    }
  }

  current_req = new req_t(request_headers);

  app_t::get().log("path requested: " + string{
    reinterpret_cast<char*>(current_request->uri.data),
    static_cast<long unsigned int>(current_request->uri.len)
  });


  app_t::get().emit_request(*current_req);

  // read request body
  ngx_int_t rc;
  rc = ngx_http_read_client_request_body(current_request, ngx_http_sample_put_handler);

  if (rc >= NGX_HTTP_SPECIAL_RESPONSE) {
    cout << "uhm special" << endl;
    return rc;
  } else if (rc == NGX_AGAIN) {
    return NGX_DONE;
  }

  return NGX_OK;
}

extern "C"
char *ngx_hoc_interface_init(ngx_conf_t *cf, ngx_command_t *, void *) {
  app_t::get().main();
  ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0, "ngx_hoc_interface_init called.");
  ngx_http_core_loc_conf_t *clcf;
  clcf = (ngx_http_core_loc_conf_t*) ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
  clcf->handler = ngx_hoc_interface_on_http_request;
  app_t::get().emit_start();
  return NGX_CONF_OK;
}
