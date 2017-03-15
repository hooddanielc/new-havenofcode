#include <dlfcn.h>

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef char *(*ngx_hoc_interface_init_t)(ngx_conf_t *cf, ngx_command_t *cmd, void *conf, ngx_module_t module);
ngx_module_t ngx_loadable_cpp_module;

static char *ngx_hoc_init(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
  // Load the dynamic library
  void *ngx_hoc_interface_handle;
  ngx_hoc_interface_handle = dlopen("/root/out/debug/main.so", RTLD_NOW);

  if (!ngx_hoc_interface_handle) {
    ngx_conf_log_error(NGX_LOG_ERR, cf, 0, "/root/out/debug/main.so shared library failed to load.");
    return NGX_CONF_ERROR;
  }

  // Load the init method
  dlerror();
  ngx_hoc_interface_init_t ngx_hoc_interface_init;
  ngx_hoc_interface_init = (ngx_hoc_interface_init_t) dlsym(ngx_hoc_interface_handle, "ngx_hoc_interface_init");

  char *dlsym_error = dlerror();

  if (dlsym_error) {
    ngx_conf_log_error(
      NGX_LOG_ERR,
      cf,
      0,
      "/root/out/debug/main.so ngx_hoc_interface_init failed to link. %s", dlsym_error
    );

    dlclose(ngx_hoc_interface_handle);
    return NGX_CONF_ERROR;
  }

  return ngx_hoc_interface_init(cf, cmd, conf, ngx_loadable_cpp_module);
}

/* The module context. */
static ngx_http_module_t ngx_hoc_ctx = {
  NULL, /* preconfiguration */
  NULL, /* postconfiguration */

  NULL, /* create main configuration */
  NULL, /* init main configuration */

  NULL, /* create server configuration */
  NULL, /* merge server configuration */

  NULL, /* create location configuration */
  NULL  /* merge location configuration */
};

static ngx_command_t ngx_hoc_commands[] = {
  {
    ngx_string("load_cpp_shared_object"), /* directive */
    NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS, /* location context and takes 1 argument*/
    ngx_hoc_init, /* configuration setup function */
    0, /* No offset. Only one context is supported. */
    0, /* No offset when storing the module configuration on struct. */
    NULL
  },

  ngx_null_command /* command termination */
};

ngx_module_t ngx_loadable_cpp_module = {
  NGX_MODULE_V1,
  &ngx_hoc_ctx, /* module context */
  ngx_hoc_commands, /* module directives */
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


