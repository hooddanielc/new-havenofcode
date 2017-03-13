// clang -Weverything -Wno-c++98-compat --std=c++14 -ocrypto crypto.cc -lstdc++ -lgmp -lm

#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <gmp.h>

namespace hoc {
namespace crypto {

class big_t;
class prng_t;

std::istream &operator>>(std::istream &strm, big_t &big);

std::ostream &operator<<(std::ostream &strm, const big_t &big);

big_t lcm(const big_t &a, const big_t &b) noexcept;

void open_ifstream(std::ifstream &strm, const std::string &path);

big_t powmod(const big_t &b, const big_t &e, const big_t &n);

big_t random_prime(prng_t &prng, mp_bitcnt_t bits);

void swap(big_t &lhs, big_t &rhs) noexcept;

bool try_invert(big_t &out, const big_t &e, const big_t &n) noexcept;

big_t use_entropy(size_t size);

class big_t final {
public:

  big_t() noexcept {
    mpz_init(z);
  }

  big_t(big_t &&that) noexcept {
    mpz_init(z);
    swap(*this, that);
  }

  big_t(const big_t &that) noexcept {
    mpz_init_set(z, that.z);
  }

  big_t(unsigned long that) noexcept {
    mpz_init_set_ui(z, that);
  }

  ~big_t() {
    mpz_clear(z);
  }

  big_t &operator=(big_t &&that) noexcept {
    if (this != &that) {
      reset();
      swap(*this, that);
    }
    return *this;
  }

  big_t &operator=(const big_t &that) noexcept {
    if (this != &that) {
      mpz_set(z, that.z);
    }
    return *this;
  }

  big_t operator-(unsigned int that) const noexcept {
    big_t result;
    mpz_sub_ui(result.z, z, that);
    return result;
  }

  big_t operator*(const big_t &that) const noexcept {
    big_t result;
    mpz_mul(result.z, z, that.z);
    return result;
  }

  bool operator==(const big_t &that) const noexcept {
    return compare(that) == 0;
  }

  int compare(const big_t &that) const noexcept {
    return mpz_cmp(z, that.z);
  }

  size_t export_bytes(char *bytes) const noexcept {
    size_t size;
    mpz_export(bytes, &size, order, sizeof(char), endian, nails, z);
    return size;
  }

  big_t get_next_prime() const noexcept {
    big_t result;
    mpz_nextprime(result.z, z);
    return result;
  }

  size_t get_size() const noexcept {
    return (mpz_sizeinbase(z, 2) + 7) / 8;
  }

  big_t &import_bytes(const char *bytes, size_t size) noexcept {
    mpz_import(z, size, order, sizeof(char), endian, nails, bytes);
    return *this;
  }

  void read(std::istream &strm, int base = 0) {
    std::string digits;
    strm >> digits;
    big_t result;
    if (mpz_set_str(result.z, digits.c_str(), base) < 0) {
      std::ostringstream msg;
      msg << std::quoted(digits) << " is not a valid integer";
      if (base) {
        msg << " in base-" << base;
      }
      throw std::runtime_error { msg.str() };
    }
    swap(*this, result);
  }

  big_t &reset() noexcept {
    mpz_clear(z);
    mpz_init(z);
    return *this;
  }

  big_t &set_bit(mp_bitcnt_t bit) noexcept {
    mpz_setbit(z, bit);
    return *this;
  }

  void write(std::ostream &strm, int base = 10) const noexcept {
    auto size = mpz_sizeinbase(z, base) + 2;
    auto *buf = static_cast<char *>(alloca(size));
    mpz_get_str(buf, 10, z);
    strm << buf;
  }

  friend std::istream &operator>>(std::istream &strm, big_t &big);

  friend std::ostream &operator<<(std::ostream &strm, const big_t &big);

  friend big_t lcm(const big_t &a, const big_t &b) noexcept;

  friend big_t powmod(const big_t &b, const big_t &e, const big_t &n);

  friend void swap(big_t &lhs, big_t &rhs) noexcept;

