DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
docker run --rm -ti -p 1337:1337 dhoodlum/havenofcode-http
