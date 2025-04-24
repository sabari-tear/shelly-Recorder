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

bool Screenrecorder::isVideoEnd() {
    lock_guard<mutex> lg(video_lock);
    return video_end;
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
    av_image_fill_arrays(avOutFrame->data, avOutFrame->linesize, (uint8_t*)outBuf, avEncoderCtx->pix_fmt, avEncoderCtx->width, avEncoderCtx->height, 1);

    AVPacket pkt;
    av_packet_ref(&pkt, av_packet_alloc()); // Copy from a properly initialized packet


    AVPacket* avRawPkt;
    int i=1;
    int j=1;

    while(true) {
        unique_lock<mutex> avRawPkt_queue_ul{avRawPkt_queue_mutex};
        if (!avRawPkt_queue.empty()) {
            avRawPkt = avRawPkt_queue.front();
            avRawPkt_queue.pop();
            avRawPkt_queue_ul.unlock();
            if (avRawPkt->stream_index == vdo_stream_index) {
                //Start DECODING
                flag = avcodec_send_packet(avRawCodecCtx, avRawPkt);

                av_packet_unref(avRawPkt);
                av_packet_free(&avRawPkt);

                if (flag < 0) {
                    throw runtime_error("Decoding Error: sending packet");
                }
                got_picture = avcodec_receive_frame(avRawCodecCtx, avOutFrame);

                //End DECODING
                if (got_picture == 0) {
                    sws_scale(swsCtx, avOutFrame->data, avOutFrame->linesize, 0, avRawCodecCtx->height, avYUVFrame->data, avYUVFrame->linesize);

                    //Start ENCODING
                    avYUVFrame->pts = (int64_t)j * (int64_t)30 * (int64_t)30 * (int64_t)100 / (int64_t)vd.fps;
                    j++;
                    flag = avcodec_send_frame(avEncoderCtx, avYUVFrame);

                    if (flag >= 0) {
                        got_picture = avcodec_receive_packet(avEncoderCtx, &pkt);
                        //Fine ENCODING
                        if (got_picture == 0) {
                            if (!gotFirstValidVideoPacket) {
                                gotFirstValidVideoPacket = true;
                            }
                            pkt.pts = (int64_t)i * (int64_t)30 * (int64_t)30 * (int64_t)100 / (int64_t)vd.fps;
                            pkt.dts = (int64_t)i * (int64_t)30 * (int64_t)30 * (int64_t)100 / (int64_t)vd.fps;

                            unique_lock<mutex> write_lock_ul{write_lock};
                            if (av_write_frame(avFmtCtxOut, &pkt) < 0) {
                                throw runtime_error("Error in writing file");
                            }
                            write_lock_ul.unlock();
                            i++;
                        }
                    }
                } else {
                    throw runtime_error("Error Decoding: receiving packet");
                }
            }
        } else {
            avRawPkt_queue_ul.unlock();
            unique_lock<mutex> ul(status_lock);
            if (status == RecordingStatus::stopped) {
                ul.unlock();
                avRawPkt_queue_ul.lock();
                if (avRawPkt_queue.empty() && isVideoEnd()) {
                    avRawPkt_queue_ul.unlock();
                    break;
                }
                avRawPkt_queue_ul.unlock();
            }
            //ul.unlock();
        }
    }

    av_packet_unref(&pkt);
    qDebug()<<"ended decode encode";
}

int Screenrecorder::getlatestFramesValue() {
    int base=10;
    int result =0;
    double numerator =5;
    double fps=vd.fps;
    if (fps<8)
        fps=8;
    double denominator =((fps-7)+0.00)/60.00;
    double factor =numerator/denominator;
    int divisor=factor/5.00;
    double rest_factor=factor-(divisor*5);
    if (rest_factor>=2.5)
        result=(divisor*5)+5;
    else
        result=divisor*5;
    return base+result;
}

bool Screenrecorder::audioReady() {
    lock_guard<mutex> lg(audio_lock);
    return audio_ready;
}

void Screenrecorder::videoEnd() {
    lock_guard<mutex> lg(video_lock);
    video_end=true;
    if (vd.audio)
        cv_audio.notify_all();
}

