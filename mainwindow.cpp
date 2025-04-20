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
}
QVariantList MainWindow::getAudioDevices() {
    QVariantList devices;
    const auto deviceInfos = QMediaDevices::audioInputs();
    for (const QAudioDevice &deviceInfo:deviceInfos)
        devices<<deviceInfo.description();
    return devices;
}

void MainWindow::set_audioDeviceName(QString OutputPath)
{
    output=OutputPath.toStdString();
}

void MainWindow::start_record() {
    recorder=new Screenrecorder(&curr, &curr_details, &output, &audiodevice_name);
}

