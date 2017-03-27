DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
docker run -tid --rm -p 5432:5432 --name hoc-db dhoodlum/havenofcode-db

docker run -ti \
  --link hoc-db \
  -v $DIR/http/src:/root/src \
  -v $DIR/http/out:/root/out \
  -v $DIR/http/config:/root/config \
  -v $DIR/http/logs:/root/logs \
  -v $DIR/http/scripts:/root/scripts \
  -v $DIR/website:/root/website \
  -p 80:80 \
  dhoodlum/havenofcode-http \
  zsh

echo 'stopping hoc-db'
docker stop hoc-db
