#include "evaluator.h"
#include <QDebug>
#include "engine.h"
#include "tools.h"

Evaluator::Evaluator(QObject *parent) :
    QObject(parent)
{
    mEngine = engOpen(NULL);
    assert(mEngine != NULL);
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
    mxArray *time =  mxCreateDoubleMatrix(data.size(), 1, mxREAL);
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
    //    const Video* original = vp->getVideo();
    //    // Calculate values
    //    vector<double> times;
    //    // Original Video Movement
    //    vector <double> xss, yss;
    //    Tools::applyAffineTransformations(start, original->getAffineTransforms(), times, xss, yss);
    //    const Video* cropped = 0;
    //    // Convert values to Matlab format
    //    mxArray *time = vectorToMatlabFormat(times);
    //    mxArray *xs = vectorToMatlabFormat(xss);
    //    mxArray *ys = vectorToMatlabFormat(yss);
    //    // Give values to Matlab
    //    engPutVariable(mEngine, "time", time);
    //    engPutVariable(mEngine, "xs", xs);
    //    engPutVariable(mEngine, "ys", ys);
    //    if (cropped != 0) {
    //        // Cropped Video Movement
    //        vector <double> cxss, cyss;
    //        Tools::applyAffineTransformations(start, cropped->getAffineTransforms(), times, cxss, cyss);
    //        mxArray *cxs = vectorToMatlabFormat(cxss);
    //        mxArray *cys = vectorToMatlabFormat(cyss);
    //        engPutVariable(mEngine, "cxs", cxs);
    //        engPutVariable(mEngine, "cys", cys);
    //        // Draw two graphs
    //        engEvalString(mEngine, "figure; plot(xs,time,cxs,time); xlabel('x'); ylabel('Time /frame'); legend('Original','Cropped')");
    //        engEvalString(mEngine, "figure; plot(ys,time,cys,time); xlabel('y'); ylabel('Time /frame'); legend('Original','Cropped')");
    //    } else {
    //        engEvalString(mEngine, "figure; plot(xs,time); xlabel('x'); ylabel('Time /frame'); legend('Original')");
    //        engEvalString(mEngine, "figure; plot(ys,time); xlabel('y'); ylabel('Time /frame'); legend('Original')");
    //    }
}
