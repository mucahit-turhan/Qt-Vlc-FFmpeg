#include "qtplayer.h"
#include "ui_qtplayer.h"

#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>
#include <QCloseEvent>
#include <QDateTime>

#define qtu( i ) ((i).toUtf8().constData())

QtPlayer::QtPlayer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QtPlayer)
{
    ui->setupUi(this);

    playButton      = ui->playButton;
    volumeSlider    = ui->volumeSlider;
    slider          = ui->horizontalSlider;
    videoWidget     = ui->videoWidget;

    /* Interface initialization */
    initUI();
}

QtPlayer::~QtPlayer()
{
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

    ui->udpLineEdit ->setInputMask("000.000.000.000:0000");
    ui->rtspLineEdit->setInputMask("000.000.000.000:0000");
    ui->rtpLineEdit ->setInputMask("000.000.000.000:0000");

    QPushButton *udpPlayButton = ui->udpPlayButton;
    QObject::connect(udpPlayButton, SIGNAL(clicked()), this, SLOT(playUDP()));

    QPushButton *rtspPlayButton = ui->rtspPlayButton;
    QObject::connect(rtspPlayButton, SIGNAL(clicked()), this, SLOT(playRTSP()));

    QPushButton *rtpPlayButton = ui->rtpPlayButton;
    QObject::connect(rtpPlayButton, SIGNAL(clicked()), this, SLOT(playRTP()));

}

void QtPlayer::openFile(const char *streamUrl, bool stream) {
   QStringList pieces = QFileDialog::getOpenFileName(this, QObject::tr("Load a file"), "~").split('/');
   QString filePath = "";
   for(int i=0; i<pieces.length(); i++){
       if(i == (pieces.length() -1))
            filePath.append(pieces[i]);
       else{
           filePath.append(pieces[i]);
           filePath.append('\\');
       }
   }
   vlc.openFile(streamUrl,stream,videoWidget,playButton,filePath);
}

void QtPlayer::play() {
    vlc.play(playButton);
}

void QtPlayer::playUDP(){
    QString s = ui->udpLineEdit->text();
    vlc.playStream(s,"udp://@",videoWidget,playButton);
}

void QtPlayer::playRTSP(){
    QString s = ui->rtspLineEdit->text();
    vlc.playStream(s,"rtsp://@",videoWidget,playButton);
}

void QtPlayer::playRTP(){
    QString s = ui->rtpLineEdit->text();
    vlc.playStream(s,"rtp://@",videoWidget,playButton);
}

int QtPlayer::changeVolume(int vol) {
    return vlc.changeVolume(vol);
}

void QtPlayer::changePosition(int pos) {
    vlc.changePosition(pos);
}

void QtPlayer::updateGUI() {
    vlc.updateGUI(slider);
}

void QtPlayer::stop() {
    vlc.stop();
    slider->setValue(0);
    playButton->setText("Play");
}

void QtPlayer::mute() {
    vlc.mute(slider);
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
    QString date = QDateTime::currentDateTime().toString();
    QByteArray ba = date.toLocal8Bit();
    char* fileName = ba.data();
    strcat(fileName,".png");

    vlc.takeSnapshot(fileName);
}

void QtPlayer::on_pushButton_clicked()
{
    QStringList pieces = QDir::current().path().split('/');
    QString s = ui->ffmpegLineEdit->text();
    QString filePath = "";
    for(int i=0; i<pieces.length(); i++){
        filePath.append(pieces[i]);
        filePath.append('\\');
    }
    QStringList list = s.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    filePath.append(list.last());
    //filePath give us what is the path of video ffmpeg saved with name

    ffmpeg.runCommandLine(s);

}
