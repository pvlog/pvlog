#include <emailnotification.h>
#include <odb/database.hxx>

#include "email.h"

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
}
