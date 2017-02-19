FROM dhoodlum/base-environment

# install postgresql
RUN echo "deb http://apt.postgresql.org/pub/repos/apt/ jessie-pgdg main" >> /etc/apt/sources.list
RUN wget --quiet -O - https://www.postgresql.org/media/keys/ACCC4CF8.asc | \
  apt-key add -
RUN apt update
RUN apt install -y postgresql-9.4
USER postgres
RUN mkdir /var/run/postgresql/9.4-main.pg_stat_tmp
ENV PATH $PATH:/usr/lib/postgresql/9.4/bin
