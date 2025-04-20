#ifndef SCREENRECORDER_H
#define SCREENRECORDER_H

#include <string>


extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
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
    Screenrecorder(RecordingWindowDetails* window_details, VideoDetails* video_Details, string* outFilePath, string* audioDevice);
    ~Screenrecorder();
    void start_record();
    void stop_record();
    void pause_record();
    void resume_record();

    //fuctions
    void readydevices_encodec();

    //variables
    AVFormatContext* avFmtCtx;
    AVFormatContext* avFmtCtxOut;
    AVOutputFormat* fmt;
    AVCodec *avEncodec;

    // constructors
    RecordingWindowDetails window_details;
    VideoDetails video_Details;
    string outFilePath;
    string audioDevice;
};

#endif // SCREENRECORDER_H
