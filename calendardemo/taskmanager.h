#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "task.h"
#include <QList>

class TaskManager
{
public:
    bool load(const QString &username);
    bool addTask(const QString &name, const QDateTime &start, Priority priority,
                 Category category, const QDateTime &remind, QString *errorMessage,
                 const QString &note = QString());
    bool updateTask(const QString &id, const QString &name, const QDateTime &start,
                    Priority priority, Category category, const QDateTime &remind,
                    QString *errorMessage, const QString *note = nullptr);
    bool deleteTask(const QString &id);
    bool updateTaskByNameAndStart(const QString &oldName, const QDateTime &oldStart,
                                  const QString &newName, const QDateTime &newStart,
                                  Priority priority, Category category, const QDateTime &remind,
                                  QString *errorMessage, const QString *note = nullptr);
    bool deleteTaskByNameAndStart(const QString &name, const QDateTime &start,
                                  QString *errorMessage);
    QList<Task> tasksForDate(const QDate &date) const;
    QList<Task> allTasks() const;

private:
    bool save() const;
    QList<Task> m_tasks;
    QString m_filePath;
};

#endif
