#ifndef FFMPEG_H
#define FFMPEG_H

extern "C" {
/*Include ffmpeg header file*/
#include <3rdparty/FFMPEG/include/libavformat/avformat.h>
#include <3rdparty/FFMPEG/include/libavcodec/avcodec.h>
#include <3rdparty/FFMPEG/include/libswscale/swscale.h>
#include <3rdparty/FFMPEG/include/libavutil/imgutils.h>
#include <3rdparty/FFMPEG/include/libavutil/opt.h>
#include <3rdparty/FFMPEG/include/libavutil/mathematics.h>
#include <3rdparty/FFMPEG/include/libavutil/samplefmt.h>
}

class FFmpeg{
public:
    FFmpeg(bool is_command_line);
    ~FFmpeg();

    bool isCommandLine;

    int saveVideoFromStream();
    void encode(AVCodecContext* enc_ctx, AVFrame* frame, AVPacket* pkt,FILE* outfile);
    void saveFrame(unsigned char* buf, int wrap, int xsize, int ysize, char* filename);
    int decode_packet(AVPacket* pPacket, AVCodecContext* pCodecContext, AVFrame* pFrame);

    void commandLine(const char* command);
private:
    void logging(const char* fmt, ...);

    AVFormatContext* ifmt_ctx;
    AVFormatContext* ofmt_ctx;
    char* in_filename;
    char* out_filename;
    AVDictionary* avdic;
    AVOutputFormat* ofmt;
};


#endif // FFMPEG_H
