#include "cli.h"
#include "mainwindow.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    // 无参数时启动图形界面；有参数时使用命令行模式。
    if (argc > 1) {
        QCoreApplication application(argc, argv);
        application.setApplicationName(QStringLiteral("myschedule"));
        application.setOrganizationName(QStringLiteral("CalendarDemo"));
        return runCommandLine(application);
    }

    QApplication application(argc, argv);
    application.setApplicationName(QStringLiteral("我的日程"));
    application.setOrganizationName(QStringLiteral("CalendarDemo"));
    MainWindow window;
    window.show();
    return application.exec();
}
