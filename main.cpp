#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QMediaDevices>
#include <QAudioDevice>
#include <iostream>

// Dummy device manager
class AudioDeviceManager : public QObject {
    Q_OBJECT
public:
    Q_INVOKABLE QVariantList getAudioDevices() {
        QVariantList devices;
        const auto deviceInfos = QMediaDevices::audioInputs();
        for (const QAudioDevice &deviceInfo:deviceInfos)
            devices<<deviceInfo.description();
        return devices;
    }
};


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    AudioDeviceManager audioManager;
    engine.rootContext()->setContextProperty("audioManager", &audioManager);
    qmlRegisterType<AudioDeviceManager>("audioManager", 1, 0, "AudioDeviceManager");

    const QUrl url(QStringLiteral("qrc:/Shall_be_recorded/main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
#include "main.moc"
