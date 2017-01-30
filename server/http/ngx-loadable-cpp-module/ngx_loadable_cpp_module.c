#include <dlfcn.h>

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

/* define shared object interface */
void *ngx_loadable_cpp_shared_object;
typedef ngx_int_t (*ngx_loadable_cpp_shared_object_on_http_data_t)(ngx_http_request_t *r);
ngx_loadable_cpp_shared_object_on_http_data_t ngx_loadable_cpp_on_http_request;

static char *ngx_loadable_cpp(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_loadable_cpp_handler(ngx_http_request_t *r);

/**
 * This module provided directive: load_cpp_shared_object.
 */
static ngx_command_t ngx_loadable_cpp_commands[] = {
  {
    ngx_string("load_cpp_shared_object"), /* directive */
    NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS, /* location context and takes 1 argument*/
    ngx_loadable_cpp, /* configuration setup function */
    0, /* No offset. Only one context is supported. */
    0, /* No offset when storing the module configuration on struct. */
    NULL
  },

  ngx_null_command /* command termination */
};

/* The module context. */
static ngx_http_module_t ngx_loadable_cpp_module_ctx = {
  NULL, /* preconfiguration */
  NULL, /* postconfiguration */

  NULL, /* create main configuration */
  NULL, /* init main configuration */

  NULL, /* create server configuration */
  NULL, /* merge server configuration */

  NULL, /* create location configuration */
  NULL /* merge location configuration */
};

/* Module definition. */
ngx_module_t ngx_loadable_cpp_module = {
  NGX_MODULE_V1,
  &ngx_loadable_cpp_module_ctx, /* module context */
  ngx_loadable_cpp_commands, /* module directives */
  NGX_HTTP_MODULE, /* module type */
  NULL, /* init master */
  NULL, /* init module */
  NULL, /* init process */
  NULL, /* init thread */
  NULL, /* exit thread */
  NULL, /* exit process */
  NULL, /* exit master */
  NGX_MODULE_V1_PADDING
};

/**
 * Content handler.
 */
static ngx_int_t ngx_loadable_cpp_handler(ngx_http_request_t *r) {
  return ngx_loadable_cpp_on_http_request(r);
} /* ngx_loadable_cpp_handler */

/**
 * config setup
 */
static char *ngx_loadable_cpp(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
  ngx_loadable_cpp_shared_object = dlopen("/root/out/debug/main.so", RTLD_LAZY);

  if (!ngx_loadable_cpp_shared_object) {
    ngx_conf_log_error(NGX_LOG_ERR, cf, 0, "/root/out/debug/main.so shared library failed to load.");
    return NGX_CONF_ERROR;
  }

  // load the symbol
  dlerror();

  ngx_loadable_cpp_on_http_request = (
    ngx_loadable_cpp_shared_object_on_http_data_t
  ) dlsym(
    ngx_loadable_cpp_shared_object,
    "on_http_request"
  );

  char *dlsym_error = dlerror();

  if (dlsym_error) {
    ngx_conf_log_error(NGX_LOG_ERR, cf, 0, "/root/out/debug/main.so shared library failed to load.");
    dlclose(ngx_loadable_cpp_shared_object);
    return NGX_CONF_ERROR;
  }

  ngx_http_core_loc_conf_t *clcf;
  clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
  clcf->handler = ngx_loadable_cpp_handler;

  return NGX_CONF_OK;
} // ngx_loadable_cpp
