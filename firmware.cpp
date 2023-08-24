#include "firmware.h"
#include <QFile>
#include "const.h"
#include <QJsonDocument>
#include <QJsonObject>

/******************************************************************************
* Private constructor/destructor
******************************************************************************/
Firmware::Firmware(){}
Firmware::~Firmware(){}

/******************************************************************************
* Get singleton instance
******************************************************************************/
Firmware& Firmware::inst()
{
    static Firmware inst;
    return inst;
}

/******************************************************************************
* Load and parse firmware descriptor json file
******************************************************************************/
bool Firmware::load()
{
    QFile file(FW_JSON_PATH);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Could not open fw descriptor: " << FW_JSON_PATH;
        return false;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();

    if (error.error != QJsonParseError::NoError)
    {
        qDebug() << "Could not parse fw descriptor.";
        return false;
    }

    if (!doc.isObject())
    {
        return false;
    }

    QJsonObject obj = doc.object();

    if(!obj.contains(FW_JSON_KEY_CMD) || !obj.contains(FW_JSON_KEY_VERSION))
    {
        qDebug() << "Invalid fw descriptor structure.";
        return false;
    }

    _version = obj.value(FW_JSON_KEY_VERSION).toString();
    _args = obj.value(FW_JSON_KEY_CMD).toString().split(" ");

    if(_version.length() < 1 || _args.length() < 1)
    {
        qDebug() << "Invalid fw descriptor values.";
        return false;
    }

    // Add --force param for newer esptool versions which do not allow the bootloader to
    // be overwritten by default
    _args << "--force";

    qDebug() << "FW version: " << _version << " Args: " << _args.join(" ");

    return true;
}

/******************************************************************************
* Get version parsed from file
******************************************************************************/
QString Firmware::get_version()
{
    return _version;
}

/******************************************************************************
* Get arguments parsed from file
******************************************************************************/
QStringList Firmware::get_args()
{
    return _args;
}
