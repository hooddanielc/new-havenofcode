#include <iostream>
#include <string>
#include <cstdio>

extern "C" {
  #include <ngx_config.h>
  #include <ngx_core.h>
  #include <ngx_http.h>
}

#include <hoc/main.h>

using namespace std;
using namespace hoc;

req_t *current_req = NULL;

/* The hello world string. */
static u_char test_string[] = "hello cruelish world";

static void ngx_http_sample_put_handler(ngx_http_request_t *r) {
  app_t::get().log("post handler called");

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
    ngx_buf_t    *buf;
    ngx_chain_t  *cl;

    cl = r->request_body->bufs;

    for (; cl != NULL; cl = cl->next) {
      buf = cl->buf;

      std::string data{
        reinterpret_cast<char*>(buf->pos),
        static_cast<long unsigned int>(buf->last - buf->pos)
      };

      current_req->emit_data(data);
    }
  } else {
    /**
     * The entire request body is available in the temporary file.
     *
     */
    size_t ret;
    size_t offset = 0;
    unsigned char data[4096];

    while ((ret = ngx_read_file(&r->request_body->temp_file->file, data, 4096, offset)) > 0) {
      // if (write(fd, data, ret) < 0) {
      //   ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "Failed to allocate response buffer.");
      //   ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
      //   close(fd);
      //   return;
      // }

      offset = offset + ret;
    }
  }
}

static ngx_int_t ngx_hoc_interface_on_http_request(ngx_http_request_t *r) {
  ngx_list_part_t *part;
  ngx_table_elt_t *h;
  ngx_uint_t i;

  part = &r->headers_in.headers.part;
  h = (ngx_table_elt_t*) part->elts;
  req_t::header_list_t headers;

  // Headers list array may consist of more than one part,
  // so loop through all of it

  for (i = 0; /* void */ ; i++) {
    if (i >= part->nelts) {
      if (part->next == NULL) {
          /* The last part, search is done. */
          break;
      }

      part = part->next;
      h = (ngx_table_elt_t*) part->elts;
      i = 0;
    }

    //app_t::get().log(string("header length") + (int) h[i].key.len);
    string header_key{
      reinterpret_cast<char*>(h[i].key.data),
      static_cast<long unsigned int>(h[i].key.len)
    };

    string header_value{
      reinterpret_cast<char*>(h[i].value.data),
      static_cast<long unsigned int>(h[i].value.len)
    };

    headers[header_key] = header_value;
  }

  current_req = new req_t(headers);
  app_t::get().emit_request(*current_req);


  ngx_buf_t *b;
  ngx_chain_t out;

  r->headers_out.content_type.len = sizeof("text/html") - 1;
  r->headers_out.content_type.data = (u_char *) "text/html";

  b = (ngx_buf_t*) ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
  out.buf = b;
  out.next = NULL;

  b->pos = test_string;
  b->last = test_string + sizeof(test_string);
  b->memory = 1;
  b->last_buf = 1;

  // send headers
  r->headers_out.status = NGX_HTTP_OK; /* 200 status code */
  // get the content length of the body
  r->headers_out.content_length_n = sizeof(test_string);
  ngx_http_send_header(r); /* Send the headers */

  // ###### read request body ######
  // ###### read request body ######
  // ###### read request body ######
  ngx_int_t rc;
  rc = ngx_http_read_client_request_body(r, ngx_http_sample_put_handler);

  app_t::get().log("on request end");

  if (rc >= NGX_HTTP_SPECIAL_RESPONSE){
    return rc;
  }
  // ###### read request body ######
  // ###### read request body ######
  // ###### read request body ######

  // send the body and return the status code of the output filter chain
  return ngx_http_output_filter(r, &out);
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
