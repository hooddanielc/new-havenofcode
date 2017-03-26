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
        deleted     boolean default 'FALSE' not null,
        salt        varchar(32) not null,
        password    text not null,
        email       text unique not null
      );

      grant all on registration to admins;
      grant update (deleted, updated_at) on registration to members;
      grant insert (salt, password, email) on registration to public;
      alter table registration enable row level security;
      create policy admin_registration on registration to admins
        using (true)
        with check (true);
      create policy member_update_registration on registration for update to members
        using (email = current_user)
        with check (current_user = email AND deleted = 'TRUE' AND updated_at = NOW());
      create policy anonymous_select_registration on registration for select to public
        using (deleted = 'FALSE');
      create policy anonymous_insert_registration on registration for insert to public
        with check (deleted = 'FALSE');

      create table account (
        id          uuid primary key default uuid_generate_v4() not null,
        created_at  timestamp with time zone default 'now()' not null,
        updated_at  timestamp with time zone default 'now()' not null,
        deleted     boolean default 'FALSE' not null,
        salt        varchar(32) not null,
        email       text not null,
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




      create table session (
        id          uuid primary key not null,
        created_at  timestamp with time zone default 'now()' not null,
        updated_at  timestamp with time zone default 'now()' not null,
        deleted     boolean default 'FALSE' not null,
        created_by  uuid not null,
        foreign key (created_by) references account(id)
      );

      create table experience (
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
