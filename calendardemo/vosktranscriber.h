#ifndef VOSKTRANSCRIBER_H
#define VOSKTRANSCRIBER_H

#include <QObject>
#include <QString>

// 不包含 Vosk 头文件：运行时动态加载 libvosk.so，因此开发机无需安装 Vosk。
class VoskTranscriber : public QObject
{
    Q_OBJECT
public slots:
    void transcribe(const QString &wavFilePath);
signals:
    void transcriptionFinished(const QString &text, const QString &errorMessage);
};

#endif
