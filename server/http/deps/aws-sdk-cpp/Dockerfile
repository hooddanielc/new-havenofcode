FROM dhoodlum/base-environment
RUN sudo mkdir /opt/aws-sdk-cpp && sudo chown developer /opt/aws-sdk-cpp
RUN git clone https://github.com/aws/aws-sdk-cpp.git /opt/aws-sdk-cpp
RUN mkdir /opt/aws-sdk-cpp/build && cd /opt/aws-sdk-cpp/build && cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DCPP_STANDARD=17 \
  -DBUILD_ONLY="s3" ..
RUN cd /opt/aws-sdk-cpp/build && make -j$(grep -c ^processor /proc/cpuinfo)
RUN sudo mkdir -p /dhoodlum/aws-sdk-cpp && sudo chown developer /dhoodlum/aws-sdk-cpp
RUN cd /opt/aws-sdk-cpp/build && sudo make DESTDIR=/dhoodlum/aws-sdk-cpp install

VOLUME /dhoodlum/aws-sdk-cpp

CMD bash
