FROM dhoodlum/base-environment
RUN sudo mkdir -p /opt/libvmime && sudo chown -R developer /opt/libvmime
RUN sudo mkdir -p /dhoodlum/libvmime && sudo chown -R developer /dhoodlum/libvmime
RUN git clone --branch v0.9.2 https://github.com/kisli/vmime.git /opt/libvmime

RUN sudo pacman -S gsasl --noconfirm
RUN sudo pacman -S gnutls --noconfirm
RUN sudo pacman -S exim --noconfirm
RUN sudo pacman -S gtk3 --noconfirm
RUN mkdir /opt/libvmime/build

WORKDIR /opt/libvmime/build

RUN cmake .. \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DLIB_INSTALL_DIR=/usr/lib/ \
    -DCMAKE_BUILD_TYPE=Release

RUN make -j$(grep -c ^processor /proc/cpuinfo)
RUN sudo make DESTDIR=/dhoodlum/libvmime install

VOLUME /dhoodlum/libvmime

CMD bash
