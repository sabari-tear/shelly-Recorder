#ifndef SCREENRECORDER_H
#define SCREENRECORDER_H

#include <string>


extern "C" {
#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavutil/audio_fifo.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
}


using namespace std;
typedef struct {
    int width;
    int height;
    int offset_x;
    int offset_y;
    int screen_number;
} RecordingWindowDetails;

typedef struct {
    int fps;
    float quality;
    int compression;
    bool audio;
} VideoDetails;

enum class RecordingStatus {
    recording,
    paused,
    stopped
};

class Screenrecorder
{
public:
    Screenrecorder(RecordingWindowDetails& wd, VideoDetails& vd, string& outFilePath, string& audioDevice);
    RecordingWindowDetails wd;
    VideoDetails vd;
    string outFilePath;
    string audioDevice;

    ~Screenrecorder();
    void start_record();
    void stop_record();
    void pause_record();
    void resume_record();

    //fuction
    void init_devicesEncodec();
    //variables
    AVFormatContext* avFmtCtx;
    AVFormatContext* avFmtCtxOut;
    AVOutputFormat* fmt;
    AVCodec* avEncodec;

    //
    void init_videoSource();
    //
    AVDictionary* avRawOptions;
    int vdo_stream_index;
    AVCodecContext* avRawCodecCtx;
    AVCodec* avDecodec;

    //
    void init_videoVariables();
    //
    AVStream* video_st;
    AVCodecContext* avEncoderCtx;
    struct SwsContext* swsCtx;
    AVFrame* avYUVFrame;

    //
    void init_audioSource();
    //
    AVFormatContext* FormatContextAudio;
    AVDictionary* AudioOptions;
    AVInputFormat* AudioInputFormat;
    int ado_stream_index;
    AVStream* audio_st;
};

#endif // SCREENRECORDER_H
