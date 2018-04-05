#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

enum LogLevel { sFatal, sError, sWarning, sInfo, sDebug };
#define LOG_MAX_LEVEL sDebug

class Logger{
private:
    std::ostringstream os;

    std::ostringstream& ToString(LogLevel level);

public:
    Logger();
    virtual ~Logger();
    static LogLevel& ReportingLevel();
    std::ostringstream& Get(const std::string& function, LogLevel level = sInfo);
};

#define LOG(function, level) \
    if (level > LOG_MAX_LEVEL) ; \
    else if (level > Logger::ReportingLevel()) ; \
    else Logger().Get(function, level)

#define GERROR LOG(__PRETTY_FUNCTION__, sError)
#define GWARNING LOG(__PRETTY_FUNCTION__, sWarning)
#define GINFO LOG(__PRETTY_FUNCTION__, sInfo)
#define GDEBUG LOG(__PRETTY_FUNCTION__, sDebug)

#endif