set -e
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd $DIR/base && $DIR/base/build.sh
cd $DIR/http && $DIR/http/build.sh
cd $DIR/db && $DIR/db/build.sh
cd $DIR