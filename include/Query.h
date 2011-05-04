#ifndef QUERY_H
#define QUERY_H

class Database;

class Query {
    friend Database;
    public:
        Query(Database *database);
        virtual ~Query();
    private:
        Database *db;
};

#endif // QUERY_H
