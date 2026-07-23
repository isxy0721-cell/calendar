// 引入核心类
#include <QCoreApplication>
#include <QDebug>       // 控制台打印输出工具
#include <QDateTime>
#include "task.h"        // 引入我们写的任务类

int main(int argc, char *argv[])
{
    // 控制台应用，不启动图形窗口
    QCoreApplication a(argc, argv);

    qDebug() << "===== 开始测试Task类 =====";

    // 1. 创建一条测试任务（带全部参数）
    Task testTask(
        "task_001",
        "学习Qt CMake开发",
        QDateTime::currentDateTime(), // 当前系统时间
        High,
        Study
        // 不传入提醒时间，自动提前5分钟
    );

    // 2. 打印任务基础信息，测试Getter函数
    qDebug() << "任务ID：" << testTask.getId();
    qDebug() << "任务名称：" << testTask.getName();
    qDebug() << "优先级文字：" << testTask.priorityStr();
    qDebug() << "分类文字：" << testTask.categoryStr();
    qDebug() << "任务开始时间：" << testTask.getStartTime().toString("yyyy-MM-dd HH:mm");
    qDebug() << "提醒时间：" << testTask.getRemindTime().toString("yyyy-MM-dd HH:mm");

    // 3. 测试序列化：Task对象转JSON
    QJsonObject jsonData = testTask.toJson();
    qDebug() << "\n序列化后的JSON数据：" << jsonData;

    // 4. 测试反序列化：JSON还原成新Task对象
    Task newTask = Task::fromJson(jsonData);
    qDebug() << "\n反序列化还原后的任务名称：" << newTask.getName();
    qDebug() << "还原后的优先级：" << newTask.priorityStr();

    qDebug() << "===== Task全部测试完成，无报错 =====";

    return a.exec();
}
