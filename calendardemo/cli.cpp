#include "cli.h"

#include "reminderworker.h"
#include "taskmanager.h"
#include "usermanager.h"

#include <QCoreApplication>
#include <QMetaObject>
#include <QProcess>
#include <QTextStream>
#include <QThread>

#include <algorithm>

namespace {

QTextStream out(stdout);
QTextStream in(stdin);

void printHelp()
{
    // QTextStream 的 const char* 输出会按 Latin-1 解释；中文必须以 QString 输出。
    out << QStringLiteral("日程管理程序（无参数启动图形界面）\n\n"
           "命令：\n"
           "  myschedule --help\n"
           "  myschedule register <用户名> <口令>\n"
           "  myschedule <用户名> <口令> addtask <任务名> <启动时间> [优先级] [分类] [提醒时间]\n"
           "  myschedule <用户名> <口令> updatetask <原任务名> <原启动时间> <新任务名> <新启动时间> [优先级] [分类] [提醒时间]\n"
           "  myschedule <用户名> <口令> showtask [日期]\n"
           "  myschedule <用户名> <口令> searchtask <任务名称>\n"
           "  myschedule <用户名> <口令> deltask <任务名> <启动时间>\n"
           "  myschedule run [用户名 口令]\n\n"
           "时间格式：yyyy-MM-ddTHH:mm，例如 2026-07-27T19:30。\n"
           "优先级：high、medium（默认）、low。分类：study、entertainment、life（默认）。\n\n"
           "示例：\n"
           "  myschedule register user1 password\n"
           "  myschedule user1 password addtask \"学习 Qt\" 2026-07-27T19:30 high study 2026-07-27T19:25\n"
           "  myschedule user1 password updatetask \"学习 Qt\" 2026-07-27T19:30 \"复习 Qt\" 2026-07-27T20:00 medium study\n"
           "  myschedule user1 password showtask 2026-07-27\n"
           "  myschedule user1 password searchtask \"Qt\"\n"
           "  myschedule user1 password deltask \"复习 Qt\" 2026-07-27T20:00\n"
           "  myschedule run user1 password\n");
    out.flush();
}

QDateTime parseDateTime(const QString &text)
{
    QDateTime value = QDateTime::fromString(text, Qt::ISODate);
    if (!value.isValid()) value = QDateTime::fromString(text, QStringLiteral("yyyy-MM-dd HH:mm"));
    return value;
}

Priority parsePriority(const QString &text, bool *ok)
{
    const QString value = text.toLower();
    if (value.isEmpty() || value == "medium") { *ok = true; return Priority::Medium; }
    if (value == "high") { *ok = true; return Priority::High; }
    if (value == "low") { *ok = true; return Priority::Low; }
    *ok = false;
    return Priority::Medium;
}

Category parseCategory(const QString &text, bool *ok)
{
    const QString value = text.toLower();
    if (value.isEmpty() || value == "life") { *ok = true; return Category::Life; }
    if (value == "study") { *ok = true; return Category::Study; }
    if (value == "entertainment") { *ok = true; return Category::Entertainment; }
    *ok = false;
    return Category::Life;
}

void printTasks(const QList<Task> &tasks)
{
    const auto cell = [](const QString &text, int width) { return text.leftJustified(width, ' '); };
    out << cell(QStringLiteral("任务名称"), 18) << "  "
        << cell(QStringLiteral("启动时间"), 16) << "  "
        << cell(QStringLiteral("提醒时间"), 16) << "  "
        << cell(QStringLiteral("优先级"), 4) << "  " << QStringLiteral("分类") << '\n';
    out << QString(70, '-') << '\n';
    for (const Task &task : tasks) {
        out << cell(task.name(), 18) << "  "
            << cell(task.startTime().toString("yyyy-MM-dd HH:mm"), 16) << "  "
            << cell(task.remindTime().toString("yyyy-MM-dd HH:mm"), 16) << "  "
            << cell(task.priorityText(), 4) << "  " << task.categoryText() << '\n';
    }
    out.flush();
}

bool addTaskFromArguments(const QStringList &arguments, TaskManager &manager)
{
    if (arguments.size() < 2 || arguments.size() > 5) {
        out << QStringLiteral("参数错误：addtask 需要 <任务名> <启动时间> [优先级] [分类] [提醒时间]。\n");
        return false;
    }
    const QDateTime start = parseDateTime(arguments.at(1));
    bool priorityOk = false;
    bool categoryOk = false;
    const Priority priority = parsePriority(arguments.value(2), &priorityOk);
    const Category category = parseCategory(arguments.value(3), &categoryOk);
    const QDateTime reminder = arguments.size() >= 5 ? parseDateTime(arguments.at(4)) : start.addSecs(-300);
    if (!start.isValid() || !reminder.isValid() || !priorityOk || !categoryOk) {
        out << QStringLiteral("时间、优先级或分类格式无效。\n");
        return false;
    }
    QString error;
    if (!manager.addTask(arguments.at(0), start, priority, category, reminder, &error)) {
        out << QStringLiteral("添加失败：") << error << '\n';
        return false;
    }
    out << QStringLiteral("任务已保存。\n");
    return true;
}

bool updateTaskFromArguments(const QStringList &arguments, TaskManager &manager)
{
    if (arguments.size() < 4 || arguments.size() > 7) {
        out << QStringLiteral("参数错误：updatetask 需要 <原任务名> <原启动时间> <新任务名> <新启动时间> [优先级] [分类] [提醒时间]。\n");
        return false;
    }
    const QDateTime oldStart = parseDateTime(arguments.at(1));
    const QDateTime newStart = parseDateTime(arguments.at(3));
    bool priorityOk = false;
    bool categoryOk = false;
    const Priority priority = parsePriority(arguments.value(4), &priorityOk);
    const Category category = parseCategory(arguments.value(5), &categoryOk);
    const QDateTime reminder = arguments.size() >= 7 ? parseDateTime(arguments.at(6)) : newStart.addSecs(-300);
    if (!oldStart.isValid() || !newStart.isValid() || !reminder.isValid() || !priorityOk || !categoryOk) {
        out << QStringLiteral("时间、优先级或分类格式无效。\n");
        return false;
    }
    QString error;
    if (!manager.updateTaskByNameAndStart(arguments.at(0), oldStart, arguments.at(2), newStart,
                                          priority, category, reminder, &error)) {
        out << QStringLiteral("更新失败：") << error << '\n';
        return false;
    }
    out << QStringLiteral("任务已更新并保存。\n");
    return true;
}

bool executeLoggedInCommand(const QStringList &arguments, TaskManager &manager)
{
    if (arguments.isEmpty()) return true;
    const QString command = arguments.first().toLower();
    if (command == "addtask") return addTaskFromArguments(arguments.mid(1), manager);
    if (command == "updatetask") return updateTaskFromArguments(arguments.mid(1), manager);
    if (command == "showtask") {
        const QDate date = arguments.size() >= 2 ? QDate::fromString(arguments.at(1), Qt::ISODate) : QDate::currentDate();
        if (!date.isValid()) { out << QStringLiteral("日期格式无效，应为 yyyy-MM-dd。\n"); return false; }
        printTasks(manager.tasksForDate(date));
        return true;
    }
    if (command == "searchtask") {
        if (arguments.size() != 2 || arguments.at(1).trimmed().isEmpty()) {
            out << QStringLiteral("用法：searchtask <任务名称>\n");
            return false;
        }
        const QString keyword = arguments.at(1).trimmed();
        QList<Task> matchedTasks;
        for (const Task &task : manager.allTasks()) {
            if (task.name().contains(keyword, Qt::CaseInsensitive)) matchedTasks.append(task);
        }
        if (matchedTasks.isEmpty()) {
            out << QStringLiteral("未找到名称包含“") << keyword << QStringLiteral("”的任务。\n");
            return true;
        }
        std::sort(matchedTasks.begin(), matchedTasks.end(), [](const Task &left, const Task &right) {
            return left.startTime() < right.startTime();
        });
        out << QStringLiteral("找到 ") << matchedTasks.size() << QStringLiteral(" 条匹配任务：\n");
        printTasks(matchedTasks);
        return true;
    }
    if (command == "deltask") {
        if (arguments.size() != 3) { out << QStringLiteral("用法：deltask <任务名> <启动时间>\n"); return false; }
        const QDateTime start = parseDateTime(arguments.at(2));
        if (!start.isValid()) { out << QStringLiteral("启动时间格式无效。\n"); return false; }
        QString error;
        if (!manager.deleteTaskByNameAndStart(arguments.at(1), start, &error)) {
            out << QStringLiteral("删除失败：") << error << '\n';
            return false;
        }
        out << QStringLiteral("任务已删除。\n");
        return true;
    }
    out << QStringLiteral("未知命令。输入 help 查看帮助。\n");
    return false;
}

int runShell(const QString &username, TaskManager &manager)
{
    QThread reminderThread;
    auto *worker = new ReminderWorker;
    qRegisterMetaType<Task>("Task");
    qRegisterMetaType<QList<Task>>("QList<Task>");
    worker->moveToThread(&reminderThread);
    QObject::connect(&reminderThread, &QThread::started, worker, &ReminderWorker::start);
    QObject::connect(&reminderThread, &QThread::finished, worker, &QObject::deleteLater);
    QObject::connect(worker, &ReminderWorker::reminderDue, [](const Task &task) {
        QTextStream notice(stdout);
        notice.setCodec("UTF-8");
        notice << QStringLiteral("\n[任务提醒] ") << task.name() << QStringLiteral("，开始时间：")
               << task.startTime().toString("yyyy-MM-dd HH:mm") << '\n' << "> ";
        notice.flush();
    });
    reminderThread.start();
    QMetaObject::invokeMethod(worker, "setTasks", Qt::QueuedConnection, Q_ARG(QList<Task>, manager.allTasks()));

    out << QStringLiteral("欢迎，") << username << QStringLiteral("。输入 help 查看命令，exit 退出。\n> ");
    out.flush();
    while (true) {
        const QString line = in.readLine().trimmed();
        if (line.isNull()) break;
        const QStringList command = QProcess::splitCommand(line);
        if (command.isEmpty()) { out << "> "; out.flush(); continue; }
        if (command.first().toLower() == "exit" || command.first().toLower() == "quit") break;
        if (command.first().toLower() == "help") printHelp();
        else {
            executeLoggedInCommand(command, manager);
            QMetaObject::invokeMethod(worker, "setTasks", Qt::QueuedConnection, Q_ARG(QList<Task>, manager.allTasks()));
        }
        out << "> ";
        out.flush();
    }
    reminderThread.quit();
    reminderThread.wait();
    return 0;
}

} // namespace

