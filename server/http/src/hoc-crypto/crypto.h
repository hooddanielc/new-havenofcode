#pragma once

#include <iostream>
#include <stdexcept>
#include <fstream>
#include <gmpxx.h>

namespace hoc {
  namespace crypto {
    class rsa_crypto_key_t final {
      public:
        rsa_crypto_key_t(
          mpz_class k1_,
          mpz_class k2_
        ) : k1(k1_), k2(k2_) {};

        mpz_class get_k1();
        mpz_class get_k2();

      private:
        mpz_class k1;
        mpz_class k2;
    };

    class rsa_crypto_t final {
      public:
        rsa_crypto_t(mpz_class e_ = 65537) : p(0), q(0), e(e_) {
          init();
        };

        rsa_crypto_key_t get_public_key();
        rsa_crypto_key_t get_private_key();

      private:
        void init();
        mpz_class p;
        mpz_class q;
        mpz_class n;
        mpz_class phi;
        mpz_class d;
        mpz_class e;
    };
  }
}
