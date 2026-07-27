#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QMap>
#include <QString>

class UserManager
{
public:
    UserManager();
    bool registerUser(const QString &username, const QString &password);
    bool login(const QString &username, const QString &password) const;

private:
    QString hashPassword(const QString &password) const;
    void loadFromFile();
    bool saveToFile() const;
    QMap<QString, QString> m_users;
    QString m_dataFilePath;
};

#endif
