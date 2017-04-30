#include <string>
#include <stdlib.h>
#include <lick/lick.h>
#include <functional>
#include <hoc/json.h>
#include <hoc-test/fixtures.h>
#include <hoc/db/connection.h>
#include <hoc/actions/account.h>
#include <hoc/actions/file.h>

using namespace std;
using namespace hoc;

void member_insert_files() {
  {
    actions::register_account("test_another@test.com", "password");
    actions::confirm_account("test_another@test.com", "password");
    auto c = db::member_connection("test_another@test.com", "password");
    pqxx::work w(*c);
    // 200 megabyte file
    auto file_200_res = w.exec("insert into file (aws_key, aws_region, bytes) values ('key', 'region', 1677721600) returning id");
    for (int i = 0; i < 40; ++i) {
      w.exec("insert into file_part (bytes, file, aws_part_number, updated_at) values (41943040, " + w.quote(file_200_res[0][0].as<string>()) + ", " + to_string(i + 1) + ", 'now()')");
    }
    w.commit();
  }
  {
    actions::register_account("test@test.com", "password");
    actions::confirm_account("test@test.com", "password");
    auto c = db::member_connection("test@test.com", "password");
    pqxx::work w(*c);
    // 200 megabyte file
    auto file_200_res = w.exec("insert into file (aws_key, aws_region, bytes) values ('key', 'region', 1677721600) returning id");
    for (int i = 0; i < 40; ++i) {
      w.exec("insert into file_part (bytes, file, aws_part_number, updated_at) values (41943040, " + w.quote(file_200_res[0][0].as<string>()) + ", " + to_string(i + 1) + ", 'now()')");
    }
    w.commit();
  }
}

FIXTURE(member_create_own_files) {
  EXPECT_OK([]() {
    member_insert_files();
    auto c = db::super_user_connection();
    pqxx::work w(*c);
    auto count = w.exec("select count(*) from file");
    EXPECT_EQ(count[0][0].as<int>(), 2);
  });

  delete_test_accounts();
}

FIXTURE(member_cant_delete_other_files) {
  EXPECT_OK([]() {
    member_insert_files();
    auto member1 = db::member_connection("test@test.com", "password");
    pqxx::work w1(*member1);
    auto res1 = w1.exec("select id from file where created_by = current_account_id()");
    auto member2 = db::member_connection("test@test.com", "password");
    pqxx::work w2(*member2);
    auto res2 = w2.exec("select id from file where created_by = current_account_id()");
    EXPECT_EQ(res1.size(), size_t(1));
    EXPECT_EQ(res2.size(), size_t(1));
    w1.exec("update file set deleted = 'TRUE' where id = " + w1.quote(res2[0][0].as<string>()));
    auto files = w2.exec("select deleted from file where created_by = current_account_id()");
    EXPECT_EQ(files[0][0].as<bool>(), false);
    w1.exec("update file set deleted = 'TRUE' where created_by = current_account_id()");
    auto not_deleted = w1.exec("select deleted from file where deleted = 'FALSE'");
    w1.commit();
    EXPECT_EQ(not_deleted.size(), size_t(1));
  });

  delete_test_accounts();
}

FIXTURE(member_cant_view_other_member_file_parts) {
  EXPECT_OK([]() {
    member_insert_files();
    auto c = db::member_connection("test@test.com", "password");
    pqxx::work w(*c);
    auto res = w.exec("select count(*) from file_part");
    EXPECT_EQ(res[0][0].as<int>(), 40);
  });

  delete_test_accounts();
}

FIXTURE(member_can_complete_parts_and_view_completed) {
  EXPECT_OK([]() {
    member_insert_files();
    {
      auto c = db::member_connection("test@test.com", "password");
      pqxx::work w(*c);
      auto res = w.exec("select id from file_part");
      EXPECT_EQ(res.size(), size_t(40));
      for (int i = 0; i < 40; ++i) {
        w.exec("update file_part set aws_etag='etagthing', pending='FALSE', updated_at='now()' where id = " + w.quote(res[i][0].as<string>()));
      }
      w.commit();
    }
    {
      auto c = db::member_connection("test@test.com", "password");
      pqxx::work w(*c);
      auto res = w.exec("select pending, aws_etag, aws_part_number from file_part order by aws_part_number asc");
      EXPECT_EQ(res.size(), size_t(40));
      for (int i = 0; i < 40; ++i) {
        EXPECT_EQ(res[i][0].as<bool>(), false);
        EXPECT_EQ(res[i][1].as<string>(), "etagthing");
        EXPECT_EQ(res[i][2].as<int>(), i + 1);
      }
      w.commit();
    }
  });

  delete_test_accounts();
}

