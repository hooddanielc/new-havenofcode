#include <hoc-test/test-helper.h>
#include <hoc-active-record/active-record.h>
#include <hoc/db/connection.h>

using namespace hoc;

class model_z_t: public model_t<model_z_t> {
public:

  primary_key_t<int> id;

  int age;

  static const table_t<model_z_t> table;

};

const table_t<model_z_t> model_z_t::table = {
  "model_z_t", {
    make_col("id", &model_z_t::id),
    make_col("age", &model_z_t::age)
  }
};  // model_z_t

class model_a_t: public model_t<model_a_t> {
public:

  primary_key_t<std::string> id;

  int age;

  double opacity;

  int64_t huge_num;

  foreign_key_t<model_z_t, int> foreign;

  static const table_t<model_a_t> table;

};

const table_t<model_a_t> model_a_t::table = {
  "model_a_t", {
    make_col("id", &model_a_t::id),
    make_col("opacity", &model_a_t::opacity),
    make_col("huge_num", &model_a_t::huge_num),
    make_col("age", &model_a_t::age),
    make_col("foreign", &model_a_t::foreign, &model_z_t::id)
  }
};  // model_a_t

class model_b_t: public model_t<model_b_t> {
public:

  primary_key_t<std::string> id;

  foreign_key_t<model_a_t, std::string> foreign;

  std::string name;

  static const table_t<model_b_t> table;
};

const table_t<model_b_t> model_b_t::table = {
  "model_b_t", {
    make_col("id", &model_b_t::id),
    make_col("foreign", &model_b_t::foreign, &model_a_t::id),
    make_col("name", &model_b_t::name)
  }
};  // model_b_t

class model_c_t: public model_t<model_c_t> {
public:

  primary_key_t<std::string> id;

  foreign_key_t<model_a_t, std::string> foreign;

  std::string name;

  static const table_t<model_c_t> table;
};

const table_t<model_c_t> model_c_t::table = {
  "model_c_t", {
    make_col("id", &model_c_t::id),
    make_col("foreign", &model_c_t::foreign, &model_a_t::id),
    make_col("name", &model_c_t::name)
  }
};  // model_c_t

class model_d_t: public model_t<model_d_t> {
public:

  primary_key_t<std::string> id;

  foreign_key_t<model_c_t, std::string> foreign;

  std::string name;

  static const table_t<model_d_t> table;
};

const table_t<model_d_t> model_d_t::table = {
  "model_d_t", {
    make_col("id", &model_d_t::id),
    make_col("foreign", &model_d_t::foreign, &model_c_t::id),
    make_col("name", &model_d_t::name)
  }
};  // model_c_t

FIXTURE(adapter_to_sql) {
  // find
  auto subject_find_one = make_sql_adapter(&model_a_t::id).find("'one'");
  EXPECT_EQ(
    subject_find_one.to_sql(),
    "select model_a_t.* from model_a_t where (model_a_t.id = 'one')"
  );
  auto subject_find_two = make_sql_adapter(&model_a_t::id).find("'one'").find("'two'");
  EXPECT_EQ(
    subject_find_two.to_sql(),
    "select model_a_t.* from model_a_t where (model_a_t.id = 'one' or model_a_t.id = 'two')"
  );

  // find_by
  auto subject_find_by = make_sql_adapter(&model_a_t::id).find_by(&model_a_t::age, "'1'");
  EXPECT_EQ(
    subject_find_by.to_sql(),
    "select model_a_t.* from model_a_t where model_a_t.age = '1'"
  );

  // distinct limit
  auto subject_limit_distinct = make_sql_adapter(&model_a_t::id).distinct().limit(10);
  EXPECT_EQ(
    subject_limit_distinct.to_sql(),
    "select distinct model_a_t.* from model_a_t  limit 10"
  );

  // limit offset
  auto subject_limit_offset = make_sql_adapter(&model_a_t::id).distinct().limit(10).offset(10);
  EXPECT_EQ(
    subject_limit_offset.to_sql(),
    "select distinct model_a_t.* from model_a_t  limit 10 offset 10"
  );

  // where find_by find
  auto subject_where_find = make_sql_adapter(&model_a_t::id)
    .where("model_a_t.id like '%keyword$%'")
    .find("'123'")
    .find_by(&model_a_t::age, "'321'")
    .limit(10);

  EXPECT_EQ(
    subject_where_find.to_sql(),
    "select model_a_t.* from model_a_t where (model_a_t.id = '123') or "
    "(model_a_t.id like '%keyword$%') and model_a_t.age = '321' limit 10"
  );
}

