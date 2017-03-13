FROM debian
RUN apt update
RUN apt upgrade -y
RUN apt install -y zsh
RUN apt install -y git-core
RUN apt install -y wget
RUN wget https://github.com/robbyrussell/oh-my-zsh/raw/master/tools/install.sh -O - | zsh || true

# install clang-3.9 like a baws
RUN apt install -y software-properties-common
RUN apt install -y python-software-properties
RUN wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
RUN apt-add-repository "deb http://apt.llvm.org/jessie/ llvm-toolchain-jessie-3.9 main"
RUN apt update
RUN apt install -y clang-3.9 lldb-3.9
