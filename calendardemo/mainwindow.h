#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "reminderworker.h"
#include "taskmanager.h"
#include "usermanager.h"
#include <QMainWindow>
#include <QThread>

class QDateEdit;
class QDateTimeEdit;
class QLineEdit;
class QComboBox;
class QTableWidget;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void login();
    void registerUser();
    void addTask();
    void deleteTask();
    void refreshTable();
    void showReminder(const Task &task);

private:
    void buildLoginPage();
    void buildSchedulePage();
    void updateWorkerTasks();
    Priority selectedPriority() const;
    Category selectedCategory() const;

    UserManager m_userManager;
    TaskManager m_taskManager;
    QString m_currentUser;
    QLineEdit *m_userEdit = nullptr;
    QLineEdit *m_passwordEdit = nullptr;
    QLineEdit *m_taskNameEdit = nullptr;
    QDateTimeEdit *m_startEdit = nullptr;
    QDateTimeEdit *m_remindEdit = nullptr;
    QDateEdit *m_filterDateEdit = nullptr;
    QComboBox *m_priorityBox = nullptr;
    QComboBox *m_categoryBox = nullptr;
    QTableWidget *m_taskTable = nullptr;
    QPushButton *m_deleteButton = nullptr;
    QThread m_reminderThread;
    ReminderWorker *m_reminderWorker = nullptr;
};

#endif