int runCommandLine(QCoreApplication &application)
{
    // Qt5 的 QTextStream 默认使用本地编码。命令行任务名和提示包含中文，
    // 因此明确使用 UTF-8，避免 Linux/Windows 终端显示或输入乱码。
    out.setCodec("UTF-8");
    in.setCodec("UTF-8");

    const QStringList args = application.arguments().mid(1);
    if (args.isEmpty() || args.first() == "--help" || args.first() == "-h" || args.first() == "help") {
        printHelp();
        return 0;
    }
    UserManager users;
    if (args.first() == "register") {
        if (args.size() != 3) { printHelp(); return 1; }
        out << (users.registerUser(args.at(1), args.at(2)) ? QStringLiteral("注册成功。\n") : QStringLiteral("注册失败：用户名或口令为空，或用户名已存在。\n"));
        return 0;
    }

    QString username;
    QString password;
    if (args.first() == "run") {
        if (args.size() == 3) { username = args.at(1); password = args.at(2); }
        else if (args.size() == 1) {
            out << QStringLiteral("用户名："); out.flush(); username = in.readLine().trimmed();
            out << QStringLiteral("口令："); out.flush(); password = in.readLine();
        } else { printHelp(); return 1; }
        if (!users.login(username, password)) { out << QStringLiteral("登录失败：用户名或口令不正确。\n"); return 1; }
        TaskManager manager;
        manager.load(username);
        return runShell(username, manager);
    }

    if (args.size() < 3 || !users.login(args.at(0), args.at(1))) {
        out << QStringLiteral("登录失败，或命令参数不完整。\n");
        return 1;
    }
    TaskManager manager;
    manager.load(args.at(0));
    return executeLoggedInCommand(args.mid(2), manager) ? 0 : 1;
}
