#ifndef VLC_H
#define VLC_H

#include <3rdparty/VLC/include/vlc/vlc.h>
#include <QLineEdit>
#include <QSlider>
#include <QPushButton>
#include <QWidget>

class VLC{
public:
    VLC();
    ~VLC();

public slots:
    void openFile(const char* streamUrl = nullptr, bool stream = false, QWidget *videoWidget = nullptr, QPushButton *playButton = nullptr,QString fileOpen = nullptr);
    void play(QPushButton *playButton);
    //streamType may be boolean -> isUDP, isRTP, is RTSP
    void playStream(QString lineEditText, QString streamType,QWidget *videoWidget, QPushButton *playButton);
    int changeVolume(int);
    void changePosition(int);
    void updateGUI(QSlider *slider); //Belki qtplayer da olabilir
    void stop();
    void mute(QSlider* slider);
    void takeSnapshot(const char* fileName);

private:
    libvlc_instance_t *vlcInstance;
    libvlc_media_player_t *vlcPlayer;
    bool isPlay;
};

#endif // VLC_H
