DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

docker run -ti \
  --entrypoint /bin/zsh \
  -v $DIR/src:/root/src \
  -v $DIR/out:/root/out \
  -v $DIR/config:/root/config \
  -v $DIR/logs:/root/logs \
  -v $DIR/../website:/root/website \
  -p 1337:1337 \
  dhoodlum/havenofcode-http
