'use strict';

module.exports = {
  up: function (queryInterface, Sequelize) {
    return queryInterface.sequelize.query(`
      create extension if not exists "uuid-ossp";

      do $$
      begin
        if not exists (select 1 from pg_roles where rolname = 'admins') then
          create role admins createrole;
        end if;
      end
      $$;

      do $$
      begin
        if not exists (select 1 from pg_roles where rolname = 'members') then
          create role members;
        end if;
      end
      $$;

      do $$
      begin
        if not exists (select * from pg_user where usename = 'anonymous') then
          create user anonymous with password 'password';
        end if;
      end
      $$;

      create table registration (
        id          uuid primary key default uuid_generate_v4() not null,
        created_at  timestamp with time zone default 'now()' not null,
        updated_at  timestamp with time zone default 'now()' not null,
        salt        varchar(32) not null,
        password    text not null,
        email       text unique not null,
        verified    boolean default 'FALSE' not null
      );

      grant all on registration to admins;
      grant update (verified, updated_at) on registration to members;
      grant select (verified, updated_at, email) on registration to members;
      grant insert (salt, password, email) on registration to public;
      grant update (salt, password) on registration to public;
      grant select (id, password, email, verified) on registration to public;
      alter table registration enable row level security;
      create policy admin_registration on registration to admins
        using (true)
        with check (true);
      create policy member_update_registration on registration to members
        using (email = current_user AND verified = 'FALSE')
        with check (current_user = email AND verified = 'TRUE');
      create policy anonymous_registration on registration to public
        using (verified = 'FALSE')
        with check (verified = 'FALSE');

      create table account (
        id          uuid primary key default uuid_generate_v4() not null,
        created_at  timestamp with time zone default 'now()' not null,
        updated_at  timestamp with time zone default 'now()' not null,
        deleted     boolean default 'FALSE' not null,
        salt        varchar(32) not null,
        email       text unique not null,
        name        text default 'unknown'
      );

      grant all on account to admins;
      grant select, update (updated_at, salt, email, name) on account to members;
      grant select (id, email, salt, created_at, updated_at) on account to public;
      alter table account enable row level security;
      create policy admin_account on account to admins
        using (true)
        with check (true);
      create policy member_account on account to members
        using (true)
        with check (email = current_user);
      create policy anonymous_account on account to public
        using (true);

      create function current_account_id() returns uuid as $$
        select id from account where email = current_user;
      $$ language sql;

      create table session (
        id          uuid primary key default uuid_generate_v4() not null,
        created_at  timestamp with time zone default 'now()' not null,
        updated_at  timestamp with time zone default 'now()' not null,
        deleted     boolean default 'FALSE' not null,
        ip          text not null,
        user_agent  text not null,
        created_by  uuid,
        foreign key (created_by) references account(id)
      );

      grant all on session to admins;
      grant select, insert on session to public;
      grant update (updated_at, deleted, ip) on session to public;
      alter table session enable row level security;
      create policy session_admin on session to admins
        using (true)
        with check (true);
      create policy session_public on session to public
        using (created_by is NULL or created_by = current_account_id())
        with check (created_by is NULL or created_by = current_account_id());

      create table session_ip_log (
        id          uuid primary key default uuid_generate_v4() not null,
        created_at  timestamp with time zone default 'now()' not null,
        ip          text not null,
        user_agent  text not null,
        created_by  uuid,
        session     uuid not null,
        foreign key (created_by) references account(id),
        foreign key (session) references session(id)
      );

      grant insert on session_ip_log to public;
      alter table session_ip_log enable row level security;
      create policy session_ip_log on session_ip_log for insert to public
        with check (created_by is null or created_by = current_account_id());

      create type file_state as enum (
        'pending', 'complete', 'canceled'
      );

      create table file (
        id          uuid primary key default uuid_generate_v4() not null,
        created_by  uuid default current_account_id() not null,
        created_at  timestamp with time zone default 'now()' not null,
        updated_at  timestamp with time zone default 'now()' not null,
        deleted     boolean default 'FALSE' not null,
        name        text,
        aws_key     text default '/' || current_account_id() || '/' || uuid_generate_v4(),
        aws_region  text default 'us-west-2' not null,
        bits        bigint not null,
        status      file_state default 'pending' not null,
        progress    real default 0,
        foreign key (created_by) references account(id)
      );

      grant all on file to admins;
      grant insert on file to members;
      grant update (
        updated_at,
        deleted,
        status,
        progress
      ) on file to members;
      grant select on file to public;
      alter table file enable row level security;
      create policy file_admin on file to admins
        using (true)
        with check (true);
      create policy file_members on file to members
        using (deleted = 'FALSE')
        with check (
          created_by = current_account_id() and
          updated_at is not null
        );
      create policy file_public on file to public
        using (deleted = 'FALSE')
        with check (false);

      create table file_part (
        id              uuid primary key default uuid_generate_v4() not null,
        created_at      timestamp with time zone default 'now()' not null,
        updated_at      timestamp with time zone default 'now()' not null,
        deleted         boolean default 'FALSE' not null,
        bits            int not null check (bits > 0),
        aws_etag        text,
        aws_part_number int not null check (aws_part_number > 0),
        pending         boolean default 'TRUE',
        created_by      uuid default current_account_id() not null,
        file            uuid,
        foreign key     (created_by) references account(id),
        foreign key     (file) references file(id)
      );

      grant all on file_part to admins;
      grant insert, select on file_part to members;
      grant update (
        updated_at,
        deleted,
        aws_etag,
        aws_part_number,
        pending
      ) on file_part to members;
      alter table file_part enable row level security;
      create policy file_part_admin on file_part to admins
        using (true)
        with check (true);
      create policy file_part_members on file_part to members
        using (created_by = current_account_id() and deleted = 'FALSE')
        with check (
          created_by = current_account_id() and
          file is not null and
          updated_at is not null
        );

      create table experience (
        id          uuid primary key default uuid_generate_v4() not null,
        created_at  timestamp with time zone default 'now()' not null,
        updated_at  timestamp with time zone default 'now()' not null,
        created_by  uuid not null,
        deleted     boolean default 'FALSE' not null,
        title       text    not null,
        slug        text    not null,
        description text    not null,
        published   boolean default 'FALSE' not null,
        foreign key (created_by) references account(id)
      );

      create table collection (
        id          uuid primary key not null,
        created_at  timestamp with time zone default 'now()' not null,
        updated_at  timestamp with time zone default 'now()' not null,
        created_by  uuid not null,
        deleted     boolean default 'FALSE' not null,
        title       text    not null,
        slug        text    not null,
        description text    not null,
        published   boolean default 'FALSE' not null,
        foreign key (created_by) references account(id)
      );

      create table experience_cross_collection (
        id          uuid primary key not null,
        created_at  timestamp with time zone default 'now()' not null,
        deleted     boolean default 'FALSE' not null,
        experience  uuid not null,
        collection  uuid not null,
        foreign key (experience) references experience(id),
        foreign key (collection) references collection(id)
      );

      create table comment_type (
        id          uuid primary key not null,
        name        text not null
      );

      create table comment (
        id          uuid not null unique,
        type        uuid not null,
        created_at  timestamp with time zone default 'now()' not null,
        updated_at  timestamp with time zone default 'now()' not null,
        created_by  uuid not null,
        deleted     boolean default 'FALSE' not null,
        message     text not null,
        parent      uuid not null,
        primary key (id, type),
        foreign key (type) references comment_type(id),
        foreign key (parent) references comment(id),
        foreign key (created_by) references account(id)
      );

      create table comment_history (
        id              uuid primary key not null,
        comment_parent  uuid not null,
        was_created_at  timestamp with time zone default 'now()' not null,
        foreign key (comment_parent) references comment(id)
      );

      create table experience_cross_comment (
        id          uuid primary key not null,
        created_at  timestamp with time zone default 'now()' not null,
        deleted     boolean default 'FALSE' not null,
        experience  uuid not null,
        comment     uuid not null,
        foreign key (experience) references experience(id),
        foreign key (comment) references comment(id)
      );

      create table fragment_type (
        id          uuid primary key not null,
        name        text not null
      );

      create table fragment (
        id          uuid not null unique,
        type        uuid not null,
        created_at  timestamp with time zone default 'now()' not null,
        updated_at  timestamp with time zone default 'now()' not null,
        created_by  uuid not null,
        deleted     boolean default 'FALSE' not null,
        parent      uuid not null,
        title           text,
        margin_top      text,
        margin_right    text,
        margin_bottom   text,
        margin_left     text,
        padding_top     text,
        padding_right   text,
        padding_bottom  text,
        padding_left    text,
        width_small     text,
        width_medium    text,
        width_large     text,
        height_small    text,
        height_medium   text,
        height_large    text,
        rel_align_parent_top    boolean,
        rel_align_parent_right  boolean,
        rel_align_parent_bottom boolean,
        rel_align_parent_left   boolean,
        rel_to_left_of          uuid,
        rel_to_right_of         uuid,
        rel_below               uuid,
        rel_above               uuid,
        rel_align_top           uuid,
        rel_align_right         uuid,
        rel_align_bottom        uuid,
        rel_align_left          uuid,
        rel_center_horizontal   boolean,
        rel_center_vertical     boolean,
        primary key (id, type),
        foreign key (type) references fragment_type(id),
        foreign key (parent) references fragment(id),
        foreign key (created_by) references account(id)
      );

      create table fragment_cross_experience (
        id          uuid primary key not null,
        created_at  timestamp with time zone default 'now()' not null,
        deleted     boolean default 'FALSE' not null,
        fragment    uuid not null,
        experience  uuid not null,
        foreign key (experience) references experience(id),
        foreign key (fragment) references fragment(id)
      );

      create table fragment_linear_layout (
        id          uuid not null unique,
        type        uuid not null,
        horizontal  boolean not null,
        primary key (id, type),
        foreign key (id, type) references fragment(id, type)
      );

      create table fragment_relative_layout (
        id          uuid not null unique,
        type        uuid not null,
        primary key (id, type),
        foreign key (id, type) references fragment(id, type)
      );

      create table fragment_embed_video (
        id          uuid not null unique,
        type        uuid not null,
        url         text not null,
        primary key (id, type),
        foreign key (id, type) references fragment(id, type)
      );

      create table fragment_image_upload (
        id          uuid not null unique,
        type        uuid not null,
        url         text not null,
        image_codec text not null,
        primary key (id, type),
        foreign key (id, type) references fragment(id, type)
      );

      create table fragment_audio_upload (
        id          uuid not null unique,
        type        uuid not null,
        url         text not null,
        bitrate     integer not null,
        primary key (id, type),
        foreign key (id, type) references fragment(id, type)
      );

      create table fragment_link_preview (
        id          uuid not null unique,
        type        uuid not null,
        url         text not null,
        primary key (id, type),
        foreign key (id, type) references fragment(id, type)
      );

      create table fragment_code_snippet (
        id               uuid not null unique,
        type             uuid not null,
        language         text not null,
        program          text not null,
        primary key (id, type),
        foreign key (id, type) references fragment(id, type)
      );

      create table app_token (
        id              text primary key not null,
        created_at      timestamp with time zone default 'now()' not null,
        updated_at      timestamp with time zone default 'now()' not null,
        refresh_token   text not null
      );
    `);
  }
};
