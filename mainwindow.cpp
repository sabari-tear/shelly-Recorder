#include "mainwindow.h"
#include <QDebug>
#include <QRect>
#include <iostream>

MainWindow::MainWindow(QObject* parent) : QObject(parent) {
}

MainWindow::~MainWindow() {
}

void MainWindow::set_fullscreen() {
    curr.width = 1920;
    curr.height = 1080;
    curr.offset_x=0;
    curr.offset_y=0;
    curr.screen_number=0;
    qDebug()<<"fullscreen";
}

void MainWindow::set_customarea(int width, int height, int x_offset, int y_offset,int screen_number) {
    curr.width=width;
    curr.height=height;
    curr.offset_x=x_offset;
    curr.offset_y=y_offset;
    curr.screen_number=screen_number;
    qDebug()<<"custom_area: " << width << "x" << height << " at " << x_offset << "," << y_offset;
}

void MainWindow::set_customlocation(QString Outputpath)
{
    output=Outputpath.toStdString();
    qDebug() << Outputpath << " is got to the backend";
}

QVariantList MainWindow::getAudioDevices() {
    QVariantList devices;
    const auto deviceInfos = QMediaDevices::audioInputs();
    for (const QAudioDevice &deviceInfo:deviceInfos)
        devices<<deviceInfo.description();
    
    qDebug() << "Detected" << devices.size() << "audio devices";
    return devices;
}

void MainWindow::set_audioDeviceName(QString device)
{
    audiodevice_name=device.toStdString();
    qDebug() << "Audio device name is got as " << device;
}

void MainWindow::set_audio(int state)
{
    curr_details.audio=state;
    qDebug() << "Audio state is set as" << state;
}

void MainWindow::get_fps(int fps)
{
    curr_details.fps=fps;
    qDebug() << fps << " is set";
}

void MainWindow::get_quality(int quality)
{
    int position = (quality/14.2857f) + 1;  // 14.2857 = 100 / 7

    if (position < 1) position = 1;
    if (position > 7) position = 7;

    if (position == 1) {
        curr_details.quality = 0.3;
        curr_details.compression = 8;
    } else if (position == 2) {
        curr_details.quality = 0.5;
        curr_details.compression = 8;
    } else if (position == 3) {
        curr_details.quality = 1;
        curr_details.compression = 8;
    } else if (position == 4) {
        curr_details.quality = 1;
        curr_details.compression = 7;
    } else if (position == 5) {
        curr_details.quality = 1;
        curr_details.compression = 6;
    } else if (position == 6) {
        curr_details.quality = 1;
        curr_details.compression = 5;
    } else if (position == 7) {
        curr_details.quality = 1;
        curr_details.compression = 4;
    }
    qDebug() << "Quality " << quality << " is set (position: " << position << 
        ", quality factor: " << curr_details.quality << 
        ", compression: " << curr_details.compression << ")";
}

void MainWindow::start_record() {
    try {
        recorder=make_unique<Screenrecorder>(curr, curr_details, output, audiodevice_name);
        qDebug() << "Built Screen recorder";
        
        auto record_thread=std::thread{[this](){
                try {
                    qDebug() << "Started recording..";
                    recorder->record();
                }
                catch (const std::exception &e) {
                    qDebug() << "Caught exception:" << e.what();
                    throw;
                }
                recorder.reset();
                cv.notify_one();
        }};
        record_thread.detach();
    }
    catch (const exception &e) {
        qDebug() << "Caught exception at start_record:" << e.what();
        throw;
    }
}

void MainWindow::pause_record() {
    recorder->pauseRecording();
}

void MainWindow::resume_record() {
    recorder->resumeRecording();
}

void MainWindow::stop_record() {
    recorder->stopRecording();
    
    auto waiting_thread =std::thread{[this]() {
            unique_lock ul{m};
            cv.wait(ul,[this](){return !recorder; });
    }};
    waiting_thread.detach();
}
