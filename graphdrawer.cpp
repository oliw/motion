#include "graphdrawer.h"
#include <QDebug>
#include "engine.h"
#include "tools.h"

GraphDrawer::GraphDrawer(QObject *parent) :
    QObject(parent)
{
    mEngine = engOpen(NULL);
    assert(mEngine != NULL);
}

GraphDrawer::~GraphDrawer()
{
    engClose(mEngine);
}

mxArray* GraphDrawer::vectorToMatlabFormat(const vector<double>& original)
{
    mxArray *arr = mxCreateDoubleMatrix(original.size(), 1, mxREAL);
    double *p_arr = mxGetPr(arr);
    for (uint i = 0; i < original.size(); i++)
    {
        p_arr[i] = original.at(i);
    }
    return arr;
}

void GraphDrawer::drawOriginalMotionGraph(int x, int y) {
    qDebug() << "GraphDrawer::drawOriginalMotionGraph - Drawing graphs starting at ("<<x<<","<<y<<")";
    const vector<Mat>& transforms = video->getAffineTransforms();
    showMotionGraph(Point2f(x,y),transforms);
}


void GraphDrawer::showMotionGraph(Point2f start, const vector<Mat>& transforms) {
    // Calculate values
    vector<double> times, xss, yss;
    Tools::applyAffineTransformations(start, transforms, times, xss, yss);
    // Convert values to Matlab format
    mxArray *time = vectorToMatlabFormat(times);
    mxArray *xs = vectorToMatlabFormat(xss);
    mxArray *ys = vectorToMatlabFormat(yss);
    // Give values to Matlab
    engPutVariable(mEngine, "time", time);
    engPutVariable(mEngine, "xs", xs);
    engPutVariable(mEngine, "ys", ys);
    // Draw two graphs
    engEvalString(mEngine, "figure; plot(xs,time); xlabel('x'); ylabel('Time /frame')");
    engEvalString(mEngine, "figure; plot(ys,time); xlabel('y'); ylabel('Time /frame')");
}
