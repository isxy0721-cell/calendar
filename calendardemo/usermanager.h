#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QString>
#include <QMap>

class UserManager
{
public:
    UserManager();

    // 注册新用户：成功返回true，失败（用户名已存在/空）返回false
    bool registerUser(const QString &username, const QString &password);

    // 登录校验：用户名密码正确返回true
    bool login(const QString &username, const QString &password) const;

    // 检查用户名是否已存在
    bool isUserExists(const QString &username) const;

private:
    // 存储所有用户：key=用户名，value=密码哈希值
    QMap<QString, QString> m_users;

    // 密码SHA256哈希加密（明文转乱码，无法反向破解）
    QString hashPassword(const QString &password) const;

    // 从本地JSON文件加载所有用户
    void loadFromFile();
    // 保存所有用户到本地JSON文件
    void saveToFile() const;

    // 用户数据文件的本地路径
    QString m_dataFilePath;
};

#endif // USERMANAGER_H
