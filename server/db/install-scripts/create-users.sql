CREATE USER admin_dev WITH SUPERUSER PASSWORD '123123';
CREATE USER admin_test WITH SUPERUSER PASSWORD '123123';
CREATE USER admin_prod WITH SUPERUSER PASSWORD '123123';

CREATE DATABASE hoc_dev OWNER admin_dev;
CREATE DATABASE hoc_test OWNER admin_test;
CREATE DATABASE hoc_prod OWNER admin_prod;
