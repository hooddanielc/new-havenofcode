FROM dhoodlum/base-environment

# download nginx
RUN sudo mkdir -p /opt/nginx && sudo chown developer /opt/nginx
RUN cd ~ && wget http://nginx.org/download/nginx-1.13.7.tar.gz
RUN cd ~ && tar -xvf nginx-1.13.7.tar.gz
RUN cp -R ~/nginx-1.13.7/* /opt/nginx

WORKDIR /opt/nginx

RUN sudo mkdir /opt/ngx-loadable-cpp-module && sudo chown developer /opt/ngx-loadable-cpp-module
ADD config /opt/ngx-loadable-cpp-module/config
ADD ngx_loadable_cpp_module.c /opt/ngx-loadable-cpp-module/ngx_loadable_cpp_module.c

RUN ./configure \
  --with-http_ssl_module \
  --with-file-aio \
  --with-threads \
  --add-module=/opt/ngx-loadable-cpp-module

RUN sudo make DESTDIR=/dhoodlum/nginx-loadable-cpp-module install
RUN sudo mkdir -p /dhoodlum/nginx-loadable-cpp-module/usr/local/src
RUN sudo cp -R /opt/nginx /dhoodlum/nginx-loadable-cpp-module/usr/local/src/nginx

VOLUME /dhoodlum/nginx-loadable-cpp-module

CMD bash
