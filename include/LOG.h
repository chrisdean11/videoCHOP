/* 
    Log function.
*/

#ifndef LOG_H
#define LOG_H

#include <iostream> 

#define LOG std::cerr

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