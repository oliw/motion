#include "coreapplication.h"
#include "tools.h"
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include "frame.h"

CoreApplication::CoreApplication(QObject *parent) :
    QObject(parent)
{
    QObject::connect(&vp, SIGNAL(processProgressChanged(float)), this, SIGNAL(processProgressChanged(float)));
    QObject::connect(this, SIGNAL(registerMatlabFunctionPath(QString)), &ev, SLOT(addFunctionLocationToPath(QString)));
    originalVideo = 0;
    newVideo = 0;
}

Video* CoreApplication::loadOriginalVideo(QString path)
{
    emit processStatusChanged(CoreApplication::LOAD_VIDEO, true);
    clear();
    cv::VideoCapture vc;
    if (!vc.open(path.toStdString())) {
        qDebug() << "VideoProcessor::loadVideo - Video could not be opened";
        emit processStatusChanged(CoreApplication::LOAD_VIDEO, false);
        return 0;
    }
    // Load Video
    int frameCount = vc.get(CV_CAP_PROP_FRAME_COUNT);
    qDebug() << "VideoProcessor::loadVideo - Frame count is " << frameCount;
    int fps = vc.get(CV_CAP_PROP_FPS);
    originalVideo = new Video(frameCount,fps);
    QFileInfo fileInfo = QFileInfo(path);
    originalVideo->setVideoName(fileInfo.fileName());
//    for (int i = 0; i < frameCount; i++) {
//        vc.set(CV_CAP_PROP_POS_FRAMES, i);
//        qDebug() << i;
//        Mat frame;
//        assert(vc.grab());
//        vc.retrieve(frame);
//        Frame* newFrame = new Frame(frame, originalVideo);
//        originalVideo->appendFrame(newFrame);
//    }
    int currentFrame = 0;
    Mat buffer;
    while (vc.read(buffer)) {
        emit processProgressChanged((float)currentFrame/frameCount);
        Frame* newFrame = new Frame(buffer.clone(),originalVideo);
        originalVideo->appendFrame(newFrame);
        currentFrame++;
        qDebug() << "VideoProcessor::loadVideo - Current frame is " << currentFrame;
    }
    emit originalVideoLoaded(originalVideo);
    emit processStatusChanged(CoreApplication::LOAD_VIDEO, false);
    return originalVideo;
}

void CoreApplication::saveNewVideo(QString path)
{
    emit processStatusChanged(CoreApplication::SAVE_VIDEO, true);
    VideoWriter record(path.toStdString(), CV_FOURCC('M','P','4','V'),25, newVideo->getSize());
    assert(record.isOpened());
    for (int f = 0; f < newVideo->getFrameCount(); f++) {
        emit processProgressChanged((float)f/newVideo->getFrameCount());
        const Frame* frame = newVideo->getFrameAt(f);
        Mat img = frame->getOriginalData().clone();
        record << img;
    }
    emit processStatusChanged(CoreApplication::SAVE_VIDEO, false);
}

void CoreApplication::saveCroppedOldVideo(QString path)
{
    emit processStatusChanged(CoreApplication::SAVE_VIDEO, true);
    //qDebug() << "Saving cropped video " << originalVideo->getCropBox().size().width << "," << originalVideo->getCropBox().size().height;
    Size croppedSize(originalVideo->getCropBox().size().width, originalVideo->getCropBox().size().height);

    VideoWriter record(path.toStdString(), CV_FOURCC('M','P','4','V'),25,croppedSize);
    assert(record.isOpened());
    for (int f = 0; f < originalVideo->getFrameCount(); f++) {
        emit processProgressChanged((float)f/originalVideo->getFrameCount());
        const Frame* frame = originalVideo->getFrameAt(f);
        Mat originalData = frame->getOriginalData();
        Mat croppedImage;
        Rect cropBox = originalVideo->getCropBox();
        Mat(originalData, cropBox).copyTo(croppedImage);
        record << croppedImage;
    }
    emit processStatusChanged(CoreApplication::SAVE_VIDEO, false);
}


void CoreApplication::calculateOriginalMotion(int radius)
{
    originalVideo->reset();
    emit processStatusChanged(CoreApplication::FEATURE_DETECTION, true);
    vp.detectFeatures(originalVideo, radius);
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
}

void CoreApplication::calculateNewMotion(bool salient, bool centered)
{
    emit processStatusChanged(CoreApplication::NEW_MOTION, true);
    if (!salient) {
        vp.calculateUpdateTransform(originalVideo);
    } else {
        vp.calculateSalientUpdateTransform(originalVideo,centered);
    }
    emit processStatusChanged(CoreApplication::NEW_MOTION, false);
    emit processStatusChanged(CoreApplication::NEW_VIDEO, true);
    newVideo = new Video(originalVideo->getFrameCount());
    vp.applyCropTransform(originalVideo, newVideo);
    emit newVideoCreated(newVideo);
    emit processStatusChanged(CoreApplication::NEW_VIDEO, false);
}


