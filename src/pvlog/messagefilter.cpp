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
