#ifndef GIT_REV
#define GIT_REV ""
#endif

#include <Arduino.h>
#include <time.h>

const char *gitrevision = GIT_REV;
const char *compTime = __TIME__;
const char *compDate    = __DATE__;
char compDateTime[32] = "";

void parseCompileDate()
{
    struct tm tmstr;
         if (strncmp(compDate, "Jan", 3) == 0) tmstr.tm_mon = 0;
    else if (strncmp(compDate, "Feb", 3) == 0) tmstr.tm_mon = 1;
    else if (strncmp(compDate, "Mar", 3) == 0) tmstr.tm_mon = 2;
    else if (strncmp(compDate, "Apr", 3) == 0) tmstr.tm_mon = 3;
    else if (strncmp(compDate, "May", 3) == 0) tmstr.tm_mon = 4;
    else if (strncmp(compDate, "Jun", 3) == 0) tmstr.tm_mon = 5;
    else if (strncmp(compDate, "Jul", 3) == 0) tmstr.tm_mon = 6;
    else if (strncmp(compDate, "Aug", 3) == 0) tmstr.tm_mon = 7;
    else if (strncmp(compDate, "Sep", 3) == 0) tmstr.tm_mon = 8;
    else if (strncmp(compDate, "Oct", 3) == 0) tmstr.tm_mon = 9;
    else if (strncmp(compDate, "Nov", 3) == 0) tmstr.tm_mon = 10;
    else if (strncmp(compDate, "Dec", 3) == 0) tmstr.tm_mon = 11;

    sscanf (compDate + 4, "%02d %04d", &tmstr.tm_mday, &tmstr.tm_year);
    tmstr.tm_year -= 1900;
    sscanf (compTime, "%02d:%02d:%02d", &tmstr.tm_hour, &tmstr.tm_min, &tmstr.tm_sec);
    tmstr.tm_isdst = 0;
    tmstr.tm_wday = 0;
    tmstr.tm_yday = 0;

    strftime(compDateTime, sizeof(compDateTime) -1, "%H:%M:%S %d/%m/%y", &tmstr);
    // Serial.println(compDate);
}