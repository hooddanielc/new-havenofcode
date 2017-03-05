FROM dhoodlum/base-environment

# install ib
RUN git clone https://github.com/JasonL9000/ib.git ~/ib
RUN apt install -y python

#install postgres
RUN echo "deb http://apt.postgresql.org/pub/repos/apt/ jessie-pgdg main" >> /etc/apt/sources.list
RUN wget --quiet -O - https://www.postgresql.org/media/keys/ACCC4CF8.asc | \
  apt-key add -
RUN apt update
RUN apt install -y postgresql-9.6
RUN apt install -y libpq-dev

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

# install gmp for cryptography
RUN apt install -y libgmp-dev

# install vmime for email
RUN cd /opt && git clone https://github.com/kisli/vmime.git
RUN apt install -y cmake
RUN apt install -y libicu-dev
RUN apt install -y gnutls-dev
RUN apt install -y libgsasl7-dev
RUN apt install -y libboost-all-dev
RUN apt install -y libgtk-3-dev
RUN cd /opt/vmime && mkdir build && cd build && cmake ..
RUN cd /opt/vmime/build && make && make install

# setup
RUN echo 'cd $HOME' >> $HOME/.zshrc

# set path
ENV PATH $PATH:/usr/local/nginx/sbin:/root/ib:/root/scripts
ENV HOC_DB_NAME hoc_dev
ENV HOC_DB_USER admin_dev
ENV HOC_DB_PASSWORD 123123
ENV HOC_DB_HOST hoc-db

# add nginx config
ADD ./config /root/config

# add dev scripts
ADD ./scripts /root/scripts

# build nginx module
ADD ./src /root/src
RUN cd $HOME/src && ib main.so

# create place for nginx logs to live
RUN mkdir $HOME/logs

# expose ports
EXPOSE 1337

# run nginx
CMD nginx -g "daemon off;" -c $HOME/config/nginx.conf