FIXTURE(create_upload_promise_for_small_file) {
  EXPECT_OK([]() {
    actions::register_account("test@test.com", "password");
    actions::confirm_account("test@test.com", "password");
    auto c = db::member_connection("test@test.com", "password");
    bool called = false;
    auto file_id = actions::create_upload_promise(c, "name", 1000000, [&called](const string &, const string &, const string &) {
      called = true;
      return "";
    });
    EXPECT_EQ(called, false);
    pqxx::work w(*c);
    auto files = w.exec(
      "select id, name, bytes, upload_id from file where id = " + w.quote(file_id)
    );
    EXPECT_EQ(files.size(), size_t(1));
    EXPECT_EQ(files[0][0].as<string>(), file_id);
    EXPECT_EQ(files[0][1].as<string>(), "name");
    EXPECT_EQ(files[0][2].as<string>(), "1000000");
    EXPECT_EQ(files[0][3].is_null(), true);
    auto file_parts = w.exec(
      "select bytes, aws_part_number, pending "
      "from file_part where file = " + w.quote(file_id) + " "
      "order by aws_part_number asc"
    );
    EXPECT_EQ(file_parts.size(), size_t(1));
    EXPECT_EQ(file_parts[0][0].as<string>(), "1000000");
    EXPECT_EQ(file_parts[0][1].as<int>(), 1);
    EXPECT_EQ(file_parts[0][2].as<bool>(), true);
  });

  delete_test_accounts();
}

FIXTURE(create_upload_promise_for_large_file) {
  EXPECT_OK([]() {
    actions::register_account("test@test.com", "password");
    actions::confirm_account("test@test.com", "password");
    auto c = db::member_connection("test@test.com", "password");
    bool called = false;
    string generated_key("");
    auto file_id = actions::create_upload_promise(c, "name", 50000001, [&called, &generated_key](const string &region, const string &bucket, const string &key) {
      called = true;
      EXPECT_EQ(region, "us-west-2");
      EXPECT_EQ(bucket, "havenofcode");
      generated_key = key;
      return "imanuploadid";
    });
    EXPECT_EQ(called, true);
    pqxx::work w(*c);
    auto files = w.exec(
      "select id, name, bytes, upload_id, aws_key from file where id = " + w.quote(file_id)
    );
    EXPECT_EQ(files.size(), size_t(1));
    EXPECT_EQ(files[0][0].as<string>(), file_id);
    EXPECT_EQ(files[0][1].as<string>(), "name");
    EXPECT_EQ(files[0][2].as<string>(), "50000001");
    EXPECT_EQ(files[0][3].as<string>(), "imanuploadid");
    EXPECT_EQ(files[0][4].as<string>(), generated_key);
    auto file_parts = w.exec(
      "select bytes, aws_part_number, pending "
      "from file_part where file = " + w.quote(file_id) + " "
      "order by aws_part_number asc"
    );
    EXPECT_EQ(file_parts.size(), size_t(11));
    for (int i = 0; i < 10; ++i) {
      EXPECT_EQ(file_parts[i][0].as<string>(), "5000000");
      EXPECT_EQ(file_parts[i][1].as<int>(), i + 1);
      EXPECT_EQ(file_parts[i][2].as<bool>(), true);
    }
    EXPECT_EQ(file_parts[10][0].as<string>(), "1");
    EXPECT_EQ(file_parts[10][1].as<int>(), 11);
    EXPECT_EQ(file_parts[10][2].as<bool>(), true);
  });

  delete_test_accounts();
}

FIXTURE(cancel_upload_promise_for_small_file) {
  EXPECT_OK([]() {
    actions::register_account("test@test.com", "password");
    actions::confirm_account("test@test.com", "password");
    auto c = db::member_connection("test@test.com", "password");
    auto file_id = actions::create_upload_promise(c, "name", 1000000, [](const string &, const string &, const string &) {
      return "asdf";
    });
    bool called = false;
    actions::cancel_upload_promise(c, file_id, [&called](const string &, const string &, const string &, const string &) {
      called = true;
    });
    pqxx::work w(*c);
    auto file = w.exec("select status from file where id = " + w.quote(file_id));
    auto parts = w.exec("select pending from file_part where file = " + w.quote(file_id));
    EXPECT_EQ(file[0][0].as<string>(), "canceled");
    EXPECT_EQ(parts[0][0].as<bool>(), false);
    EXPECT_EQ(called, false);
  });

  delete_test_accounts();
}

