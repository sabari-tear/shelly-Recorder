#include "screenrecorder.h"
#include <QDebug>
void Screenrecorder::init_devicesEncodec()
{
    //to get all the input devices and register it to the internal list in FFmpeg
    avdevice_register_all();

    //format context pointer
    avFmtCtx=NULL;
    avFmtCtx=avformat_alloc_context();

    // guess the format
    fmt=av_guess_format(NULL,outFilePath.c_str(),NULL);
    if (fmt==NULL) {
        throw runtime_error{"Error: cannot guess format"};
    }
    // this context is used to write the output
    avformat_alloc_output_context2(&avFmtCtxOut, fmt, fmt->name, outFilePath.c_str());

    // find encoder
    avEncodec = avcodec_find_encoder(AV_CODEC_ID_H264);

    //throw error if encoder not found
    if (avEncodec==nullptr) {
        throw logic_error{"Encoder codec not found"};
    }

    qDebug()<<"done ready.";
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

void Screenrecorder::init_videoVariables() {
    //setup the video streamer iwth output context and encoding details
    video_st = avformat_new_stream(avFmtCtxOut, avEncodec);
    //initialize the encoder context
    avEncoderCtx=avcodec_alloc_context3(NULL);
    avEncoderCtx->codec_id=AV_CODEC_ID_H264;
    avEncoderCtx->codec_type=AVMEDIA_TYPE_VIDEO;
    avEncoderCtx->pix_fmt=AV_PIX_FMT_YUV420P;
    avEncoderCtx->bit_rate = 4000;
    avEncoderCtx->width = (int)(wd.width * vd.quality) / 32 * 32;
    avEncoderCtx->height = (int)(wd.height * vd.quality) / 2 * 2;
    avEncoderCtx->time_base.num = 1;
    avEncoderCtx->time_base.den = vd.fps;
    avEncoderCtx->gop_size = vd.fps * 2;
    avEncoderCtx->qmin = vd.compression * 5;
    avEncoderCtx->qmax = 5 + vd.compression * 5;
    avEncoderCtx->max_b_frames = 10;

    av_opt_set(avEncoderCtx, "preset", "ultrafast", 0);
    av_opt_set(avEncoderCtx, "tune", "zerolatency", 0);
    av_opt_set(avEncoderCtx, "cabac", "1", 0);
    av_opt_set(avEncoderCtx, "ref", "3", 0);
    av_opt_set(avEncoderCtx, "deblock", "1:0:0", 0);
    av_opt_set(avEncoderCtx, "analyse", "0x3:0x113", 0);
    av_opt_set(avEncoderCtx, "subme", "7", 0);
    av_opt_set(avEncoderCtx, "chroma_qp_offset", "4", 0);
    av_opt_set(avEncoderCtx, "rc", "crf", 0);
    av_opt_set(avEncoderCtx, "rc_lookahead", "40", 0);
    av_opt_set(avEncoderCtx, "crf", "10.0", 0);
    av_opt_set(avEncoderCtx, "threads", "8", 0);

    //set the encoder header to global if it didnt set then every raw packets it will carry the header
    if (avFmtCtxOut->oformat->flags & AVFMT_GLOBALHEADER)
        avEncoderCtx->flags|=AV_CODEC_FLAG_GLOBAL_HEADER;

    //open the decoder
    if (avcodec_open2(avRawCodecCtx, avDecodec, nullptr) < 0) {
        qDebug()<<"Could not open decodec. ";
        return;
    }

    //open the encoder
    if (avcodec_open2(avEncoderCtx, avEncodec, NULL) < 0) {
        qDebug()<<"Could not open Encodec. ";
        return;
    }

    //find the empty stream to use for encoder
    int outvdost=-1;
    for(int i=0;i<(int)avFmtCtx->nb_streams;i++) {
        if (avFmtCtxOut->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_UNKNOWN)
            outvdost=i;
    }
    if (outvdost<0) {
        qDebug()<<"Cannot find empty stream for enoder";
        return;
    }

    //setting the encoder context to the empty stream
    avcodec_parameters_from_context(avFmtCtx->streams[outvdost]->codecpar,avEncoderCtx);

    //open output url
    if (avio_open(&avFmtCtx->pb, outFilePath.c_str(),AVIO_FLAG_READ_WRITE)<0) {
        qDebug()<<"Can't open the path with read and write";
        return;
    }

    //get the context for setting it up to convert raw frames to the format your encoder
    swsCtx=sws_getContext(avRawCodecCtx->width,
                            avRawCodecCtx->height,
                            avRawCodecCtx->pix_fmt,
                            (int)(avRawCodecCtx->width*vd.quality)/32*32,
                            (int)(avRawCodecCtx->height*vd.quality)/2*2,
                            AV_PIX_FMT_YUV420P,
                            SWS_FAST_BILINEAR,
                            nullptr,
                            nullptr,
                            nullptr);

    //init to store the converted frame
    avYUVFrame=av_frame_alloc();
    int yuvLen=av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
                                          avRawCodecCtx->width,
                                          avRawCodecCtx->height,1);

    //init var to save one frame of yuv420p frame
    uint8_t *yuvBuf =new uint8_t[yuvLen];
    av_image_fill_arrays(avYUVFrame->data,
                         avYUVFrame->linesize,
                         (uint8_t*)yuvBuf,
                         AV_PIX_FMT_YUV420P,
                         avRawCodecCtx->width,
                         avRawCodecCtx->height,1);

    avYUVFrame->width = avRawCodecCtx->width;
    avYUVFrame->height = avRawCodecCtx->height;
    avYUVFrame->format = AV_PIX_FMT_YUV420P;

    qDebug()<<"initvideovariables succeeded";
}

void Screenrecorder::init_audioSource() {
    int c=0;
    qDebug()<<c;
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
    init_videoVariables();

    if (vd.audio) {
        init_audioSource();
    }
}
