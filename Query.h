#ifndef QUERY_H
#define QUERY_H

#include <vector>
#include <string>

#include "PvlogException.h"

class Query {
public:
    class Value {
    public:
        enum Type {
            String,
            Blob,
            Double,
            Float,
            Int,
            UnsignedInt,
            Long,
            LongLong,
            UnsignedLongLong,
            Null
        };

    private:
        union Data {
            float           f;
            int             i;
            unsigned int    ui;
            double          d;
            long long       ll;
            const char      *s;
            const unsigned char *b;
        } data;
        int len;
        std::string str;

        Type    type;

    public:
        Value(const char * value)
        {
            type   = String;
            str    = std::string(value);
        }

        Value(const std::string & value)
        {
            type   = String;
            str    = value;
        }


        Value(float value)
        {
            type   = Float;
            data.f = value;
        }

        Value(double value)
        {
            type   = Double;
            data.d = value;
        }

        Value(int value)
        {
            type   = Int;
            data.i = value;
        }

        Value(unsigned int value)
        {
            type    = UnsignedInt;
            data.ui = value;
        }

        Value(long long value)
        {
            type   = LongLong;
            data.i = value;
        }

        Value(const unsigned char *value, int len)
        {
            type   = Blob;
            data.b = value;
            len    = len;
        }

        Value()
        {
            type   = Null;
        }

        Type getType() const
        {
            return type;
        }

        bool isNull()
        {
            return type == Null;
        }

        long long getLongLong() const
        {
            if (type != LongLong && type != Int)
                PVLOG_EXCEPT("Can not convert to long long.");
            return data.ll;
        }

        int getInt() const
        {
            if (type != Int && type != UnsignedInt)
                PVLOG_EXCEPT("Can not convert to int.");
            return data.i;
        }

        double getDouble() const
        {
            if (type != Double && type != Float)
                PVLOG_EXCEPT("Can not convert to double");
            if (type == Double) return data.d;
            else return data.f;
        }

        float getFloat() const
        {
            if (type != Float)
                PVLOG_EXCEPT("Con not convert to float");
            return data.f;
        }

        const std::string & getString() const
        {
            if (type != String)
                PVLOG_EXCEPT("Can not convert to string");
            return str;
        }

        const unsigned char * getBlob(int * len) const
        {
            if (type != Blob)
                PVLOG_EXCEPT("Can not convert to BLOB");
            return data.b;
        }
    };
private :
    std::string         statment;
    std::vector<Value>  values;
protected :
    virtual void exec(std::vector<Value> & values) = 0;
public:
    virtual ~Query() {}

    virtual void beginTransaction() = 0;

    virtual void commitTransaction() = 0;

    virtual void prepare(const std::string & statment) = 0;

    void bindValueAdd(const Value & value)
    {
        values.push_back(value);
    }

    void bindValueAdd()
    {
        values.push_back(Value());
    }

    void exec()
    {
        exec(values);
    }

    virtual void exec(const std::string & statment)
    {
        prepare(statment);
        std::vector<Value> values;
        exec(values);
    }

    virtual bool next() = 0;

    virtual Value getValue(int index) = 0;
};

#endif // QUERY_H
