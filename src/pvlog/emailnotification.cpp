#include <emailnotification.h>
#include <odb/database.hxx>

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

#include "email.h"
#include "log.h"

#include "models/config.h"
#include "models/configservice.h"
#include "models/config_odb.h"

using model::ConfigPtr;
using model::Config;


EmailNotification::EmailNotification(odb::database* db) : db(db) {
	//nothing to do
}

void EmailNotification::sendMessage(const std::string& message) {
	using Query = odb::query<Config>;

	try {
		odb::transaction t(db->begin());
		ConfigPtr smtpServerConf   = db->query_one<Config>(Query::key == "smtpServer");
		ConfigPtr smtpPortConf     = db->query_one<Config>(Query::key == "smtpPort");
		ConfigPtr smtpUserConf     = db->query_one<Config>(Query::key == "smtpUser");
		ConfigPtr smtpPasswordConf = db->query_one<Config>(Query::key == "smtpPassword");

		ConfigPtr emailConf = db->query_one<Config>(Query::key == "email");
		t.commit();

		if (smtpServerConf == nullptr || smtpPortConf == nullptr || smtpUserConf == nullptr ||
				smtpPasswordConf == nullptr || emailConf == nullptr) {
			return;
		}

		const std::string& smtpServer = smtpServerConf->value;
		int smtpPort                  = std::stoi(smtpPortConf->value);
		const std::string& user       = smtpUserConf->value;
		const std::string& password   = smtpPasswordConf->value;

		const std::string& targetEmail = emailConf->value;

		Email email(smtpServer, smtpPort, user, password);
		email.send(user, targetEmail, "Pvlog email notification", message);
		LOG(Info) << "Sended pvlog email notification: " << message;
	} catch (const std::exception& ex) {
		LOG(Error) << "Failed sending email notification: " << ex.what();
	}
}
