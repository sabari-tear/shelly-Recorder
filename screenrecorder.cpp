#include "screenrecorder.h"
#include <QDebug>
void Screenrecorder::readydevices_encodec()
{
    //to get all the input devices and register it to the internal list in FFmpeg
    //avcodec_register_all();
    qDebug()<<"ready is called";
    //format context pointer
    avFmtCtx=NULL;
    //vFmtCtx=avformat_alloc_context();
    outFilePath="C:/Users/YourUsername/Videos/out.mp4";

    // //guess the format
    fmt=av_guess_format(NULL,outFilePath.c_str(),NULL);

    // //throw error if NULL

    // //this context is used to write the output
    avformat_alloc_output_context2(&avFmtCtxOut, fmt, fmt->name, outFilePath.c_str());

    // //find encoder
    avEncodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    qDebug()<<"done ready.";
    //throw error if encoder not found
}
Screenrecorder::Screenrecorder(RecordingWindowDetails* window_details, VideoDetails* video_Details, string* outFilePath, string* audioDevice)
{
    qDebug() << "RecordingWindowDetails:";
    if(window_details) {
        qDebug() << "  width:" << window_details->width;
        qDebug() << "  height:" << window_details->height;
        qDebug() << "  offset_x:" << window_details->offset_x;
        qDebug() << "  offset_y:" << window_details->offset_y;
        qDebug() << "  screen_number:" << window_details->screen_number;
    } else {
        qDebug() << "  window_details is null";
    }

    qDebug() << "VideoDetails:";
    if(video_Details) {
        qDebug() << "  fps:" << video_Details->fps;
        qDebug() << "  quality:" << video_Details->quality;
        qDebug() << "  compression:" << video_Details->compression;
        qDebug() << "  audio:" << (video_Details->audio ? "true" : "false");
    } else {
        qDebug() << "  video_Details is null";
    }

    qDebug() << "Output File Path:";
    if(outFilePath) {
        qDebug() << "  outFilePath:" << QString::fromStdString(*outFilePath);
    } else {
        qDebug() << "  outFilePath is null";
    }

    qDebug() << "Audio Device:";
    if(audioDevice) {
        qDebug() << "  audioDevice:" << QString::fromStdString(*audioDevice);
    } else {
        qDebug() << "  audioDevice is null";
    }

    readydevices_encodec();
}

