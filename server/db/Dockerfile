FROM dhoodlum/base-environment

# install postgres using current user
RUN sudo pacman --noconfirm -S postgresql
ENV PGDATA $HOME/data
RUN pg_ctl initdb -D $HOME/data -U developer
RUN sudo mkdir /run/postgresql && sudo chown developer /run/postgresql

# install nodejs db tools
RUN mkdir $HOME/db-tools
ADD ./db-tools/package.json /home/developer/db-tools/package.json
RUN cd $HOME/db-tools && npm install

# install users
ADD ./install-scripts /home/developer/install-scripts
RUN  cd $HOME/data && pg_ctl -w start -D $HOME/data -l run.log && \
  createdb -O developer developer && \
  psql -f $HOME/install-scripts/create-users.sql && \
  pg_ctl -w stop -D $HOME/data

# adjust PostgreSQL configuration to allow remote connections
RUN echo "host all  all    0.0.0.0/0  md5" >> $HOME/data/pg_hba.conf

# And add ``listen_addresses`` to ``/etc/postgresql/9.3/main/postgresql.conf``
RUN echo "listen_addresses='*'" >> $HOME/data/postgresql.conf

# Add nodejs tools
ADD ./db-tools/config /home/developer/db-tools/config
ADD ./db-tools/models /home/developer/db-tools/models
ADD ./db-tools/src /home/developer/db-tools/src
ADD ./db-tools/gulpfile.js /home/developer/db-tools/gulpfile.js

RUN cd $HOME/db-tools && ./node_modules/.bin/gulp

ADD ./db-tools/migrations /home/developer/db-tools/migrations

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
