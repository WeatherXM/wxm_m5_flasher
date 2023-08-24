#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QThread>
#include "esptool.h"
#include <QTimer>
#include "firmware.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Runner;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void flash_start();
    void flash_stop();

private slots:
    void on_btn_run_clicked();

    void esptool_state_changed(EspTool::State state, QString data);
    void esptool_data_received(QString line, bool error);
    void esptool_app_error(QProcess::ProcessError error);
    void esptool_app_finished(int code, QProcess::ExitStatus status);

    void load_serial_port_list();

    void on_btnViewLog_clicked();

private:
    void run();
    void enable_controls(bool en);
    void reset();

    Ui::MainWindow *ui;
    QProcess *_process;
    QTimer _load_serial_ports_timer;

    /** Is process currently running */
    bool _running = false;

    /** EspTool instance */
    EspTool *_esp_tool = nullptr;

    std::map<EspTool::State, std::vector<QWidget*>> _state_element_map;
};

#endif // MAINWINDOW_H
