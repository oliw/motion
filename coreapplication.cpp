#include "coreapplication.h"
#include "tools.h"
#include <QDebug>
#include <QFileInfo>

CoreApplication::CoreApplication(QObject *parent) :
    QObject(parent)
{
    QObject::connect(&vp, SIGNAL(processProgressChanged(float)), this, SIGNAL(processProgressChanged(float)));
    clear();
}

void CoreApplication::loadOriginalVideo(QString path)
{
    emit processStatusChanged(CoreApplication::LOAD_VIDEO, true);
    clear();
    cv::VideoCapture vc;
    if (!vc.open(path.toStdString())) {
        qDebug() << "VideoProcessor::loadVideo - Video could not be opened";
        emit processStatusChanged(CoreApplication::LOAD_VIDEO, false);
        return;
    }
    // Load Video
    int frameCount = vc.get(CV_CAP_PROP_FRAME_COUNT);
    int fps = vc.get(CV_CAP_PROP_FPS);
    originalVideo = new Video(frameCount,fps);
    QFileInfo fileInfo = QFileInfo(path);
    originalVideo->setVideoName(fileInfo.fileName());
    int currentFrame = 0;
    Mat buffer;
    while (vc.read(buffer)) {
        emit processProgressChanged((float)currentFrame/frameCount);
        Frame* newFrame = new Frame(buffer.clone(),originalVideo);
        originalVideo->appendFrame(newFrame);
        currentFrame++;
    }
    emit originalVideoLoaded(originalVideo);
    emit processStatusChanged(CoreApplication::LOAD_VIDEO, false);
}

void CoreApplication::saveNewVideo(QString path)
{
    emit processStatusChanged(CoreApplication::SAVE_VIDEO, true);
    VideoWriter record(path.toStdString(), CV_FOURCC('D','I','V','X'),originalVideo->getOrigFps(), newVideo->getSize());
    assert(record.isOpened());
    for (int f = 0; f < newVideo->getFrameCount(); f++) {
        emit processProgressChanged((float)f/newVideo->getFrameCount());
        const Frame* frame = newVideo->getFrameAt(f);
        const Mat& img = frame->getOriginalData();
        record << img;
    }
    emit processStatusChanged(CoreApplication::SAVE_VIDEO, false);
}

void CoreApplication::calculateOriginalMotion()
{
    emit processStatusChanged(CoreApplication::FEATURE_DETECTION, true);
    vp.detectFeatures(originalVideo);
    emit processStatusChanged(CoreApplication::FEATURE_DETECTION, false);
    emit processStatusChanged(CoreApplication::FEATURE_TRACKING, true);
    vp.trackFeatures(originalVideo);
    emit processStatusChanged(CoreApplication::FEATURE_TRACKING, false);
    emit processStatusChanged(CoreApplication::OUTLIER_REJECTION, true);
    vp.rejectOutliers(originalVideo);
    emit processStatusChanged(CoreApplication::OUTLIER_REJECTION, false);
    emit processStatusChanged(CoreApplication::ORIGINAL_MOTION, true);
    vp.calculateMotionModel(originalVideo);
    emit processStatusChanged(CoreApplication::ORIGINAL_MOTION, false);
    setOriginalGlobalMotion();
}

void CoreApplication::calculateNewMotion() {
    emit processStatusChanged(CoreApplication::NEW_MOTION, true);
    vp.calculateUpdateTransform(originalVideo);
    emit processStatusChanged(CoreApplication::NEW_MOTION, false);
    emit processStatusChanged(CoreApplication::NEW_VIDEO, true);
    newVideo = new Video(originalVideo->getFrameCount());
    vp.applyCropTransform(originalVideo, newVideo);
    emit newVideoCreated(newVideo);
    emit processStatusChanged(CoreApplication::NEW_VIDEO, false);
    setNewGlobalMotion();
}

void CoreApplication::evaluateNewMotion() {
    qDebug() << "evaluateNewMotion - Not yet implemented";
}

void CoreApplication::drawGraph() {
    qDebug() << "drawGraph - Not yet implemented";
}

void CoreApplication::setOriginalPointMotion(QMap<int, QPoint> locations) {
    emit processStatusChanged(CoreApplication::PLOTTING_MOVEMENT, true);
    originalPointMotion.clear();
    // Convert QPoint to Point2f
    QMapIterator<int, QPoint> i(locations);
    while (i.hasNext()) {
        i.next();
        QPoint p = i.value();
        Point2f pNew = Tools::QPointToPoint2f(p);
        originalPointMotion.insert(i.key(),pNew);
    }
    // Normalise
    originalPointMotion = Tools::moveToOriginDataSet(originalPointMotion);
    emit processStatusChanged(CoreApplication::PLOTTING_MOVEMENT, false);
}

void CoreApplication::setOriginalGlobalMotion() {
    emit processStatusChanged(CoreApplication::PLOTTING_MOVEMENT, true);
    originalGlobalMotion.clear();
    // Work backwards
    Point2f p(0,0);
    originalGlobalMotion.insert(originalVideo->getFrameCount()-1, p);
    for (int f = originalVideo->getFrameCount()-1; f > 0; f--) {
        Frame* frame = originalVideo->accessFrameAt(f);
        const Mat& aff = frame->getAffineTransform();
        p = Tools::applyAffineTransformation(aff, p);
        originalGlobalMotion.insert(f-1, p);
    }
    // Normalise
    originalGlobalMotion = Tools::moveToOriginDataSet(originalGlobalMotion);
    emit processStatusChanged(CoreApplication::PLOTTING_MOVEMENT, false);
}

void CoreApplication::setNewGlobalMotion() {
    emit processStatusChanged(CoreApplication::PLOTTING_MOVEMENT, true);
    // Work backwards
    Point2f p(0,0);
    newGlobalMotion.insert(originalVideo->getFrameCount()-1, p);
    for (int f = originalVideo->getFrameCount()-1; f > 0; f--) {
        Frame* frame = originalVideo->accessFrameAt(f);
        const Mat& aff = frame->getAffineTransform();
        p = Tools::applyAffineTransformation(aff, p);
        const Mat& update = frame->getUpdateTransform();
        p = Tools::applyAffineTransformation(update, p);
        newGlobalMotion.insert(f-1, p);
    }
    // Normalise
    newGlobalMotion = Tools::moveToOriginDataSet(newGlobalMotion);
    emit processStatusChanged(CoreApplication::PLOTTING_MOVEMENT, false);
}

void CoreApplication::drawGraph(bool originalPointMotion, bool originalGlobalMotion, bool newGlobalMotion, bool x, bool y)
{
    qDebug() << "Ignoring function params and drawing original Global motion";
    ev.drawData(this->originalGlobalMotion);
}


void CoreApplication::clear() {
    delete(originalVideo);
    delete(newVideo);
    originalPointMotion.clear();
    originalGlobalMotion.clear();
    newGlobalMotion.clear();
}

