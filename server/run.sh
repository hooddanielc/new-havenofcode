DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# persist db and create volume labeled dbstore
docker create -v /home/developer/data --name dbstore dhoodlum/havenofcode-db /bin/true
docker run -tid --rm --volumes-from dbstore -p 5432:5432 --name hoc-db dhoodlum/havenofcode-db

touch $DIR/.zsh_history
docker run -ti \
  --link hoc-db \
  -v $DIR/.zsh_history:/home/developer/.zsh_history \
  -v $DIR/http/src:/home/developer/src \
  -v $DIR/http/out:/home/developer/out \
  -v $DIR/http/config:/home/developer/config \
  -v $DIR/http/logs:/home/developer/logs \
  -v $DIR/http/scripts:/home/developer/scripts \
  -v $DIR/http/website/dist:/home/developer/website/dist \
  -v $DIR/tmp:/home/developer/tmp \
  -p 80:80 \
  dhoodlum/havenofcode-http \
  zsh
