#include "mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QObject* parent) : QObject(parent) {}

void MainWindow::set_fullscreen() {
    QScreen* scr = QGuiApplication::primaryScreen();
    curr.width=scr->size().width();
    curr.height=scr->size().height();
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
    qDebug()<<"custom_area";
}

void MainWindow::set_customlocation(QString Outputpath)
{
    output=Outputpath.toStdString();
    qDebug()<<Outputpath<<" is got to the backend";
}
QVariantList MainWindow::getAudioDevices() {
    QVariantList devices;
    const auto deviceInfos = QMediaDevices::audioInputs();
    for (const QAudioDevice &deviceInfo:deviceInfos)
        devices<<deviceInfo.description();
    return devices;
}

void MainWindow::set_audioDeviceName(QString device)
{
    audiodevice_name=device.toStdString();
    qDebug()<<"audio device name is got as "<<device;
}

void MainWindow::set_audio(int state)
{
    curr_details.audio=state;
    qDebug()<<"state is set as"<<state;
}

void MainWindow::get_fps(int fps)
{
    curr_details.fps=fps;
    qDebug()<<fps<<" is set";
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
    qDebug()<<quality<<" is set";
}
void MainWindow::start_record() {
    recorder=new Screenrecorder(curr, curr_details, output, audiodevice_name);

}

