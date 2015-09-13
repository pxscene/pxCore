// rtLog.cpp CopyRight 2005-2015 John Robinson
//#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "rtLog.h"

#ifndef WIN32
#include <unistd.h>
#include <sys/syscall.h>
#include <pthread.h>
#else
#include <Windows.h>
#define strcasecmp _stricmp
#endif

#include <inttypes.h>


struct LogLevelSetter
{
  LogLevelSetter()
  {
    const char* s = getenv("RT_LOG_LEVEL");
    if (s && strlen(s))
    {
      rtLogLevel level = RT_LOG_ERROR;
      if      (strcasecmp(s, "debug") == 0) level = RT_LOG_DEBUG;
      else if (strcasecmp(s, "info") == 0)  level = RT_LOG_INFO;
      else if (strcasecmp(s, "warn") == 0)  level = RT_LOG_WARN;
      else if (strcasecmp(s, "error") == 0) level = RT_LOG_ERROR;
      else if (strcasecmp(s, "fatal") == 0) level = RT_LOG_FATAL;
      else
      {
        fprintf(stderr, "invalid RT_LOG_LEVEL set: %s", s);
        abort();
      }
      rtLogSetLevel(level);
    }
  }
};

static LogLevelSetter __logLevelSetter; // force RT_LOG_LEVEL to be read from env

static const char* rtLogLevelStrings[] =
{
  "DEBUG",
  "INFO",
  "WARN",
  "ERROR",
  "FATAL"
};
static const int numRTLogLevels = sizeof(rtLogLevelStrings)/sizeof(rtLogLevelStrings[0]);

static const char* rtLogLevelToString(rtLogLevel l)
{
  const char* s = "OUT-OF-BOUNDS";
  if (l < numRTLogLevels)
    s = rtLogLevelStrings[l];
  return s;
}

static const char* rtTrimPath(const char* s)
{
  if (!s)
    return s;

  const char* t = strrchr(s, (int) '/');
  if (t) t++;
  if (!t) t = s;

  return t;
}

static rtLogHandler sLogHandler = NULL;
void rtLogSetLogHandler(rtLogHandler logHandler)
{
  sLogHandler = logHandler;
}

static rtLogLevel sLevel = RT_LOG_INFO;
void rtLogSetLevel(rtLogLevel level)
{
  sLevel = level;
}

void rtLogPrintf(rtLogLevel level, const char* file, int line, const char* format, ...)
{
  if (level < sLevel)
    return;

  const char* logLevel = rtLogLevelToString(level);
  const char* path = rtTrimPath(file);
// TODO fix for real... 
#ifdef __APPLE__
  uint64_t threadId;
  pthread_threadid_np(NULL, &threadId);
  #define THREAD_ID_FORMAT PRIu64
#elif defined WIN32
  const DWORD threadId = GetCurrentThreadId();
  #define THREAD_ID_FORMAT "l"
#else
  const int threadId = syscall(__NR_gettid);
  #define THREAD_ID_FORMAT "d"
#endif

  if (sLogHandler == NULL)
  {
    printf(RTLOGPREFIX "%5s %s:%d -- Thread-%" THREAD_ID_FORMAT ": ", logLevel, path, line, threadId);

    va_list ptr;
    va_start(ptr, format);
    vprintf(format, ptr);
    va_end(ptr);

    printf("\n");
  }
  else
  {
    size_t n = 0;
    char buff[1024];
    va_list args;

    va_start(args, format);
    n = vsnprintf(buff, sizeof(buff), format, args);
    va_end(args);

    if (n >= sizeof(buff))
      buff[sizeof(buff) - 1] = '\0';

    sLogHandler(level, path, line, threadId, buff);
  }
  
  if (level == RT_LOG_FATAL)
    abort();
}

