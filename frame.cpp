#include "frame.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
Frame::Frame()
{
}

Frame::Frame(Mat originalData, int i):originalData(originalData),i(i)
{

}
