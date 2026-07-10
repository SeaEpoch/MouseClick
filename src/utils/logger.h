#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QDebug>

enum LogLevel {
    Debug,
    Warning,
    Critical,
    Fatal
};

// 通用日志记录函数
void logMessage(LogLevel level, const char *file, int line, const char *function, const QString &message);

// 各种日志级别的宏定义
#define LOG_DEBUG(message) logMessage(Debug, __FILE__, __LINE__, __FUNCTION__, message)
#define LOG_WARNING(message) logMessage(Warning, __FILE__, __LINE__, __FUNCTION__, message)
#define LOG_CRITICAL(message) logMessage(Critical, __FILE__, __LINE__, __FUNCTION__, message)
#define LOG_FATAL(message) logMessage(Fatal, __FILE__, __LINE__, __FUNCTION__, message)

#endif // LOGGER_H
