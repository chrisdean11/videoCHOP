/* 
    Log function.
*/

#ifndef LOG_H
#define LOG_H

#include <iostream> 

enum class LogLevel
{
    debug,
    info,
    error
};

// Above is useless so I'm going to complete its original goal eventually:
namespace Log
{
    extern bool logToFile;
    extern std::string filename;

    void Log(const char *fmt, ...) __attribute__((format (printf, 1, 2)));

    void Log(LogLevel level, const char *fmt, ...) __attribute__((format (printf, 2, 3)));

    void logFile(const std::string &fn);
}

#endif // LOG_H