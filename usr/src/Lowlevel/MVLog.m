#import <ObjFW/OFStdIOStream.h>
#include <time.h>

#import "MVLog.h"

struct Level
{
    const char * name;
    const char * colour;
};

struct Level levels[5] = {{.name = "TRACE", .colour = "\x1b[94m"},
                          {.name = "DEBUG", .colour = "\x1b[36m"},
                          {.name = "INFO", .colour = "\x1b[32m"},
                          {.name = "WARN", .colour = "\x1b[33m"},
                          {.name = "ERROR", .colour = "\x1b[31m"}};

void MVLogLevelInternal (MVLogLevel level, const char * file, int lineNo,
                         const char * func, OFString * text)
{
    time_t t = time (NULL);
    struct tm * lt = localtime (&t);
    char buf[16];
    /* need for blocks */
    char * pBuf = (char *)&buf;

    buf[strftime (buf, sizeof (buf), "%H:%M:%S", lt)] = '\0';

    [text enumerateLinesUsingBlock:^(OFString * line, bool * stop) {
      [of_stdout writeFormat:@"%s %s%-5s\x1b[0m",
                             pBuf,
                             levels[level].colour,
                             levels[level].name];

      if (0 /* show file/line */)
          [of_stdout writeFormat:@" \x1b[90m%s:%d", file, lineNo];
      if (0 /* show func */)
          [of_stdout writeFormat:@"\x1b[92m%s: ", func];
      [of_stdout writeString:@"\x1b[0m"];
      [of_stdout writeLine:line];
    }];
}