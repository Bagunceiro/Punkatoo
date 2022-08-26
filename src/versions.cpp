/**
 * @file versions.cpp
 * @brief Compilation information
 * 
 * This file should always be recompiled. It creates version number
 * and time stamps
 */
#ifndef GIT_REV
#define GIT_REV ""
#endif

#include <Arduino.h>
#include <time.h>

const char *gitrevision = GIT_REV;
const char *compTime = __TIME__;
const char *compDate    = __DATE__;
