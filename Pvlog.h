#ifndef PVLOG_H
#define PVLOG_H

#include <string>
class Database;

class Pvlog
{
    private:
        std::string name;
        std::string hostname;
        std::string port;
        std::string password;

        //Database database;

        void readConfig(Database & database);

        void openDatabase();


    public:
        Pvlog();

        ~Pvlog();

        void start(bool daemon);
};

#endif // PVLOG_H
