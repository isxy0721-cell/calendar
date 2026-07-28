#include "mainwindow.h"

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMediaPlayer>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QAudioOutput>
#endif
#include <QProcess>
#include <QPlainTextEdit>
#include <QSettings>
#include <QStandardPaths>
#include <QUrl>
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
    m_reminderSoundPath = QSettings().value(QStringLiteral("reminder/soundPath")).toString();
    m_player = new QMediaPlayer(this);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);
#endif
    m_voiceProcess = new QProcess(this);
    connect(m_voiceProcess, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this, &MainWindow::voiceInputFinished);
    connect(m_voiceProcess, &QProcess::errorOccurred, this, &MainWindow::voiceProcessError);
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
    layout->setContentsMargins(28, 24, 28, 24);
    auto *header = new QHBoxLayout;
    auto *titleLayout = new QVBoxLayout;
    auto *title = new QLabel(QStringLiteral("我的日程"));
    title->setStyleSheet(QStringLiteral("font-size: 26px; font-weight: 700;"));
    auto *subtitle = new QLabel(QStringLiteral("%1 的计划安排").arg(m_currentUser));
    subtitle->setStyleSheet(QStringLiteral("color: #6b7280;"));
    titleLayout->addWidget(title);
    titleLayout->addWidget(subtitle);
    header->addLayout(titleLayout);
    header->addStretch();
    auto *soundButton = new QPushButton(QStringLiteral("提醒铃声"));
    auto *addButton = new QPushButton(QStringLiteral("＋ 添加任务"));
    addButton->setStyleSheet(QStringLiteral("QPushButton { background:#2563eb; color:white; border:0; border-radius:18px; padding:9px 18px; font-weight:600; } QPushButton:hover { background:#1d4ed8; }"));
    header->addWidget(soundButton);
    header->addWidget(addButton);
    layout->addLayout(header);
    layout->addSpacing(18);

    auto *filterLayout = new QHBoxLayout;
    auto *dateLabel = new QLabel(QStringLiteral("查看日期"));
    dateLabel->setStyleSheet(QStringLiteral("font-weight: 600;"));
    filterLayout->addWidget(dateLabel);
    m_filterDateEdit = new QDateEdit(QDate::currentDate());
    m_filterDateEdit->setCalendarPopup(true);
    m_filterDateEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    filterLayout->addWidget(m_filterDateEdit);
    auto *todayButton = new QPushButton(QStringLiteral("今天"));
    filterLayout->addWidget(todayButton);
    filterLayout->addStretch();
    m_deleteButton = new QPushButton(QStringLiteral("删除选中任务"));
    filterLayout->addWidget(m_deleteButton);
    layout->addLayout(filterLayout);

    m_taskTable = new QTableWidget(0, 7);
    m_taskTable->setHorizontalHeaderLabels({QStringLiteral("任务 ID"), QStringLiteral("任务名称"), QStringLiteral("启动时间"), QStringLiteral("提醒时间"), QStringLiteral("优先级"), QStringLiteral("分类"), QStringLiteral("备注")});
    m_taskTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_taskTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_taskTable->setAlternatingRowColors(true);
    m_taskTable->setColumnHidden(0, true);
    m_taskTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_taskTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_taskTable->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);
    m_taskTable->verticalHeader()->setVisible(false);
    layout->addWidget(m_taskTable);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::showAddTaskDialog);
    connect(soundButton, &QPushButton::clicked, this, &MainWindow::chooseReminderSound);
    connect(m_deleteButton, &QPushButton::clicked, this, &MainWindow::deleteTask);
    connect(m_filterDateEdit, &QDateEdit::dateChanged, this, &MainWindow::refreshTable);
    connect(todayButton, &QPushButton::clicked, this, [this] { m_filterDateEdit->setDate(QDate::currentDate()); });
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
    if (!m_taskManager.addTask(m_taskNameEdit->text(), m_startEdit->dateTime(), selectedPriority(), selectedCategory(), m_remindEdit->dateTime(), &error, m_noteEdit->toPlainText())) {
        QMessageBox::warning(this, QStringLiteral("添加失败"), error);
        return;
    }
    m_taskNameEdit->clear();
    m_noteEdit->clear();
    refreshTable();
    updateWorkerTasks();
}

