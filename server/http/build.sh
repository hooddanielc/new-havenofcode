set -e
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
$DIR/deps/build.sh
docker build -t dhoodlum/havenofcode-http $DIR
