#include "localransacrejector.h"
#include "tools.h"
#include <QDebug>

LocalRANSACRejector::LocalRANSACRejector(QObject *parent) :
    QObject(parent)
{
    gridSize = 50;
    localRansacTolerance = 2;
    newInliersThreshold = 0;
    cellSize = Size(gridSize, gridSize);
}

LocalRANSACRejector::LocalRANSACRejector(int gridSize, int localRansacTolerance, int newInliersThreshold, QObject *parent):
    QObject(parent), gridSize(gridSize), localRansacTolerance(localRansacTolerance),newInliersThreshold(newInliersThreshold)
{
    cellSize = Size(gridSize, gridSize);
}

/*
 *  Implementation based on unstable unreleased source code of OpenCV3
 */
void LocalRANSACRejector::process(Size frameSize, InputArray from, InputArray to, OutputArray mask) {
    int npoints = from.getMat().checkVector(2);

    const Point2f* from_ = from.getMat().ptr<Point2f>();
    const Point2f* to_ = to.getMat().ptr<Point2f>();

    mask.create(1, npoints, CV_8U);
    uchar* mask_ = mask.getMat().ptr<uchar>();

    Size ncells((frameSize.width + cellSize.width - 1) / cellSize.width,
                    (frameSize.height + cellSize.height - 1) / cellSize.height);

    int cx, cy;

    // fill grid cells with our points
    grid_.assign(ncells.area(), Cell());

    for (int i = 0; i < npoints; ++i)
    {
        cx = std::min(cvRound(from_[i].x / cellSize.width), ncells.width - 1);
        cy = std::min(cvRound(from_[i].y / cellSize.height), ncells.height - 1);
        grid_[cy * ncells.width + cx].push_back(i);
    }

    int niters = 30;
    int ninliers, ninliersMax;
    std::vector<int> inliers;
    float dx, dy, dxBest, dyBest;
    float x1, y1;
    int idx;

    // Iterate over each grid
    for (size_t ci = 0; ci < grid_.size(); ci++) {
        // estimate translation model for this cell using RANSAC
        const Cell &cell = grid_[ci];
        ninliersMax =0;
        dxBest = dyBest = 0.f;
        if (!cell.empty()) {
            for (int iter = 0; iter < niters; iter++) {
                idx = cell[static_cast<unsigned>(rand()) % cell.size()];
                dx = to_[idx].x - from_[idx].x;
                dy = to_[idx].y - from_[idx].y;

                ninliers = 0;
                for (size_t i = 0; i < cell.size(); ++i)
                {
                    x1 = from_[cell[i]].x + dx;
                    y1 = from_[cell[i]].y + dy;
                    if (pow((x1 - to_[cell[i]].x),2) + pow((y1 - to_[cell[i]].y),2) <
                        pow((localRansacTolerance),2.0))
                    {
                        ninliers++;
                    }
                }

                if (ninliers > ninliersMax)
                {
                    ninliersMax = ninliers;
                    dxBest = dx;
                    dyBest = dy;
                }
            }

            // get the best hypothesis inliers

            ninliers = 0;
            inliers.resize(ninliersMax);
            for (size_t i = 0; i < cell.size(); ++i)
            {
                x1 = from_[cell[i]].x + dxBest;
                y1 = from_[cell[i]].y + dyBest;
                if (pow((x1 - to_[cell[i]].x),2) + pow((y1 - to_[cell[i]].y),2) <
                    pow((localRansacTolerance),2))
                {
                    inliers[ninliers++] = cell[i];
                }
            }

            // refine the best hypothesis

            dxBest = dyBest = 0.f;
            for (size_t i = 0; i < inliers.size(); ++i)
            {
                dxBest += to_[inliers[i]].x - from_[inliers[i]].x;
                dyBest += to_[inliers[i]].y - from_[inliers[i]].y;
            }
            if (!inliers.empty())
            {
                dxBest /= inliers.size();
                dyBest /= inliers.size();
            }

            // set mask elements for refined model inliers

            for (size_t i = 0; i < cell.size(); ++i)
            {
                x1 = from_[cell[i]].x + dxBest;
                y1 = from_[cell[i]].y + dyBest;
                if (pow((x1 - to_[cell[i]].x),2) + pow((y1 - to_[cell[i]].y),2) <
                    std::pow((localRansacTolerance),2))
                {
                    mask_[cell[i]] = 1;
                }
                else
                {
                    mask_[cell[i]] = 0;
                }
            }
        }
    }
}