FIXTURE(adapter) {
  auto subject = make_sql_adapter(&model_a_t::id);
  subject.find_by(&model_a_t::age, "'321'");
  subject.find("123");
  subject.find("321");
  subject.find_by("id", "123");
  subject.find_by(&model_a_t::id, "'321'");
  subject.find_by(&model_c_t::id, "'420'");
  subject.find_by(&model_c_t::name, "alexander");
  subject.find_by(&model_c_t::foreign, "for_321");
  subject.where("id > 0");
  subject.where("id is not null");
  subject.order("id", "asc");
  subject.order(&model_a_t::id, "asc");
  subject.order(&model_a_t::age, "asc");
  subject.order(&model_a_t::opacity, "desc");
  subject.order(&model_b_t::id);
  subject.order(&model_c_t::id);
  subject.joins(&model_b_t::foreign, &model_c_t::foreign, &model_d_t::foreign);
  //subject.write(std::cout);

  auto related_subject = make_sql_adapter(&model_b_t::id);
  related_subject.find("123");
  related_subject.find("321");
  related_subject.find_by("id", "123");
  related_subject.find_by(&model_b_t::id, "321");
  related_subject.find_by(&model_b_t::foreign, "321");
  related_subject.find_by(&model_b_t::name, "this is name");
  related_subject.where("id > 0");
  related_subject.where("id is not null");
  related_subject.order("id", "asc");
  related_subject.order(&model_b_t::foreign, "desc");
  // related_subject.write(std::cout);
}

FIXTURE(valuable_t) {
  valuable_t<int> subject = 5;

  // conditional true
  EXPECT_TRUE(subject == 5); // ==
  EXPECT_TRUE(5 == subject);
  EXPECT_TRUE(subject != 0); // !=
  EXPECT_TRUE(0 != subject);
  EXPECT_TRUE(subject >= 5); // >=
  EXPECT_TRUE(5 >= subject);
  EXPECT_TRUE(subject <= 5); // <=
  EXPECT_TRUE(5 <= subject);
  EXPECT_TRUE(subject < 6); // <
  EXPECT_TRUE(4 < subject);
  EXPECT_TRUE(subject > 4); // >
  EXPECT_TRUE(6 > subject);

  // conditional false
  EXPECT_FALSE(subject == 0); // ==
  EXPECT_FALSE(0 == subject);
  EXPECT_FALSE(subject != 5); // !=
  EXPECT_FALSE(5 != subject);
  EXPECT_FALSE(subject >= 6); // >=
  EXPECT_FALSE(4 >= subject);
  EXPECT_FALSE(subject <= 4); // <=
  EXPECT_FALSE(6 <= subject);
  EXPECT_FALSE(subject < 5); // <
  EXPECT_FALSE(5 < subject);
  EXPECT_FALSE(subject > 5); // >
  EXPECT_FALSE(5 > subject);

  // arithmetic operator
  EXPECT_TRUE(subject + 1 == 6);
  EXPECT_TRUE(subject - 1 == 4);
  EXPECT_TRUE(subject * 2 == 10);
  EXPECT_TRUE((subject + 5) / 2 == 5);
  EXPECT_TRUE(subject % 2 == 1);
  EXPECT_TRUE(subject++ == 5);
  EXPECT_TRUE(subject == 6);
  EXPECT_TRUE(subject-- == 6);
  EXPECT_TRUE(subject == 5);

  // can't steal reference
  auto what = ++subject;
  EXPECT_TRUE(what == 6);
  EXPECT_TRUE(++what == 7);
  EXPECT_TRUE(subject == 6);
  EXPECT_TRUE(--subject == 5);

  // reassignment
  subject = 1;
  EXPECT_TRUE(subject == 1);

  // logical
  EXPECT_TRUE(1 && subject);
  EXPECT_FALSE(subject && 0);
  EXPECT_TRUE(0 || subject);
  EXPECT_TRUE(subject || 0);
  subject = 0;
  EXPECT_FALSE(subject || 0);
  EXPECT_FALSE(0 || subject);

  // operator assignment
  subject = 5;
  int another = 5;
  EXPECT_TRUE((subject += 1) == (another += 1));
  EXPECT_TRUE(subject == 6);
  EXPECT_TRUE((subject -= 1) == (another -= 1));
  EXPECT_TRUE(subject == 5);
}

FIXTURE(model_cols) {
  auto primary_col = make_col("primary_col", &model_a_t::id);
  auto foreign_col = make_col("foreign_col", &model_b_t::foreign, &model_a_t::id);
  auto regular_col = make_col("regular_col", &model_b_t::name);
  EXPECT_EQ(primary_col->is_primary_key(), true);
  EXPECT_EQ(primary_col->is_foreign_key(), false);
  EXPECT_EQ(foreign_col->is_primary_key(), false);
  EXPECT_EQ(foreign_col->is_foreign_key(), true);
  EXPECT_EQ(regular_col->is_primary_key(), false);
  EXPECT_EQ(regular_col->is_foreign_key(), false);
}

