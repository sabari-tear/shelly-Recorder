#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QObject>
#include <QString>

#include <QScreen>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QVariant>
#include <QVariantList>
#include <QDebug>
#include <QTimer>

#include "screenrecorder.h"

class MainWindow : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isRecording READ isRecording NOTIFY recordingStateChanged)
    Q_PROPERTY(bool isPaused READ isPaused NOTIFY pauseStateChanged)
    Q_PROPERTY(int recordingDuration READ recordingDuration NOTIFY durationChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY errorOccurred)

public:
    explicit MainWindow(QObject* parent = nullptr);
    ~MainWindow();

    Q_INVOKABLE void set_fullscreen();
    Q_INVOKABLE void set_customarea(int width, int height, int x_offset, int y_offset,int screen_number);

    Q_INVOKABLE void set_customlocation(QString Outputpath);

    //Q_INVOKABLE void collect_data();
    Q_INVOKABLE void start_record();
    Q_INVOKABLE void pause_record();
    Q_INVOKABLE void resume_record();
    Q_INVOKABLE void stop_record();


    Q_INVOKABLE QVariantList getAudioDevices();
    //Q_INVOKABLE void set_audioDevice();

    Q_INVOKABLE void set_audio(int state);
    Q_INVOKABLE void set_audioDeviceName(QString device);

    Q_INVOKABLE void get_fps(int fps);
    Q_INVOKABLE void get_quality(int quality);
    Q_INVOKABLE void cancelRecording();

    bool isRecording() const { return m_isRecording; }
    bool isPaused() const { return m_isPaused; }
    int recordingDuration() const { return m_recordingDuration; }
    QString lastError() const { return m_lastError; }

signals:
    void recordingStateChanged(bool isRecording);
    void pauseStateChanged(bool isPaused);
    void durationChanged(int duration);
    void errorOccurred(const QString &error);

private slots:
    void updateRecordingDuration();

private:
    //details
    RecordingWindowDetails curr;
    VideoDetails curr_details;
    string output;
    bool audio;
    string audiodevice_name;
    unique_ptr<Screenrecorder> recorder;

    mutex m;
    condition_variable cv;

    bool m_isRecording = false;
    bool m_isPaused = false;
    int m_recordingDuration = 0;
    QString m_lastError;
    QTimer m_durationTimer;
};

#endif // MAINWINDOW_H
