FROM dhoodlum/arch-base-devel

RUN echo "en_US.UTF-8 UTF-8" >> /etc/locale.gen
RUN locale-gen
RUN echo "LANG=en_US.UTF-8" > /etc/locale.conf

RUN pacman -Syyu
RUN pacman --noconfirm -S zsh
RUN pacman --noconfirm -S git
RUN pacman --noconfirm -S wget
RUN pacman --noconfirm -S sudo
RUN pacman --noconfirm -S clang

# set default make flags
RUN echo MAKEFLAGS=\"-j$(grep -c ^processor /proc/cpuinfo)\" >> /etc/makepkg.conf

# setup sudoers for developer
RUN echo "developer ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# add and use developer user
RUN useradd -m -G wheel -s /bin/zsh developer
USER developer
RUN wget https://github.com/robbyrussell/oh-my-zsh/raw/master/tools/install.sh -O - | zsh || true

# install nodejs
RUN sudo pacman --noconfirm -S nodejs
RUN sudo pacman --noconfirm -S npm
RUN wget -qO- https://raw.githubusercontent.com/creationix/nvm/v0.33.2/install.sh | bash
RUN sudo mkdir /opt/ib && sudo chown -R developer /opt/ib

# install ib
RUN git clone https://github.com/JasonL9000/ib.git /opt/ib
RUN sudo pacman --noconfirm -S python2
RUN sudo ln -s /usr/bin/python2 /usr/bin/python

# install cmake
RUN sudo pacman --noconfirm -S cmake

# set path env
ENV PATH /opt/ib:$PATH

CMD /bin/zsh
