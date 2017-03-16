#ifndef MESSAGE_FILTER_H
#define MESSAGE_FILTER_H

#include <string>
#include <unordered_map>

#include <boost/signals2.hpp>

class MessageFilter {
public:
	boost::signals2::signal<void (const std::string&)> newMessageSignal;

	MessageFilter();

	void addMessage(const std::string& message);

private:
	std::unordered_map<std::string, time_t> messages;
};

#endif //#ifndef MESSAGE_FILTER_H
