#include "usermanager.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

UserManager::UserManager()
{
    QDir().mkpath(QStringLiteral("data"));
    m_dataFilePath = QDir::current().filePath(QStringLiteral("data/users.json"));
    loadFromFile();
}

QString UserManager::hashPassword(const QString &password) const
{
    return QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();
}

void UserManager::loadFromFile()
{
    QFile file(m_dataFilePath);
    if (!file.open(QIODevice::ReadOnly)) return;
    const QJsonArray users = QJsonDocument::fromJson(file.readAll()).object().value("users").toArray();
    for (const QJsonValue &value : users) {
        const QJsonObject user = value.toObject();
        m_users.insert(user.value("username").toString(), user.value("passwordHash").toString());
    }
}

bool UserManager::saveToFile() const
{
    QJsonArray users;
    for (auto it = m_users.cbegin(); it != m_users.cend(); ++it)
        users.append(QJsonObject{{"username", it.key()}, {"passwordHash", it.value()}});
    QSaveFile file(m_dataFilePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(QJsonDocument(QJsonObject{{"users", users}}).toJson());
    return file.commit();
}

bool UserManager::registerUser(const QString &username, const QString &password)
{
    const QString trimmed = username.trimmed();
    if (trimmed.isEmpty() || password.isEmpty() || m_users.contains(trimmed)) return false;
    m_users.insert(trimmed, hashPassword(password));
    return saveToFile();
}

bool UserManager::login(const QString &username, const QString &password) const
{
    return m_users.value(username.trimmed()) == hashPassword(password) && !username.trimmed().isEmpty();
}
