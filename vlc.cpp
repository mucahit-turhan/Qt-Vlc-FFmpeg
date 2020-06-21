#include "vlc.h"
#include <3rdparty/VLC/include/vlc/vlc.h>

#define qtu( i ) ((i).toUtf8().constData())

/*
//--------------------------------------------GET FRAME FROM VLC----------------------------------------
//https://stackoverflow.com/questions/32825363/working-with-vlc-smem
//http://www.programmersought.com/article/590595045/
//alttakiler smem ile
//https://stackoverflow.com/questions/23092940/get-frame-from-video-with-libvlc-smem-and-convert-it-to-opencv-mat-c
//https://github.com/jrterven/OpenCV-VLC/blob/master/OpenCV_VLC/OpenCV_VLC/OpenCV_VLC.cpp
// Define the resolution of the output video
#define VIDEO_WIDTH   640
#define VIDEO_HEIGHT  480

struct context {
    QMutex mutex;
    uchar *pixels;
};

static void *lock(void *opaque, void **planes)
{
    printf("lock");
    struct context *ctx = (context *)opaque;
    ctx->mutex.lock();

         // tell VLC to put the decoded data in the buffer
    *planes = ctx->pixels;

    return NULL;
}

 // get the argb image and save it to a file
static void unlock(void *opaque, void *picture, void *const *planes)
{
    Q_UNUSED(picture);
    printf("unlock");
    struct context *ctx = (context *)opaque;
    unsigned char *data = (unsigned char *)*planes;
    static int frameCount = 1;

    QImage image(data, VIDEO_WIDTH, VIDEO_HEIGHT, QImage::Format_ARGB32);
    image.save(QString("frame_%1.png").arg(frameCount++));

    ctx->mutex.unlock();
}

static void display(void *opaque, void *picture)
{
    Q_UNUSED(picture);
    (void)opaque;
}

static void *lock_frame(void *opaque, void **planes)
{
    struct context *ctx = (context *)opaque;
    ctx->mutex.lock();

         // tell VLC to put the decoded data in the buffer
    *planes = ctx->pixels;

    return NULL;
}

 // get the argb image and save it to a file
static void unlock_frame(void *opaque, void *picture, void *const *planes)
{
    Q_UNUSED(picture);

    struct context *ctx = (context *)opaque;
    unsigned char *data = (unsigned char *)*planes;
    static int frameCount = 1;

    //QImage image(data, VIDEO_WIDTH, VIDEO_HEIGHT, QImage::Format_ARGB32);
    //image.save(QString("frame_%1.png").arg(frameCount++));

    ctx->mutex.unlock();
}

static void format_callback(void *opaque, void *picture)
{
    Q_UNUSED(picture);

    (void)opaque;
}


unsigned format_callback(void**user_data_ptr, char*chroma, unsigned *width, unsigned *height, unsigned *pitches, unsigned *lines) {
   context*user_data=(context*)(*user_data_ptr);


   memcpy(chroma, "RV32", 4); /* 'RV32' is 'RGBA'

   //user_data->width =*width;
   //user_data->height=*height;
   *pitches=(*width)*4;
   *lines=*height;

   return 1;
}
*/

//--------------------------------------------GET FRAME FROM VLC----------------------------------------

VLC::VLC(){
    isPlay = false;

    vlcPlayer = NULL;

    /* Initialize libVLC */
    vlcInstance = libvlc_new(0, NULL);

    /* Complain in case of broken installation */
    if (vlcInstance == NULL) {
        //QMessageBox::critical(this, "Qt libVLC player", "Could not init libVLC");
        exit(1);
    }
}

VLC::~VLC(){
    if (vlcInstance)
        libvlc_release(vlcInstance);
}

