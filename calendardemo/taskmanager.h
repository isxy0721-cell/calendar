#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "task.h"
#include <QList>

class TaskManager
{
public:
    bool load(const QString &username);
    bool addTask(const QString &name, const QDateTime &start, Priority priority,
                 Category category, const QDateTime &remind, QString *errorMessage);
    bool deleteTask(const QString &id);
    QList<Task> tasksForDate(const QDate &date) const;
    QList<Task> allTasks() const;

private:
    bool save() const;
    QList<Task> m_tasks;
    QString m_filePath;
};

#endif
