FROM debian
RUN apt update
RUN apt upgrade -y
RUN apt install -y zsh
RUN apt install -y git-core
RUN apt install -y wget
RUN wget https://github.com/robbyrussell/oh-my-zsh/raw/master/tools/install.sh -O - | zsh || true
