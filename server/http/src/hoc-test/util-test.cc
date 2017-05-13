#include <lick/lick.h>
#include <string>
#include <hoc/env.h>
#include <hoc/util.h>

using namespace std;
using namespace hoc;

FIXTURE(random_characters) {
  for (int i = 0; i < 100; ++i) {
    std::string chars(random_characters(50));
    EXPECT_EQ(chars.size(), size_t(50));
    for (auto it = chars.begin(); it != chars.end(); ++it) {
      EXPECT_TRUE(isalnum(*it));

      if (!isalnum(*it)) {
        return;
      }
    }
  }
}

FIXTURE(random_file_path) {
  std::string tmp_env(env_t::get().upload_tmp_path);

  for (int i = 0; i < 100; ++i) {
    auto path1 = random_tmp_path();
    auto path2 = random_tmp_path();
    EXPECT_EQ(path1.substr(0, tmp_env.size()), tmp_env);
    EXPECT_EQ(path2.substr(0, tmp_env.size()), tmp_env);
    auto random_part1 = path1.substr(tmp_env.size(), path1.size());
    auto random_part2 = path2.substr(tmp_env.size(), path2.size());
    EXPECT_NE(random_part1, random_part2);
    EXPECT_EQ(random_part1[0], '/');
    EXPECT_EQ(random_part2[0], '/');
  }
}

int main(int argc, char *argv[]) {
  return dj::lick::main(argc, argv);
}
