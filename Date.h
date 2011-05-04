#ifndef DATE_H
#define DATE_H

#include <ctime>
#include <cstring>

class Date {
private :
    struct tm timeInfo;
    char time[20];

public:
    Date(time_t time)
    {
        struct tm *timeInfo = localtime(&time);
        memcpy(&this->timeInfo, timeInfo, sizeof(*timeInfo));
    }

    Date(int year, int month, int day)
    {
        struct tm timeInfo;
        memset(&timeInfo, 0x00, sizeof(timeInfo));

        timeInfo.tm_year = year;
        timeInfo.tm_mon  = month;
        timeInfo.tm_mday = day;
    }

    time_t getDayBeginn() const
    {
        //TODO
        return 0;
    }

    int getYear() const
    {
        return timeInfo.tm_year;
    }

    int getMonth() const
    {
        return timeInfo.tm_mon;
    }

    int getDay() const
    {
        return timeInfo.tm_mday;
    }
/*
    const char *string() const
    {
        strftime(time, 20, "%Y-%m-%d %H:%M:%s", timeInfo);
        return time;
    }

    */
};



#endif // #ifdef DATE_H
