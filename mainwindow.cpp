#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cstdio>
#include "QMessageBox"
#include <QScreen>
#include <QtSerialPort/QtSerialPort>
#include "esptool.h"
#include "const.h"
#include "log.h"
#include <QDesktopServices>

/******************************************************************************
* Constructor
******************************************************************************/
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    Firmware::inst().load();

    //
    // UI
    //
    ui->setupUi(this);
    ui->vertLayoutProgress->setAlignment(Qt::AlignTop);
    // Update fw label
    ui->lblFwVersion->setText("Firmware v" + Firmware::inst().get_version());
    ui->lblFwVersion->setStyleSheet("color: rgba(255, 255, 255, 0.5);");

    // View log button
    ui->btnViewLog->setStyleSheet("color: rgba(255, 255, 255, 0.5);");

    // Not resizable
    setFixedSize(this->size());
    statusBar()->setVisible(false);
    setStatusBar(nullptr);

    // Center
    move(QApplication::primaryScreen()->geometry().center() - frameGeometry().center());

    setWindowIcon(QIcon(":/assets/icon.ico"));

    // Load ports
    load_serial_port_list();

    connect(&_load_serial_ports_timer, &QTimer::timeout, this, &MainWindow::load_serial_port_list);
    _load_serial_ports_timer.start(RELOAD_SERIAL_PORTS_INTERVAL_MS);

    // States matched to labels
    _state_element_map = {
        {EspTool::State::STARTING, {ui->lblPreparing}},
        {EspTool::State::FLASHING_FW, {ui->lblFlashing, ui->progFlash}},
        {EspTool::State::FINISH_SUCCESS, {ui->lblFinished}},
        {EspTool::State::FINISH_ERROR, {ui->lblFinished}}
    };

    // Init
    Log::inst();

    reset();
}

MainWindow::~MainWindow()
{
    delete ui;
}

/******************************************************************************
* EspTool flashing state update
******************************************************************************/
void MainWindow::esptool_state_changed(EspTool::State state, QString data)
{
    // Show corresponding label(s)
    auto el_it= _state_element_map.find(state);
    if(el_it != _state_element_map.end())
    {
        auto widgets = el_it->second;
        for(QWidget *widget : widgets)
        {
            widget->setVisible(true);
        }
    }

    if(state == EspTool::State::FLASHING_FW)
    {
        ui->progFlash->setValue(data.toInt());
    }
    else if(state == EspTool::State::FINISH_SUCCESS)
    {
        ui->lblFinished->setText("Done!");
        QMessageBox::information(nullptr, APP_NAME, "Finished!");
    }
    else if(state == EspTool::State::FINISH_ERROR)
    {
        ui->lblFinished->setText("Flashing failed.");

        QMessageBox::critical(nullptr, APP_NAME, "Flashing failed. Restart your M5 and try again.");
    }
}

/******************************************************************************
* EspTool process finished
******************************************************************************/
void MainWindow::esptool_app_finished(int code, QProcess::ExitStatus status)
{
    flash_stop();
}

/******************************************************************************
* EspTool failed to run
******************************************************************************/
void MainWindow::esptool_app_error(QProcess::ProcessError error)
{
    if(error == QProcess::ProcessError::FailedToStart)
    {
        QMessageBox::critical(nullptr, QCoreApplication::applicationName(), "Failed to run flasher utility.");

        qDebug() << "Could not run esptool.";
    }

    flash_stop();
}

/******************************************************************************
* Start button click
******************************************************************************/
void MainWindow::on_btn_run_clicked()
{
    //
    // Start
    //
    if(!_running)
    {
        reset();
        flash_start();
    }
    //
    // Stop
    //
    else
    {
        flash_stop();
    }
}

/******************************************************************************
* Start flash process
******************************************************************************/
void MainWindow::flash_start()
{
    if(_running)
    {
        return;
    }

    if(_esp_tool != nullptr)
    {
        delete _esp_tool;
    }

    if(ui->lstPorts->currentData().toInt() == -1)
    {
        QMessageBox::information(nullptr, APP_NAME, "Please connect your device.");
        return;
    }

    _esp_tool = new EspTool;
    connect(_esp_tool, &EspTool::on_state_changed, this, &MainWindow::esptool_state_changed);
    connect(_esp_tool, &EspTool::on_data_received, this, &MainWindow::esptool_data_received);
    connect(_esp_tool, &EspTool::on_app_error, this, &MainWindow::esptool_app_error);
    connect(_esp_tool, &EspTool::on_app_finished, this, &MainWindow::esptool_app_finished);

    _running = true;
    ui->btn_run->setText("Cancel");
    enable_controls(false);

    _esp_tool->set_port(ui->lstPorts->currentText());

    // Go
    _esp_tool->run();

    return;
}

/******************************************************************************
* Stop flash process
******************************************************************************/
void MainWindow::flash_stop()
{
    if(!_running)
    {
        return;
    }

    _running = false;
    ui->btn_run->setText("Update");

    if(_esp_tool != nullptr)
    {
        _esp_tool->stop();
    }

    enable_controls(true);
}

/******************************************************************************
* Reset to reuse
******************************************************************************/
void MainWindow::reset()
{
    if(_running)
    {
        flash_stop();
    }

    enable_controls(true);

    // Hide labels by default
    std::for_each(_state_element_map.cbegin(), _state_element_map.cend(), [](auto pair){
        for(QWidget *widget : pair.second)
        {
            widget->setVisible(false);
        }
    });

    // Reset prog bar
    ui->progFlash->setValue(0);
}

/******************************************************************************
* Enable/disable all controls/components during run
******************************************************************************/
void MainWindow::enable_controls(bool en)
{
    if(en)
    {
        ui->lstPorts->setDisabled(false);

        _load_serial_ports_timer.start();
    }
    else
    {
        ui->lstPorts->setDisabled(true);

        _load_serial_ports_timer.stop();
    }
}

/******************************************************************************
* EspTool output received - debug only
******************************************************************************/
void MainWindow::esptool_data_received(QString line, bool error)
{
    if(!error)
    {
        QTextCursor cursor = ui->txtOut->textCursor();
        cursor.movePosition(QTextCursor::End);
        ui->txtOut->setTextCursor(cursor);
        ui->txtOut->ensureCursorVisible();
    }
    else
    {
        QMessageBox::critical(nullptr, APP_NAME, line);
    }
}

/******************************************************************************
* Load serial port listbox
******************************************************************************/
void MainWindow::load_serial_port_list()
{
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    if(ports.length() < 1)
    {
        ui->lstPorts->clear();
        ui->lstPorts->addItem("No devices found", QVariant(-1));
        return;
    }

    // Keep current port to set again as active after refresh
    QString cur_port = ui->lstPorts->currentText();

    // Rebuild list
    ui->lstPorts->clear();
    for (const QSerialPortInfo &port : ports)
    {
        QString name = port.portName();

        #if defined(Q_OS_LINUX)
        name = port.systemLocation();
        #endif

        ui->lstPorts->addItem(name);

        // Was previously selected, set active
        if(name == cur_port)
        {
            int index = ui->lstPorts->findText(name);
            if (index != -1)
            {
                ui->lstPorts->setCurrentIndex(index);
            }
        }
    }
}

/******************************************************************************
* Open log file in default system editor
******************************************************************************/
void MainWindow::on_btnViewLog_clicked()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile("log.txt"));
}

