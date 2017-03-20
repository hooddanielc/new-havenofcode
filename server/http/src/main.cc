#include <iostream>
#include <string>
#include <cstdio>

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

static void ngx_http_sample_put_handler(ngx_http_request_t *r) {
  req_t *request_wrapper = reinterpret_cast<req_t*>(ngx_http_get_module_ctx(r, current_module));

  if (r->request_body->temp_file == NULL) {
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
    ngx_chain_t *cl = r->request_body->bufs;

    while (cl != NULL) {
      ngx_buf_t *buf = cl->buf;

      std::string data{
        reinterpret_cast<char*>(buf->pos),
        static_cast<long unsigned int>(buf->last - buf->pos)
      };

      request_wrapper->emit_data(data);
      cl = cl->next;
    }
  } else {
    // The entire request body is available in the temporary file.
    size_t ret;
    size_t offset = 0;
    unsigned char data[4096];

    while ((ret = ngx_read_file(&r->request_body->temp_file->file, data, 4096, offset)) > 0) {
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
  request_wrapper->emit_end();
  ngx_http_send_header(r);

  // send the body and return the status code of the output filter chain
  ngx_http_finalize_request(r, ngx_http_output_filter(r, request_wrapper->out));
  delete request_wrapper;
  ngx_pfree(r->pool, request_wrapper->out);
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

  app_t::get().emit_request(*request_wrapper);

  // read request body
  ngx_int_t rc;
  rc = ngx_http_read_client_request_body(r, ngx_http_sample_put_handler);

  if (rc >= NGX_HTTP_SPECIAL_RESPONSE) {
    cout << "uhm special" << endl;
    return rc;
  } else if (rc == NGX_AGAIN) {
    return NGX_DONE;
  }

  return NGX_OK;
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
