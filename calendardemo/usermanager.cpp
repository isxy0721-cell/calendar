#include "usermanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>  // 新增
#include <QCryptographicHash>
#include <QDir>
#include <QDebug>

UserManager::UserManager()
{
    // 数据文件路径：程序运行目录下的 data/users.json
    m_dataFilePath = QDir::currentPath() + "/data/users.json";

    // 确保data文件夹存在，不存在就自动创建
    QDir dir;
    dir.mkpath("data");

    // 构造对象时自动加载已有用户
    loadFromFile();
}

// 密码SHA256哈希加密
QString UserManager::hashPassword(const QString &password) const
{
    QByteArray byteArray = password.toUtf8();
    QByteArray hash = QCryptographicHash::hash(byteArray, QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

// 从文件加载所有用户数据
void UserManager::loadFromFile()
{
    QFile file(m_dataFilePath);
    // 文件不存在直接返回，保持空用户列表
    if (!file.exists()) {
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "打开用户文件失败";
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();
    QJsonArray userArray = root["users"].toArray();

    m_users.clear();
    for (const auto &userVal : userArray) {
        QJsonObject userObj = userVal.toObject();
        QString username = userObj["username"].toString();
        QString passwordHash = userObj["passwordHash"].toString();
        m_users[username] = passwordHash;
    }
    qDebug() << "加载用户完成，共" << m_users.size() << "个用户";
}

// 保存所有用户数据到文件
void UserManager::saveToFile() const
{
    QJsonArray userArray;
    // 遍历所有用户，转成JSON数组
    for (auto it = m_users.begin(); it != m_users.end(); ++it) {
        QJsonObject userObj;
        userObj["username"] = it.key();
        userObj["passwordHash"] = it.value();
        userArray.append(userObj);
    }

    QJsonObject root;
    root["users"] = userArray;

    QJsonDocument doc(root);
    QFile file(m_dataFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "保存用户文件失败";
        return;
    }

    file.write(doc.toJson());
    file.close();
    qDebug() << "用户数据已保存到" << m_dataFilePath;
}

// 检查用户名是否已存在
bool UserManager::isUserExists(const QString &username) const
{
    return m_users.contains(username);
}

// 注册新用户
bool UserManager::registerUser(const QString &username, const QString &password)
{
    // 空用户名/空密码不允许注册
    if (username.isEmpty() || password.isEmpty()) {
        return false;
    }

    // 用户名已存在，注册失败
    if (isUserExists(username)) {
        return false;
    }

    // 密码哈希后存入内存
    QString hash = hashPassword(password);
    m_users[username] = hash;

    // 自动持久化到本地文件
    saveToFile();
    return true;
}

// 登录校验
bool UserManager::login(const QString &username, const QString &password) const
{
    // 用户不存在直接失败
    if (!isUserExists(username)) {
        return false;
    }

    // 计算输入密码的哈希，和存储的哈希做比对
    QString inputHash = hashPassword(password);
    return m_users[username] == inputHash;
}