void Screenrecorder::getRawPackets(){
    AVPacket *avRawPkt;
    int value = -1;
    int framesValue = getlatestFramesValue();

    if (vd.audio) {
        unique_lock<mutex> ul_video(video_lock);
        video_ready=true;
        qDebug()<<"video Ready";
        cv_video.wait(ul_video,[this]() {return audioReady();});
        cv_audio.notify_all();
        ul_video.unlock();
        qDebug()<<"video started";
    }
    try {
        while (framesValue!=0) {
            unique_lock<mutex> ul(status_lock);

            if (status==RecordingStatus::paused)
                qDebug()<<"Video Pause";

            cv.wait(ul,[this]() { return status!=RecordingStatus::paused; });

            if (status==RecordingStatus::stopped && (audio_end|| !vd.audio)) {
                if (value>=0)
                    framesValue--;
            }

            ul.unlock();

            avRawPkt=av_packet_alloc();
            value=av_read_frame(avFmtCtx,avRawPkt);

            if (value>=0 && avRawPkt->size) {
                unique_lock<mutex> avRawPkt_queue_ul{avRawPkt_queue_mutex};
                avRawPkt_queue.push(avRawPkt);
                avRawPkt_queue_ul.unlock();
            }
        }
    }
    catch (const std::exception &e) {
        videoEnd();
        qDebug()<<"Caught exception:"<<e.what();
        throw;
    }
    videoEnd();
    qDebug()<<"End get RawPacket";
}

void Screenrecorder::init_fifo() {
    if (!(AudioFifoBuff=av_audio_fifo_alloc(AudioEncoderCtx->sample_fmt, AudioEncoderCtx->channels,1))) {
        throw runtime_error{"allocationg fifo error..."};
    }
}

bool Screenrecorder::videoReady() {
    lock_guard<mutex> lg(video_lock);
    return video_ready;
}

void Screenrecorder::audioEnd() {
    lock_guard<mutex> lg(audio_lock);
    audio_end = true;
}

void Screenrecorder::initConvertedSamples(uint8_t ***converted_input_samples, AVCodecContext *output_codec_context, int frame_size) {
    if (!(*converted_input_samples = (uint8_t **)calloc(output_codec_context->channels, sizeof(**converted_input_samples)))) {
        throw runtime_error("Could not allocate converted input sample pointers");
    }
    if (av_samples_alloc(*converted_input_samples, nullptr, output_codec_context->channels, frame_size, output_codec_context->sample_fmt, 0) < 0) {
        throw runtime_error("could not allocate memory for samples in all channels (audio)");
    }
}

void Screenrecorder::add_samples_to_fifo(uint8_t **converted_input_samples, const int frame_size) {
    int error;
    // Make the FIFO as large as it needs to be to hold both,
    // the old and the new samples.
    if ((error = av_audio_fifo_realloc(AudioFifoBuff, av_audio_fifo_size(AudioFifoBuff) + frame_size)) < 0) {
        throw runtime_error("Could not reallocate FIFO");
    }
    // Store the new samples in the FIFO buffer.
    if (av_audio_fifo_write(AudioFifoBuff, (void **)converted_input_samples, frame_size) < frame_size) {
        throw runtime_error("Could not write data to FIFO");
    }
}


