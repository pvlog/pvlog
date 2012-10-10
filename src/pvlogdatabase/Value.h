#ifndef VALUE_H
#define VALUE_H

#include <vector>
#include <string>
#include <stdint.h>
#include <limits>

#include "PvlogException.h"
#include "Utility.h"

class Value {
public:
	enum Type {
		String,
		Blob,
		Double,
		Float,
		Int16,
		Int32,
		Int64,
		Null
	};

private:
	union Data {
		float f;
		double d;
		int64_t int64;
		const char* s;
		const uint8_t* b;
	} data;
	int len;
	std::string str;

	Type type;

public:
	Value(const char * value)
	{
		type = String;
		str = std::string(value);
	}

	Value(const std::string& value)
	{
		type = String;
		str = value;
	}

	Value(float value)
	{
		type = Float;
		data.f = value;
	}

	Value(double value)
	{
		type = Double;
		data.d = value;
	}

	Value(int16_t value)
	{
		type = Int16;
		data.int64 = value;
	}

	Value(int32_t value)
	{
		type = Int32;
		data.int64 = value;
	}

	Value(int64_t value)
	{
		type = Int64;
		data.int64 = value;
	}

	Value(const unsigned char *value, int len)
	{
		type = Blob;
		data.b = value;
		this->len = len;
	}

	Value()
	{
		type = Null;
	}

#if __cplusplus >= 201103L
	Value(std::nullptr_t)
	{
		type = Null;
	}
#endif
	Type getType() const
	{
		return type;
	}

	bool isNull()
	{
		return type == Null;
	}

	int64_t getInt64() const
	{
		switch (type) {
		case Int64: //Fallthrough
		case Int32: //Fallthrough
		case Int16:
			return data.int64;
			break;
		default:
			PVLOG_EXCEPT("Can not convert to Int64!");
		}
	}

	operator int64_t() const
	{
		return getInt64();
	}

	int32_t getInt32() const
	{
		switch (type) {
		case Int32: //Fallthrough
		case Int16:
			return static_cast<int32_t> (data.int64);
			break;
		case Int64:
			if ((data.int64 <= std::numeric_limits<int32_t>::min()) && (data.int64
			        >= std::numeric_limits<int32_t>::max())) return static_cast<int32_t> (data.int64);
			//else Fallthrough
		default:
			PVLOG_EXCEPT("Can not convert to Int32!");
		}
	}

	operator int32_t() const
	{
		return getInt32();
	}

	int16_t getInt16() const
	{
		switch (type) {
		case Int16:
			return static_cast<int16_t> (data.int64);
			break;
		case Int32: //Fallthrough
		case Int64: //Fallthrough
			if ((data.int64 <= std::numeric_limits<int16_t>::max()) && (data.int64
			        >= std::numeric_limits<int16_t>::min())) return static_cast<int16_t> (data.int64);
			//else Fallthrough
		default:
			PVLOG_EXCEPT("Can not convert to Int16!");
		}
	}

	operator int16_t() const
	{
		return getInt16();
	}

	double getDouble() const
	{
		if (type != Double && type != Float) PVLOG_EXCEPT("Can not convert to double");
		if (type == Double) return data.d;
		else return data.f;
	}

	operator double() const
	{
		return getDouble();
	}

	float getFloat() const
	{
		if (type != Float) PVLOG_EXCEPT("Can not convert to float");
		return data.f;
	}

	operator float() const
	{
		return getFloat();
	}

	std::string getString() const
	{
		if (type != String) PVLOG_EXCEPT("Can not convert to string");
		return str;
	}

	operator std::string() const
	{
		return getString();
	}

	const uint8_t* getBlob(int& len) const
	{
		if (type != Blob) PVLOG_EXCEPT("Can not convert to BLOB");
		len = this->len;
		return data.b;
	}
};

#endif // #ifndef VALUE_H
