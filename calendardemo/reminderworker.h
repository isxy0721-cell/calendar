#ifndef REMINDERWORKER_H
#define REMINDERWORKER_H

#include "task.h"
#include <QObject>
#include <QSet>

class QTimer;
class ReminderWorker : public QObject
{
    Q_OBJECT
public slots:
    void setTasks(const QList<Task> &tasks);
    void start();
signals:
    void reminderDue(const Task &task);
private slots:
    void checkTasks();
private:
    QList<Task> m_tasks;
    QSet<QString> m_remindedIds;
    QTimer *m_timer = nullptr;
};

#endif
