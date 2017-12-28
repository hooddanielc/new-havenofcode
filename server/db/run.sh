DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

docker run --rm -ti \
  -p 5432:5432 \
  -e POSTGRES_PASSWORD=123123 \
  -v $DIR/log:/home/developer/data/log
  dhoodlum/havenofcode-db
