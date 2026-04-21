#pragma once
#include <string>

enum class LogLevel {
    INFO,
    ERROR,
    DEBUG
};

class Logger {
public:
    static void init(const std::string& filePath);
    static void log(LogLevel level, const std::string& message);
};

#define LOG_INFO(msg) Logger::log(LogLevel::INFO, msg)
#define LOG_ERROR(msg) Logger::log(LogLevel::ERROR, msg)
#define LOG_DEBUG(msg) Logger::log(LogLevel::DEBUG, msg)