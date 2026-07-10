#include "logger.h"

void logMessage(LogLevel level, const char *file, int line, const char *function, const QString &message) {
    const QString file_name = QString(file).section('/', -1).section('\\', -1);
    QString formatted_message = QString("[%1:%2] %4")
    .arg(file_name)
        .arg(line)
        .arg(message);

    switch (level) {
    case Debug:
        QMessageLogger(file, line, function).debug() << formatted_message << Qt::endl;
        break;
    case Warning:
        QMessageLogger(file, line, function).warning() << formatted_message << Qt::endl;
        break;
    case Critical:
        QMessageLogger(file, line, function).critical() << formatted_message << Qt::endl;
        break;
    case Fatal:
        QMessageLogger(file, line, function).fatal() << formatted_message << Qt::endl;
        break;
    }
}
