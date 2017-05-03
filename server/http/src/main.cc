#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <cstdio>
#include <fstream>

#include <hoc/main.h>
#include <hoc/env.h>
#include <hoc-mail/mail.h>

extern "C" {
  #include <ngx_config.h>
  #include <ngx_core.h>
  #include <ngx_http.h>
}

using namespace std;
using namespace hoc;

ngx_module_t current_module;
std::mutex nginx_mutex;

static void ngx_http_sample_put_handler(ngx_http_request_t *r) {
  std::thread t([r]() {
    req_t *request_wrapper = reinterpret_cast<req_t*>(ngx_http_get_module_ctx(r, current_module));
    app_t::get().emit_request(*request_wrapper);

    if (r->request_body->temp_file == NULL) {
      // request body is less than client_body_buffer_size
      // defined in config
      ngx_chain_t *cl = r->request_body->bufs;

      while (cl != NULL) {
        ngx_buf_t *buf = cl->buf;
        vector<uint8_t> data(buf->start, buf->last);
        request_wrapper->emit_data(data);
        cl = cl->next;
      }
    } else {
      size_t ret;
      size_t offset = 0;
      size_t size = env_t::get().upload_buffer_size;
      unsigned char buff[size];

      while((ret = ngx_read_file(&r->request_body->temp_file->file, buff, size, offset)) > 0) {
        std::vector<uint8_t> data(buff, buff + ret);
        request_wrapper->emit_data(data);
        offset += ret;
      }
    }

    // we are done reading request
    request_wrapper->emit_end();
    lock_guard<mutex> guard(nginx_mutex);
    ngx_http_send_header(r);
    ngx_http_finalize_request(r, ngx_http_output_filter(r, request_wrapper->out));
    delete request_wrapper;
  });
  t.detach();
}

static ngx_int_t ngx_hoc_interface_on_http_request(ngx_http_request_t *r) {
  req_t::header_list_t request_headers;
  ngx_list_part_t *part = &r->headers_in.headers.part;

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

      if (request_headers.count(header_key)) {
        request_headers[header_key].push_back(header_value);
      } else {
        request_headers[header_key] = vector<string>({ header_value  });
      }
    }

    // there are no more parts
    if (part->next == NULL) {
      break;
    } else {
      part = part->next;
    }
  }

  auto request_wrapper = new req_t(request_headers, r);
  ngx_http_set_ctx(r, reinterpret_cast<void*>(request_wrapper), current_module);

  app_t::get().log("path requested: " + string{
    reinterpret_cast<char*>(r->uri.data),
    static_cast<long unsigned int>(r->uri.len)
  });

  // read request body
  ngx_int_t rc;
  rc = ngx_http_read_client_request_body(r, ngx_http_sample_put_handler);

  if (rc >= NGX_HTTP_SPECIAL_RESPONSE) {
    return rc;
  }

  return NGX_DONE;
}

extern "C"
char *ngx_hoc_interface_init(ngx_conf_t *cf, ngx_command_t *, void *, ngx_module_t module) {
  current_module = module;
  env_t::get();
  app_t::get().main();
  ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0, "ngx_hoc_interface_init called.");
  ngx_http_core_loc_conf_t *clcf;
  clcf = (ngx_http_core_loc_conf_t*) ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
  clcf->handler = ngx_hoc_interface_on_http_request;
  app_t::get().emit_start();
  return NGX_CONF_OK;
}
