#ifndef CLI_H
#define CLI_H

class QCoreApplication;

// 处理命令行模式。返回值可直接作为程序退出码。
int runCommandLine(QCoreApplication &application);

#endif
