#ifndef CONST_H
#define CONST_H

#include <QString>

/** Application name **/
const QString APP_NAME = "WeatherXM M5 Flasher";

/** esptool binary path */
const QString ESPTOOL_DIR = "./bin/";

const QString ESPTOOL_BINARY = "./esptool";
 const QString FW_DIR_PATH = "./bin";

/** Path for firmware descriptor */
const QString FW_JSON_PATH = FW_DIR_PATH + "/fw.json";

/** Time to wait for esptool proc to finish before killing */
const int ESPTOOL_WAIT_PROC_FINISHED_MS = 1000;

/** Interval at which serial port list will be refreshed */
const int RELOAD_SERIAL_PORTS_INTERVAL_MS = 2000;

/** FW descriptor JSON keys **/
const QString FW_JSON_KEY_VERSION = "fw_version";
const QString FW_JSON_KEY_CMD = "cmd";

/** Log file path */
const QString LOG_FILE_PATH = "./log.txt";

#endif // CONST_H
