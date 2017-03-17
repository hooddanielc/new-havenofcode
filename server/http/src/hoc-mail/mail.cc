#include <hoc-mail/mail.h>

using namespace std;

namespace hoc {
namespace mail {

  certificate_list_t trust_all_certificates_t::trusted_certs;

  void trust_all_certificates_t::verify(certificate_chain_t chain, const vmime::string &hostname) {
    try {
      setX509TrustedCerts(trusted_certs);
      defaultCertificateVerifier::verify(chain, hostname);
    } catch (vmime::security::cert::certificateException &) {
      // certificate needs to be added to trusted list
      // accept it, and remember choice for later
      // Obtain subject's certificate
      certificate_t cert = chain->getAt(0);

      if (cert->getType() == "X.509") {
        trusted_certs.push_back(vmime::dynamicCast <vmime::security::cert::X509Certificate>(cert));
        setX509TrustedCerts(trusted_certs);
        defaultCertificateVerifier::verify(chain, hostname);
      }
    }
  }

  msg_t get_message(
    const std::string &from,
    const std::vector<std::string> &to_list,
    const std::vector<std::string> &bcc_list,
    const std::string &subject,
    const std::string &message
  ) {
    vmime::messageBuilder mb;
    vmime::addressList to;
    vmime::addressList bcc;

    for (size_t i = 0; i < to_list.size(); ++i) {
      to.appendAddress(vmime::make_shared<vmime::mailbox>(to_list[i]));
    }

    for (size_t i = 0; i < bcc_list.size(); ++i) {
      bcc.appendAddress(vmime::make_shared<vmime::mailbox>(bcc_list[i]));
    }

    mb.setExpeditor(vmime::mailbox(from));
    mb.setRecipients(to);
    mb.setBlindCopyRecipients(bcc);
    mb.setSubject(vmime::text(subject));
    mb.getTextPart()->setText(vmime::make_shared <vmime::stringContentHandler>(message));
    return mb.construct();
  }

  transport_t get_transporter(
    const std::string &username,
    const std::string &token
  ) {
    // Indicate that we want to use XOAUTH2 SASL mechanism
    vmime::security::sasl::SASLMechanismFactory::getInstance()->
        registerMechanism <vmime::security::sasl::XOAuth2SASLMechanism>("XOAUTH2");

    // Create a new session
    vmime::shared_ptr <vmime::net::session> sess = vmime::net::session::create();

    // Use a custom authenticator to force using XOAUTH2 mechanism
    vmime::shared_ptr <vmime::security::authenticator> xoauth2Auth =
        vmime::make_shared <vmime::security::sasl::XOAuth2SASLAuthenticator>
            (vmime::security::sasl::XOAuth2SASLAuthenticator::MODE_EXCLUSIVE);

    // Create a new SMTPS service to GMail
    vmime::shared_ptr <vmime::net::transport> tr = sess->getTransport(
      vmime::utility::url("smtps://smtp.gmail.com:465"), xoauth2Auth
    );

    tr->setCertificateVerifier(vmime::make_shared <trust_all_certificates_t>());
    tr->setProperty("options.need-authentication", true);
    tr->setProperty("auth.username", username);
    tr->setProperty("auth.accesstoken", token);

    return tr;
  }

  void send_message(msg_t &msg, transport_t &tr) {
    tr->connect();
    tr->send(msg);
    tr->disconnect();
  }

  void send_registration_email(const std::string &email, const std::string hash) {
    string username;

    for (size_t i = 0; i < email.size(); ++i) {
      char c = email.at(i);

      if (c == '@') {
        break;
      } else {
        username += c;
      }
    }

    string msg_str;
    msg_str.append(
      "Hi ").append(username).append(",\n\n"
      "Please click on the link below "
      "to verify your email and start living the dream.\n\n"
      "http://").append(env_t::get().host).append("/confirm-email/"
    );

    msg_str.append(hash).append(
      "\n\n"
      "Sincerely,\n"
      "The Haven of Code Team"
    );

    auto msg = mail::get_message(
      env_t::get().no_reply_email,
      vector<string>({ email }),
      vector<string>(),
      "Welcome Aboard",
      msg_str
    );

    db_t db;
    db.exec("BEGIN");
    auto db_res = db.exec("SELECT refresh_token FROM app_token WHERE id = 'no_reply_email'");
    db.exec("END");

    // send an http request to get a new 
    // access token to send email with

    std::string get_token_url("https://www.googleapis.com/oauth2/v4/token");
    std::string get_token_args("client_id=");

    get_token_args.append(env_t::get().google_api_client_id)
      .append("&client_secret=").append(env_t::get().google_api_client_secret)
      .append("&refresh_token=").append(db_res[0][0].data())
      .append("&grant_type=refresh_token");

    request_t get_token_request;
    get_token_request.set_url(get_token_url.c_str());
    get_token_request.add_header("Expect:");
    get_token_request.add_header("Transfer-Encoding: chunked");
    string get_token_response;

    get_token_request.on_data([&get_token_response](const char* data, size_t len) {
      get_token_response.append(string{ data, len });
    });

    get_token_request.send(get_token_args);
    auto json = dj::json_t::from_string(get_token_response.c_str());

    if (!json.contains("access_token")) {
      throw runtime_error("could not get access token");
    }

    auto transport = mail::get_transporter(
      env_t::get().no_reply_email,
      json["access_token"].as<string>()
    );

    mail::send_message(msg, transport);
  }

} // mail
} // hoc
