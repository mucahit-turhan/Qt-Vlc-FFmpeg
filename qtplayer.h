#ifndef QTPLAYER_H
#define QTPLAYER_H

#include <QMainWindow>
#include <ffmpeg.h>
#include <vlc.h>
#include<QPushButton>
#include<QSlider>

QT_BEGIN_NAMESPACE
namespace Ui { class QtPlayer; }
QT_END_NAMESPACE

class QtPlayer : public QMainWindow
{
    Q_OBJECT

public:
    QtPlayer(QWidget *parent = nullptr);
    ~QtPlayer();

private slots:
    void openFile(const char* streamUrl = nullptr, bool stream = false);
    void play();

    void playUDP();
    void playRTSP();
    void playRTP();

    void stop();
    void mute();
    void aboutVLC();
    void aboutQt();
    void fullscreen();

    int changeVolume(int);
    void changePosition(int);
    void updateGUI();

    void on_takeSnapshot_clicked();

    void on_pushButton_clicked();

protected:
    virtual void closeEvent(QCloseEvent*);

private:
    FFmpeg ffmpeg;
    VLC vlc;
    QPushButton *playButton;
    QSlider *volumeSlider;
    QSlider *slider;
    QWidget *videoWidget;

    void initUI(); //burdakileri qtplayer.ui da tasarlamaya calistik ya bununla yapacaz yada ui ile

    Ui::QtPlayer *ui;


};
#endif // QTPLAYER_H
