#include "mainwindow.h"

#include <QComboBox>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    qRegisterMetaType<Task>("Task");
    qRegisterMetaType<QList<Task>>("QList<Task>");
    setMinimumSize(900, 560);
    setWindowTitle(QStringLiteral("我的日程管理"));
    buildLoginPage();
    m_reminderWorker = new ReminderWorker;
    m_reminderWorker->moveToThread(&m_reminderThread);
    connect(&m_reminderThread, &QThread::started, m_reminderWorker, &ReminderWorker::start);
    connect(m_reminderWorker, &ReminderWorker::reminderDue, this, &MainWindow::showReminder);
    m_reminderThread.start();
}

MainWindow::~MainWindow()
{
    m_reminderThread.quit();
    m_reminderThread.wait();
    delete m_reminderWorker;
}

void MainWindow::buildLoginPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(250, 90, 250, 90);
    auto *title = new QLabel(QStringLiteral("我的日程管理"));
    title->setStyleSheet(QStringLiteral("font-size: 26px; font-weight: bold;"));
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);
    layout->addSpacing(22);
    auto *form = new QFormLayout;
    m_userEdit = new QLineEdit;
    m_userEdit->setPlaceholderText(QStringLiteral("输入用户名"));
    m_passwordEdit = new QLineEdit;
    m_passwordEdit->setPlaceholderText(QStringLiteral("输入口令"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    form->addRow(QStringLiteral("用户名："), m_userEdit);
    form->addRow(QStringLiteral("口令："), m_passwordEdit);
    layout->addLayout(form);
    auto *loginButton = new QPushButton(QStringLiteral("登录"));
    auto *registerButton = new QPushButton(QStringLiteral("注册新账户"));
    layout->addWidget(loginButton);
    layout->addWidget(registerButton);
    layout->addStretch();
    connect(loginButton, &QPushButton::clicked, this, &MainWindow::login);
    connect(registerButton, &QPushButton::clicked, this, &MainWindow::registerUser);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &MainWindow::login);
    setCentralWidget(page);
}

void MainWindow::buildSchedulePage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    auto *welcome = new QLabel(QStringLiteral("当前用户：%1").arg(m_currentUser));
    welcome->setStyleSheet(QStringLiteral("font-size: 17px; font-weight: bold;"));
    layout->addWidget(welcome);

    auto *form = new QFormLayout;
    m_taskNameEdit = new QLineEdit;
    m_taskNameEdit->setPlaceholderText(QStringLiteral("例如：完成 Qt 作业"));
    m_startEdit = new QDateTimeEdit(QDateTime::currentDateTime());
    m_remindEdit = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(-5 * 60));
    for (QDateTimeEdit *edit : {m_startEdit, m_remindEdit}) {
        edit->setCalendarPopup(true);
        edit->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm"));
    }
    m_priorityBox = new QComboBox;
    m_priorityBox->addItems({QStringLiteral("中（默认）"), QStringLiteral("高"), QStringLiteral("低")});
    m_categoryBox = new QComboBox;
    m_categoryBox->addItems({QStringLiteral("生活（默认）"), QStringLiteral("学习"), QStringLiteral("娱乐")});
    form->addRow(QStringLiteral("任务名称："), m_taskNameEdit);
    form->addRow(QStringLiteral("启动时间："), m_startEdit);
    form->addRow(QStringLiteral("提醒时间："), m_remindEdit);
    form->addRow(QStringLiteral("优先级："), m_priorityBox);
    form->addRow(QStringLiteral("分类："), m_categoryBox);
    layout->addLayout(form);
    auto *addButton = new QPushButton(QStringLiteral("添加任务并自动保存"));
    layout->addWidget(addButton);

    auto *filterLayout = new QHBoxLayout;
    filterLayout->addWidget(new QLabel(QStringLiteral("显示日期：")));
    m_filterDateEdit = new QDateEdit(QDate::currentDate());
    m_filterDateEdit->setCalendarPopup(true);
    m_filterDateEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    filterLayout->addWidget(m_filterDateEdit);
    filterLayout->addStretch();
    m_deleteButton = new QPushButton(QStringLiteral("删除选中任务"));
    filterLayout->addWidget(m_deleteButton);
    layout->addLayout(filterLayout);

    m_taskTable = new QTableWidget(0, 6);
    m_taskTable->setHorizontalHeaderLabels({QStringLiteral("任务 ID"), QStringLiteral("任务名称"), QStringLiteral("启动时间"), QStringLiteral("提醒时间"), QStringLiteral("优先级"), QStringLiteral("分类")});
    m_taskTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_taskTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_taskTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(m_taskTable);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addTask);
    connect(m_deleteButton, &QPushButton::clicked, this, &MainWindow::deleteTask);
    connect(m_filterDateEdit, &QDateEdit::dateChanged, this, &MainWindow::refreshTable);
    setCentralWidget(page);
    refreshTable();
}