void LocalRANSACRejector::execute(Video* video) {
    // Divide each frame into grids
//    for (int f=1; f<video->getFrameCount()-1; f++) {
//        emit processProgressChanged(float(f)/video->getFrameCount()-2);
//        Frame* frame = video->accessFrameAt(f);
//        uint actualNum = frame->getDisplacements().size();
//        uint numDisplacements = 0;
//        for (int x=0; x < video->getWidth(); x+=gridSize)
//        {
//            for (int y=0; y < video->getHeight(); y+=gridSize)
//            {
//                vector<Displacement> displacements = frame->getDisplacements(x,y, gridSize);
//                numDisplacements += displacements.size();
//                if (!displacements.empty()) {
//                    RansacModel model = localRansac(displacements);
//                    frame->registerOutliers(model.getOutliers());
//                }
//            }
//        }
//        assert(actualNum == numDisplacements);
//    }
    for (int f = 1; f < video->getFrameCount()-1; f++) {
        emit processProgressChanged(float(f)/video->getFrameCount()-2);
        Frame* frame = video->accessFrameAt(f);
        vector<Point2f> from = frame->getFrom();
        vector<Point2f> to = frame->getTo();
        vector<uchar> mask;
        vector<Displacement> outliers;
        process(frame->getSize(),from,to,mask);
        for (uint i = 0; i < from.size(); i++) {
            if (mask[i] != 1) {
                Displacement d(from[i],to[i]);
                outliers.push_back(d);
            }
        }
        frame->registerOutliers(outliers);
    }
}


Point2f fitTranslationModel(std::vector<Displacement> points) {
    // Option 1
    Point2f avgDisp = Point2f(0,0);
    for (uint r = 0; r < points.size(); r++) {
        avgDisp += points.at(r).getDisplacement();
    }
    return avgDisp = avgDisp * (1.0 / points.size());
}

RansacModel LocalRANSACRejector::localRansac(const std::vector<Displacement>& points) {

    std::vector<RansacModel> models;
    for (int i = 0; i < 3; i++)
    {
        // Step 1: Select a random number of points as inliers
        std::vector<Displacement> outliers = points;
        std::vector<Displacement> inliers;
        uint numInitialInliers = (rand() % outliers.size()) + 1;
        assert(points.size() >= numInitialInliers);
        for (uint r = 0; r < numInitialInliers; r++) {
            int randomIndex = rand() % outliers.size();
            Displacement randomPoint = outliers.at(randomIndex);
            outliers.erase(outliers.begin()+randomIndex);
            inliers.push_back(randomPoint);
        }
        /*  Step 2: Fit a displacement model to these points
         *          OPTION 1: Fit a translation model
         *          OPTION 2: Fit a fundamental matrix
         *          OPTION 3: Fit a affine model
         */
        Point2f model = fitTranslationModel(inliers);
        // Step 3: Compare each of remaining points with the new model
        // Step 4: If a point lies within a tolerance make it an inlier also
        int newInliers = 0;
        vector<Displacement>::iterator it = outliers.begin();
        while (it != outliers.end()) {
            const Displacement& disp = *it;
            Point2f estimate = disp.getFrom() + model;
            if (Tools::eucDistance(estimate, disp.getTo()) <= localRansacTolerance) {
                // Inlier
                inliers.push_back(disp);
                it = outliers.erase(it);
                newInliers++;
            } else {
                // Outlier
                it++;
            }
        }
        // Step 5: If num of inliers is above a certain threshold, fit a new model
        if (newInliers > newInliersThreshold)
        {
            model = fitTranslationModel(inliers);
        }
        RansacModel m = RansacModel(model, outliers, inliers);
        models.push_back(m);
        // Step 6: Repeat steps 1-5 i times
    }
    // Step 7: Keep the saved model which has the lowest error score with the inliers
    RansacModel finalResult = models.at(0);
    float finalError = finalResult.getInlierError();
    for (uint i = 1; i < models.size(); i++) {
        float currError = models[i].getInlierError();
        if (currError < finalError)
        {
            finalResult = models[i];
            finalError = currError;
        }
    }
    // Step 8: Done
    return finalResult;
}
