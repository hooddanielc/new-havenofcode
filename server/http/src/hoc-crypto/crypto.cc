#include <hoc-crypto/crypto.h>

using namespace std;

namespace hoc {
  namespace crypto {

    mpz_class another_rnd(unsigned bits) {
      static constexpr size_t buffer_size = 64; // number of random bytes to seed with
      char buffer[buffer_size];
      {
        ifstream urandom("/dev/urandom", ios::in|ios::binary);
        if (!urandom) {
          throw std::runtime_error { "borked" };
        }
        urandom.read(buffer, buffer_size);
      }
      mpz_class seed, rnd;
      mpz_import(seed.get_mpz_t(), buffer_size, 1, 1, 0, 0, buffer);
      gmp_randstate_t state;
      gmp_randinit_default(state);
      gmp_randseed(state, seed.get_mpz_t());
      mpz_urandomb(rnd.get_mpz_t(), state, bits);
      gmp_randclear(state);
      return rnd;
    }

    mpz_class rsa_crypto_key_t::get_k1() {
      return k1;
    }

    mpz_class rsa_crypto_key_t::get_k2() {
      return k2;
    }

    mpz_class get_huge_phi(const mpz_class &p, const mpz_class &q) {
      return (p - 1) * (q - 1);
    }

    rsa_crypto_key_t rsa_crypto_t::get_public_key() {
      return rsa_crypto_key_t(e, n);
    }

    rsa_crypto_key_t rsa_crypto_t::get_private_key() {
      return rsa_crypto_key_t(d, n);
    }

    void rsa_crypto_t::init() {
      mpz_class tmp1;
      mpz_class tmp2;

      static constexpr unsigned bits = 64;

      // choose p
      if (p == 0) {
        p = another_rnd(bits);
        mpz_setbit(p.get_mpz_t(), bits - 1);
        mpz_nextprime(p.get_mpz_t(), p.get_mpz_t());
        mpz_mod(tmp1.get_mpz_t(), p.get_mpz_t(), e.get_mpz_t());

        while(!mpz_cmp_ui(tmp1.get_mpz_t(), 1)) {
         mpz_nextprime(p.get_mpz_t(), p.get_mpz_t());
         mpz_mod(tmp1.get_mpz_t(), p.get_mpz_t(), e.get_mpz_t());
        }
      }

      // choose q
      if (q == 0) {
        do {
          q = another_rnd(bits);
          mpz_setbit(q.get_mpz_t(), bits - 1);
          mpz_nextprime(q.get_mpz_t(), q.get_mpz_t());
          mpz_mod(tmp1.get_mpz_t(), q.get_mpz_t(), e.get_mpz_t());

          while(!mpz_cmp_ui(tmp1.get_mpz_t(), 1)) {
            mpz_nextprime(q.get_mpz_t(), q.get_mpz_t());
            mpz_mod(tmp1.get_mpz_t(), q.get_mpz_t(), e.get_mpz_t());
          }
        } while(mpz_cmp(p.get_mpz_t(), q.get_mpz_t()) == 0);
      }

      // calculate n
      mpz_mul(n.get_mpz_t(), p.get_mpz_t(), q.get_mpz_t());

      // calculate phi
      mpz_sub_ui(tmp1.get_mpz_t(), p.get_mpz_t(), 1);
      mpz_sub_ui(tmp2.get_mpz_t(), q.get_mpz_t(), 1);
      mpz_mul(phi.get_mpz_t(), tmp1.get_mpz_t(), tmp2.get_mpz_t());

      // calculate d
      if (mpz_invert(d.get_mpz_t(), e.get_mpz_t(), phi.get_mpz_t()) == 0) {
        throw std::runtime_error("Finding modulus inverse failed");
      }
    }
  }
}
