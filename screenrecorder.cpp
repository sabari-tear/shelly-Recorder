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
    readydevices_encodec();
}

