#include "task.h"

Task::Task() = default;

Task::Task(const QString &id, const QString &name, const QDateTime &startTime,
           Priority priority, Category category, const QDateTime &remindTime, const QString &note)
    : m_id(id), m_name(name.trimmed()), m_startTime(startTime), m_priority(priority),
      m_category(category), m_remindTime(remindTime), m_note(note.trimmed())
{
    if (!m_remindTime.isValid())
        m_remindTime = startTime.addSecs(-5 * 60);
}

QJsonObject Task::toJson() const
{
    return {{"id", m_id}, {"name", m_name},
            {"startTime", m_startTime.toString(Qt::ISODate)},
            {"priority", static_cast<int>(m_priority)},
            {"category", static_cast<int>(m_category)},
            {"remindTime", m_remindTime.toString(Qt::ISODate)}, {"note", m_note}};
}

Task Task::fromJson(const QJsonObject &object)
{
    return Task(object.value("id").toString(), object.value("name").toString(),
                QDateTime::fromString(object.value("startTime").toString(), Qt::ISODate),
                static_cast<Priority>(object.value("priority").toInt(static_cast<int>(Priority::Medium))),
                static_cast<Category>(object.value("category").toInt(static_cast<int>(Category::Life))),
                QDateTime::fromString(object.value("remindTime").toString(), Qt::ISODate),
                object.value("note").toString());
}

QString Task::id() const { return m_id; }
QString Task::name() const { return m_name; }
QDateTime Task::startTime() const { return m_startTime; }
Priority Task::priority() const { return m_priority; }
Category Task::category() const { return m_category; }
QDateTime Task::remindTime() const { return m_remindTime; }
QString Task::note() const { return m_note; }

QString Task::priorityText() const
{
    switch (m_priority) {
    case Priority::High: return QStringLiteral("高");
    case Priority::Medium: return QStringLiteral("中");
    case Priority::Low: return QStringLiteral("低");
    }
    return {};
}

QString Task::categoryText() const
{
    switch (m_category) {
    case Category::Study: return QStringLiteral("学习");
    case Category::Entertainment: return QStringLiteral("娱乐");
    case Category::Life: return QStringLiteral("生活");
    }
    return {};
}
