#include "ffmpeg.h"
#include <QProcess>
#include <QDebug>

extern "C" {
#include <3rdparty/FFMPEG/include/libavcodec/avcodec.h>
#include <3rdparty/FFMPEG/include/libavutil/imgutils.h>
#include <3rdparty/FFMPEG/include/libavutil/opt.h>
}

//For FFmpeg gerekli mi dene
//#ifndef INT64_C
//#define INT64_C(c) (c ## LL)
//#define UINT64_C(c) (c ## ULL)
//#endif

//#define AV_CODEC_FLAG_GLOBAL_HEADER (1 << 22)
//#define CODEC_FLAG_GLOBAL_HEADER AV_CODEC_FLAG_GLOBAL_HEADER
//#define AVFMT_RAWPICTURE 0x0020
//For FFmpeg gerekli mi dene

//This code should be updated for latest FFmpeg version

FFmpeg::FFmpeg(){
    qInfo("FFMPEG CONSTRUCTOR");
    ifmt_ctx = NULL;
    ofmt_ctx = NULL;
    avdic = NULL;
    ofmt = NULL;
}

FFmpeg::~FFmpeg(){
    qInfo("FFMPEG DESTRUCTOR");
    av_dict_free(&avdic);
    avformat_close_input(&ifmt_ctx);

    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx->pb);

    avformat_free_context(ofmt_ctx);
}

void FFmpeg::runCommandLine(const QString command){
    //Thread must be used. If not, gui wait the end of the process
    QProcess FFMPEG;
    QString process = "ffmpeg";
    QStringList paramList = command.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    paramList.removeFirst();
    FFMPEG.start(process,paramList);
    if ( !(FFMPEG.waitForFinished()) )
        qDebug() << "Conversion failed:" << FFMPEG.errorString();
    else
        qDebug() << "Conversion output:" << FFMPEG.readAll();
}
/*
int FFmpeg::saveVideoFromStream(){
        in_filename = "rtp://233.233.233.233:6666";

        //date format should be defined
        QString date = QDateTime::currentDateTime().toString();
        QByteArray ba = date.toLocal8Bit();
        out_filename = ba.data();
        //out_filename = "output.ts";

        av_register_all();
        avformat_network_init();

        char option_key[] = "rtsp_transport";
        char option_value[] = "tcp";
        av_dict_set(&avdic, option_key, option_value, 0);
        char option_key2[] = "max_delay";
        char option_value2[] = "5000000";
        av_dict_set(&avdic, option_key2, option_value2, 0);

        AVPacket pkt;
        int video_index = -1;
        int frame_index = 0;

        int i;

        //Open the input stream
        int ret;
        if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, &avdic)) < 0)
        {
            std::cout << "Could not open input file." << '\n';
            goto end;
        }
        if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0)
        {
            std::cout << "Failed to retrieve input stream information" << '\n';
            goto end;
        }


        //nb_streams represent several streams

        for (i = 0; i < ifmt_ctx->nb_streams; i++)
        {
            if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                //Video streaming
                video_index = i;
                std::cout << "get videostream." << '\n';
                break;
            }
        }

        av_dump_format(ifmt_ctx, 0, in_filename, 0);

        //Open the output stream
        avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);

        if (!ofmt_ctx)
        {
            printf("Could not create output context\n");
            ret = AVERROR_UNKNOWN;
            goto end;
        }
        ofmt = ofmt_ctx->oformat;

        for (i = 0; i < ifmt_ctx->nb_streams; i++)
        {
            AVStream* in_stream = ifmt_ctx->streams[i];
            AVStream* out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);

            if (!out_stream)
            {
                printf("Failed allocating output stream.\n");
                ret = AVERROR_UNKNOWN;
                goto end;
            }

            //Copy the encoding information of the output stream to the input stream
            ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
            if (ret < 0)
            {
                printf("Failed to copy context from input to output stream codec context\n");
                goto end;
            }

            out_stream->codec->codec_tag = 0;

            if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        }

        av_dump_format(ofmt_ctx, 0, out_filename, 1);

        if (!(ofmt->flags & AVFMT_NOFILE))
        {
            ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
            if (ret < 0)
            {
                printf("Could not open output URL '%s'", out_filename);
                goto end;
            }
        }

        //Write file header to output file
        ret = avformat_write_header(ofmt_ctx, NULL);
        if (ret < 0)
        {
            printf("Error occured when opening output URL\n");
            goto end;
        }

        //Continuous access to data packets in the while loop, regardless of audio and video, is stored in the file
        while (1)
        {
            AVStream* in_stream, * out_stream;
            //Get a packet from the input stream
            ret = av_read_frame(ifmt_ctx, &pkt);
            if (ret < 0)
                break;

            in_stream = ifmt_ctx->streams[pkt.stream_index];
            out_stream = ofmt_ctx->streams[pkt.stream_index];
            //copy packet
            //Conversion of PTS/DTS Timing
            pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            //printf("pts %d dts %d base %d\n",pkt.pts,pkt.dts, in_stream->time_base);
            pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
            pkt.pos = -1;

            //Not all packet s in this while loop are video frames. Record when you receive a video frame
            if (pkt.stream_index == video_index)
            {
                printf("Receive %8d video frames from input URL\n", frame_index);
                frame_index++;
            }

            //Write the package data to a file.
            ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
            if (ret < 0)
            {
                if (ret == -22) {
                    continue;
                }
                else {
                    printf("Error muxing packet.error code %d\n", ret);
                    break;
                }

            }

            //Av_free_packet(&pkt); // This sentence has been replaced by av_packet_unref in the new version.
            av_packet_unref(&pkt);
        }


        //Write the end of the file
        av_write_trailer(ofmt_ctx);

    end:
        av_dict_free(&avdic);
        avformat_close_input(&ifmt_ctx);
        //Close input
        if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
            avio_close(ofmt_ctx->pb);
        avformat_free_context(ofmt_ctx);
        if (ret < 0 && ret != AVERROR_EOF)
        {
            std::cout << "Error occured." << '\n';
            return -1;
        }

        return 0;
}
*/


