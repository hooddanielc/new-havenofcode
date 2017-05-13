FROM dhoodlum/base-environment

# install deps
RUN sudo pacman --noconfirm -S postgresql libpqxx
RUN sudo pacman --noconfirm -S curl
RUN sudo pacman --noconfirm -S zlib
RUN sudo pacman --noconfirm -S libutil-linux
RUN sudo pacman --noconfirm -S gsasl
RUN sudo pacman --noconfirm -S gnutls
RUN sudo pacman --noconfirm -S gtk3

# download nginx
RUN cd ~ && wget http://nginx.org/download/nginx-1.13.0.tar.gz
RUN cd ~ && tar -xvf nginx-1.13.0.tar.gz

# download libvmime
RUN sudo mkdir /opt/libvmime && sudo chown -R developer /opt/libvmime
RUN git clone https://aur.archlinux.org/libvmime.git /opt/libvmime

# download aws-sdk-cpp
RUN sudo mkdir /opt/aws-sdk-cpp && sudo chown developer /opt/aws-sdk-cpp
RUN git clone --branch 1.0.127 https://github.com/aws/aws-sdk-cpp.git /opt/aws-sdk-cpp

# build vmime
RUN cd /opt/libvmime && makepkg --noconfirm -s
RUN sudo pacman --noconfirm -U /opt/libvmime/*.pkg.tar.xz

# build aws c++ sdk
RUN mkdir /opt/aws-sdk-cpp/build && cd /opt/aws-sdk-cpp/build && cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_ONLY="s3" ..
RUN cd /opt/aws-sdk-cpp/build && make -j$(grep -c ^processor /proc/cpuinfo)
RUN cd /opt/aws-sdk-cpp/build && sudo make install

# build nginx module
ADD ./ngx-loadable-cpp-module /home/developer/ngx-loadable-cpp-module

RUN cd ~/nginx-1.13.0 && ./configure \
  --with-http_ssl_module \
  --with-file-aio \
  --with-threads \
  --add-module=/home/developer/ngx-loadable-cpp-module

RUN cd ~/nginx-1.13.0 && make -j$(grep -c ^processor /proc/cpuinfo) && sudo make install
RUN rm ~/nginx-1.13.0.tar.gz

# setup
RUN echo 'cd $HOME' >> $HOME/.zshrc

# set path
ENV PATH $PATH:/usr/local/nginx/sbin:/opt/ib:/home/developer/scripts

# add dev scripts
ADD ./scripts /home/developer/scripts

# build nginx module before setting environment flags
ADD ./src /home/developer/src

# build nginx module
RUN cd $HOME/src && ib main.so

# environment env
ENV HOC_DOMAIN havenofcode.com
ENV HOC_HTTP_UPLOAD_TEMP_PATH /home/developer/tmp
ENV HOC_HTTP_UPLOAD_BUFFER_SIZE 16000

# add directory for debugging uploads
RUN mkdir /home/developer/tmp

# db env
ENV HOC_DB_NAME hoc_dev
ENV HOC_DB_USER admin_dev
ENV HOC_DB_PASSWORD 123123
ENV HOC_DB_HOST hoc-db

# registration with gmail
ENV HOC_GOOGLE_API_CLIENT_ID XXXXXXXX
ENV HOC_GOOGLE_API_CLIENT_SECRET XXXXXXXX
ENV HOC_NOREPLY_EMAIL noreply@havenofcode.com

# amazon web services access
ENV HOC_AWS_KEY XXXXXXXX
ENV HOC_AWS_SECRET XXXXXXXX

# should uploads be stored locally? 1 = true 0 = false
ENV HOC_MOCK_S3_UPLOADS 1

# create place for nginx logs to live
RUN mkdir $HOME/logs

# add nginx config
ADD ./config /home/developer/config

# add static files built seperately
ADD ./website/dist /home/developer/website/dist

# expose ports
EXPOSE 80

# run nginx
CMD sudo nginx -g "daemon off;" -c $HOME/config/nginx.conf
