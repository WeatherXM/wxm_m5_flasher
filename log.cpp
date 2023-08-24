#include "log.h"
#include <QFile>
#include <QMessageBox>
#include <QDateTime>
#include <iostream>
#include <QTextStream>

/******************************************************************************
* Get singleton instance
******************************************************************************/
Log& Log::inst()
{
    static Log inst;
    return inst;
}

/******************************************************************************
* Constructor
******************************************************************************/
Log::Log()
{
    _file = new QFile(LOG_FILE_PATH);
    if(!_file->open(QIODevice::Append | QIODevice::Text))
    {
        QMessageBox::critical(nullptr, APP_NAME, "Could not create log file.");
    }

    // Keep default handler to forward msgs to after handling
    _default_handler = qInstallMessageHandler(0);

    // Hook msg outut
    qInstallMessageHandler(qt_message_handler);
}

/******************************************************************************
* QT debug message handler
******************************************************************************/
void Log::qt_message_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString message =
        QString("%1 - %2\r\n")
                  .arg(QDateTime::currentDateTime().toString("dd-mm-yyyy hh:mm:ss"))
                  .arg(msg);

    Log::inst()._file->write(message.toLocal8Bit());
    Log::inst()._file->flush();

    // Forward to default msg handler
    Log::inst()._default_handler(type, context, msg);
}
