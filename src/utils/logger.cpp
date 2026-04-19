#include "logger.h"
#include <iostream>
#include <fstream>
#include <mutex>
#include <chrono>
#include <ctime>

static std::ofstream logFile;
static std::mutex logMutex;

std::string getTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    char buf[100];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now_c));
    return std::string(buf);
}

std::string levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::INFO: return "INFO";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::DEBUG: return "DEBUG";
    }
    return "UNKNOWN";
}

void Logger::init(const std::string& filePath) {
    logFile.open(filePath, std::ios::app);
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);

    std::string logLine = "[" + getTime() + "] [" + levelToString(level) + "] " + message;

    // Console
    std::cout << logLine << std::endl;

    // File
    if (logFile.is_open()) {
        logFile << logLine << std::endl;
    }
}