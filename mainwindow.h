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


    Q_INVOKABLE QVariantList getAudioDevices();
    void set_audioDeviceName(QString OutputPath);
    //details
    RecordingWindowDetails curr;
    VideoDetails curr_details;
    string output;
    string audiodevice_name;
    Screenrecorder* recorder;
};

#endif // MAINWINDOW_H