//This method is appropriate for last version of FFmpeg
void FFmpeg::encode(AVCodecContext* enc_ctx, AVFrame* frame, AVPacket* pkt,FILE* outfile)
{
    int ret;


    if (frame)
        printf("Send frame %3\n", frame->pts);

    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0) {
        fprintf(stderr, "Error sending a frame for encoding\n");
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }

        printf("Write packet %3 (size=%5d)\n", pkt->pts, pkt->size);
        fwrite(pkt->data, 1, pkt->size, outfile);
        av_packet_unref(pkt);
    }
}


//This method is appropriate last version of FFmpeg
void FFmpeg::saveFrame(unsigned char* buf, int wrap, int xsize, int ysize, char* filename)
{
    FILE* f;
    int i;
    f = fopen(filename, "w");
    // writing the minimal required header for a pgm file format
    // portable graymap format -> https://en.wikipedia.org/wiki/Netpbm_format#PGM_example
    fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);

    // writing line by line
    for (i = 0; i < ysize; i++)
        fwrite(buf + i * wrap, 1, xsize, f);
    fclose(f);
}

void FFmpeg::logging(const char* fmt, ...)
{
    va_list args;
    fprintf(stderr, "LOG: ");
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}


int FFmpeg::decode_packet(AVPacket* pPacket, AVCodecContext* pCodecContext, AVFrame* pFrame)
{
    // Supply raw packet data as input to a decoder
    // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga58bc4bf1e0ac59e27362597e467efff3
    int response = avcodec_send_packet(pCodecContext, pPacket);

    if (response < 0) {
        //logging("Error while sending a packet to the decoder: %s", av_err2str(response));
        logging("Error while sending a packet to the decoder:");
        return response;
    }

    while (response >= 0)
    {
        // Return decoded output data (into a frame) from a decoder
        // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga11e6542c4e66d3028668788a1a74217c
        response = avcodec_receive_frame(pCodecContext, pFrame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            break;
        }
        else if (response < 0) {
            //logging("Error while receiving a frame from the decoder: %s", av_err2str(response));
            logging("Error while receiving a frame from the decoder:");
            return response;
        }

        if (response >= 0) {
            logging(
                "Frame %d (type=%c, size=%d bytes) pts %d key_frame %d [DTS %d]",
                pCodecContext->frame_number,
                av_get_picture_type_char(pFrame->pict_type),
                pFrame->pkt_size,
                pFrame->pts,
                pFrame->key_frame,
                pFrame->coded_picture_number
            );

            //char frame_filename[1024];
            //snprintf(frame_filename, sizeof(frame_filename), "%s-%d.pgm", "frame", pCodecContext->frame_number);
            // save a grayscale frame into a .pgm file
            //saveFrame(pFrame->data[0], pFrame->linesize[0], pFrame->width, pFrame->height, frame_filename);
        }
    }
    return 0;
}

