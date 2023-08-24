#ifndef LOG_H
#define LOG_H
#include <QFile>
#include <QtMessageHandler>
#include "const.h"

class Log
{
public:
    static Log& inst();
private:
    Log();

    static void qt_message_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    QFile *_file;
    QtMessageHandler _default_handler;
};

#endif // LOG_H
