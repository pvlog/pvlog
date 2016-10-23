#ifndef RFCOMM_H
#define RFCOMM_H

#include <Connection.h>
#include <memory>

namespace pvlib {

class Rfcomm : public Connection {
public:
	Rfcomm();

	virtual ~Rfcomm() override;

	virtual int connect(const char *address, const void *param) override;

	virtual void disconnect() override;

	virtual int write(const uint8_t *data, int len, const std::string &to) override;

	virtual int read(uint8_t *data, int max_len, std::string& from) override;
private:
	bool connected;
	int timeout;
	int socket;
	uint8_t src_mac[6];
	uint8_t dst_mac[6];
	char src_name[128];
	char dst_name[128];
};

} //namespace pvlib {

#endif /* #ifndef RFCOMM_H */
