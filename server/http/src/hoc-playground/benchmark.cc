#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <mutex>
#include <hoc/request.h>

using namespace std;
using namespace std::chrono;
using namespace hoc;

std::mutex mtx;
int dropped = 0;
int success = 0;

void do_get_requests() {
  int localdrop = 0;
  int localsuccess = 0;

  for (int i = 0; i < 1000; ++i) {
    bool error = false;

    try {
      auto t1 = high_resolution_clock::now();
      request_t req;
      req.set_url("http://localhost/api/echo");
      req.on_data([&error](const char *, size_t) {});
      req.send();
      auto t2 = high_resolution_clock::now();
      auto duration = duration_cast<std::chrono::milliseconds>(t2 - t1).count();
      ++localsuccess;
    } catch (const std::exception &e) {
      ++localdrop;
      error = true;
      std::cout << e.what() << std::endl;
    }
  }

  std::lock_guard<std::mutex> guard(mtx);
  dropped += localdrop;
  success += localsuccess;
}

int main(int, char*[]) {
  vector<std::thread> threads;

  for (int i = 0; i < 12; ++i) {
    threads.emplace_back(do_get_requests);
  }

  for(int i = 0; i < 12; ++i) {
    threads[i].join();
  }

  std::cout << "dropped: " << dropped << std::endl;
  std::cout << "success: " << success << std::endl;

  return 0;
}