FIXTURE(cancel_upload_promise_for_large_file) {
  EXPECT_OK([]() {
    actions::register_account("test@test.com", "password");
    actions::confirm_account("test@test.com", "password");
    auto c = db::member_connection("test@test.com", "password");
    auto file_id = actions::create_upload_promise(c, "name", 5000001, [](const string &, const string &, const string &) {
      return "asdf";
    });
    bool called = false;
    pqxx::work w2(*c);
    auto generated_key = w2.exec("select aws_key from file where id = " + w2.quote(file_id))[0][0].as<string>();
    w2.commit();
    actions::cancel_upload_promise(c, file_id, [&called, &generated_key](const string &region, const string &bucket, const string &key, const string &upload_id) {
      EXPECT_EQ(upload_id, "asdf");
      EXPECT_EQ(region, "us-west-2");
      EXPECT_EQ(bucket, "havenofcode");
      EXPECT_EQ(key, generated_key);
      called = true;
    });
    pqxx::work w(*c);
    auto file = w.exec("select status from file where id = " + w.quote(file_id));
    auto parts = w.exec("select pending from file_part where file = " + w.quote(file_id));
    EXPECT_EQ(file[0][0].as<string>(), "canceled");
    for (size_t i = 0; i < parts.size(); ++i) {
      EXPECT_EQ(parts[i][0].as<bool>(), false);
    }
  });

  delete_test_accounts();
}

FIXTURE(complete_file_part_promise_for_small_file) {
  EXPECT_OK([]() {
    actions::register_account("test@test.com", "password");
    actions::confirm_account("test@test.com", "password");
    auto c = db::member_connection("test@test.com", "password");
    auto file_id = actions::create_upload_promise(c, "name", 1000000, [](const string &, const string &, const string &) {
      return "asdf";
    });
    string file_name = "file_name.exe";

    pqxx::work w1(*c);
    auto file_part = w1.exec("select id from file_part where file = " + w1.quote(file_id));
    auto generated_key = w1.exec("select aws_key from file where id = " + w1.quote(file_id))[0][0].as<string>();
    w1.commit();
    auto file_part_id = file_part[0][0].as<string>();
    EXPECT_FAIL([&file_name, c, &file_part_id]() {
      actions::complete_file_part_promise(
        c,
        file_part_id,
        file_name,
        [](
          const string &,
          const string &,
          const string &,
          const string &,
          const int,
          const string &
        ) {
          return ""; // non multipart upload returns empty string
        }
      );
    });
    actions::start_file_part_promise(c, file_part_id);
    actions::complete_file_part_promise(
      c,
      file_part_id,
      file_name,
      [&generated_key, &file_name](
        const string &region,
        const string &bucket,
        const string &key,
        const string &upload_id,
        const int part_number,
        const string &f_name
      ) {
        EXPECT_EQ(region, "us-west-2");
        EXPECT_EQ(bucket, "havenofcode");
        EXPECT_EQ(key, generated_key);
        EXPECT_EQ(upload_id, ""); // small files do not have an upload_id
        EXPECT_EQ(part_number, 1);
        EXPECT_EQ(f_name, file_name);
        return "";
      }
    );

    pqxx::work w2(*c);
    auto fp = w2.exec("select aws_etag, pending from file_part where id = " + w2.quote(file_part_id));
    EXPECT_EQ(fp[0][0].is_null(), true);
    EXPECT_EQ(fp[0][1].as<bool>(), false);
  });

  delete_test_accounts();
}

