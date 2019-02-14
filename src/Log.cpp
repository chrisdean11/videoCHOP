/* 
    Log function.
*/
#include "LOG.h"
#include <iostream> 
#include <stdarg.h>

// Above is useless so I'm going to complete its original goal eventually:
namespace Log
{
    bool printToConsole = true;
    bool logToFile = false;
    std::string filename;

    void _Log(LogLevel level, const char *fmt, va_list args)
    {
        if (level == LogLevel::debug) ; // Do something with log levels eventually

        if (printToConsole)
        {
            std::vprintf(fmt, args);
        }

        if (logToFile)
        {
            FILE * file = fopen(filename.c_str(), "a");
            vfprintf(file, fmt, args);
            fclose(file);
        }
    }

    // Printf-style log that will print to console and/or a log file depending on options to come later
    void Log(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        _Log(LogLevel::debug, fmt, args);
        va_end(args);
    }
    
    void Log(LogLevel level, const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        _Log(level, fmt, args);
        va_end(args);
    }
    
    void logFile(const std::string &fn)
    {
        filename = fn;        
    }
}

/*  
    Going for something like this solution:
    from https://stackoverflow.com/questions/7031116/how-to-create-function-like-printf-variable-argument

void _proxy_log(log_level_t level, const char *fmt, ...)
    __attribute__((format (printf, 2, 3)));

#define proxy_log(level, fmt, ...) _proxy_log(level, fmt"\n", ##__VA_ARGS__)

void _proxy_log(log_level_t level, const char *fmt, ...) {
    va_list arg;
    FILE *log_file = (level == LOG_ERROR) ? err_log : info_log;

    // Check if the message should be logged
    if (level > log_level)
        return;

    // Write the error message
    va_start(arg, fmt);
    vfprintf(log_file, fmt, arg);
    va_end(arg);

#ifdef DEBUG
    fflush(log_file);
    fsync(fileno(log_file));
#endif
}

*/