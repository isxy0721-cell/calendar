#ifndef TASK_H
#define TASK_H

#include <QDateTime>
#include <QJsonObject>
#include <QList>
#include <QMetaType>
#include <QString>

enum class Priority { High, Medium, Low };
enum class Category { Study, Entertainment, Life };

class Task
{
public:
    Task();
    Task(const QString &id, const QString &name, const QDateTime &startTime,
         Priority priority = Priority::Medium, Category category = Category::Life,
         const QDateTime &remindTime = QDateTime(), const QString &note = QString());

    QJsonObject toJson() const;
    static Task fromJson(const QJsonObject &object);

    QString id() const;
    QString name() const;
    QDateTime startTime() const;
    Priority priority() const;
    Category category() const;
    QDateTime remindTime() const;
    QString note() const;
    QString priorityText() const;
    QString categoryText() const;

private:
    QString m_id;
    QString m_name;
    QDateTime m_startTime;
    Priority m_priority = Priority::Medium;
    Category m_category = Category::Life;
    QDateTime m_remindTime;
    QString m_note;
};

Q_DECLARE_METATYPE(Task)
Q_DECLARE_METATYPE(QList<Task>)

#endif
