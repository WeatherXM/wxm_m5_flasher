#ifndef ESPTOOL_H
#define ESPTOOL_H

#include <QObject>
#include <QProcess>
#include "struct.h"

class EspTool : public QObject
{
    Q_OBJECT
public:
    enum class State
    {
        IDLE,
        STARTING,
        CONNECTING,
        CONNECTED,
        FLASHING_BOOTLOADER,
        FLASHING_PARTITIONS,
        FLASHING_DATA,
        FLASHING_FW,
        FINISH_SUCCESS,
        FINISH_ERROR
    };

    EspTool();
    ~EspTool();

    Ret run();
    void stop();

    bool is_running();

    void parse(QString line);

    void set_port(QString port);

    State get_state();
private:
    QProcess *_proc = nullptr;

    State _cur_state = EspTool::State::IDLE;

    /** Current serial portport */
    QString _port = "";

    Ret reset();
private slots:
    void on_proc_standard_output();
    void on_proc_standard_error();
    void on_proc_finished(int code, QProcess::ExitStatus status);
    void on_proc_error_occurred(QProcess::ProcessError error);

signals:
    void on_state_changed(State state, QString data = "");
    void on_data_received(QString line, bool error);
    void on_app_error(QProcess::ProcessError status);
    void on_app_finished(int exitCode, QProcess::ExitStatus exitStatu);

};

#endif // ESPTOOL_H
