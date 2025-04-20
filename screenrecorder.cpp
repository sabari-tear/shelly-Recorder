#include "screenrecorder.h"
#include <QDebug>
void Screenrecorder::init_devicesEncodec()
{
    qDebug()<<"ready is called";
    //to get all the input devices and register it to the internal list in FFmpeg
    avdevice_register_all();

    //format context pointer
    avFmtCtx=NULL;
    //vFmtCtx=avformat_alloc_context();

    // //guess the format
    fmt=av_guess_format(NULL,outFilePath.c_str(),NULL);
    if (fmt==NULL) {
        throw runtime_error{"Error: cannot guess format"};
    }
    // //this context is used to write the output
    avformat_alloc_output_context2(&avFmtCtxOut, fmt, fmt->name, outFilePath.c_str());

    // //find encoder
    avEncodec = avcodec_find_encoder(AV_CODEC_ID_H264);

    if (avEncodec==nullptr) {
        throw logic_error{"Encoder codec not found"};
    }

    qDebug()<<"done ready.";
    //throw error if encoder not found
}

void Screenrecorder::init_videoSource(){
    wd.width=wd.width/32*32;
    wd.height=wd.height/2*2;

    avRawOptions=nullptr;
    av_dict_set(&avRawOptions,"video_size",(to_string(wd.width)+"x"+to_string(wd.height)).c_str(),0);
    av_dict_set(&avRawOptions,"framerate",to_string(vd.fps).c_str(),0);

    AVInputFormat *avInputfmt=av_find_input_format("gdigrab");
    if(avInputfmt==nullptr){
        qDebug()<<"av_find_input_format not found......";
        return;
    }

    av_dict_set(&avRawOptions,"offset_x",to_string(wd.offset_x).c_str(),0);
    av_dict_set(&avRawOptions,"offset_y",to_string(wd.offset_y).c_str(),0);

    if(avformat_open_input(&avFmtCtx,"desktop",avInputfmt,&avRawOptions)!=0){
        qDebug()<<"Failed to open video device: avformat_open_input()";
        return;
    }

    if(avformat_find_stream_info(avFmtCtx,&avRawOptions)<0){
        qDebug()<<"Failed to get stream information: avformat_find_stream_info";
        return;
    }

    vdo_stream_index=-1;
    for(int i=0;i<(int)avFmtCtx->nb_streams;i++){
        if(avFmtCtx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO){
            vdo_stream_index=i;
            break;
        }
    }

    if(vdo_stream_index==-1||vdo_stream_index>=(int)avFmtCtx->nb_streams){
        qDebug()<<"Couldn't find the video stream.";
        return;
    }

    avRawCodecCtx=avcodec_alloc_context3(nullptr);
    avcodec_parameters_to_context(avRawCodecCtx,avFmtCtx->streams[vdo_stream_index]->codecpar);

    avDecodec=avcodec_find_decoder(avRawCodecCtx->codec_id);
    if(avDecodec==nullptr){
        qDebug()<<"Decoder codec not found.";
        return;
    }

    qDebug()<<"videoSource is done";
}


Screenrecorder::Screenrecorder(RecordingWindowDetails& wd, VideoDetails& vd, string& outFilePath, string& audioDevice)
{
    this->wd=wd;
    this->vd=vd;
    this->outFilePath=outFilePath;
    this->audioDevice=audioDevice;
    qDebug() << "RecordingWindowDetails:";
        qDebug() << "  width:" << wd.width;
        qDebug() << "  height:" << wd.height;
        qDebug() << "  offset_x:" << wd.offset_x;
        qDebug() << "  offset_y:" << wd.offset_y;
        qDebug() << "  screen_number:" << wd.screen_number;

    qDebug() << "VideoDetails:";
        qDebug() << "  fps:" << vd.fps;
        qDebug() << "  quality:" << vd.quality;
        qDebug() << "  compression:" << vd.compression;
        qDebug() << "  audio:" << (vd.audio ? "true" : "false");

    qDebug() << "  outFilePath:" << QString::fromStdString(outFilePath);
    qDebug() << "  audioDevice:" << QString::fromStdString(audioDevice);

    init_devicesEncodec();
    init_videoSource();
}
