#ifndef SRC_PVLOG_MODELS_ODBHELPER_H_
#define SRC_PVLOG_MODELS_ODBHELPER_H_

#include <ostream>
#include <odb/nullable.hxx>

template<typename T>
std::ostream& operator<< (std::ostream& o, const odb::nullable<T>& nullable) {
	if (nullable.null()) {
		o << "null";
	} else {
		o << nullable.get();
	}

	return o;
}


#endif /* SRC_PVLOG_MODELS_ODBHELPER_H_ */