void MainWindow::showAddTaskDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("添加任务"));
    dialog.setMinimumWidth(460);
    auto *layout = new QVBoxLayout(&dialog);
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
    m_noteEdit = new QPlainTextEdit;
    m_noteEdit->setPlaceholderText(QStringLiteral("可记录地点、准备事项或其他说明（选填）"));
    m_noteEdit->setFixedHeight(84);
    form->addRow(QStringLiteral("任务名称："), m_taskNameEdit);
    form->addRow(QStringLiteral("启动时间："), m_startEdit);
    form->addRow(QStringLiteral("提醒时间："), m_remindEdit);
    form->addRow(QStringLiteral("优先级："), m_priorityBox);
    form->addRow(QStringLiteral("分类："), m_categoryBox);
    form->addRow(QStringLiteral("备注："), m_noteEdit);
    layout->addLayout(form);
    auto *voiceButton = new QPushButton(QStringLiteral("语音识别任务名称"));
    layout->addWidget(voiceButton);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel);
    auto *saveButton = buttons->addButton(QStringLiteral("保存任务"), QDialogButtonBox::AcceptRole);
    layout->addWidget(buttons);
    connect(voiceButton, &QPushButton::clicked, this, &MainWindow::startVoiceInput);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(saveButton, &QPushButton::clicked, this, [this, &dialog] {
        const int beforeCount = m_taskManager.allTasks().size();
        addTask();
        if (m_taskManager.allTasks().size() > beforeCount) dialog.accept();
    });
    dialog.exec();
    m_taskNameEdit = nullptr;
    m_noteEdit = nullptr;
    m_startEdit = nullptr;
    m_remindEdit = nullptr;
    m_priorityBox = nullptr;
    m_categoryBox = nullptr;
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
        const QStringList cells = {task.id(), task.name(), task.startTime().toString("yyyy-MM-dd HH:mm"), task.remindTime().toString("yyyy-MM-dd HH:mm"), task.priorityText(), task.categoryText(), task.note()};
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
    if (m_reminderSoundPath.isEmpty()) {
        QApplication::beep();
    } else {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_player->setSource(QUrl::fromLocalFile(m_reminderSoundPath));
#else
        m_player->setMedia(QUrl::fromLocalFile(m_reminderSoundPath));
#endif
        m_player->play();
    }
    QMessageBox::information(this, QStringLiteral("任务提醒"),
                             QStringLiteral("提醒：%1\n开始时间：%2").arg(task.name(), task.startTime().toString("yyyy-MM-dd HH:mm")));
}

void MainWindow::chooseReminderSound()
{
    const QString file = QFileDialog::getOpenFileName(
        this, QStringLiteral("选择提醒音频"), m_reminderSoundPath,
        QStringLiteral("音频文件 (*.wav *.mp3 *.ogg *.m4a);;所有文件 (*.*)"));
    if (file.isEmpty()) return;
    m_reminderSoundPath = file;
    QSettings().setValue(QStringLiteral("reminder/soundPath"), file);
    QMessageBox::information(this, QStringLiteral("设置成功"), QStringLiteral("提醒铃声已保存。"));
}

