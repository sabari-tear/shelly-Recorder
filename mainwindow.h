#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QElapsedTimer>

#include <QScreen>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QVariant>
#include <QVariantList>
#include <QDebug>

#include "screenrecorder.h"

class MainWindow : public QObject {
    Q_OBJECT
    Q_PROPERTY(int recordingDuration READ getRecordingDuration NOTIFY recordingDurationChanged)
    Q_PROPERTY(bool isRecording READ isRecording NOTIFY recordingStateChanged)
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

    // New methods for recording status
    int getRecordingDuration() const { return m_recordingDuration; }
    bool isRecording() const { return m_isRecording; }
    Q_INVOKABLE QString getRecordingStatus() const;

signals:
    void recordingDurationChanged();
    void recordingStateChanged();

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

    // New members for recording duration tracking
    QTimer m_durationTimer;
    QElapsedTimer m_recordingTimer;
    int m_recordingDuration;
    bool m_isRecording;
};

#endif // MAINWINDOW_H
