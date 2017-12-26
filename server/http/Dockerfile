FROM dhoodlum/base-environment

# install deps
RUN sudo pacman --noconfirm -S postgresql libpqxx
RUN sudo pacman --noconfirm -S curl
RUN sudo pacman --noconfirm -S zlib
RUN sudo pacman --noconfirm -S libutil-linux
RUN sudo pacman --noconfirm -S gsasl
RUN sudo pacman --noconfirm -S gnutls
RUN sudo pacman --noconfirm -S gtk3

ADD deps/aws-sdk-cpp/pkg/aws-sdk-cpp /
ADD deps/libvmime/pkg/libvmime /
ADD deps/nginx-loadable-cpp-module/pkg/nginx-loadable-cpp-module /
ADD deps/nlohmann-json/pkg/nlohmann-json /

# setup
RUN echo 'cd $HOME' >> $HOME/.zshrc

# set path
ENV PATH $PATH:/usr/local/nginx/sbin:/opt/ib:/home/developer/scripts

# add dev scripts
ADD ./scripts /home/developer/scripts

# build nginx module before setting environment flags
ADD ./src /home/developer/src

# build nginx module
RUN chmod +x /opt/ib/ib
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
