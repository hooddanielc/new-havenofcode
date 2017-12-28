FROM dhoodlum/base-environment
RUN sudo mkdir /opt/libpqxx && sudo chown developer /opt/libpqxx
RUN git clone --branch 5.1.1 https://github.com/jtv/libpqxx.git /opt/libpqxx
WORKDIR /opt/libpqxx
RUN sudo pacman --noconfirm -S postgresql
RUN ./configure --prefix /usr/local --disable-documentation --enable-shared
RUN make -j$(grep -c ^processor /proc/cpuinfo)
RUN sudo make DESTDIR=/dhoodlum/libpqxx install

VOLUME /dhoodlum/libpqxx

CMD bash
