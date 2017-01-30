dhoodlum/havenofcode-http
=========================

An http server running nginx with C++ http module.

```
# builds nginx module
# builds ib application

./build.sh
```

```
# launches docker dev environment
./run.sh

# starts the http server
start-http

# rebuilds and restarts nginx
cd ~/src
ib main.so
nginx -s reload
```

Please note that changing ngx_loadable_cpp_module.c will require a rebuild using `./build.sh`