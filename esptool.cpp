#include "esptool.h"
#include "mainwindow.h"
#include "struct.h"
#include "const.h"
#include <QCoreApplication>
#include <regex>
#include <QMessageBox>

EspTool::EspTool()
{

}

EspTool::~EspTool()
{
    reset();
}

/******************************************************************************
* Create esptool process
******************************************************************************/
Ret EspTool::run()
{
    qDebug() << "Running esptool.";

    // TODO:
    _proc = new QProcess();
    _proc->setWorkingDirectory(QCoreApplication::applicationDirPath() + "/" + ESPTOOL_DIR);

    _proc->setProcessChannelMode(QProcess::MergedChannels);

    // Proc signals
    connect(_proc, &QProcess::readyReadStandardOutput, this, &EspTool::on_proc_standard_output);
    connect(_proc, &QProcess::readyReadStandardError, this, &EspTool::on_proc_standard_error);
    connect(_proc, &QProcess::errorOccurred, this, &EspTool::on_proc_error_occurred);
    connect(_proc, &QProcess::finished, this, &EspTool::on_proc_finished);

    _proc->start(ESPTOOL_BINARY, QStringList() << "--port" << _port << Firmware::inst().get_args());

    return Ret::OK;
}

/******************************************************************************
* Terminate esptool process if its running
******************************************************************************/
void EspTool::stop()
{
    if(is_running())
    {
        _proc->terminate();

        // Stall for a while before commiting sudoku
        _proc->waitForFinished(ESPTOOL_WAIT_PROC_FINISHED_MS);

        if(is_running())
        {
            _proc->kill();
        }

        // Cleanup
        reset();
    }
}

/******************************************************************************
* Cleanup resources and states.
* If process running, does nothing.
******************************************************************************/
Ret EspTool::reset()
{
    if(is_running())
    {
        return Ret::ERROR;
    }

    delete _proc;
    _proc = nullptr;

    _cur_state = State::IDLE;

    return Ret::OK;
}

/******************************************************************************
* Parse esptool outputs, emit signals and update state accordingly
******************************************************************************/
void EspTool::parse(QString line)
{
    struct StateEntry
    {
        EspTool::State state;
        const char *trigger;
    };

    // States and the sequence they are expected in
    static const std::vector<StateEntry> STATE_SEQ_LIST = {
        { EspTool::State::IDLE, "" },
        { EspTool::State::STARTING, "^esptool\\.py" },
        { EspTool::State::CONNECTING, "^Connecting" },
        { EspTool::State::CONNECTED, "^Chip is" },
        { EspTool::State::FLASHING_BOOTLOADER, "^Compressed \\d+ bytes" },
        { EspTool::State::FLASHING_PARTITIONS, "^Compressed \\d+ bytes" },
        { EspTool::State::FLASHING_DATA, "^Compressed \\d+ bytes" },
        { EspTool::State::FLASHING_FW, "^Compressed \\d+ bytes" },
        { EspTool::State::FINISH_SUCCESS, "^Leaving" }
    };

    // Find cur state in sequence list
    auto cur_entry = std::find_if(STATE_SEQ_LIST.begin(), STATE_SEQ_LIST.end(), [&](const StateEntry &entry){
        return entry.state == get_state() || get_state() == State::IDLE;
    });

    const StateEntry* next_entry = nullptr;
    if (cur_entry != STATE_SEQ_LIST.end() && std::next(cur_entry) != STATE_SEQ_LIST.end())
    {
        next_entry = &(*std::next(cur_entry));

        // Are we in the next state? switch
        std::regex reg(next_entry->trigger);

        if(std::regex_search(line.toStdString(), reg))
        {
            emit on_state_changed(next_entry->state);

            _cur_state = next_entry->state;
        }
    }

    // Parse progess in flashing state
    if(get_state() == EspTool::State::FLASHING_FW)
    {
        std::smatch match;
        std::regex regex_pct("Writing at .*?... \\((\\d+) %\\)");
        std::string inp = line.toStdString();
        if(std::regex_search(inp, match, regex_pct))
        {
            emit on_state_changed(_cur_state, QString::fromStdString(match[1]));
        }
    }
}

/******************************************************************************
* Set serial port to use
******************************************************************************/
void EspTool::set_port(QString port)
{
    _port = port;
}

/******************************************************************************
* Slot: Standard output ready
******************************************************************************/
void EspTool::on_proc_standard_output()
{
    if(!_proc->canReadLine())
    {
        return;
    }

    while(_proc->canReadLine())
    {
        QString line = QString::fromUtf8(_proc->readLine());
        parse(line);
        emit on_data_received(line, false);
        qDebug() << line;
    }
}
/******************************************************************************
* Slot: Standard error output ready
* Note: Looks like esptool outputs everything to stdout
******************************************************************************/
void EspTool::on_proc_standard_error()
{
    if(!_proc->canReadLine())
    {
        return;
    }

    while(_proc->canReadLine())
    {
        QString line = QString::fromUtf8(_proc->readLine());
        parse(line);
        emit on_data_received(line, true);
        qDebug() << line;
    }
}

/******************************************************************************
* Esptool process error
******************************************************************************/
void EspTool::on_proc_error_occurred(QProcess::ProcessError error)
{
    qDebug() << "Esptool proc error: " << (int)error;
    if(error == QProcess::ProcessError::FailedToStart)
    {
        emit on_app_error(error);
    }
}

/******************************************************************************
* Esptool process finished
******************************************************************************/
void EspTool::on_proc_finished(int code, QProcess::ExitStatus status)
{
    // Proc exited before a successful finish
    if(code != 0)
    {
        qDebug() << "Esp tool finished with error.";

        emit on_state_changed(State::FINISH_ERROR);
    }
    emit on_app_finished(code, status);
}

/******************************************************************************
* Get current flash state
******************************************************************************/
EspTool::State EspTool::get_state()
{
    return _cur_state;
}

/******************************************************************************
* Get esptool run status
* @return
*   True - Running or starting
*   False - Stopped
******************************************************************************/
bool EspTool::is_running()
{
    if(_proc == nullptr)
    {
        return false;
    }

    if(_proc->state() == QProcess::ProcessState::Running ||
        _proc->state() == QProcess::ProcessState::Starting)
    {
        return true;
    }
    return false;
}
