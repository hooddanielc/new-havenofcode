DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
docker build -t dhoodlum/havenofcode-http $DIR

# build the nginx C++ module
docker run -t \
  -v $DIR/src:/root/src \
  -v $DIR/out:/root/out \
  -v $DIR/config:/root/config \
  -v $DIR/logs:/root/logs \
  dhoodlum/havenofcode-http \
  /bin/sh -c "cd /root/src && /root/ib/ib main.so"