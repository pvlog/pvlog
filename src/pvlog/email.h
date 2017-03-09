#ifndef EMAIL_H
#define EMAIL_H

#include <memory>

namespace Poco {
namespace Net {
class SMTPClientSession;
}
}

class Email {
	std::unique_ptr<Poco::Net::SMTPClientSession> session;

public:
	Email(const std::string& smptServer, int port, const std::string& username,
			const std::string& password);

	~Email();

	void send(const std::string& from, const std::string& to,
			const std::string& subject, const std::string& content);
};

#endif //#ifndef EMAIL_H
