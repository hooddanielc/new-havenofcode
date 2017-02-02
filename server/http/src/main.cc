#include <iostream>
#include <string>
#include <sstream>

extern "C" {
  #include <ngx_config.h>
  #include <ngx_core.h>
  #include <ngx_http.h>

  /* The hello world string. */
  static u_char test_string[] = "hello world";

  static void ngx_http_sample_put_handler(ngx_http_request_t *r) {
    ngx_log_error(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "put handler called :)");

    if (r->request_body->temp_file == NULL) {
      /*
       * The entire request body is available in the list of buffers pointed by r->request_body->bufs.
       *
       * The list can have a maixmum of two buffers. One buffer contains the request body that was pre-read along with the request headers.
       * The other buffer contains the rest of the request body. The maximum size of the buffer is controlled by 'client_body_buffer_size' directive.
       * If the request body cannot be contained within these two buffers, the entire body  is writtin to the temp file and the buffers are cleared.
       */
      ngx_buf_t    *buf;
      ngx_chain_t  *cl;


      cl = r->request_body->bufs;

      std::ostringstream oss;

      for (; cl != NULL; cl = cl->next) {
        ngx_log_error(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "Writing data from buffers.");
        buf = cl->buf;
        oss << cl->buf->pos;
      }
    } else {
      /**
       * The entire request body is available in the temporary file.
       *
       */
      size_t ret;
      size_t offset = 0;
      unsigned char data[4096];

      ngx_log_error(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "Writing data from temp file.");

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
    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, "it called");
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

    if (rc >= NGX_HTTP_SPECIAL_RESPONSE){
      return rc;
    }
    // ###### read request body ######
    // ###### read request body ######
    // ###### read request body ######

    // send the body and return the status code of the output filter chain
    return ngx_http_output_filter(r, &out);
  }

  char *ngx_hoc_interface_init(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0, "ngx_hoc_interface_init called.");
    ngx_http_core_loc_conf_t *clcf;
    clcf = (ngx_http_core_loc_conf_t*) ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_hoc_interface_on_http_request;
    return NGX_CONF_OK;
  }
}
