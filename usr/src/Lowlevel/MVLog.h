#ifndef MVLOG_H_
#define MVLOG_H_

#import <ObjFW/OFString.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum MVLogLevel
    {
        kMVLogLevelTrace,
        kMVLogLevelDebug,
        kMVLogLevelInfo,
        kMVLogLevelWarn,
    } MVLogLevel;

    void MVLogLevelInternal (MVLogLevel level, const char * file, int lineNo,
                             const char * func, OFString * text);

#define MVLogLevel(level, fmt, ...)                                            \
    MVLogLevelInternal (level,                                                 \
                        __FILE__,                                              \
                        __LINE__,                                              \
                        __PRETTY_FUNCTION__,                                   \
                        [OFString stringWithFormat:fmt, ##__VA_ARGS__])

    //#define MVLogLevel(level, fmt, ...)

#define MVTrace(fmt, ...) MVLogLevel (kMVLogLevelTrace, fmt, ##__VA_ARGS__)
#define MVDebug(fmt, ...) MVLogLevel (kMVLogLevelDebug, fmt, ##__VA_ARGS__)
#define MVInfo(fmt, ...) MVLogLevel (kMVLogLevelInfo, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif