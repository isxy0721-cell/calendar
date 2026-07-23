#ifndef TASK_H
#define TASK_H
#include <QString>
#include <QDateTime>
#include <QJsonObject>

enum Priority{High,Medium,Low};
enum Category{Study,Entertainment,Life};
class Task
{
public:
    Task();
    Task(QString id,QString name,QDateTime starttime,Priority priority=Medium,Category category=Life,QDateTime remindTime=QDateTime());
    QJsonObject toJson() const;
    static Task fromJson(const QJsonObject &obj);
    QString getId() const;
    QString getName() const;
    QDateTime getStartTIme() const;
    Priority getPriority() const;
    Category getCategory() const;
    QDateTime getRemindTime() const;
    QString priorityStr() const;
    QString categoryStr() const;
private:
    QString m_id;
    QString m_name;
    QDateTime m_startTime;
    Priority m_priority;
    Category m_category;
    QDateTime m_remindTime;
};

#endif // TASK_H
