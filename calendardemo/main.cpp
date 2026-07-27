#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    application.setApplicationName(QStringLiteral("我的日程"));
    MainWindow window;
    window.show();
    return application.exec();
}
