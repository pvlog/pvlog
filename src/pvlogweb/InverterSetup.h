#ifndef INVERTERSETUP_H
#define INVERTERSETUP_H

#include <string>

#include <ctemplate/template.h>

class InverterSetupView {
private:
	ctemplate::TemplateDictionary* dict;

public:
	struct FormData {
		std::string name;
		bool highlightName;

		std::string plant;
		bool highlightPlant;

		std::string wattpeak;
		bool highlightWattpeak;
	};
public:
	InverterSetupView(ctemplate::TemplateDictionary* dict);

	void handleRequest();

	void handleRequest(FormData& formData);
};

#endif /* INVERTERSETUP_H */
