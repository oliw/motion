#include "localransacrejector.h"
#include "tools.h"

LocalRANSACRejector::LocalRANSACRejector(QObject *parent) :
    QObject(parent)
{
    gridSize = 50;
    localRansacTolerance = 2;
}

LocalRANSACRejector::LocalRANSACRejector(int gridSize, int localRansacTolerance, QObject *parent):
    QObject(parent), gridSize(gridSize), localRansacTolerance(localRansacTolerance)
{

}


void LocalRANSACRejector::execute(Video& video) {
    // Divide each frame into grids
    for (int f=0; f<video.getFrameCount()-1; f++) {
        Frame& frame = video.accessFrameAt(f);
        uint actualNum = frame.getDisplacements().size();
        uint numDisplacements = 0;
        for (int x=0; x < video.getWidth(); x+=gridSize)
        {
            for (int y=0; y < video.getHeight(); y+=gridSize)
            {
                //qDebug() << "VideoProcessor::outlierRejection - Detecting outliers in grid at "<<x<<","<<y;
                vector<Displacement> displacements = frame.getDisplacements(x,y, gridSize);
                numDisplacements += displacements.size();
                //qDebug() << "VideoProcessor::outlierRejection - Feature count in grid: "<< displacements.size();
                if (!displacements.empty()) {
                    RansacModel model = localRansac(displacements);
                    //qDebug() << "VideoProcessor::outlierRejection - Outliers:" << model.getOutliers().size() << " Inliers:" << model.getInliers().size();
                    frame.registerOutliers(model.getOutliers());
                }
            }
        }
        assert(actualNum == numDisplacements);
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
        int newInliersThreshold;
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
        if (newInliers <= newInliersThreshold)
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