void MainWindow::startVoiceInput()
{
#ifdef Q_OS_WIN
    if (m_voiceProcess->state() != QProcess::NotRunning) return;
    const QString script = QStringLiteral(
        "$ErrorActionPreference='Stop'; "
        "Add-Type -AssemblyName System.Speech; "
        "$r=New-Object System.Speech.Recognition.SpeechRecognitionEngine; "
        "$r.LoadGrammar((New-Object System.Speech.Recognition.DictationGrammar)); "
        "[Console]::OutputEncoding=New-Object System.Text.UTF8Encoding; "
        "$x=$r.Recognize([TimeSpan]::FromSeconds(8)); "
        "if($x){Write-Output ('RESULT:'+$x.Text)}");
    m_voiceProcess->start(QStringLiteral("powershell.exe"),
                          {QStringLiteral("-NoProfile"), QStringLiteral("-ExecutionPolicy"), QStringLiteral("Bypass"),
                           QStringLiteral("-Command"), script});
    QMessageBox::information(this, QStringLiteral("语音输入"),
                             QStringLiteral("请在 8 秒内说出任务名称。识别完成后会自动填入名称输入框。"));
#elif defined(Q_OS_LINUX)
    if (m_voiceProcess->state() != QProcess::NotRunning) return;
    const QString script = linuxVoiceScriptPath();
    if (script.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("语音识别不可用"),
                             QStringLiteral("未找到 linux_voice.py。请将它放在程序可执行文件旁或当前工作目录。"));
        return;
    }
    m_linuxVoiceAudioFile = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
                               .filePath(QStringLiteral("myschedule-voice.wav"));
    m_linuxRecording = true;
    m_voiceProcess->start(QStringLiteral("arecord"),
                          {QStringLiteral("-q"), QStringLiteral("-f"), QStringLiteral("S16_LE"),
                           QStringLiteral("-r"), QStringLiteral("16000"), QStringLiteral("-c"), QStringLiteral("1"),
                           QStringLiteral("-d"), QStringLiteral("8"), m_linuxVoiceAudioFile});
    QMessageBox::information(this, QStringLiteral("语音输入"),
                             QStringLiteral("请在 8 秒内说出任务名称。正在使用本机离线 Vosk 识别。"));
#else
    QMessageBox::information(this, QStringLiteral("语音输入"),
                             QStringLiteral("当前平台暂未支持语音识别。Windows 使用 System.Speech，Linux 使用 Vosk。"));
#endif
}

void MainWindow::voiceInputFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (m_linuxRecording) {
        m_linuxRecording = false;
        if (exitStatus != QProcess::NormalExit || exitCode != 0) {
            const QString error = QString::fromLocal8Bit(m_voiceProcess->readAllStandardError()).trimmed();
            QMessageBox::warning(this, QStringLiteral("录音失败"),
                                 QStringLiteral("无法使用 Linux 录音设备。请安装 alsa-utils，并检查麦克风权限。%1").arg(error));
            return;
        }
        m_voiceProcess->start(QStringLiteral("python3"), {linuxVoiceScriptPath(), m_linuxVoiceAudioFile});
        return;
    }
    const QString output = QString::fromUtf8(m_voiceProcess->readAllStandardOutput()).trimmed();
    const QString error = QString::fromLocal8Bit(m_voiceProcess->readAllStandardError()).trimmed();
    const int resultAt = output.indexOf(QStringLiteral("RESULT:"));
    if (exitStatus == QProcess::NormalExit && exitCode == 0 && resultAt >= 0 && m_taskNameEdit) {
        m_taskNameEdit->setText(output.mid(resultAt + 7).trimmed());
        return;
    }
    QMessageBox::warning(this, QStringLiteral("语音识别失败"),
                         QStringLiteral("未识别到语音，或系统未安装可用的语音识别语言。%1").arg(error));
}

void MainWindow::voiceProcessError(QProcess::ProcessError error)
{
    if (error != QProcess::FailedToStart) return;
#ifdef Q_OS_LINUX
    if (m_linuxRecording) {
        m_linuxRecording = false;
        QMessageBox::warning(this, QStringLiteral("录音工具不可用"),
                             QStringLiteral("找不到 arecord。请安装 alsa-utils：sudo apt install alsa-utils"));
        return;
    }
    QMessageBox::warning(this, QStringLiteral("Vosk 不可用"),
                         QStringLiteral("找不到 python3。请安装 Python 3，并执行 python3 -m pip install vosk。"));
#else
    QMessageBox::warning(this, QStringLiteral("语音识别不可用"),
                         QStringLiteral("无法启动系统语音识别服务。"));
#endif
}

QString MainWindow::linuxVoiceScriptPath() const
{
    const QString fileName = QStringLiteral("linux_voice.py");
    const QString besideProgram = QDir(QCoreApplication::applicationDirPath()).filePath(fileName);
    if (QFileInfo::exists(besideProgram)) return besideProgram;
    const QString workingDirectory = QDir::current().filePath(fileName);
    return QFileInfo::exists(workingDirectory) ? workingDirectory : QString();
}
