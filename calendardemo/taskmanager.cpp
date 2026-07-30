#include "taskmanager.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QUuid>
#include <algorithm>

bool TaskManager::load(const QString &username)
{
    QDir().mkpath(QStringLiteral("data/tasks"));
    // 十六进制用户名只含安全文件名字符，避免用户名意外改变保存路径。
    const QString safeUserFileName = QString::fromLatin1(username.toUtf8().toHex());
    m_filePath = QDir::current().filePath(QStringLiteral("data/tasks/") + safeUserFileName + QStringLiteral(".json"));
    m_tasks.clear();
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) return true; // 第一次登录没有任务文件是正常情况
    const QJsonArray array = QJsonDocument::fromJson(file.readAll()).object().value("tasks").toArray();
    for (const QJsonValue &value : array) m_tasks.append(Task::fromJson(value.toObject()));
    return true;
}

bool TaskManager::save() const
{
    QJsonArray array;
    for (const Task &task : m_tasks) array.append(task.toJson());
    QSaveFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(QJsonDocument(QJsonObject{{"tasks", array}}).toJson());
    return file.commit();
}

bool TaskManager::addTask(const QString &name, const QDateTime &start, Priority priority,
                          Category category, const QDateTime &remind, QString *errorMessage,
                          const QString &note)
{
    if (name.trimmed().isEmpty() || !start.isValid() || !remind.isValid()) {
        if (errorMessage) *errorMessage = QStringLiteral("请填写有效的任务名称、开始时间和提醒时间。");
        return false;
    }
    for (const Task &task : m_tasks) {
        if (task.startTime() == start) {
            if (errorMessage) *errorMessage = QStringLiteral("已有任务使用该开始时间，开始时间必须唯一。");
            return false;
        }
        if (task.name() == name.trimmed() && task.startTime() == start) {
            if (errorMessage) *errorMessage = QStringLiteral("任务名称和开始时间的组合必须唯一。");
            return false;
        }
    }
    m_tasks.append(Task(QUuid::createUuid().toString(QUuid::WithoutBraces), name, start, priority, category, remind, note));
    if (!save()) {
        m_tasks.removeLast();
        if (errorMessage) *errorMessage = QStringLiteral("保存任务文件失败。");
        return false;
    }
    return true;
}

bool TaskManager::deleteTask(const QString &id)
{
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks.at(i).id() == id) {
            const Task task = m_tasks.takeAt(i);
            if (save()) return true;
            m_tasks.insert(i, task);
            return false;
        }
    }
    return false;
}

bool TaskManager::deleteTaskByNameAndStart(const QString &name, const QDateTime &start,
                                           QString *errorMessage)
{
    const QString trimmedName = name.trimmed();
    for (const Task &task : m_tasks) {
        if (task.name() == trimmedName && task.startTime() == start) {
            if (deleteTask(task.id())) return true;
            if (errorMessage) *errorMessage = QStringLiteral("保存任务文件失败。");
            return false;
        }
    }
    if (errorMessage) *errorMessage = QStringLiteral("未找到指定名称和启动时间的任务。");
    return false;
}

bool TaskManager::updateTask(const QString &id, const QString &name, const QDateTime &start,
                             Priority priority, Category category, const QDateTime &remind,
                             QString *errorMessage, const QString *note)
{
    if (name.trimmed().isEmpty() || !start.isValid() || !remind.isValid()) {
        if (errorMessage) *errorMessage = QStringLiteral("请填写有效的任务名称、开始时间和提醒时间。");
        return false;
    }
    int index = -1;
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks.at(i).id() == id) { index = i; break; }
    }
    if (index < 0) {
        if (errorMessage) *errorMessage = QStringLiteral("未找到指定任务 ID。");
        return false;
    }
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (i != index && m_tasks.at(i).startTime() == start) {
            if (errorMessage) *errorMessage = QStringLiteral("已有任务使用该开始时间，开始时间必须唯一。");
            return false;
        }
    }
    const Task original = m_tasks.at(index);
    m_tasks[index] = Task(id, name, start, priority, category, remind,
                          note ? *note : original.note());
    if (save()) return true;
    m_tasks[index] = original;
    if (errorMessage) *errorMessage = QStringLiteral("保存任务文件失败。");
    return false;
}

bool TaskManager::updateTaskByNameAndStart(const QString &oldName, const QDateTime &oldStart,
                                           const QString &newName, const QDateTime &newStart,
                                           Priority priority, Category category,
                                           const QDateTime &remind, QString *errorMessage,
                                           const QString *note)
{
    const QString trimmedOldName = oldName.trimmed();
    for (const Task &task : m_tasks) {
        if (task.name() == trimmedOldName && task.startTime() == oldStart) {
            return updateTask(task.id(), newName, newStart, priority, category, remind,
                              errorMessage, note);
        }
    }
    if (errorMessage) *errorMessage = QStringLiteral("未找到指定名称和启动时间的任务。");
    return false;
}

QList<Task> TaskManager::tasksForDate(const QDate &date) const
{
    QList<Task> result;
    for (const Task &task : m_tasks) if (task.startTime().date() == date) result.append(task);
    std::sort(result.begin(), result.end(), [](const Task &a, const Task &b) { return a.startTime() < b.startTime(); });
    return result;
}

QList<Task> TaskManager::allTasks() const { return m_tasks; }