FIXTURE(complete_file_part_promise_for_large_file) {
  EXPECT_OK([]() {
    actions::register_account("test@test.com", "password");
    actions::confirm_account("test@test.com", "password");
    auto c = db::member_connection("test@test.com", "password");
    auto file_id = actions::create_upload_promise(c, "name", 5000001, [](const string &, const string &, const string &) {
      return "asdf";
    });
    string file_name = "file_name.exe";

    pqxx::work w1(*c);
    auto file_parts = w1.exec("select id, aws_part_number from file_part where file = " + w1.quote(file_id) + " order by aws_part_number asc");
    auto generated_key = w1.exec("select aws_key from file where id = " + w1.quote(file_id))[0][0].as<string>();
    w1.commit();
    for (size_t i = 0; i < file_parts.size(); ++i) {
      auto file_part_id = file_parts[i][0].as<string>();
      auto file_part_number = file_parts[i][1].as<int>();
      EXPECT_FAIL([&file_name, c, &file_part_id]() {
        actions::complete_file_part_promise(
          c,
          file_part_id,
          file_name,
          [](
            const string &,
            const string &,
            const string &,
            const string &,
            const int,
            const string &
          ) {
            return "";
          }
        );
      });
      actions::start_file_part_promise(c, file_part_id);
      actions::complete_file_part_promise(
        c,
        file_part_id,
        file_name,
        [&file_name, &generated_key, &file_part_number](
          const string &region,
          const string &bucket,
          const string &key,
          const string &upload_id,
          const int part_number,
          const string &f_name
        ) {
          EXPECT_EQ(region, "us-west-2");
          EXPECT_EQ(bucket, "havenofcode");
          EXPECT_EQ(key, generated_key);
          EXPECT_EQ(upload_id, "asdf"); // small files do not have an upload_id
          EXPECT_EQ(part_number, file_part_number);
          EXPECT_EQ(file_name, f_name);
          return "aws_etag";
        }
      );
    }

    pqxx::work w2(*c);
    auto fps = w2.exec("select aws_etag, pending from file_part where file = " + w2.quote(file_id) + " order by aws_part_number asc");
    for (size_t i = 0; i < fps.size(); ++i) {
      EXPECT_EQ(fps[i][0].as<string>(), "aws_etag");
      EXPECT_EQ(fps[i][1].as<bool>(), false);
    }
  });

  delete_test_accounts();
}

FIXTURE(failed_file_part_promise_rolls_back) {
  EXPECT_OK([]() {
    actions::register_account("test@test.com", "password");
    actions::confirm_account("test@test.com", "password");
    auto c = db::member_connection("test@test.com", "password");
    auto file_id = actions::create_upload_promise(c, "name", 5000001, [](const string &, const string &, const string &) {
      return "asdf";
    });
    string file_name = "file_name.exe";

    pqxx::work w1(*c);
    auto file_parts = w1.exec("select id, aws_part_number from file_part where file = " + w1.quote(file_id) + " order by aws_part_number asc");
    auto generated_key = w1.exec("select aws_key from file where id = " + w1.quote(file_id))[0][0].as<string>();
    w1.commit();
    for (size_t i = 0; i < file_parts.size(); ++i) {
      auto file_part_id = file_parts[i][0].as<string>();
      auto file_part_number = file_parts[i][1].as<int>();
      EXPECT_FAIL([&file_name, c, &file_part_id]() {
        actions::complete_file_part_promise(
          c,
          file_part_id,
          file_name,
          [](
            const string &,
            const string &,
            const string &,
            const string &,
            const int,
            const string &
          ) {
            return "";
          }
        );
      });

      actions::start_file_part_promise(c, file_part_id);

      EXPECT_FAIL([&]() {
        actions::complete_file_part_promise(
          c,
          file_part_id,
          file_name,
          [&generated_key, &file_part_number](
            const string &,
            const string &,
            const string &,
            const string &,
            const int,
            const string &
          ) {
            throw runtime_error("this should autoremove promise row");
            return "aws_etag";
          }
        );
      });

      pqxx::work wp(*c);
      auto promise = wp.exec("select count(*) from file_part_promise where id = " + wp.quote(file_part_id));
      wp.commit();
      EXPECT_EQ(promise[0][0].as<int>(), 0);

      // promise needs to be started again
      EXPECT_FAIL([&]() {
        actions::complete_file_part_promise(
          c,
          file_part_id,
          file_name,
          [&generated_key, &file_part_number](
            const string &,
            const string &,
            const string &,
            const string &,
            const int,
            const string &
          ) {
            return "aws_etag";
          }
        );
      });

      actions::start_file_part_promise(c, file_part_id);
      actions::complete_file_part_promise(
        c,
        file_part_id,
        file_name,
        [&generated_key, &file_part_number, &file_name](
          const string &region,
          const string &bucket,
          const string &key,
          const string &upload_id,
          const int part_number,
          const string &f_name
        ) {
          EXPECT_EQ(region, "us-west-2");
          EXPECT_EQ(bucket, "havenofcode");
          EXPECT_EQ(key, generated_key);
          EXPECT_EQ(upload_id, "asdf"); // small files do not have an upload_id
          EXPECT_EQ(part_number, file_part_number);
          EXPECT_EQ(f_name, file_name);
          return "aws_etag";
        }
      );
    }

    pqxx::work w2(*c);
    auto fps = w2.exec("select aws_etag, pending from file_part where file = " + w2.quote(file_id) + " order by aws_part_number asc");
    for (size_t i = 0; i < fps.size(); ++i) {
      EXPECT_EQ(fps[i][0].as<string>(), "aws_etag");
      EXPECT_EQ(fps[i][1].as<bool>(), false);
    }
  });

  delete_test_accounts();
}