FIXTURE(table_converts_member_to_pointer_to_string) {
  EXPECT_TRUE(model_b_t::table.get_col_name(&model_b_t::foreign) == "foreign");
  EXPECT_TRUE(model_b_t::table.get_col_name(&model_b_t::name) == "name");
  EXPECT_TRUE(model_b_t::table.get_col_name(&model_b_t::id) == "id");
}

FIXTURE(create_models_and_assign) {
  model_b_t b;
  b.id = "id_b";
  b.name = "name";
  b.foreign = "id_a";
  EXPECT_TRUE(b.id == std::string("id_b"));
  EXPECT_TRUE(b.name == "name");
  EXPECT_TRUE(b.foreign == "id_a");
  EXPECT_TRUE(b.foreign->id.get().empty());
  EXPECT_TRUE(b.foreign->id->empty());
}

FIXTURE(test_assignment) {
  model_a_t instance_a;
  instance_a.id = "young self";
  instance_a.age = 23;
  std::stringstream ss1;
  model_a_t::table.write(&instance_a, ss1);
  EXPECT_TRUE(instance_a.id == "young self");
  EXPECT_TRUE(instance_a.age == 23);

  model_b_t instance_b;
  instance_b.id = "b model";
  instance_b.foreign = instance_a;
  instance_b.name = "cool";
  std::stringstream ss2;
  model_b_t::table.write(&instance_b, ss2);

  EXPECT_TRUE(instance_b.id == "b model");
  EXPECT_TRUE(instance_b.name == "cool");
  EXPECT_TRUE(instance_b.foreign == "young self");
  EXPECT_TRUE(instance_b.foreign->id == "young self");
  EXPECT_TRUE(instance_b.foreign->age == 23);
}

FIXTURE(pqxx_adapter_t) {
  model_a_t instance_a;
  instance_a.id = "young self";
  instance_a.age = 23;
  std::stringstream ss1;
  model_a_t::table.write(&instance_a, ss1);
  EXPECT_TRUE(instance_a.id == "young self");
  EXPECT_TRUE(instance_a.age == 23);

  model_b_t instance_b;
  instance_b.id = "b model";
  instance_b.foreign = instance_a;
  instance_b.name = "cool";
  std::stringstream ss2;
  model_b_t::table.write(&instance_b, ss2);
}


const char *create_tables = R"(
create table model_z_t (
  id      serial primary key,
  age     int default 1 not null
);

create table model_a_t (
  id          uuid primary key default uuid_generate_v4() not null,
  opacity     numeric (1, 1) default 1.0 not null,
  age         int default 1 not null,
  huge_num    bigint not null,
  "foreign"     int not null,
  foreign key ("foreign") references model_z_t(id)
);

insert into model_z_t (id, age) values (1, 2);
insert into model_a_t (opacity, age, huge_num, "foreign") values (0.1, 3, 6444, 1);
)";

const char *drop_tables = R"(
delete from model_a_t where id is not null;
delete from model_z_t where id is not null;
drop table model_a_t;
drop table model_z_t;
)";

FIXTURE(look_at_session_stuff) {
  EXPECT_OK([]() {
    auto c = hoc::db::super_user_connection();
    pqxx::work w(*c);
    w.exec(create_tables);

    w.exec("insert into model_z_t (id, age) values (2, 2);");
    w.exec("insert into model_z_t (id, age) values (3, 2);");

    for (int i = 0; i < 20; ++i) {
      w.exec("insert into model_a_t (opacity, age, huge_num, \"foreign\") values (0.1, 3, 6444, 1);");
    }

    for (int i = 0; i < 20; ++i) {
      w.exec("insert into model_a_t (opacity, age, huge_num, \"foreign\") values (0.1, 3, 6444, 2);");
    }

    for (int i = 0; i < 20; ++i) {
      w.exec("insert into model_a_t (opacity, age, huge_num, \"foreign\") values (0.1, 3, 6444, 3);");
    }


    w.commit();
  });

  EXPECT_OK([]() {
    auto c = hoc::db::super_user_connection();
    pqxx::work w(*c);
    auto res = w.exec(
      "select * from model_a_t "
      "inner join model_z_t on model_z_t.id = model_a_t.foreign"
    );

    for (int i = 0; i < int(res.size()); ++i) {
      auto record = factory_t<model_a_t, std::string>::get()->require(res[i]);
      EXPECT_TRUE(record->id == res[i].at("id").as<std::string>());
      EXPECT_TRUE(record->opacity == 0.1);
      EXPECT_TRUE(record->age == 3);
      EXPECT_TRUE(record->huge_num == 6444);
      EXPECT_TRUE(record->foreign == 1 || record->foreign == 2 || record->foreign == 3);
    }
  });

  EXPECT_OK([]() {
    auto c = hoc::db::super_user_connection();
    pqxx::work w(*c);
    w.exec(drop_tables);
    w.commit();
  });
}

int main(int argc, char *argv[]) {
  return test_main(argc, argv);
}