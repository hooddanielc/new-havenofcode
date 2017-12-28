set -e
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEPS=(
  aws-sdk-cpp
  libvmime
  nlohmann-json
  nginx-loadable-cpp-module
  libpqxx
)

echo $DIR

for dep in ${DEPS[@]}; do
  mkdir -p $DIR/$dep/pkg
  docker build -t dhoodlum/${dep} -f $DIR/${dep}/Dockerfile $DIR/$dep
  docker run dhoodlum/${dep}
  docker cp $(docker ps -q -l):/dhoodlum/$dep $DIR/$dep/pkg
  docker rm $(docker ps -q -l)
done
