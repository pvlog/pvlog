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
