#include <lick/lick.h>
#include <aws/core/Aws.h>

using namespace std;

FIXTURE(aws_initializes) {
  Aws::SDKOptions options;
  Aws::InitAPI(options);
  Aws::ShutdownAPI(options);
}

int main(int argc, char *argv[]) {
  return dj::lick::main(argc, argv);
}
