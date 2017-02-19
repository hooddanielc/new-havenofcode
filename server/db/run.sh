DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

docker run -ti dhoodlum/havenofcode-db postgres -D /etc/postgresql/9.4/main
