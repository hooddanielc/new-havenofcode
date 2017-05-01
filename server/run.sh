DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# keep the data volume running for persistence
DB_CONTAINER_NAME="hoc-db"
CID=$(docker ps -q -f status=running -f name=$DB_CONTAINER_NAME)
if [ ! "${CID}" ]; then
  docker run -tid --rm -p 5432:5432 --name hoc-db dhoodlum/havenofcode-db
fi
unset CID

docker run -ti \
  --link hoc-db \
  -v $DIR/http/src:/root/src \
  -v $DIR/http/out:/root/out \
  -v $DIR/http/config:/root/config \
  -v $DIR/http/logs:/root/logs \
  -v $DIR/http/scripts:/root/scripts \
  -v $DIR/website:/root/website \
  -v $DIR/http/tmp:/root/tmp \
  -p 80:80 \
  dhoodlum/havenofcode-http \
  zsh
