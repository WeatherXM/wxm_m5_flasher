#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QMessageBox>
#include "firmware.h"
#include "const.h"
#include <QDir>
//#include "version.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QApplication::setStyle(QStyleFactory::create("Fusion"));

    // Load firmware descriptor, abort otherwise
    if(!Firmware::inst().load())
    {
        QMessageBox::critical(nullptr, APP_NAME, "Could not load firmware descriptor");
        return 1;
    }

    // Create a dark color palette
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    QDir::setCurrent(a.applicationDirPath());

    // Apply the dark color palette
    a.setPalette(darkPalette);
    // Temp
    w.setWindowTitle(APP_NAME + " v" + PROJECT_VERSION);
    w.show();
    return a.exec();
}
