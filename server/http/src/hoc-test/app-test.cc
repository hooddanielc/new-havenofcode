#include <hoc/lick.h>
#include <hoc/app.h>

using namespace hoc;
using namespace std;

FIXTURE(initial_state) {
  auto &app = app_t::get().clear();
  EXPECT_EQ(app.get_request_events().size(), 0u);
}

FIXTURE(clear_event_state) {
  auto &app = app_t::get().clear();

  app.on_request([](const req_t &) {});

  app.on_start([]() {});
  app.on_exit([]() {});

  EXPECT_EQ(app.get_request_events().size(), 1u);
  EXPECT_EQ(app.get_start_events().size(), 1u);
  EXPECT_EQ(app.get_exit_events().size(), 1u);

  app.clear();

  EXPECT_EQ(app.get_request_events().size(), 0u);
  EXPECT_EQ(app.get_start_events().size(), 0u);
  EXPECT_EQ(app.get_exit_events().size(), 0u);
}

FIXTURE(events_call_back) {
  auto &app = app_t::get().clear();

  int request_val = 0;
  int start_val = 0;
  int exit_val = 0;

  app.on_request([&request_val](const req_t &) {
    request_val += 1;
  });

  app.on_start([&start_val]() {
    start_val += 1;
  });

  app.on_exit([&exit_val]() {
    exit_val += 1;
  });

  req_t::header_list_t headers;
  app.emit_request(req_t(headers, headers));
  app.emit_start();
  app.emit_exit();

  EXPECT_EQ(request_val, 1);
  EXPECT_EQ(exit_val, 1);
  EXPECT_EQ(start_val, 1);
}

// FIXTURE(is_equal) {
//   AddNumbers a;
//   EXPECT_EQ(a.getA(), 0) << "it does not initialize with 0";
// }

// FIXTURE(not_equal) {
//   AddNumbers a;
//   EXPECT_NE(a.getA(), 1) << "0 != 1";
// }

// FIXTURE(falsy_values) {
//   AddNumbers a;
//   EXPECT_FALSE(a.getSum() < 0) << "0 + 0 is not negative";
// }

// FIXTURE(true_values) {
//   AddNumbers a;
//   EXPECT_TRUE(a.getSum() == 0) << "0 + 0 == 0";
// }

// FIXTURE(greater_than) {
//   AddNumbers a;
//   EXPECT_GT(a.getA(), -1) << "0 > -1";
// }

// FIXTURE(lesser_than) {
//   AddNumbers a;
//   EXPECT_LT(a.getA(), 1) << "0 < 1";
// }

// FIXTURE(greater_than_or_equal) {
//   AddNumbers a;
//   EXPECT_GE(a.getA(), 0) << "0 >= 0";
//   EXPECT_GE(a.getA(), -1) << "0 >= -1";
// }

// FIXTURE(lesser_than_or_equal) {
//   AddNumbers a;
//   EXPECT_LE(a.getA(), 0) << "0 <= 0";
//   EXPECT_LE(a.getA(), 1) << "0 <= -1";
// }

int main(int argc, char *argv[]) {
  return dj::lick::main(argc, argv);
}