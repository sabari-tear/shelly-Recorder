#ifndef SCREENRECORDER_H
#define SCREENRECORDER_H

#include <string>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <queue>

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
    AVStream* vdo_input_st;
    AVCodecContext* avRawCodecCtx;
    AVCodec* avDecodec;
    //
    void init_videoVariables();
    //
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
    AVStream* ado_input_st;

    //
    void init_audioVariables();
    //
    AVCodec* AudioDecodec;
    AVCodec* AudioEncodec;
    AVCodecContext* AudioEncoderCtx;
    AVCodecContext* AudioDecoderCtx;
    AVStream* audio_output_st;
    int audioIndexOut;

    //
    void init_outputFile();
    //

    //
    void record();
    //
    bool audio_stop;
    bool gotFirstValidVideoPacket;
    unique_ptr<thread> eloborate_thread;
    unique_ptr<thread> captureVideo_thread;
    unique_ptr<thread> captureAudio_thread;

    //
    function<void(void)> make_error_handler(function<void(void)> f);
    //
    mutex error_queue_m;
    int terminated_threads=0;
    condition_variable error_queue_cv;
    queue <string> error_queue;

    //
    void decodeAndEncode();
    //
    mutex avRawPkt_queue_mutex;
    queue<AVPacket *> avRawPkt_queue;
    mutex write_lock;
    mutex status_lock;
    RecordingStatus status;

    //
    bool isVideoEnd();
    //
    mutex video_lock;
    bool video_end = false;

    //
    void getRawPackets();
    //
    bool video_ready=false;
    condition_variable cv_video;
    condition_variable cv_audio;
    condition_variable cv;
    bool audio_end = false;

    //
    int getlatestFramesValue();
    //

    //
    bool audioReady();
    //
    mutex audio_lock;
    bool audio_ready;

    //
    void videoEnd();
    //

    //
    void acquireAudio();
    //

    //
    void init_fifo();
    //
    AVAudioFifo *AudioFifoBuff;

};

#endif // SCREENRECORDER_H
