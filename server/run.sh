DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# persist db and create volume labeled dbstore
docker create -v /root/data --name dbstore dhoodlum/havenofcode-db /bin/true
docker run -tid --rm --volumes-from dbstore -p 5432:5432 --name hoc-db dhoodlum/havenofcode-db

docker run -ti \
  --link hoc-db \
  -v $DIR/http/src:/root/src \
  -v $DIR/http/out:/root/out \
  -v $DIR/http/config:/root/config \
  -v $DIR/http/logs:/root/logs \
  -v $DIR/http/scripts:/root/scripts \
  -v $DIR/http/website/dist:/root/website/dist \
  -v $DIR/tmp:/root/tmp \
  -p 80:80 \
  dhoodlum/havenofcode-http \
  zsh
