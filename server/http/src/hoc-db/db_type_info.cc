#include <hoc-db/db_type_info.h>

using namespace std;

namespace hoc {
  void db_type_info_t::gen_types() {
    db_t db;
    db.exec("BEGIN");
    auto res = db.exec("SELECT oid, typname, typcategory FROM pg_type LIMIT 300");
    db.exec("END");

    for (int y = 0; y < res.rows(); ++y) {
      size_t oid_size = res[y][0].size();
      size_t name_size = res[y][1].size();
      size_t cat_size = res[y][2].size();

      string oid_label{ res[y][0].data(), oid_size };
      string name_label{ res[y][1].data(), name_size };
      string cat_label{ res[y][2].data(), cat_size };

      names[oid_label.c_str()] = name_label.c_str();
      categories[oid_label.c_str()] = cat_label.c_str();
    }
  }

  const char *name(int oid);
  const char category(int oid);
}
