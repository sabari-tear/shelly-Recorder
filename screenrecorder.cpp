#include "screenrecorder.h"
#include <QDebug>
void Screenrecorder::init_devicesEncodec()
{
    //to get all the input devices and register it to the internal list in FFmpeg
    avdevice_register_all();

    //format context pointer
    avFmtCtx=nullptr;
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
    if (!avEncodec) {
        throw logic_error{"Encoder codec not found"};
    }

}

void Screenrecorder::init_videoSource(){
    wd.width=wd.width/32*32;
    wd.height=wd.height/2*2;

    avRawOptions=nullptr;
    if (vd.fps > 15) {
        vd.fps = 15;
    }
    av_dict_set(&avRawOptions,"video_size",(to_string(wd.width)+"x"+to_string(wd.height)).c_str(),0);
    av_dict_set(&avRawOptions,"framerate",to_string(vd.fps).c_str(),0);
    av_dict_set(&avRawOptions, "probesize", "30M", 0);

    AVInputFormat *avInputfmt=av_find_input_format("gdigrab");
    if(avInputfmt==nullptr){
        throw logic_error{"av_find_input_format not found......"};
    }

    av_dict_set(&avRawOptions,"offset_x",to_string(wd.offset_x).c_str(),0);
    av_dict_set(&avRawOptions,"offset_y",to_string(wd.offset_y).c_str(),0);

    if(avformat_open_input(&avFmtCtx,"desktop",avInputfmt,&avRawOptions)!=0){
        throw logic_error{"Failed to open video device: avformat_open_input()"};
    }

    if(avformat_find_stream_info(avFmtCtx,&avRawOptions)<0){
        throw logic_error{"Failed to get stream information: avformat_find_stream_info"};
    }

    vdo_stream_index=-1;
    vdo_input_st=NULL;
    for(int i=0;i<(int)avFmtCtx->nb_streams;i++){
        if(avFmtCtx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO){
            vdo_stream_index=i;
            break;
        }
    }

    if(vdo_stream_index==-1||vdo_stream_index>=(int)avFmtCtx->nb_streams){
        throw logic_error{"Couldn't find the video stream."};
    }

    avRawCodecCtx=avcodec_alloc_context3(nullptr);
    avcodec_parameters_to_context(avRawCodecCtx,avFmtCtx->streams[vdo_stream_index]->codecpar);

    avDecodec=avcodec_find_decoder(avRawCodecCtx->codec_id);
    if(avDecodec==nullptr){
        throw runtime_error{"Decoder codec not found."};
    }
}

void Screenrecorder::init_videoVariables() {
    //setup the video streamer iwth output context and encoding details
    AVStream* video_output_st = avformat_new_stream(avFmtCtxOut, avEncodec);
    //initialize the encoder context
    avEncoderCtx=avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(avEncoderCtx,video_output_st->codecpar);

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
        avEncoderCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    //open the decoder
    if (avcodec_open2(avRawCodecCtx, avDecodec, nullptr) < 0) {
        throw runtime_error{"Could not open decodec. "};
    }

    //open the encoder
    if (avcodec_open2(avEncoderCtx, avEncodec, NULL) < 0) {
        throw runtime_error{"Could not open Encodec. "};
    }

    //find the empty stream to use for encoder
    int videoIndexOut=-1;
    for(int i=0;i<(int)avFmtCtx->nb_streams;i++) {
        if (avFmtCtxOut->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_UNKNOWN) {
            videoIndexOut=i;
        }
    }
    if (videoIndexOut<0) {
        throw runtime_error{"Cannot find empty stream for encoder"};
    }

    //setting the encoder context to the empty stream
    avcodec_parameters_from_context(avFmtCtxOut->streams[videoIndexOut]->codecpar,avEncoderCtx);

    //open output url
    if (avio_open(&avFmtCtxOut->pb, outFilePath.c_str(),AVIO_FLAG_READ_WRITE)<0) {
        throw runtime_error{"Can't open the path with read and write"};
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
}

void Screenrecorder::init_audioSource() {
    //init
    FormatContextAudio = avformat_alloc_context();
    AudioOptions=NULL;

    //setip dict
    av_dict_set(&AudioOptions, "sample_rate","44100",0);
    av_dict_set(&AudioOptions, "async","25",0);

    //make the format
    audioDevice="audio="+audioDevice;

    //set input format
    AudioInputFormat=av_find_input_format("dshow");
    if (avformat_open_input(&FormatContextAudio, audioDevice.c_str(), AudioInputFormat, &AudioOptions)!=0) {
        throw runtime_error{"Unable to find input audio"};
    }

    //check stream info
    if (avformat_find_stream_info(FormatContextAudio,NULL)<0) {
        throw runtime_error{"Unable to find stream information"};
    }

    //find audio stream index
    for(int i=0;i<(int)FormatContextAudio->nb_streams;i++) {
        if (FormatContextAudio->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_AUDIO) {
            ado_stream_index=i;
            audio_output_st =FormatContextAudio->streams[i];
            break;
        }
    }

    //couldnt find throw error
    if (ado_stream_index==-1 || ado_stream_index>=(int)FormatContextAudio->nb_streams) {
        throw runtime_error{"Unable to find the audio stream.."};
    }
}

void Screenrecorder::init_audioVariables() {
    //decoder parameters
    AVCodecParameters * AudioParams =audio_output_st->codecpar;
    //find decoder codec
    AudioDecodec=avcodec_find_decoder(AudioParams->codec_id);
    if (AudioDecodec==NULL) {
        throw runtime_error{"Couldnt find a decoder"};
    }

    //alloc audio decoder ctx
    AudioDecoderCtx=avcodec_alloc_context3(AudioDecodec);
    if (avcodec_parameters_to_context(AudioDecoderCtx,AudioParams)<0) {
        throw runtime_error("Unable to set audio parameters ctx to decoder ctx");
    }

    //open the codec
    if (avcodec_open2(AudioDecoderCtx,AudioDecodec,NULL)<0) {
        throw runtime_error("cant open the decoder...");
    }

    // new audio stream output
    //qDebug()<<"checkpoint 1 reached successfully";
    AVStream* ado_out_st=avformat_new_stream(avFmtCtxOut,NULL);
    if (!ado_out_st) {
        throw runtime_error("it does not detect any stream for output");
    }

    //find the encodec output
    AudioEncodec=avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!AudioEncodec) {
        throw runtime_error("can not find encoder");
    }
    //setting up the context for the encoder
    AudioEncoderCtx=avcodec_alloc_context3(AudioEncodec);
    if (!AudioEncoderCtx) {
        throw runtime_error("Can not perform alloc for EncoderCtx");
    }

    if (AudioEncodec->supported_samplerates) {
        AudioEncoderCtx->sample_rate=AudioEncodec->supported_samplerates[0];
        for(int i=0;(AudioEncodec)->supported_samplerates[i];i++) {
            if (AudioEncodec->supported_samplerates[i]==AudioDecoderCtx->sample_rate)
                AudioEncoderCtx->sample_rate=AudioDecoderCtx->sample_rate;
        }
    }

    //init encoder ctx
    AudioEncoderCtx->codec_id=AV_CODEC_ID_AAC;
    AudioEncoderCtx->bit_rate=128000;
    AudioEncoderCtx->channels=AudioDecoderCtx->channels;
    AudioEncoderCtx->channel_layout=av_get_default_channel_layout(AudioEncoderCtx->channels);
    AudioEncoderCtx->sample_fmt=AudioEncodec->sample_fmts? AudioEncodec->sample_fmts[0]:AV_SAMPLE_FMT_FLTP;

    AudioEncoderCtx->time_base= {1,AudioDecoderCtx->sample_rate};
    AudioEncoderCtx->strict_std_compliance=FF_COMPLIANCE_EXPERIMENTAL;

    if (avFmtCtxOut->oformat->flags & AVFMT_GLOBALHEADER)
        AudioEncoderCtx->flags|=AV_CODEC_FLAG_GLOBAL_HEADER;

    if (avcodec_open2(AudioEncoderCtx,AudioEncodec,NULL)<0) {
        throw runtime_error("Error on opening encoder");
    }

    audioIndexOut=-1;
    for(int i=0;i<(int)avFmtCtxOut->nb_streams;i++) {
        if (avFmtCtxOut->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_UNKNOWN){
            audioIndexOut=i;
        }
    }
    if (audioIndexOut<0) {
        throw runtime_error("cannot find empty stream for audio");
    }
    avcodec_parameters_from_context(avFmtCtxOut->streams[audioIndexOut]->codecpar,AudioEncoderCtx);
}

