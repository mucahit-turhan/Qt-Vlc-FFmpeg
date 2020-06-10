#include "qtplayer.h"
#include "ui_qtplayer.h"

#include<QDebug>
#include<QMessageBox>
#include<QTimer>
#include<QFileDialog>
#include<QCloseEvent>
#include<QRegExpValidator>
#include<QLineEdit>
#include <QMutex>
#include<QGraphicsView>
#include <QDateTime>
#include <windows.h>
#include <vlc/vlc.h>
#include <QImage>
#include <QMutex>
#include <QCoreApplication>

#define qtu( i ) ((i).toUtf8().constData())

QtPlayer::QtPlayer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QtPlayer)
{

    ui->setupUi(this);

    playButton = ui->playButton;
    volumeSlider = ui->volumeSlider;
    slider = ui->horizontalSlider;
    videoWidget = ui->videoWidget;

    isPlay = false;

    vlcPlayer = NULL;

    /* Initialize libVLC */
    vlcInstance = libvlc_new(0, NULL);

    /* Complain in case of broken installation */
    if (vlcInstance == NULL) {
        QMessageBox::critical(this, "Qt libVLC player", "Could not init libVLC");
        exit(1);
    }

    /* Interface initialization */
    initUI();
}

QtPlayer::~QtPlayer()
{
    if (vlcInstance)
            libvlc_release(vlcInstance);

    delete ui;
}

void QtPlayer::initUI() {

    QAction* Open = ui->actionOpen;
    QAction* Quit = ui->actionQuit;
    QAction* AboutVLC = ui->actionAbout_VLC;
    QAction* AboutQt = ui->actionAbout_Qt;

    Open->setShortcut(QKeySequence("Ctrl+O"));
    Quit->setShortcut(QKeySequence("Ctrl+Q"));

    connect(Open,    SIGNAL(triggered()), this, SLOT(openFile()));
    connect(Quit,    SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(AboutVLC,SIGNAL(triggered()), this, SLOT(aboutVLC()));
    connect(AboutQt, SIGNAL(triggered()), this, SLOT(aboutQt()));

    QObject::connect(playButton, SIGNAL(clicked()), this, SLOT(play()));

    QPushButton *stopButton = ui->stopButton;
    QObject::connect(stopButton, SIGNAL(clicked()), this, SLOT(stop()));

    QPushButton *muteButton = ui->muteButton;
    QObject::connect(muteButton, SIGNAL(clicked()), this, SLOT(mute()));

    QPushButton *fullscreenButton = ui->fullScreenButton;
    QObject::connect(fullscreenButton, SIGNAL(clicked()), this, SLOT(fullscreen()));

    QObject::connect(volumeSlider, SIGNAL(sliderMoved(int)), this, SLOT(changeVolume(int)));
    volumeSlider->setValue(61);

    slider->setMaximum(1000);
    QObject::connect(slider, SIGNAL(sliderMoved(int)), this, SLOT(changePosition(int)));

    /* A timer to update the sliders */
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateGUI()));
    timer->start(100);

    ui->udpLineEdit->setInputMask("000.000.000.000:0000");
    ui->rtspLineEdit->setInputMask("000.000.000.000:0000");
    ui->rtpLineEdit->setInputMask("000.000.000.000:0000");

    QPushButton *udpPlayButton = ui->udpPlayButton;
    QObject::connect(udpPlayButton, SIGNAL(clicked()), this, SLOT(playUDP()));

    QPushButton *rtspPlayButton = ui->rtspPlayButton;
    QObject::connect(rtspPlayButton, SIGNAL(clicked()), this, SLOT(playRTSP()));

    QPushButton *rtpPlayButton = ui->rtpPlayButton;
    QObject::connect(rtpPlayButton, SIGNAL(clicked()), this, SLOT(playRTP()));

}

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
 /*
static void format_callback(void *opaque, void *picture)
{
    Q_UNUSED(picture);

    (void)opaque;
}
*/

unsigned format_callback(void**user_data_ptr, char*chroma, unsigned *width, unsigned *height, unsigned *pitches, unsigned *lines) {
   context*user_data=(context*)(*user_data_ptr);

   /* set the output format to RGBA */
   memcpy(chroma, "RV32", 4); /* 'RV32' is 'RGBA'
   /* leave dimensions intact, but store them
    * now's the time to resize user_data->img to hold that much memory
    */
   //user_data->width =*width;
   //user_data->height=*height;
   *pitches=(*width)*4; /* 4 is the pixel size for RGBA */
   *lines=*height;

   return 1;
}

//--------------------------------------------GET FRAME FROM VLC----------------------------------------

