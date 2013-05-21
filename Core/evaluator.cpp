#include "evaluator.h"
#include <QDebug>
#include "engine.h"
#include "tools.h"
#include "mat.h"
#include <QDir>

Evaluator::Evaluator(QObject *parent) :
    QObject(parent)
{
    mEngine = engOpen(NULL);
    assert(mEngine != NULL);
    functionLocationSet = false;
}

Evaluator::~Evaluator() {
    engClose(mEngine);
}

void DataSetToMatlabFormat(const DataSet& data, mxArray *time, mxArray *xs, mxArray *ys)
{
    double *p_time = mxGetPr(time);
    double *p_xs = mxGetPr(xs);
    double *p_ys = mxGetPr(ys);
    QMapIterator<int, Point2f> i(data);
    int count = 0;
    while (i.hasNext()) {
        i.next();
        Point2f p = i.value();
        p_time[count] = i.key();
        p_xs[count] = p.x;
        p_ys[count] = p.y;
        count++;
    }
}

void Evaluator::drawData(const DataSet& data) {
    // Convert data to Matlab Format
    mxArray *time = mxCreateDoubleMatrix(data.size(), 1, mxREAL);
    mxArray *xs = mxCreateDoubleMatrix(data.size(), 1, mxREAL);
    mxArray *ys = mxCreateDoubleMatrix(data.size(), 1, mxREAL);

    DataSetToMatlabFormat(data, time, xs, ys);

    // Give values to Matlab
    engPutVariable(mEngine, "time", time);
    engPutVariable(mEngine, "xs", xs);
    engPutVariable(mEngine, "ys", ys);

    // Instruct Matlab to Draw Graphs
    engEvalString(mEngine, "figure; plot(xs,time); xlabel('x'); ylabel('Time /frame')");
    engEvalString(mEngine, "figure; plot(ys,time); xlabel('y'); ylabel('Time /frame')");
}

void Evaluator::drawData(const DataSet& origData, const DataSet& newData) {
    // Convert data to Matlab Format
    mxArray *oldtime = mxCreateDoubleMatrix(origData.size(), 1, mxREAL);
    mxArray *oldxs = mxCreateDoubleMatrix(origData.size(), 1, mxREAL);
    mxArray *oldys = mxCreateDoubleMatrix(origData.size(), 1, mxREAL);
    mxArray *newtime = mxCreateDoubleMatrix(newData.size(), 1, mxREAL);
    mxArray *newxs = mxCreateDoubleMatrix(newData.size(), 1, mxREAL);
    mxArray *newys = mxCreateDoubleMatrix(newData.size(), 1, mxREAL);

    DataSetToMatlabFormat(origData, oldtime, oldxs, oldys);
    DataSetToMatlabFormat(newData, newtime, newxs, newys);

    // Give values to Matlab
    engPutVariable(mEngine, "oldtime", oldtime);
    engPutVariable(mEngine, "oldxs", oldxs);
    engPutVariable(mEngine, "oldys", oldys);
    engPutVariable(mEngine, "newtime", newtime);
    engPutVariable(mEngine, "newxs", newxs);
    engPutVariable(mEngine, "newys", newys);

    // Instruct Matlab to Draw Graphs
    engEvalString(mEngine, "figure; plot(oldxs,oldtime,newxs,newtime); xlabel('x'); ylabel('Time /frame'); legend('Original','New')");
    engEvalString(mEngine, "figure; plot(oldys,oldtime,newys,newtime); xlabel('y'); ylabel('Time /frame'); legend('Original','New')");
}

string boolToStr(bool b) {
    return (b ? "true" : "false");
}

void Evaluator::drawOriginalPath(const QList<Mat>& transforms, bool showX, bool showY) {
    qDebug() << "Evaluator::drawOriginalPath - Begin";

    // Convert data to Matlab Format
    mxArray *cellArray = convertToMatlab(transforms);

    // Give values to Matlab
    engPutVariable(mEngine, "originalTransforms", cellArray);

    // Instruct Matlab to Draw Graphs
    qDebug() << "Evaluator::drawOriginalPath - Calling Matlab";
    std::ostringstream stringStream;
    stringStream << "motionGraphDrawer(originalTransforms, 'showX', " << boolToStr(showX)  << ", 'showY', " << boolToStr(showY) << ");";
    qDebug() << QString::fromStdString(stringStream.str());
    engEvalString(mEngine, stringStream.str().c_str());
}

void Evaluator::drawNewPath(const QList<Mat>& originalTransforms, QList<Mat>& updateTransforms, bool showOriginal, bool showX, bool showY) {
    qDebug() << "Evaluator::drawNewPath - Begin";

    // Convert data to Matlab Format
    mxArray *originalPath = convertToMatlab(originalTransforms);
    mxArray *updateTrans = convertToMatlab(updateTransforms);

    // Give values to Matlab
    engPutVariable(mEngine, "originalTransforms", originalPath);
    engPutVariable(mEngine, "updateTransforms", updateTrans);

    // Instruct Matlab to Draw Graphs
    std::ostringstream stringStream;
    stringStream << "motionGraphDrawer(originalTransforms, updateTransforms, 'showOriginal'," << boolToStr(showOriginal) << ",'showX', " << boolToStr(showX) << ", 'showY', " << boolToStr(showY) << ");";
    engEvalString(mEngine, stringStream.str().c_str());

}

void Evaluator::exportMatrices(QList<Mat> matrices, QString filePath, QString name) {
    // Move matrices into a cell array
    mxArray *cellArray = convertToMatlab(matrices);

    // Save into mat File
    QFile file(filePath);
    MATFile* matFile;
    if (file.exists()) {
        matFile = matOpen(filePath.toStdString().c_str(), "u");
    } else {
        matFile = matOpen(filePath.toStdString().c_str(), "w");
    }
    assert(matFile != 0);
    matPutVariable(matFile, name.toStdString().c_str(), cellArray);
    matClose(matFile);
}

mxArray* Evaluator::convertToMatlab(const QList<Mat>& transforms){
    qDebug() << "Evaluator::convertToMatlab - Begin";
    // Convert N 2x3 transforms into a cell of 2x3 arrays
    mxArray *cellArray = mxCreateCellMatrix(transforms.size(), 1);
    for (int i = 0; i < transforms.size(); i++) {
        Mat m = transforms[i];
        m.convertTo(m, CV_32F);
        int rows = m.rows;
        int cols = m.cols;
        //Mat data is float, and mxArray uses double, so we need to convert.
        mxArray *T=mxCreateDoubleMatrix(rows, cols, mxREAL);
        double *buffer = (double*)mxGetPr(T);
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                buffer[c*rows+r] = (double)m.at<float>(r,c);
            }
        }
        mxSetCell(cellArray, i, T);
    }
    qDebug() << "Evaluator::convertToMatlab - End";
    return cellArray;
}

void Evaluator::addFunctionLocationToPath(QString path) {
    QString instruction = "path(path, '"+ path +"')";
    engEvalString(mEngine, instruction.toStdString().c_str());
    functionLocationSet = true;
    qDebug() << "Matlab Path includes: " << path;
}