void Screenrecorder::init_outputFile() {

    //create an empty file it not exists
    if (!(avFmtCtxOut->flags & AVFMT_NOFILE)) {
        if (avio_open2(&avFmtCtxOut->pb, outFilePath.c_str(),AVIO_FLAG_WRITE,NULL,NULL)<0) {
            throw runtime_error("Error creating new file");
        }
    }

    //find encoder stream
    if (avFmtCtxOut->nb_streams==0) {
        throw runtime_error("no stream is init...");
    }

    if (avformat_write_header(avFmtCtxOut,NULL)<0) {
        throw runtime_error("error in writing the header context...");
    }
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
    try {
        init_devicesEncodec();
        qDebug()<<"done ready.";
        init_videoSource();
        qDebug()<<"videoSource is done";
        init_videoVariables();
        qDebug()<<"initvideovariables succeeded";
        if (vd.audio) {
            init_audioSource();
            qDebug()<<"init Audio source successfullll";
            init_audioVariables();
            qDebug()<<"init of audio variables was success";
        }
        init_outputFile();
        qDebug()<<"initializing the output file... is done";
    }
    catch (const std::exception &e) {
        qDebug() << "Caught exception:" << e.what();
        throw;
    }
}

function<void(void)> Screenrecorder::make_error_handler(function<void(void)> f) {
    return [&]() {
        try {
            f();
            lock_guard<mutex> lg{error_queue_m};
            terminated_threads++;
            error_queue_cv.notify_one();
        }
        catch (const std::exception &e) {
            lock_guard<mutex> lg{error_queue_m};
            error_queue.emplace(e.what());
            error_queue_cv.notify_one();
        }
    };
}

void Screenrecorder::decodeAndEncode() {
    int got_picture=0;
    int flag=0;
    int bufLen=0;
    uint8_t *outBuf=nullptr;

    bufLen=wd.width*wd.height*3;
    outBuf=(uint8_t*)malloc(bufLen);

    AVFrame* avOutFrame;
    avOutFrame=av_frame_alloc();
    av_image_fill_arrays(avOutFrame->data, avOutFrame->linesize, (uint8_t*)outBuf, avEncoderCtx->pix_fmt, avEncoderCtx->width, evEncoderCtx->height, 1);

    AVPacket pkt;
    av_init_packet(&pkt);

    AVPacket* avRawPkt;
    int i=1;
    int j=1;

    while(true) {
        unique_lock<mutex> avRawPkt_queue_ul{avRawPkt_queue_mutex};
    }

}

void Screenrecorder::record()
{
    audio_stop=false;
    gotFirstValidVideoPacket=false;

    eloborate_thread = make_unique<thread> ([this]() {
        this->make_error_handler([this]() {
            this->decodeAndEncode();
        })
    })
}
