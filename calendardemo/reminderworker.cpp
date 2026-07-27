#include "reminderworker.h"
#include <QTimer>

void ReminderWorker::setTasks(const QList<Task> &tasks)
{
    m_tasks = tasks;
    m_remindedIds.clear();
}

void ReminderWorker::start()
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ReminderWorker::checkTasks);
    m_timer->start(1000);
    checkTasks();
}

void ReminderWorker::checkTasks()
{
    const QDateTime now = QDateTime::currentDateTime();
    for (const Task &task : m_tasks) {
        if (!m_remindedIds.contains(task.id()) && task.remindTime() <= now && task.startTime() >= now.addSecs(-60)) {
            m_remindedIds.insert(task.id());
            emit reminderDue(task);
        }
    }
}