void VLC::openFile(const char *streamUrl, bool stream, QWidget *videoWidget, QPushButton *playButton, QString fileOpen) {

//   libvlc_media_t *media;

//         // wait 20 seconds
//    int waitTime = 1000 * 20;

//    struct context ctx;
//    ctx.pixels = new uchar[VIDEO_WIDTH * VIDEO_HEIGHT * 4];
//    memset(ctx.pixels, 0, VIDEO_WIDTH * VIDEO_HEIGHT * 4);

//         // Create and initialize a libvlc instance
//    vlcInstance = libvlc_new(0, NULL);

//         // Create a media, the parameter is a property location (for example: a valid URL).
//    media = libvlc_media_new_location(vlcInstance, "rtp://233.233.233.233:6666");

//    libvlc_media_add_option(media, ":avcodec-hw=none");

//         // Create a media player playback environment
//    vlcPlayer = libvlc_media_player_new_from_media(media);

//         // Now, there is no need to keep media.
//    libvlc_media_release(media);

//         // Set the callback to extract the frame or display it on the screen.
//    libvlc_video_set_callbacks(vlcPlayer, lock, unlock, display, &ctx);
//    libvlc_video_set_format(vlcPlayer, "RGBA", VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_WIDTH * 4);

//         // play media player
//    libvlc_media_player_play(vlcPlayer);

//         // let it play for a while
//    Sleep(waitTime);

//         // Stop play
//    libvlc_media_player_stop(vlcPlayer);

//         // release media player
//    libvlc_media_player_release(vlcPlayer);

//         // release the libvlc instance
//    libvlc_release(vlcInstance);


    if (vlcPlayer && libvlc_media_player_is_playing(vlcPlayer))
        stop();

    libvlc_media_t *vlcMedia{nullptr};

    if(stream)
        vlcMedia = libvlc_media_new_location(vlcInstance, streamUrl);
    else
        vlcMedia = libvlc_media_new_path(vlcInstance, qtu(fileOpen));

    if (!vlcMedia)
        return;

    vlcPlayer = libvlc_media_player_new_from_media (vlcMedia);

    libvlc_media_release(vlcMedia);

#if defined(Q_OS_MAC)
    libvlc_media_player_set_nsobject(vlcPlayer, (void *)videoWidget->winId());
#elif defined(Q_OS_UNIX)
    libvlc_media_player_set_xwindow(vlcPlayer, videoWidget->winId());
#elif defined(Q_OS_WIN)
    libvlc_media_player_set_hwnd(vlcPlayer,(void*)videoWidget->winId());
#endif

    libvlc_media_player_play(vlcPlayer);

    if (libvlc_media_player_is_playing(vlcPlayer)){
        playButton->setText("Pause");
        isPlay = !isPlay;
    }
    else{
        qInfo("There is no stream");
        //QMessageBox::about(qtPlayer,"Stream","There is no stream");
    }
}

void VLC::play(QPushButton *playButton) {

    if (!vlcPlayer)
        return;

    if (libvlc_media_player_is_playing(vlcPlayer)){
        libvlc_media_player_pause(vlcPlayer);
        playButton->setText("Play");
    }
    else{
        libvlc_media_player_play(vlcPlayer);
        playButton->setText("Pause");
    }
}

void VLC::playStream(QString lineEditText, QString streamType,QWidget *videoWidget, QPushButton *playButton){
    if(!isPlay)
        stop();

    QString st = streamType;
    st.append(lineEditText);

    QByteArray udpArray = st.toLocal8Bit();
    char* buffer = udpArray.data();

    openFile(buffer,true,videoWidget,playButton);

    buffer = nullptr;
    delete[] buffer;
}

int VLC::changeVolume(int vol) { /* Called on volume slider change */
    if (vlcPlayer)
        return libvlc_audio_set_volume (vlcPlayer,vol);
    return 0;
}

void VLC::changePosition(int pos) { /* Called on position slider change */
    if (vlcPlayer)
        libvlc_media_player_set_position(vlcPlayer, (float)pos/1000.0);
}

void VLC::updateGUI(QSlider *slider) { //Update interface and check if song is finished
    if (!vlcPlayer)
        return;

    /* update the timeline */
    float pos = libvlc_media_player_get_position(vlcPlayer);
    slider->setValue((int)(pos*1000.0));

    /* Stop the media */
    if (libvlc_media_player_get_state(vlcPlayer) == libvlc_Ended)
        this->stop();
}

void VLC::stop() {
    if(vlcPlayer) {
        /* stop the media player */
        libvlc_media_player_stop(vlcPlayer); //_async idi

        /* release the media player */
        libvlc_media_player_release(vlcPlayer);

        /* Reset application values */
        vlcPlayer = NULL;
        isPlay = !isPlay;
    }
}

void VLC::mute(QSlider* slider) {
    if(vlcPlayer) {
        if(slider->value() == 0) { //if already muted...
            this->changeVolume(80);
            slider->setValue(80);
        }
        else { //else mute volume
            this->changeVolume(0);
            slider->setValue(0);
        }
    }
}

void VLC::takeSnapshot(const char* fileName)
{
    if(vlcPlayer){
        libvlc_video_take_snapshot(vlcPlayer, 0, fileName, 0, 0);
    }
}
