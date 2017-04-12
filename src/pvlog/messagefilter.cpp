#include "messagefilter.h"

#include <ctime>

MessageFilter::MessageFilter() {
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
		if (curTime - t > 10 * 60) {
			it->second = curTime;
			newMessageSignal(message);
		}
	}
}