FIXTURE(complete_entire_file_promise_for_small_file) {
  EXPECT_OK([]() {
    actions::register_account("test@test.com", "password");
    actions::confirm_account("test@test.com", "password");
    auto c = db::member_connection("test@test.com", "password");
    auto file_id = actions::create_upload_promise(c, "name", 1000000, [](const string &, const string &, const string &) {
      return "asdf";
    });
    std::string file_name = "file_name.exe";
    pqxx::work w1(*c);
    auto file = w1.exec("select status, aws_key from file where id = " + w1.quote(file_id));
    EXPECT_EQ(file[0][0].as<string>(), "pending");
    auto generated_key = file[0][1].as<string>();
    auto file_part = w1.exec("select id from file_part where file = " + w1.quote(file_id));
    w1.commit();
    auto file_part_id = file_part[0][0].as<string>();
    actions::start_file_part_promise(c, file_part_id);
    actions::complete_file_part_promise(
      c,
      file_part_id,
      file_name,
      [](
        const string &,
        const string &,
        const string &,
        const string &,
        const int,
        const string &
      ) {
        return "";
      }
    );
    actions::complete_upload_promise(
      c,
      file_id,
      [&generated_key](
        const string &region,
        const string &bucket,
        const string &key,
        const string &upload_id,
        const vector<string> &keys
      ) {
        EXPECT_EQ(region, "us-west-2");
        EXPECT_EQ(bucket, "havenofcode");
        EXPECT_EQ(key, generated_key);
        EXPECT_EQ(upload_id, "");
        EXPECT_EQ(keys.size(), size_t(1));
      }
    );
    pqxx::work w2(*c);
    auto finished_status = w2.exec("select status from file where id = " + w2.quote(file_id));
    EXPECT_EQ(finished_status[0][0].as<string>(), "complete");
  });

  delete_test_accounts();
}

FIXTURE(complete_entire_file_promise_for_large_file) {
  EXPECT_OK([]() {
    actions::register_account("test@test.com", "password");
    actions::confirm_account("test@test.com", "password");
    auto c = db::member_connection("test@test.com", "password");
    auto file_id = actions::create_upload_promise(c, "name", 10000001, [](const string &, const string &, const string &) {
      return "asdf";
    });
    pqxx::work w1(*c);
    auto file = w1.exec("select status, aws_key from file where id = " + w1.quote(file_id));
    EXPECT_EQ(file[0][0].as<string>(), "pending");
    auto generated_key = file[0][1].as<string>();
    auto file_parts = w1.exec("select id from file_part where file = " + w1.quote(file_id) + " order by aws_part_number asc");
    w1.commit();
    for (size_t i = 0; i < file_parts.size(); ++i) {
      auto file_part_id = file_parts[i][0].as<string>();
      string file_name = "file_name.exe";
      actions::start_file_part_promise(c, file_part_id);
      actions::complete_file_part_promise(
        c,
        file_part_id,
        file_name,
        [](
          const string &,
          const string &,
          const string &,
          const string &,
          const int,
          const string &
        ) {
          return "imanawsetag";
        }
      );
    }
    actions::complete_upload_promise(
      c,
      file_id,
      [&generated_key](
        const string &region,
        const string &bucket,
        const string &key,
        const string &upload_id,
        const vector<string> &keys
      ) {
        EXPECT_EQ(region, "us-west-2");
        EXPECT_EQ(bucket, "havenofcode");
        EXPECT_EQ(key, generated_key);
        EXPECT_EQ(upload_id, "asdf");
        EXPECT_EQ(keys.size(), size_t(3));
      }
    );
    pqxx::work w2(*c);
    auto finished_status = w2.exec("select status from file where id = " + w2.quote(file_id));
    EXPECT_EQ(finished_status[0][0].as<string>(), "complete");
  });

  delete_test_accounts();
}

int main(int argc, char *argv[]) {
  return dj::lick::main(argc, argv);
}
