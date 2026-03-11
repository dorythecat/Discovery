#ifndef LOGGER_H
#define LOGGER_H

#include <cstdint>
#include <iostream>
#include <stdexcept>

enum LogLevel : uint8_t {
    FATAL = 0,
    ERROR = 1,
    WARN = 2,
    INFO = 3,
    DEBUG = 4,
    VALIDATION = 5
};

class Logger {
public:
    LogLevel level;

    explicit Logger(const LogLevel level = ERROR) : level(level) {}

    void log(const LogLevel severity, const char* message) const {
        if (severity > level) return;
        switch (severity) {
            case FATAL:
                throw std::runtime_error(message);
            case ERROR:
                std::cerr << "[ERROR] " << message << std::endl;
                break;
            case WARN:
                std::cerr << "[WARN] " << message << std::endl;
                break;
            case INFO:
                std::cout << "[INFO] " << message << std::endl;
                break;
            case DEBUG:
                std::cout << "[DEBUG] " << message << std::endl;
                break;
            case VALIDATION:
                std::cout << "[VALIDATION] " << message << std::endl;
                break;
        }
    }
};

#endif