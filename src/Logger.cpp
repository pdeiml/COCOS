#include "Logger.hpp"

Logger::Logger()
{
}

Logger::~Logger()
{
    os << std::endl;;
    std::cout << os.str();
}

LogLevel& Logger::ReportingLevel()
{
    static LogLevel fLevel = sDebug;
    return fLevel;
}

std::ostringstream& Logger::ToString(LogLevel level)
{
    if (level == sError) {
        os << "\033[91m ERROR\033[0m";
    } else if (level == sWarning) {
        os << "\033[33m WARNING\033[0m";
    } else if (level == sInfo){
        os << "\033[32m INFO\033[0m";
    } else if (level == sDebug){
        os << "\033[36m DEBUG\033[0m";
    }
}

std::ostringstream& Logger::Get(const std::string& function, LogLevel level)
{   
    size_t colons = function.find("::");
    size_t begin = function.substr(0, colons).rfind(" ") + 1;
    size_t end = function.rfind("(") - begin;

    os << "[" << function.substr(begin, end) << "]";
    os << "\t";
    ToString(level); 
    os << ": ";
    os << std::string ( level > sDebug ? level - sDebug : 0, '\t');
    return os;
}