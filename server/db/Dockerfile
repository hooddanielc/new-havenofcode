FROM dhoodlum/base-environment

# install postgresql
RUN echo "deb http://apt.postgresql.org/pub/repos/apt/ jessie-pgdg main" >> /etc/apt/sources.list
RUN wget --quiet -O - https://www.postgresql.org/media/keys/ACCC4CF8.asc | \
  apt-key add -
RUN apt update
RUN apt install -y postgresql-9.4

# install nodejs
RUN apt install -y xz-utils
RUN cd /opt && wget https://nodejs.org/dist/v6.9.5/node-v6.9.5-linux-x64.tar.xz
RUN cd /opt && tar -xvf ./node-v6.9.5-linux-x64.tar.xz

# set up
USER postgres
RUN mkdir /var/run/postgresql/9.4-main.pg_stat_tmp
ENV PATH $PATH:/usr/lib/postgresql/9.4/bin:/opt/node-v6.9.5-linux-x64/bin
RUN mkdir $HOME/data
RUN pg_ctl init -D $HOME/data

# add users
ADD ./install-scripts /var/lib/postgresql/install-scripts
RUN  pg_ctl -w start -D $HOME/data && \
  psql -U postgres -f $HOME/install-scripts/create-users.sql && \
  pg_ctl -w stop -D $HOME/data

# adjust PostgreSQL configuration so that remote connections to the
# database are possible.
RUN echo "host all  all    0.0.0.0/0  md5" >> $HOME/data/pg_hba.conf

# And add ``listen_addresses`` to ``/etc/postgresql/9.3/main/postgresql.conf``
RUN echo "listen_addresses='*'" >> $HOME/data/postgresql.conf

# Postgres listens on 5432
EXPOSE 5432

# Allow backup
VOLUME  ["/etc/postgresql", "/var/log/postgresql", "/var/lib/postgresql"]

CMD postgres -D $HOME/data