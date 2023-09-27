#ifndef FIRMWARE_H
#define FIRMWARE_H

#include <QString>
#include <QArgument>
#include <QStringList>

class Firmware
{
public:
    bool load();

    static Firmware& inst();

    QString get_version();
    QStringList get_args();
private:
    Firmware();
    ~Firmware();

    QString _version;
    QStringList _args;

    Firmware(const Firmware&) = delete;
    Firmware& operator=(const Firmware&) = delete;
};

#endif // FIRMWARE_H