  friend bool try_invert(big_t &out, const big_t &e, const big_t &n) noexcept;

  friend big_t use_entropy(size_t size);

private:

  static constexpr int order = -1, endian = -1, nails = 0;

  mpz_t z;

  friend class prng_t;

};  // big_t

class prng_t final {
public:

  prng_t() noexcept {
    gmp_randinit_default(state);
  }

  prng_t(prng_t &&) = delete;

  prng_t(const prng_t &that) noexcept {
    gmp_randinit_set(state, that.state);
  }

  explicit prng_t(unsigned long that)
      : prng_t() {
    set_seed(that);
  }

  explicit prng_t(const big_t &that)
      : prng_t() {
    set_seed(that);
  }

  ~prng_t() {
    gmp_randclear(state);
  }

  prng_t &operator=(prng_t &&) = delete;

  prng_t &operator=(const prng_t &) = delete;

  big_t operator()(mp_bitcnt_t bits) noexcept {
    big_t result;
    mpz_urandomb(result.z, state, bits);
    return result;
  }

  void set_seed(unsigned long seed) noexcept {
    gmp_randseed_ui(state, seed);
  }

  void set_seed(const big_t &seed) noexcept {
    gmp_randseed(state, seed.z);
  }

private:

  gmp_randstate_t state;

};  // prng_t

std::istream &operator>>(std::istream &strm, big_t &big) {
  big.read(strm);
  return strm;
}

std::ostream &operator<<(std::ostream &strm, const big_t &big) {
  big.write(strm);
  return strm;
}

big_t lcm(const big_t &a, const big_t &b) noexcept {
  big_t result;
  mpz_lcm(result.z, a.z, b.z);
  return result;
}

void open_ifstream(std::ifstream &strm, const std::string &path) {
  strm.open(path);
  if (!strm) {
    std::ostringstream msg;
    msg << "could not open " << std::quoted(path) << " for reading";
    throw std::runtime_error { msg.str() };
  }
  strm.exceptions(std::ios::badbit | std::ios::failbit);
}

big_t powmod(const big_t &b, const big_t &e, const big_t &n) {
  big_t result;
  mpz_powm(result.z, b.z, e.z, n.z);
  return result;
}

big_t random_prime(prng_t &prng, mp_bitcnt_t bits) {
  auto rnd = prng(bits);
  rnd.set_bit(bits - 1);
  return rnd.get_next_prime();
}

void swap(big_t &lhs, big_t &rhs) noexcept {
  mpz_swap(lhs.z, rhs.z);
}

bool try_invert(big_t &out, const big_t &e, const big_t &n) noexcept {
  return mpz_invert(out.z, e.z, n.z) != 0;
}

big_t use_entropy(size_t size) {
  auto *bytes = static_cast<char *>(alloca(size));
  {
    std::ifstream strm;
    open_ifstream(strm, "/dev/urandom");
    strm.read(bytes, static_cast<std::streamsize>(size));
  }
  big_t result;
  result.import_bytes(bytes, size);
  return result;
}

namespace rsa {

class key_t;

std::ostream &operator<<(std::ostream &strm, const key_t &key);

mp_bitcnt_t get_min_bits(unsigned long e);

std::pair<key_t, key_t> make_pair(
    prng_t &prng, mp_bitcnt_t bits = 0, unsigned long e = 65537);

class key_t final {
public:

  key_t(key_t &&that) noexcept = default;

  key_t(const key_t &that) noexcept = default;

  key_t &operator=(key_t &&that) noexcept = default;

  key_t &operator=(const key_t &that) noexcept = default;

  big_t operator()(const big_t &a) const noexcept {
    return powmod(a, exp, mod);
  }

