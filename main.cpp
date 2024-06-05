#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.showFullScreen();

    QFile styleFile( ":/stylesheet/silo.qss" );
    if (!styleFile.open(QFile::ReadOnly))
    {
        QMessageBox::warning(nullptr, "Warning", "Failed to open style sheet file!");
    }
    else
    {
        QString style(styleFile.readAll());
        styleFile.close();
        a.setStyleSheet(style);
    }
    return a.exec();
}
