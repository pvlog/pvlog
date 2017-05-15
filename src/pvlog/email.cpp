#include "email.h"

#include <Poco/Net/MailMessage.h>
#include <Poco/Net/MailRecipient.h>
#include <Poco/Net/SecureStreamSocket.h>
#include <Poco/Net/SMTPClientSession.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/AutoPtr.h>

#include "util/log.h"
#include "util/pvlogexception.h"

using namespace Poco::Net;
using namespace Poco;

Email::Email(const std::string& smtpServer, int port,
		const std::string& user, const std::string& password)
{
	SharedPtr<InvalidCertificateHandler> ptrHandler =
			new AcceptCertificateHandler(false);

	Context::Ptr ptrContext = new Context(Context::CLIENT_USE, "", "", "",
			Context::VERIFY_RELAXED, 9, true,
			"ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");

	SSLManager::instance().initializeClient(0, ptrHandler, ptrContext);

	LOG(Debug) << "Establishing connection to " << smtpServer << " " << port;
	SocketAddress sa(smtpServer, port);
	SecureStreamSocket socket(sa);
	session = std::unique_ptr<SMTPClientSession>(new SMTPClientSession(socket));
	LOG(Debug) << "Established connection to " << smtpServer << " " << port;

	LOG(Info) << "Login: " << user;
	try {
		session->login(SMTPClientSession::AUTH_LOGIN, user, password);
	} catch (const Exception &e) {
		LOG(Error) << "Login error: " << e.displayText();
		session->close();
		PVLOG_EXCEPT(e.displayText());
	}

	LOG(Info) << "Logged in: " << user;
}

Email::~Email() {
	session->close();
}

void Email::send(const std::string& from, const std::string& to, const std::string& subject, const std::string& content) {
	MailMessage message;
	message.setSender(from);
	message.addRecipient(MailRecipient(MailRecipient::PRIMARY_RECIPIENT, to));
	message.setSubject(subject);
	message.setContentType("text/html; charset=UTF-8");
	std::string pre =
			"<html>\n"
			"<body>\n"
			"<pre style=\"font: monospace\">\n";
	std::string footer =
			"</pre>\n"
			"</body>\n"
			"</html>\n";

	message.setContent(pre + content + footer, MailMessage::ENCODING_8BIT);

	try {
		session->sendMessage(message);
	} catch (const Exception &e) {
		LOG(Error) << "Send message error: " << e.displayText();
		PVLOG_EXCEPT(e.displayText());
	}
}
