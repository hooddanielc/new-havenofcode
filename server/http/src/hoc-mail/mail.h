#pragma once

#include <vector>
#include <string>

// undefine nginx conflicting
// macros
#undef CRLF

#include <vmime/vmime.hpp>
#include <vmime/security/sasl/XOAuth2SASLMechanism.hpp>
#include <vmime/security/sasl/XOAuth2SASLAuthenticator.hpp>
#include <vmime/platforms/posix/posixHandler.hpp>
#include <curl/curl.h>

#include <hoc/env.h>
#include <hoc/app.h>
#include <hoc/request.h>
#include <hoc-db/db.h>

namespace hoc {
  namespace mail {
    using msg_t = vmime::shared_ptr <vmime::message>;
    using transport_t = vmime::shared_ptr<vmime::net::transport>;
    using default_cert_verifier_t = vmime::security::cert::defaultCertificateVerifier;
    using certificate_chain_t = vmime::shared_ptr<vmime::security::cert::certificateChain>;
    using certificate_t = vmime::shared_ptr<vmime::security::cert::certificate>;
    using certificate_list_t = std::vector<vmime::shared_ptr<vmime::security::cert::X509Certificate>>;

    class trust_all_certificates_t : public default_cert_verifier_t {
      public:
        void verify(certificate_chain_t chain, const vmime::string &hostname);
      private:
        static certificate_list_t trusted_certs;
    };

    msg_t get_message(
      const std::string &from,
      const std::vector<std::string> &to,
      const std::vector<std::string> &bcc,
      const std::string &subject,
      const std::string &message
    );

    transport_t get_transporter(
      const std::string &username,
      const std::string &token
    );

    void send_message(msg_t &msg, transport_t &tr);
    void send_registration_email(const std::string &email, const std::string hash);
  }
}