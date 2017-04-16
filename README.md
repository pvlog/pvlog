## pvlog
Pvlog is tool to log data from solar inverters. It supports
email notification on errors. And dail summary email notification.


## Installation

Install the dependencies:
- cmake
- boost
- libjsonrpccpp-dev
- Poco Crypto, Net NetSSL_OpenSSL
- odb
- sqlite
- pvlib

Od debian like system they can be installed by:
```sh
sudo apt-get install cmake libboost-dev libboost-date-time-dev libboost-log-dev \
	     libboost-signals-dev libpoco-dev libodb-dev libodb-boost-dev \
	     libodb-sqlite-dev odb libsqlite3-dev
```

For installation of pvlib see ...

## Usage
To run it as systemd service.
Create extra user for pvlog:

Copy file pvlog.service to /etc/systemd/system/

add user pvlog:
```sh
sudo useradd --system pvlog
```
create folder for log files:
```
sudo mkdir /var/log/pvlog
sudo chown /var/log/pvlog

Enable service:
sudo systemctl enable pvlog.service