void Screenrecorder::acquireAudio() {
    int ret;
    AVPacket *inPacket, *outPacket;
    AVFrame *rawFrame, *scaledFrame;
    uint8_t **resampledData;

    init_fifo();

    inPacket = (AVPacket *)av_malloc(sizeof(AVPacket));
    if (!inPacket) {
        throw runtime_error("failed to allocate an AVPacket for encoded video");
    }

    av_packet_ref(inPacket, av_packet_alloc());

    rawFrame = av_frame_alloc();
    if (!rawFrame) {
        throw runtime_error("failed allocate an AVPacket for encoded video");
    }

    scaledFrame = av_frame_alloc();
    if (!scaledFrame) {
        throw runtime_error("failed allocate an AVPacket for encoded video");
    }

    outPacket = (AVPacket *)av_malloc(sizeof(AVPacket));
    if (!outPacket) {
        throw runtime_error("failed allocate an AVPacket for encoded video");
    }


    SwrContext *swrContext = nullptr;
    swrContext = swr_alloc_set_opts(swrContext,
                                    av_get_default_channel_layout(AudioEncoderCtx->channels),
                                    AudioEncoderCtx->sample_fmt,
                                    AudioEncoderCtx->sample_rate,
                                    av_get_default_channel_layout(AudioDecoderCtx->channels),
                                    AudioDecoderCtx->sample_fmt,
                                    AudioDecoderCtx->sample_rate,
                                    0,
                                    nullptr);

    if (!swrContext) {
        throw runtime_error("Cannot allocate the resample context");
    }
    if (swr_init(swrContext) < 0) {
        throw runtime_error("Could not open resample context");
        swr_free(&swrContext);
    }

    unique_lock<mutex> ul_audio(audio_lock);
    audio_ready = true;

    qDebug()<< "audio ready";

    cv_audio.wait(ul_audio, [this]() { return videoReady(); });
    cv_video.notify_all();
    ul_audio.unlock();

    qDebug()<< "audio started";

    while (true) {
        unique_lock<mutex> ul(status_lock);
        if (status == RecordingStatus::paused) {
            qDebug()<<"audio pause";
            avformat_close_input(&FormatContextAudio);
            if (FormatContextAudio != nullptr) {
                throw std::logic_error("Error: unable to close the FormatContextAudio (before pause)");
            }
        }
        cv.wait(ul, [this]() { return status != RecordingStatus::paused; });
        if (status == RecordingStatus::stopped) {

            qDebug() << "audio end" ;

            audioEnd();
            ul.unlock();

            ul_audio.lock();

            cv_audio.wait(ul_audio, [this]() { return isVideoEnd(); });

            ul_audio.unlock();

            break;
        }
        ul.unlock();
        if (av_read_frame(FormatContextAudio, inPacket) >= 0 && inPacket->stream_index == ado_stream_index) {

            //decoding part

            av_packet_rescale_ts(outPacket, FormatContextAudio->streams[ado_stream_index]->time_base, AudioDecoderCtx->time_base);

            if ((ret = avcodec_send_packet(AudioDecoderCtx, inPacket)) < 0) {
                throw runtime_error("failed to decode current audio packet ");
                continue;
            }
            while (ret >= 0) {
                ret = avcodec_receive_frame(AudioDecoderCtx, rawFrame);

                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    break;
                else if (ret < 0) {
                    throw runtime_error("processed error during decoding");
                }
                if (avFmtCtxOut->streams[audioIndexOut]->start_time <= 0) {
                    avFmtCtxOut->streams[audioIndexOut]->start_time = rawFrame->pts;
                }
                initConvertedSamples(&resampledData, AudioEncoderCtx, rawFrame->nb_samples);

                swr_convert(swrContext,
                            resampledData, rawFrame->nb_samples,
                            (const uint8_t **)rawFrame->extended_data, rawFrame->nb_samples);
                add_samples_to_fifo(resampledData, rawFrame->nb_samples);

                //raw frame init
                av_packet_ref(outPacket, av_packet_alloc());
                outPacket->data = nullptr;
                outPacket->size = 0;

                scaledFrame = av_frame_alloc();
                if (!scaledFrame) {
                    throw runtime_error("Cannot allocate an AVPacket for encoded audio");
                }

                scaledFrame->nb_samples = AudioEncoderCtx->frame_size;
                scaledFrame->channel_layout = AudioEncoderCtx->channel_layout;
                scaledFrame->format = AudioEncoderCtx->sample_fmt;
                scaledFrame->sample_rate = AudioEncoderCtx->sample_rate;
                av_frame_get_buffer(scaledFrame, 0);
                while (av_audio_fifo_size(AudioFifoBuff) >= AudioEncoderCtx->frame_size) {
                    ret = av_audio_fifo_read(AudioFifoBuff, (void **)(scaledFrame->data), AudioEncoderCtx->frame_size);
                    scaledFrame->pts = pts;
                    pts += scaledFrame->nb_samples;

                    if (avcodec_send_frame(AudioEncoderCtx, scaledFrame) < 0) {
                        throw runtime_error("Couldnt encode current audio packet ");
                    }

                    while (ret >= 0) {
                        ret = avcodec_receive_packet(AudioEncoderCtx, outPacket);
                        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                            break;
                        else if (ret < 0) {
                            throw runtime_error("failed part o during encoding");
                        }
                        av_packet_rescale_ts(outPacket, AudioEncoderCtx->time_base, avFmtCtxOut->streams[audioIndexOut]->time_base);
                        outPacket->stream_index = audioIndexOut;

                        unique_lock<mutex> write_lock_ul{write_lock};

                        if (av_write_frame(avFmtCtxOut, outPacket) != 0) {
                            throw runtime_error("Error in writing audio frame");
                        }

                        write_lock_ul.unlock();
                        av_packet_unref(outPacket);
                    }
                    ret = 0;
                }

                av_frame_free(&scaledFrame);
                av_packet_unref(outPacket);
            }
        }
    }
    qDebug()<<"ended the aqcuire AUDIO func";
}


void Screenrecorder::record()
{
    audio_stop=false;
    gotFirstValidVideoPacket=false;

    eloborate_thread = make_unique<thread> ([this]() {
        this->make_error_handler([this]() {
            this->decodeAndEncode();
        })();
    });

    captureVideo_thread = make_unique<thread>([this]() {
        this->make_error_handler([this]() {
            this->getRawPackets();
        })();
    });
    if (vd.audio) {
        captureAudio_thread=make_unique<thread>([this]() {
            this->make_error_handler([this]() {
                this->acquireAudio();
            })();
        });
    }
}
