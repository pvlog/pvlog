#ifndef MAIL_NOTIFICATION_H
#define MAIL_NOTIFICATION_H

#include <string>

namespace odb {
	class database;
}

class EmailNotification {
public:
	EmailNotification(odb::database* db);

	void sendMessage(const std::string& message);
private:
	odb::database* db;
};

#endif //#ifndef MAIL_NOTIFICATION_H