void MainWindow::registerUser()
{
    if (m_userManager.registerUser(m_userEdit->text(), m_passwordEdit->text())) {
        QMessageBox::information(this, QStringLiteral("注册成功"), QStringLiteral("账户已创建。口令只保存 SHA-256 摘要，不保存明文。"));
    } else {
        QMessageBox::warning(this, QStringLiteral("注册失败"), QStringLiteral("用户名不能为空、口令不能为空，且用户名不能重复。"));
    }
}

void MainWindow::login()
{
    if (!m_userManager.login(m_userEdit->text(), m_passwordEdit->text())) {
        QMessageBox::warning(this, QStringLiteral("登录失败"), QStringLiteral("用户名或口令不正确。"));
        return;
    }
    m_currentUser = m_userEdit->text().trimmed();
    m_taskManager.load(m_currentUser);
    buildSchedulePage();
    updateWorkerTasks();
}

Priority MainWindow::selectedPriority() const
{
    return m_priorityBox->currentIndex() == 1 ? Priority::High : m_priorityBox->currentIndex() == 2 ? Priority::Low : Priority::Medium;
}

Category MainWindow::selectedCategory() const
{
    return m_categoryBox->currentIndex() == 1 ? Category::Study : m_categoryBox->currentIndex() == 2 ? Category::Entertainment : Category::Life;
}

void MainWindow::addTask()
{
    QString error;
    if (!m_taskManager.addTask(m_taskNameEdit->text(), m_startEdit->dateTime(), selectedPriority(), selectedCategory(), m_remindEdit->dateTime(), &error)) {
        QMessageBox::warning(this, QStringLiteral("添加失败"), error);
        return;
    }
    m_taskNameEdit->clear();
    refreshTable();
    updateWorkerTasks();
}

void MainWindow::deleteTask()
{
    const int row = m_taskTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择一条任务。"));
        return;
    }
    const QString id = m_taskTable->item(row, 0)->text();
    if (m_taskManager.deleteTask(id)) {
        refreshTable();
        updateWorkerTasks();
    } else {
        QMessageBox::warning(this, QStringLiteral("删除失败"), QStringLiteral("无法保存删除结果。"));
    }
}

void MainWindow::refreshTable()
{
    const QList<Task> tasks = m_taskManager.tasksForDate(m_filterDateEdit->date());
    m_taskTable->setRowCount(tasks.size());
    for (int row = 0; row < tasks.size(); ++row) {
        const Task &task = tasks.at(row);
        const QStringList cells = {task.id(), task.name(), task.startTime().toString("yyyy-MM-dd HH:mm"), task.remindTime().toString("yyyy-MM-dd HH:mm"), task.priorityText(), task.categoryText()};
        for (int column = 0; column < cells.size(); ++column)
            m_taskTable->setItem(row, column, new QTableWidgetItem(cells.at(column)));
    }
}

void MainWindow::updateWorkerTasks()
{
    QMetaObject::invokeMethod(m_reminderWorker, "setTasks", Qt::QueuedConnection,
                              Q_ARG(QList<Task>, m_taskManager.allTasks()));
}

void MainWindow::showReminder(const Task &task)
{
    QMessageBox::information(this, QStringLiteral("任务提醒"),
                             QStringLiteral("提醒：%1\n开始时间：%2").arg(task.name(), task.startTime().toString("yyyy-MM-dd HH:mm")));
}
