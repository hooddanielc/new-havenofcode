#include <lick/lick.h>
#include <hoc-crypto/crypto.h>

using namespace std;
using namespace hoc;

FIXTURE(rsa_crypto_t_init) {
  crypto::rsa_crypto_t keys;
  auto pub = keys.get_public_key();
  auto pri = keys.get_private_key();
  mpz_class message(255);
  mpz_class encrypted;
  mpz_class decrypted;

  mpz_powm(encrypted.get_mpz_t(), message.get_mpz_t(), pub.get_k1().get_mpz_t(), pub.get_k2().get_mpz_t());
  mpz_powm(decrypted.get_mpz_t(), encrypted.get_mpz_t(), pri.get_k1().get_mpz_t(), pri.get_k2().get_mpz_t());
  EXPECT_EQ(message, decrypted);
  EXPECT_NE(encrypted, decrypted);
}

int main(int argc, char *argv[]) {
  return dj::lick::main(argc, argv);
}