void CoreApplication::evaluateNewMotion() {
    qDebug() << "evaluateNewMotion - Not yet implemented";
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

void CoreApplication::drawGraph(bool usePointOriginal, bool showOriginal, bool showNew, bool x, bool y)
{
    // Build Original Path Object
    QList<Mat> original;
    if (!usePointOriginal) {
        for (int f = 1; f < originalVideo->getFrameCount(); f++) {
            Frame* frame = originalVideo->accessFrameAt(f);
            const Mat& aff = frame->getAffineTransform();
            original.push_back(aff);
        }
    } else {
        qDebug() << "CoreApplication::drawGraph - Not yet compatible with originalPoint";
        return;
    }

    // Build Update Transform Object
    QList<Mat> update;
    if (showNew) {
        for (int f = 1; f < originalVideo->getFrameCount(); f++) {
            Frame* frame = originalVideo->accessFrameAt(f);
            const Mat& aff = frame->getUpdateTransform();
            update.push_back(aff);
        }
    }

    if (showNew) {
        ev.drawNewPath(original, update, showOriginal, x, y);
    } else if (showOriginal) {
        ev.drawOriginalPath(original, x, y);
    }
}

void CoreApplication::saveOriginalGlobalMotionMat(QString path) {
    qDebug() << "Saving original motion to Matlab";
    QList<Mat> matrices;
    for (int f = 1; f < originalVideo->getFrameCount(); f++) {
        Frame* frame = originalVideo->accessFrameAt(f);
        matrices.push_back(frame->getAffineTransform());
    }
    ev.exportMatrices(matrices, path, "originalGlobalMotion");
}

void CoreApplication::saveNewGlobalMotionMat(QString path) {
    qDebug() << "Saving new motion to Matlab";
    QList<Mat> matrices;
    for (int f = 1; f < originalVideo->getFrameCount(); f++) {
        Frame* frame = originalVideo->accessFrameAt(f);
        matrices.push_back(frame->getUpdateTransform());
    }
    ev.exportMatrices(matrices, path, "newGlobalMotion");
}


void CoreApplication::clear() {
    delete(originalVideo);
    delete(newVideo);
    originalPointMotion.clear();
}

void CoreApplication::setGFTTDetector() {
    vp.setGFTTDetector();
}

void CoreApplication::setSURFDetector() {
    vp.setSURFDetector();
}

void CoreApplication::setSIFTDetector() {
    vp.setSIFTDetector();
}

void CoreApplication::setFASTDetector() {
    vp.setFASTDetector();
}

void CoreApplication::setGFTTHDetector() {
    vp.setGFTTHDetector();
}

void CoreApplication::loadFeatures(QString path) {
    QMap<int, Point2f*> locations;
    QFile file(path);
    if (file.exists()) {
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream in(&file);
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList lineParts = line.split(",");
            int frame = lineParts.at(0).toInt();
            Point2f* p = new Point2f(lineParts.at(1).toInt(), lineParts.at(2).toInt());
            locations.insert(frame,p);
        }
    }
    QMapIterator<int, Point2f*> i(locations);
    while (i.hasNext()) {
        i.next();
        Frame* f = originalVideo->accessFrameAt(i.key());
        f->setFeature(i.value());
    }
}

void CoreApplication::saveOriginalFrame(QString path, int frame, bool cropped){
    const Frame* f = originalVideo->getFrameAt(frame);
    const Mat& originalData = f->getOriginalData();
    if (cropped) {
        Mat cropped = originalData(originalVideo->getCropBox());
        cv::imwrite(path.toStdString(), cropped);
    } else {
        cv::imwrite(path.toStdString(), originalData);
    }
}

void CoreApplication::saveNewFrame(QString path, int frame) {
    if (frame == 0) {
        saveOriginalFrame(path, frame, true);
        return;
    }
    const Frame* f = originalVideo->getFrameAt(frame);
    const Mat& originalData = f->getOriginalData();
    const Rect& cropBox = originalVideo->getCropBox();
    RotatedRect newCropWindow = Tools::transformRectangle(f->getUpdateTransform(), cropBox);
    Mat newImage = Tools::getCroppedImage(originalData, newCropWindow);
    resize(newImage,newImage,cropBox.size());
    cv::imwrite(path.toStdString(), newImage);
}
