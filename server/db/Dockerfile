FROM dhoodlum/base-environment

# install postgresql
RUN echo "deb http://apt.postgresql.org/pub/repos/apt/ jessie-pgdg main" >> /etc/apt/sources.list
RUN wget --quiet -O - https://www.postgresql.org/media/keys/ACCC4CF8.asc | \
  apt-key add -
RUN apt update
RUN apt install -y postgresql-9.6

# install nodejs
RUN apt install -y xz-utils
RUN cd /opt && wget https://nodejs.org/dist/v6.9.5/node-v6.9.5-linux-x64.tar.xz
RUN cd /opt && tar -xvf ./node-v6.9.5-linux-x64.tar.xz

# set up postgres
USER postgres
RUN mkdir /var/run/postgresql/9.6-main.pg_stat_tmp
ENV PATH $PATH:/usr/lib/postgresql/9.6/bin:/opt/node-v6.9.5-linux-x64/bin
RUN mkdir $HOME/data
RUN pg_ctl init -D $HOME/data

# install nodejs modules
RUN mkdir $HOME/db-tools
ADD ./db-tools/package.json /var/lib/postgresql/db-tools/package.json
RUN cd $HOME/db-tools && npm install

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

# Add nodejs tools
ADD ./db-tools/config /var/lib/postgresql/db-tools/config
ADD ./db-tools/models /var/lib/postgresql/db-tools/models
ADD ./db-tools/src /var/lib/postgresql/db-tools/src
ADD ./db-tools/gulpfile.js /var/lib/postgresql/db-tools/gulpfile.js

RUN cd $HOME/db-tools && ./node_modules/.bin/gulp

ADD ./db-tools/migrations /var/lib/postgresql/db-tools/migrations

RUN  pg_ctl -w start -D $HOME/data && \
  cd $HOME/db-tools && ./node_modules/.bin/sequelize db:migrate --env development && \
  cd $HOME/db-tools && ./node_modules/.bin/sequelize db:migrate --env test && \
  cd $HOME/db-tools && ./node_modules/.bin/sequelize db:migrate --env production && \
  pg_ctl -w stop -D $HOME/data

RUN  pg_ctl -w start -D $HOME/data && \
  psql -U admin_dev -d hoc_dev -f $HOME/install-scripts/insert-app-specific-tokens.sql && \
  psql -U admin_test -d hoc_test -f $HOME/install-scripts/insert-app-specific-tokens.sql && \
  psql -U admin_prod -d hoc_prod -f $HOME/install-scripts/insert-app-specific-tokens.sql && \
  pg_ctl -w stop -D $HOME/data

# Postgres listens on 5432
EXPOSE 5432

# Allow backup
VOLUME  ["/root/data"]

CMD postgres -D $HOME/data