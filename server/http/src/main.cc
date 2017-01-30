#include <iostream>

extern "C" {
  #include <ngx_config.h>
  #include <ngx_core.h>
  #include <ngx_http.h>

  /* The hello world string. */
  static u_char test_string[] = "hello world";

  ngx_int_t on_http_request(ngx_http_request_t *r) {
    ngx_buf_t *b;
    ngx_chain_t out;

    r->headers_out.content_type.len = sizeof("text/plain") - 1;
    r->headers_out.content_type.data = (u_char *) "text/plain";

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

    // send the body and return the status code of the output filter chain
    return ngx_http_output_filter(r, &out);
  }
}
