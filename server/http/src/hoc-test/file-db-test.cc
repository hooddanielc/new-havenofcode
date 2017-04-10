#include <string>
#include <stdlib.h>
#include <lick/lick.h>
#include <functional>
#include <hoc/json.h>
#include <hoc-test/fixtures.h>
#include <hoc/db/connection.h>
#include <hoc/actions/account.h>

using namespace std;
using namespace hoc;

void member_insert_files() {
  {
    actions::register_account("test_another@test.com", "password");
    actions::confirm_account("test_another@test.com", "password");
    auto c = db::member_connection("test_another@test.com", "password");
    pqxx::work w(*c);
    // 200 megabyte file
    auto file_200_res = w.exec("insert into file (aws_key, aws_region, bits) values ('key', 'region', 1677721600) returning id");
    for (int i = 0; i < 40; ++i) {
      w.exec("insert into file_part (bits, file, aws_part_number, updated_at) values (41943040, " + w.quote(file_200_res[0][0].as<string>()) + ", " + to_string(i + 1) + ", 'now()')");
    }
    w.commit();
  }
  {
    actions::register_account("test@test.com", "password");
    actions::confirm_account("test@test.com", "password");
    auto c = db::member_connection("test@test.com", "password");
    pqxx::work w(*c);
    // 200 megabyte file
    auto file_200_res = w.exec("insert into file (aws_key, aws_region, bits) values ('key', 'region', 1677721600) returning id");
    for (int i = 0; i < 40; ++i) {
      w.exec("insert into file_part (bits, file, aws_part_number, updated_at) values (41943040, " + w.quote(file_200_res[0][0].as<string>()) + ", " + to_string(i + 1) + ", 'now()')");
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

int main(int argc, char *argv[]) {
  return dj::lick::main(argc, argv);
}
