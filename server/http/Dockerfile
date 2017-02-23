FROM dhoodlum/base-environment

# install ib
RUN git clone https://github.com/JasonL9000/ib.git ~/ib
RUN apt install -y python

# install nginx
RUN apt install -y build-essential
RUN apt install -y libpcre3 libpcre3-dev
RUN apt install -y zlib1g zlib1g-dev
RUN apt install -y libssl-dev
RUN cd ~ && wget http://nginx.org/download/nginx-1.10.2.tar.gz
RUN cd ~ && tar -xvf nginx-1.10.2.tar.gz
ADD ./ngx-loadable-cpp-module /root/ngx-loadable-cpp-module
RUN cd ~/nginx-1.10.2 && ./configure --with-http_ssl_module --add-module=/root/ngx-loadable-cpp-module
RUN cd ~/nginx-1.10.2 && make -j$(grep -c ^processor /proc/cpuinfo) && make install
RUN rm ~/nginx-1.10.2.tar.gz

# install libpq++
RUN apt install -y libpq-dev

# setup
RUN echo 'cd $HOME' >> $HOME/.zshrc

# alias
RUN echo 'start-http() { nginx -c $HOME/config/nginx.conf }' >> $HOME/.zshrc
RUN echo 'stop-http() { nginx -s stop }' >> $HOME/.zshrc
RUN echo 'rebuild() { stop-http; cd /root/src; ib main.so; start-http; }' >> $HOME/.zshrc
ENV PATH $PATH:/usr/local/nginx/sbin:/root/ib

# add nginx config
ADD ./config /root/config

# build nginx module
ADD ./src /root/src
RUN cd $HOME/src && ib main.so

# create place for nginx logs to live
RUN mkdir $HOME/logs

# expose ports
EXPOSE 1337

# run nginx
CMD nginx -g "daemon off;" -c $HOME/config/nginx.conf
