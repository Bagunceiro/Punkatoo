#ifndef GIT_REV
#define GIT_REV ""
#endif

#include <Arduino.h>
#include <time.h>

const char *gitrevision = GIT_REV;
const char *compTime = __TIME__;
const char *compDate    = __DATE__;
