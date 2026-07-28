#include "vosktranscriber.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLibrary>
#include <QtEndian>

namespace {
struct VoskModel;
struct VoskRecognizer;
using ModelNew = VoskModel *(*)(const char *);
using ModelFree = void (*)(VoskModel *);
using RecognizerNew = VoskRecognizer *(*)(VoskModel *, float);
using RecognizerFree = void (*)(VoskRecognizer *);
using AcceptWaveform = int (*)(VoskRecognizer *, const short *, int);
using FinalResult = const char *(*)(VoskRecognizer *);
using SetLogLevel = void (*)(int);

bool readWavSamples(const QString &filePath, QByteArray *samples, QString *error)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) { *error = QStringLiteral("无法读取录音文件。"); return false; }
    const QByteArray wav = file.readAll();
    if (wav.size() < 44 || wav.left(4) != "RIFF" || wav.mid(8, 4) != "WAVE") {
        *error = QStringLiteral("录音文件不是有效的 WAV 格式。"); return false;
    }
    int offset = 12;
    while (offset + 8 <= wav.size()) {
        const QByteArray id = wav.mid(offset, 4);
        const quint32 size = qFromLittleEndian<quint32>(reinterpret_cast<const uchar *>(wav.constData() + offset + 4));
        offset += 8;
        if (offset + static_cast<int>(size) > wav.size()) break;
        if (id == "data") { *samples = wav.mid(offset, size); return true; }
        offset += static_cast<int>(size) + (size % 2);
    }
    *error = QStringLiteral("WAV 文件中没有音频数据。");
    return false;
}
}

void VoskTranscriber::transcribe(const QString &wavFilePath)
{
#ifndef Q_OS_LINUX
    emit transcriptionFinished({}, QStringLiteral("Vosk 离线识别仅在 Linux 后端使用。"));
    return;
#else
    QByteArray samples;
    QString error;
    if (!readWavSamples(wavFilePath, &samples, &error)) { emit transcriptionFinished({}, error); return; }

    const QString libraryPath = qEnvironmentVariable("MYSCHEDULE_VOSK_LIBRARY", QStringLiteral("vosk"));
    QLibrary library(libraryPath);
    if (!library.load()) {
        emit transcriptionFinished({}, QStringLiteral("无法加载 libvosk.so。请设置 MYSCHEDULE_VOSK_LIBRARY 或 LD_LIBRARY_PATH。"));
        return;
    }
    const auto modelNew = reinterpret_cast<ModelNew>(library.resolve("vosk_model_new"));
    const auto modelFree = reinterpret_cast<ModelFree>(library.resolve("vosk_model_free"));
    const auto recognizerNew = reinterpret_cast<RecognizerNew>(library.resolve("vosk_recognizer_new"));
    const auto recognizerFree = reinterpret_cast<RecognizerFree>(library.resolve("vosk_recognizer_free"));
    const auto acceptWaveform = reinterpret_cast<AcceptWaveform>(library.resolve("vosk_recognizer_accept_waveform_s"));
    const auto finalResult = reinterpret_cast<FinalResult>(library.resolve("vosk_recognizer_final_result"));
    const auto setLogLevel = reinterpret_cast<SetLogLevel>(library.resolve("vosk_set_log_level"));
    if (!modelNew || !modelFree || !recognizerNew || !recognizerFree || !acceptWaveform || !finalResult) {
        emit transcriptionFinished({}, QStringLiteral("libvosk.so 缺少所需的 Vosk C API。"));
        return;
    }
    const QString modelPath = qEnvironmentVariable("MYSCHEDULE_VOSK_MODEL");
    if (modelPath.isEmpty()) {
        emit transcriptionFinished({}, QStringLiteral("请设置 MYSCHEDULE_VOSK_MODEL 为解压后的 Vosk 中文模型目录。"));
        return;
    }
    if (setLogLevel) setLogLevel(-1);
    VoskModel *model = modelNew(modelPath.toUtf8().constData());
    if (!model) { emit transcriptionFinished({}, QStringLiteral("无法加载 Vosk 模型：%1").arg(modelPath)); return; }
    VoskRecognizer *recognizer = recognizerNew(model, 16000.0f);
    if (!recognizer) { modelFree(model); emit transcriptionFinished({}, QStringLiteral("无法创建 Vosk 识别器。")); return; }
    acceptWaveform(recognizer, reinterpret_cast<const short *>(samples.constData()), samples.size() / static_cast<int>(sizeof(short)));
    const QJsonObject result = QJsonDocument::fromJson(QByteArray(finalResult(recognizer))).object();
    recognizerFree(recognizer);
    modelFree(model);
    const QString text = result.value(QStringLiteral("text")).toString().trimmed();
    emit transcriptionFinished(text, text.isEmpty() ? QStringLiteral("没有识别到语音，请靠近麦克风后重试。") : QString());
#endif
}
