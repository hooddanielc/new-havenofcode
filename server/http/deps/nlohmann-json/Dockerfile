FROM dhoodlum/base-environment

RUN sudo mkdir /opt/json && sudo chown developer /opt/json
RUN git clone --branch v2.1.1 https://github.com/nlohmann/json.git /opt/json
RUN mkdir /opt/json/build && cd /opt/json/build && cmake ..
WORKDIR /opt/json/build
RUN make -j$(grep -c ^processor /proc/cpuinfo)
RUN sudo make DESTDIR=/dhoodlum/nlohmann-json install
VOLUME /dhoodlum/nlohmann-json
