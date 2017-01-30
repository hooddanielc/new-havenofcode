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

# setup
RUN echo 'cd $HOME' >> $HOME/.zshrc

# alias
RUN echo 'start-http() { nginx -c $HOME/config/nginx.conf }' >> $HOME/.zshrc
RUN echo 'stop-http() { nginx -s stop }' >> $HOME/.zshrc
RUN echo 'export PATH=$PATH:/usr/local/nginx/sbin:$HOME/ib' >> $HOME/.zshrc
