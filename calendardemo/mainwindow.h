#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "reminderworker.h"
#include "taskmanager.h"
#include "usermanager.h"
#include "vosktranscriber.h"
#include <QMainWindow>
#include <QMediaPlayer>
#include <QProcess>
#include <QThread>

class QDateEdit;
class QDateTimeEdit;
class QLineEdit;
class QComboBox;
class QTableWidget;
class QPushButton;
class QPlainTextEdit;

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
    void showAddTaskDialog();
    void toggleTaskSearch();
    void deleteTask();
    void refreshTable();
    void showReminder(const Task &task);
    void handleAudioError(QMediaPlayer::Error error);
    void chooseReminderSound();
    void startVoiceInput();
    void voiceInputFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void voiceProcessError(QProcess::ProcessError error);
    void voskTranscriptionFinished(const QString &text, const QString &errorMessage);

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
    QLineEdit *m_searchEdit = nullptr;
    QLineEdit *m_taskNameEdit = nullptr;
    QPlainTextEdit *m_noteEdit = nullptr;
    QDateTimeEdit *m_startEdit = nullptr;
    QDateTimeEdit *m_remindEdit = nullptr;
    QDateEdit *m_filterDateEdit = nullptr;
    QComboBox *m_priorityBox = nullptr;
    QComboBox *m_categoryBox = nullptr;
    QTableWidget *m_taskTable = nullptr;
    QPushButton *m_deleteButton = nullptr;
    QMediaPlayer *m_player = nullptr;
    QProcess *m_voiceProcess = nullptr;
    QString m_reminderSoundPath;
    QString m_linuxVoiceAudioFile;
    bool m_linuxRecording = false;
    QThread m_reminderThread;
    ReminderWorker *m_reminderWorker = nullptr;
    QThread m_voskThread;
    VoskTranscriber *m_voskTranscriber = nullptr;
};

#endif
