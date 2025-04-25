#include "mainwindow.h"
#include <QDebug>
#include <QRect>
#include <iostream>

// Global variable to track the MainWindow instance for logging
static MainWindow* g_mainWindowInstance = nullptr;

// Custom message handler to redirect qDebug output
void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    // Format the message
    QString formattedMessage;
    switch (type) {
        case QtDebugMsg:
            formattedMessage = QString("Debug: %1").arg(msg);
            break;
        case QtInfoMsg:
            formattedMessage = QString("Info: %1").arg(msg);
            break;
        case QtWarningMsg:
            formattedMessage = QString("Warning: %1").arg(msg);
            break;
        case QtCriticalMsg:
            formattedMessage = QString("Critical: %1").arg(msg);
            break;
        case QtFatalMsg:
            formattedMessage = QString("Fatal: %1").arg(msg);
            break;
    }
    
    // Output to standard console
    std::cout << formattedMessage.toStdString() << std::endl;
    
    // Emit signal if instance is available
    if (g_mainWindowInstance) {
        QMetaObject::invokeMethod(g_mainWindowInstance, "logMessage", Qt::QueuedConnection, 
                                 Q_ARG(QString, formattedMessage));
    }
}

MainWindow::MainWindow(QObject* parent) : QObject(parent) {
    // Register this instance for logging
    g_mainWindowInstance = this;
    setupLogging();
}

MainWindow::~MainWindow() {
    // Clean up the global instance
    if (g_mainWindowInstance == this) {
        g_mainWindowInstance = nullptr;
    }
}

void MainWindow::setupLogging() {
    // Install the custom message handler
    qInstallMessageHandler(customMessageHandler);
    
    // Log a startup message
    qDebug() << "Application started, logging initialized";
}

void MainWindow::set_fullscreen() {
    // QScreen* scr = QGuiApplication::primaryScreen();
    // QRect geo = scr->availableVirtualGeometry();
    curr.width = 1920;
    curr.height = 1080;
    curr.offset_x=0;
    curr.offset_y=0;
    curr.screen_number=0;
    qDebug()<<"fullscreen";
    emit logMessage("Setting fullscreen mode");
}

void MainWindow::set_customarea(int width, int height, int x_offset, int y_offset,int screen_number) {
    curr.width=width;
    curr.height=height;
    curr.offset_x=x_offset;
    curr.offset_y=y_offset;
    curr.screen_number=screen_number;
    qDebug()<<"custom_area: " << width << "x" << height << " at " << x_offset << "," << y_offset;
    emit logMessage(QString("Setting custom area: %1x%2 at (%3,%4)").arg(width).arg(height).arg(x_offset).arg(y_offset));
}

void MainWindow::set_customlocation(QString Outputpath)
{
    output=Outputpath.toStdString();
    qDebug() << Outputpath << " is got to the backend";
    emit logMessage(QString("Output location set to: %1").arg(Outputpath));
}

QVariantList MainWindow::getAudioDevices() {
    QVariantList devices;
    const auto deviceInfos = QMediaDevices::audioInputs();
    for (const QAudioDevice &deviceInfo:deviceInfos)
        devices<<deviceInfo.description();
    
    // Log the detected devices
    qDebug() << "Detected" << devices.size() << "audio devices";
    emit logMessage(QString("Detected %1 audio devices").arg(devices.size()));
    
    return devices;
}

void MainWindow::set_audioDeviceName(QString device)
{
    audiodevice_name=device.toStdString();
    qDebug() << "Audio device name is got as " << device;
    emit logMessage(QString("Audio device set to: %1").arg(device));
}

void MainWindow::set_audio(int state)
{
    curr_details.audio=state;
    qDebug() << "Audio state is set as" << state;
    emit logMessage(QString("Audio %1").arg(state ? "enabled" : "disabled"));
}

void MainWindow::get_fps(int fps)
{
    curr_details.fps=fps;
    qDebug() << fps << " is set";
    emit logMessage(QString("FPS set to: %1").arg(fps));
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
    emit logMessage(QString("Quality set to: %1% (level %2)").arg(quality).arg(position));
}

void MainWindow::start_record() {
    try {
        emit logMessage("Building screen recorder...");
        recorder=make_unique<Screenrecorder>(curr, curr_details, output, audiodevice_name);
        qDebug() << "Built Screen recorder";
        emit logMessage("Screen recorder built successfully");
        
        auto record_thread=std::thread{[this](){  // Capture 'this' instead of by reference
                try {
                    qDebug() << "Started recording..";
                    
                    // We need to use the global method to emit a signal from a non-Qt thread
                    if (g_mainWindowInstance) {
                        QMetaObject::invokeMethod(g_mainWindowInstance, "logMessage", Qt::QueuedConnection, 
                                               Q_ARG(QString, QString("Recording started")));
                    }
                    
                    recorder->record();
                }
                catch (const std::exception &e) {
                    QString errorMsg = QString("Error during recording: %1").arg(e.what());
                    qDebug() << "Caught exception:" << e.what();
                    
                    // Emit log message from the thread
                    if (g_mainWindowInstance) {
                        QMetaObject::invokeMethod(g_mainWindowInstance, "logMessage", Qt::QueuedConnection, 
                                               Q_ARG(QString, errorMsg));
                    }
                    throw;
                }
                recorder.reset();
                
                // Emit log message from the thread
                if (g_mainWindowInstance) {
                    QMetaObject::invokeMethod(g_mainWindowInstance, "logMessage", Qt::QueuedConnection, 
                                           Q_ARG(QString, QString("Recording finished")));
                }
                cv.notify_one();
        }};
        record_thread.detach();
    }
    catch (const exception &e) {
        QString errorMsg = QString("Failed to start recording: %1").arg(e.what());
        qDebug() << "Caught exception at start_record:" << e.what();
        emit logMessage(errorMsg);
        throw;
    }
}

void MainWindow::pause_record() {
    recorder->pauseRecording();
    emit logMessage("Recording paused");
}

void MainWindow::resume_record() {
    recorder->resumeRecording();
    emit logMessage("Recording resumed");
}

void MainWindow::stop_record() {
    emit logMessage("Stopping recording...");
    recorder->stopRecording();
    emit logMessage("Recording stopped");
    
    auto waiting_thread =std::thread{[this]() {  // Capture 'this' instead of by reference
            unique_lock ul{m};
            cv.wait(ul,[this](){return !recorder; });
            
            // Emit log message from the thread
            if (g_mainWindowInstance) {
                QMetaObject::invokeMethod(g_mainWindowInstance, "logMessage", Qt::QueuedConnection, 
                                       Q_ARG(QString, QString("Recorder cleanup complete")));
            }
    }};
    waiting_thread.detach();
}