void QtPlayer::openFile(const char *streamUrl, bool stream) {

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

    /* Stop if something is playing */
    if (vlcPlayer && libvlc_media_player_is_playing(vlcPlayer))
        stop();

    libvlc_media_t *vlcMedia{nullptr};

    if(stream)
        vlcMedia = libvlc_media_new_location(vlcInstance, streamUrl);
    else{
        QString fileOpen = QFileDialog::getOpenFileName(this, tr("Load a file"), "~");
        vlcMedia = libvlc_media_new_path(vlcInstance, qtu(fileOpen));
    }

    if (!vlcMedia)
        return;

    /* Create a new libvlc player */
    vlcPlayer = libvlc_media_player_new_from_media (vlcMedia);

    /* Release the media */
    libvlc_media_release(vlcMedia);

    /* Integrate the video in the interface */
#if defined(Q_OS_MAC)
    libvlc_media_player_set_nsobject(vlcPlayer, (void *)videoWidget->winId());
#elif defined(Q_OS_UNIX)
    libvlc_media_player_set_xwindow(vlcPlayer, videoWidget->winId());
#elif defined(Q_OS_WIN)
    libvlc_media_player_set_hwnd(vlcPlayer,(void*)videoWidget->winId());
#endif

    /* And start playback */
    libvlc_media_player_play (vlcPlayer);

    if (libvlc_media_player_is_playing(vlcPlayer))
    {
        /* Update playback button */
        playButton->setText("Pause");
        isPlay = !isPlay;
    }
    else
    {
        QMessageBox::about(this,"Stream","There is no stream");
    }
}

void QtPlayer::play() {

    if (!vlcPlayer)
        return;

    if (libvlc_media_player_is_playing(vlcPlayer))
    {
        /* Pause */
        libvlc_media_player_pause(vlcPlayer);
        playButton->setText("Play");
    }
    else
    {
        /* Play again */
        libvlc_media_player_play(vlcPlayer);
        playButton->setText("Pause");
    }
}

void QtPlayer::playUDP(){

    if(!isPlay)
        stop();

    QString udp = "udp://@";
    udp.append(ui->udpLineEdit->text());

    QByteArray udpArray = udp.toLocal8Bit();
    char* buffer = udpArray.data();

    openFile(buffer,true);

    buffer = nullptr;
    delete[] buffer;
}

void QtPlayer::playRTSP(){

    if(!isPlay)
        stop();

    QString rtsp = "rtsp://@"; //yanlis olabilir
    rtsp.append(ui->rtspLineEdit->text());

    QByteArray rtspArray = rtsp.toLocal8Bit();
    char* buffer = rtspArray.data();

    openFile(buffer,true);
    qInfo() << rtsp;

    buffer = nullptr;
    delete[] buffer;
}

void QtPlayer::playRTP(){

    if(!isPlay)
        stop();

    QString rtp = "rtp://@"; //yanlis olabilir
    rtp.append(ui->rtpLineEdit->text());

    QByteArray rtpArray = rtp.toLocal8Bit();
    char* buffer = rtpArray.data();

    openFile(buffer,true);
    qInfo() << rtp;

    buffer = nullptr;
    delete[] buffer;
}

int QtPlayer::changeVolume(int vol) { /* Called on volume slider change */

    if (vlcPlayer)
        return libvlc_audio_set_volume (vlcPlayer,vol);

    return 0;
}

void QtPlayer::changePosition(int pos) { /* Called on position slider change */

    if (vlcPlayer)
        libvlc_media_player_set_position(vlcPlayer, (float)pos/1000.0);
}

void QtPlayer::updateGUI() { //Update interface and check if song is finished

    if (!vlcPlayer)
        return;

    /* update the timeline */
    float pos = libvlc_media_player_get_position(vlcPlayer);
    slider->setValue((int)(pos*1000.0));

    /* Stop the media */
    if (libvlc_media_player_get_state(vlcPlayer) == libvlc_Ended)
        this->stop();
}

void QtPlayer::stop() {
    if(vlcPlayer) {
        /* stop the media player */
        libvlc_media_player_stop(vlcPlayer); //_async idi

        /* release the media player */
        libvlc_media_player_release(vlcPlayer);

        /* Reset application values */
        vlcPlayer = NULL;
        slider->setValue(0);
        playButton->setText("Play");
        isPlay = !isPlay;
    }
}

void QtPlayer::mute() {
    if(vlcPlayer) {
        if(volumeSlider->value() == 0) { //if already muted...

                this->changeVolume(80);
                volumeSlider->setValue(80);

        } else { //else mute volume

                this->changeVolume(0);
                volumeSlider->setValue(0);

        }
    }
}

void QtPlayer::aboutVLC()
{
    QMessageBox::about(this, "Qt libVLC player demo", QString::fromUtf8(libvlc_get_version()) );
}

void QtPlayer::aboutQt()
{
    QMessageBox::about(this, tr("About Application"),
                  tr("The <b>Application</b> example demonstrates how to "
                     "write modern GUI applications using Qt, with a menu bar, "
                     "toolbars, and a status bar."));
}

void QtPlayer::fullscreen()
{
   if (isFullScreen()) {
       showNormal();
       menuWidget()->show();
   }
   else {
       showFullScreen();
       menuWidget()->hide();
   }
}

void QtPlayer::closeEvent(QCloseEvent *event) {
    stop();
    event->accept();
}

void QtPlayer::on_takeSnapshot_clicked()
{
    if(vlcPlayer){
        //date format should be defined
        QString date = QDateTime::currentDateTime().toString();
        QByteArray ba = date.toLocal8Bit();
        char* fileName = ba.data();
        strcat(fileName,".png");
        libvlc_video_take_snapshot(vlcPlayer, 0, fileName, 0, 0);
    }
}