  std::string decrypt(const std::vector<char> &ciphertext) const {
    // Set up to read the ciphertext.
    const auto *csr = ciphertext.data();
    size_t size = ciphertext.size();
    // The ciphertext must be evenly divisible into input blocks, each the
    // size of our key.
    auto in_size = get_size();
    if (size % in_size) {
      std::ostringstream msg;
      msg
          << "ciphertext size (" << size
          << " bytes) is not an even multiple of the block size ("
          << in_size << " bytes)";
      throw std::runtime_error { msg.str() };
    }
    size_t block_cnt = size / in_size;
    // Each output block is half the size of our key.
    auto out_size = in_size / 2;
    // The first block is the initialization vector (IV).  This contains the
    // length of the plaintext, as well as a lot of random noise.  Decrypt it
    // into a separate workspace.
    auto *iv = static_cast<char *>(alloca(out_size));
    decrypt_block(iv, csr);
    auto *keep = csr;
    csr += in_size;
    // Verify that the length of the plaintext is consistent with the length
    // of the ciphertext.
    size_t final_size = *reinterpret_cast<size_t *>(iv);
    if (((final_size + out_size - 1) / out_size) + 1 != block_cnt) {
      throw std::runtime_error { "oopth" };
    }
    // Allocate a workspace for the plaintext.
    std::vector<char> plaintext((block_cnt - 1) * out_size);
    auto *out = plaintext.data();
    // Each block after the IV contains part of the message.
    for (size_t block_idx = 1; block_idx < block_cnt; ++block_idx) {
      // Decrypt the block.
      decrypt_block(out, csr);
      // XOR the block with the previous block of ciphertext to undo the
      // cipher block chaning (CBC) obfuscation done at encryption time.
      xor_block(out, keep);
      // Advance the output cursor and keep a pointer to this cipher text for
      // use in the next CBC XOR.
      out += out_size;
      keep = csr;
      csr += in_size;
    }  // for
    // Construct the return string from the plaintext buffer.
    return std::string { plaintext.data(), final_size };
  }

  void decrypt_block(char *out, const char *in) const noexcept {
    auto size = get_size();
    big_t a;
    a.import_bytes(in, size);
    (*this)(a);
    auto actl = a.export_bytes(out);
    size /= 2;
    for (; actl < size; ++actl) {
      out[actl] = '\0';
    }
  }

  std::vector<char> encrypt(prng_t &prng, const std::string &plaintext) const {
    // Set up to read the plaintext.
    const char *csr = plaintext.c_str();
    size_t size = plaintext.size();
    // The outgoing (ciphertext) blocks are each the full size of the key.
    auto out_size = get_size();
    // The incoming (plaintext) blocks are each half the size of the key.
    auto in_size = out_size / 2;
    // We'll produce exactly enough blocks to hold the message, plus one extra
    // block for the initialization vector.  (See below.)
    auto block_cnt = ((size + in_size - 1) / in_size) + 1;
    // The last block may be smaller than the others.
    auto last_size = in_size - (size % in_size);
    // Make a space in which to work on plaintext.
    auto *in = static_cast<char *>(alloca(in_size));
    // Make a space in which to collect ciphertext as we produce it.
    std::vector<char> ciphertext(out_size * block_cnt);
    auto *out = ciphertext.data();
    // The first block is the initialization vector (IV).  It contains the
    // length of the plaintext, plus random bytes.  The randomness makes
    // sure a particular plaintext always encrypts to a different ciphertext.
    prng(in_size * 8).export_bytes(in);
    *reinterpret_cast<size_t *>(in) = size;
    // Encrypt the IV and store it as part of the ciphertext.
    encrypt_block(out, in);
    auto *keep = out;
    out += out_size;
    // Each subsequent block contains part of the message.
    for (size_t block_idx = 1; block_idx < block_cnt; ++block_idx) {
      // Copy plaintext to the workspace.
      if (block_idx < block_cnt - 1) {
        // This isn't the last block, so it's a full block of input.
        memcpy(in, csr, in_size);
        csr += in_size;
      } else {
        // This is the last block, so it may be smaller than the others.
        // Pad it out with zeros.
        memcpy(in, csr, last_size);
        memset(in + last_size, 0, in_size - last_size);
      }
      // XOR the plaintext block with the previous encrypted block.  This is
      // called cipher block chaining (CBC) and it helps to disguise patterns
      // in the plaintext.
      xor_block(in, keep);
      // Encrypt the mangled plaintext and store it as part of the ciphertext.
      encrypt_block(out, in);
      keep = out;
      out += out_size;
    }  // for
    return ciphertext;
  }

