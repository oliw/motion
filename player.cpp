#include <player.h>
#include <video.h>
#include <frame.h>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QImage>
#include <QDebug>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "MatToQImage.h"

Player::Player(QObject *parent):QThread(parent)
{
    stopped = true;
}

void Player::setVideo(Video* video)
{
    this->video = video;
    frameNumber = 0;
    frameRate = 27;
}

void Player::play()
{
    if (!isRunning()) {
        if (isStopped()){
            stopped = false;
        }
        qDebug() << "Player::play - playing video on a new thread";
        start(LowPriority);
    }
}

void Player::step()
{
    if (!isStopped())
        return;
    if (frameNumber == video->getFrameCount()) {
        stop();
        rewind();
    }
    showImage(frameNumber);
    frameNumber++;
}

void Player::showImage(int frameNumber)
{
    frame = video->getImageAt(frameNumber);
    image = MatToQImage(frame);
    emit processedImage(image, frameNumber);
}

void Player::run()
{
    int delay = (1000/frameRate);
    while (!stopped) {
        if (frameNumber == video->getFrameCount()) {
            stop();
            rewind();
        }
        else {
            qDebug() << "Player::run - playing frame " << frameNumber;
            showImage(frameNumber);
            frameNumber++;
            this->msleep(delay);
        }
    }
}

Player::~Player()
{
    mutex.lock();
    stopped = true;
    condition.wakeOne();
    mutex.unlock();
    wait();
}

bool Player::isStopped() const
{
    return stopped;
}

void Player::rewind()
{
    frameNumber = 0;
    showImage(0);
}

void Player::stop()
{
    stopped = true;
    qDebug() << "Player::stop - Playing stopped";
}

void Player::msleep(int ms) {
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000};
    nanosleep(&ts, NULL);
}
