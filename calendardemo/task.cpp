#include "task.h"

Task::Task() : m_priority(Medium), m_category(Life) {}

Task::Task(QString id, QString name, QDateTime startTime,
           Priority priority, Category category, QDateTime remindTime)
    : m_id(id), m_name(name), m_startTime(startTime),
      m_priority(priority), m_category(category), m_remindTime(remindTime)
{
    // 默认值：提醒时间 = 开始时间提前5分钟
    if (!m_remindTime.isValid())
        m_remindTime = startTime.addSecs(-5 * 60);
}

QJsonObject Task::toJson() const
{
    QJsonObject obj;
    obj["id"] = m_id;
    obj["name"] = m_name;
    obj["startTime"] = m_startTime.toString(Qt::ISODate);
    obj["priority"] = m_priority;
    obj["category"] = m_category;
    obj["remindTime"] = m_remindTime.toString(Qt::ISODate);
    return obj;
}

Task Task::fromJson(const QJsonObject &obj)
{
    Task task;
    task.m_id = obj["id"].toString();
    task.m_name = obj["name"].toString();
    task.m_startTime = QDateTime::fromString(obj["startTime"].toString(), Qt::ISODate);
    task.m_priority = static_cast<Priority>(obj["priority"].toInt());
    task.m_category = static_cast<Category>(obj["category"].toInt());
    task.m_remindTime = QDateTime::fromString(obj["remindTime"].toString(), Qt::ISODate);
    return task;
}

QString Task::getId() const { return m_id; }
QString Task::getName() const { return m_name; }
QDateTime Task::getStartTime() const { return m_startTime; }
Priority Task::getPriority() const { return m_priority; }
Category Task::getCategory() const { return m_category; }
QDateTime Task::getRemindTime() const { return m_remindTime; }

QString Task::priorityStr() const
{
    switch (m_priority) {
    case High: return "高";
    case Medium: return "中";
    case Low: return "低";
    }
    return "中";
}

QString Task::categoryStr() const
{
    switch (m_category) {
    case Study: return "学习";
    case Entertainment: return "娱乐";
    case Life: return "生活";
    }
    return "生活";
}
