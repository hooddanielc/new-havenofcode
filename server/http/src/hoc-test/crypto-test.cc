#include <lick/lick.h>
#include <hoc-crypto/crypto.h>
#include <hoc-crypto/sha-224.h>
#include <hoc-crypto/sha-256.h>
#include <hoc-crypto/sha-384.h>
#include <hoc-crypto/sha-512.h>
#include <hoc-crypto/rsa.h>

using namespace std;
using namespace hoc;
using namespace crypto;

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

FIXTURE(sha256) {
  EXPECT_EQ(
    "0f78fcc486f5315418fbf095e71c0675ee07d318e5ac4d150050cd8e57966496",
    sha256("grape")
  );

  EXPECT_EQ(
    "226d88a80fd9f3b61d65f541eeb890dd16972e15f1ee8e37bd85bfbcc33c328e",
    sha256("Man on a wire!**##??ZKMXC")
  );

  EXPECT_EQ(
    "a2335b78d56be3a599bef6118294e0af67a9b58a1b09bb246e094b90ddda308e",
    sha256("!@#$%^&*()~")
  );
}

FIXTURE(sha224) {
  EXPECT_EQ(
    "571f3896fb694dc268b032d7940dabbfbcd7ee64c07f45c01c9e64db",
    sha224("grape")
  );

  EXPECT_EQ(
    "ffa7fa5727c808bf5add2171208f800d426e0637e51dd6f79385c7a7",
    sha224("Man on a wire!**##??ZKMXC")
  );

  EXPECT_EQ(
    "0c28005060ae56567f783cb4408a6029372cda2844452f5026ea90d8",
    sha224("!@#$%^&*()~")
  );
}

FIXTURE(sha384) {
  EXPECT_EQ(
    "c2dafc387656342580027e2dbbbc2afcc77df4294b2542a9"
    "83cf225735b88821302b9fa3c5948ba48b8dacd43da156d9",
    sha384("grape")
  );

  EXPECT_EQ(
    "e94efbfe21c33d9d6dbbedad98fd6fd2d3d741ff0b20fa2f"
    "155ef433671090fb32ec2f9a2fed162b91fd16eae1eac437",
    sha384("Man on a wire!**##??ZKMXC")
  );

  EXPECT_EQ(
    "bc6bbdbddeabfaac21330a73b1e4d14e4ca6153ba0fc360e"
    "c1b2395749c1ccfcab15621a129919aa668d5b719957618b",
    sha384("!@#$%^&*()~")
  );
}

FIXTURE(sha512) {
  EXPECT_EQ(
    "9375d1abdb644a01955bccad12e2f5c2bd8a3e226187e548d99c559a99461453"
    "b980123746753d07c169c22a5d9cc75cb158f0e8d8c0e713559775b5e1391fc4",
    sha512("grape")
  );

  EXPECT_EQ(
    "eb659b6a6592d36d43ce167f1c0d450d86831ca6888840d82a134fc22637e825"
    "29a501479bf20cd3f2600f223eb02813962f5396f9ce3de59ee3013ca31daed8",
    sha512("Man on a wire!**##??ZKMXC")
  );

  EXPECT_EQ(
    "a3850f8637d110be3d5541fc0fae0d7bd09f111b54a7414948cf7ec531fefdda"
    "9a8e40df0d5cf886dd46ac6c8fafb126af222ba1b26fe89955a9f38f49a015c1",
    sha512("!@#$%^&*()~")
  );
}

FIXTURE(use_entropy) {
  auto rnd = use_entropy(64);
  ostringstream os;
  os << rnd;
  EXPECT_EQ(rnd.get_size() > 0, true);
  EXPECT_EQ(os.str().size() > 0, true);
}

FIXTURE(rsa_test) {
  // The number of bytes to read from /dev/urandom to seed the prng.
  static const size_t entropy_size = 1024;
  // The size of the keys.
  static const mp_bitcnt_t bits = 2048;
  // Make a key pair.
  prng_t prng(use_entropy(entropy_size));
  auto pair = rsa::make_pair(prng, bits);
  const auto &pub = pair.first;
  const auto &pvt = pair.second;
  EXPECT_EQ(pub.get_size() > 0, true);
  EXPECT_EQ(pub.get_size() > 0, true);

  // Encrypt-decrypt a number.
  const big_t m = 65;
  auto c = pub(m);
  auto u = pvt(c);
  EXPECT_EQ(m.get_size() > 0, true);
  EXPECT_EQ(c.get_size() > 0, true);
  EXPECT_EQ(u.get_size() > 0, true);
  // Encrypt-decrypt a block of bytes.
  auto *b1 = static_cast<char *>(alloca(pub.get_size() / 2));
  strcpy(b1, "hello doctor name");
  auto *b2 = static_cast<char *>(alloca(pub.get_size()));
  pub.encrypt_block(b2, b1);
  auto *b3 = static_cast<char *>(alloca(pub.get_size() / 2));
  pvt.decrypt_block(b3, b2);
  ostringstream os;
  os << b3;
  EXPECT_EQ(os.str(), "hello doctor name");

  // Encrypt-decrypt a string.
  auto ciphertext = pub.encrypt(prng, "This is a test.");
  auto plaintext = pvt.decrypt(ciphertext);
  os.str("");
  os << plaintext;
  EXPECT_EQ(os.str(), "This is a test.");
}

int main(int argc, char *argv[]) {
  return dj::lick::main(argc, argv);
}
