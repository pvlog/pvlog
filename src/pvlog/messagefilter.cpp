#include "messagefilter.h"

#include <ctime>

MessageFilter::MessageFilter(time_t timeout) : timeout(timeout) {
	//nothing to do
}

void MessageFilter::addMessage(const std::string& message) {
	auto it = messages.find(message);
	time_t curTime = time(nullptr);

	if (it == messages.end()) {
		messages.emplace(message, curTime);
		newMessageSignal(message);
	} else {
		time_t t = it->second;
		if (curTime - t > timeout) {
			it->second = curTime;
			newMessageSignal(message);
		}
	}
}
