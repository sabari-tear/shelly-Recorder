#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QObject>
#include <QString>

#include <QScreen>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QVariant>
#include <QVariantList>

#include "screenrecorder.h"

class MainWindow : public QObject {
    Q_OBJECT
public:
    explicit MainWindow(QObject* parent = nullptr);

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

    //details
    RecordingWindowDetails curr;
    VideoDetails curr_details;
    string output;
    bool audio;
    string audiodevice_name;
    unique_ptr<Screenrecorder> recorder;

    mutex m;
    condition_variable cv;
};

#endif // MAINWINDOW_H
