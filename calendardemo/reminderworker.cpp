#include "reminderworker.h"
#include <QTimer>

void ReminderWorker::setTasks(const QList<Task> &tasks)
{
    m_tasks = tasks;
    QSet<QString> activeIds;
    for (const Task &task : m_tasks) activeIds.insert(task.id());
    m_remindedIds.intersect(activeIds);

    // 加载历史任务时不补发过期很久的提醒；新任务仍会在约定时间提醒。
    const QDateTime expiredBefore = QDateTime::currentDateTime().addSecs(-60);
    for (const Task &task : m_tasks) {
        if (task.remindTime() < expiredBefore) m_remindedIds.insert(task.id());
    }
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
        if (!m_remindedIds.contains(task.id()) && task.remindTime() <= now) {
            m_remindedIds.insert(task.id());
            emit reminderDue(task);
        }
    }
}
