/*
 * This file is part of Pvlog.
 *
 * Copyright (C) 2017 pvlogdev@gmail.com
 *
 * Pvlog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pvlog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pvlog.  If not, see <http://www.gnu.org/licenses/>.
 */

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