  void encrypt_block(char *out, const char *in) const noexcept {
    auto size = get_size();
    big_t a;
    a.import_bytes(in, size / 2);
    (*this)(a);
    auto actl = a.export_bytes(out);
    for (; actl < size; ++actl) {
      out[actl] = '\0';
    }
  }

  size_t get_size() const noexcept {
    return mod.get_size();
  }

  friend std::ostream &operator<<(std::ostream &strm, const key_t &key);

  friend std::pair<key_t, key_t> make_pair(
      prng_t &prng, mp_bitcnt_t bits, unsigned long e);

private:

  key_t(const big_t &exp_, const big_t &mod_) noexcept
      : exp(exp_), mod(mod_) {}

  void xor_block(char *plain, const char *cipher) const noexcept {
    size_t plain_size = get_size() / 2;
    for (size_t i = 0; i < plain_size; ++i) {
      plain[i] ^= cipher[i];
    }
  }

  big_t exp, mod;

};  // key_t

std::ostream &operator<<(std::ostream &strm, const key_t &key) {
  return strm << "{ exp: " << key.exp << ", mod: " << key.mod << " }";
}

mp_bitcnt_t get_min_bits(unsigned long e) {
  return static_cast<mp_bitcnt_t>(ceil(log2(e))) * 2;
}

std::pair<key_t, key_t> make_pair(
    prng_t &prng, mp_bitcnt_t bits, unsigned long e) {
  bits = std::max(bits, get_min_bits(e));
  auto half_bits = bits / 2;
  big_t d, n;
  for (;;) {
    auto p = random_prime(prng, half_bits);
    auto q = random_prime(prng, half_bits);
    if (p == q) {
      continue;
    }
    auto tot = lcm(p - 1, q - 1);
    if (try_invert(d, e, tot)) {
      n = p * q;
      break;
    }
  }  // for
  return { key_t { e, n }, key_t { d, n } };
}

} // rsa
} // crypto
} // hoc

/****************************************************

// example program

#include <iostream>

using namespace hoc;
using namespace crypto;

int main(int, char *[]) {
  // The number of bytes to read from /dev/urandom to seed the prng.
  static const size_t entropy_size = 1024;
  // The size of the keys.
  static const mp_bitcnt_t bits = 2048;
  // Make a key pair.
  prng_t prng(use_entropy(entropy_size));
  auto pair = rsa::make_pair(prng, bits);
  const auto &pub = pair.first;
  const auto &pvt = pair.second;
  std::cout << "pub=" << pub << std::endl;
  std::cout << "pvt=" << pvt << std::endl;
  // Encrypt-decrypt a number.
  const big_t m = 65;
  auto c = pub(m);
  auto u = pvt(c);
  std::cout << "m=" << m << std::endl;
  std::cout << "c=" << c << std::endl;
  std::cout << "u=" << u << std::endl;
  // Encrypt-decrypt a block of bytes.
  auto *b1 = static_cast<char *>(alloca(pub.get_size() / 2));
  strcpy(b1, "hello doctor name");
  auto *b2 = static_cast<char *>(alloca(pub.get_size()));
  pub.encrypt_block(b2, b1);
  auto *b3 = static_cast<char *>(alloca(pub.get_size() / 2));
  pvt.decrypt_block(b3, b2);
  std::cout << std::quoted(b3) << std::endl;
  // Encrypt-decrypt a string.
  auto ciphertext = pub.encrypt(prng, "This is a test.");
  auto plaintext = pvt.decrypt(ciphertext);
  std::cout << std::quoted(plaintext) << std::endl;
  return 0;
}

****************************************************/