#include "l1salientmodel.h"
#include "l1model.h"
#include <QDebug>
#include <coin/CoinPackedMatrix.hpp>
#include <coin/CoinPackedVector.hpp>
#include <coin/CoinModel.hpp>
#include <coin/OsiClpSolverInterface.hpp>

L1SalientModel::L1SalientModel(int dof):L1Model(dof)
{
    varPerFrame = 6;
    salientSlackVarPerFrame = 4;
    slackVarPerFrame = varPerFrame+salientSlackVarPerFrame;
}

void L1SalientModel::prepare(Video* video, bool centered)
{
    maxT = video->getFrameCount()-1;

    // Convert F into G (the inverse of F)
    vector<Mat> frameMotions = video->getAffineTransforms();
    vector<Mat> gs;
    for (uint f = 0; f < frameMotions.size(); f++) {
        Mat g = Mat::zeros(2,3,DataType<float>::type);
        cv::invertAffineTransform(frameMotions[f], g);
        gs.push_back(g.clone());
    }
    Rect cropBox = video->getCropBox();
    int vidWidth = video->getWidth();
    int vidHeight = video->getHeight();

    if (problemLoaded) {
        si = ClpSimplex(false);
        problemLoaded = false;
    }

    setObjectives();        // Set the objective equation involving transform params, slack error variables, slack salient variables
    setSmoothnessConstraints(gs);                            // Define what motion is permissible
    setInclusionConstraints(cropBox, vidWidth, vidHeight);   // Prevent frame from not containing crop window
    setSalientConstraints(video,centered);                            // Prevent feature from leaving crop window
    setProximityConstraints();

    if (isSimilarityTransform) {
        setSimilarityConstraints(); // Restrict transformation to 4 DOF
    }

    // Load problem into COIN
    matrix.setDimensions(0,getWidth());
    for (uint i = 0; i < constraints.size(); i++) {
        matrix.appendRow(constraints[i]);
    }
    si.loadProblem(matrix,&colLb[0],&colUb[0],&objectiveCoefficients[0],&constraintsLb[0],&constraintsUb[0]);
    problemLoaded = true;
}

void L1SalientModel::setObjectives()
{
    qDebug() << "L1SalientModel::setObjectives - Setting Objectives";
    int width = getWidth();
    objectiveCoefficients.reserve(width);
    colLb.reserve(width);
    colUb.reserve(width);
    for (int t = 0; t < maxT; t++) {
        // Set coefficients and bounds on variables
        for (int i = 0; i < varPerFrame; i++) {
            objectiveCoefficients[toIndex(t,i)] = 0;      // We don't mind about what p is
            colLb[toIndex(t,i)] = -1 * osiInterface.getInfinity(); // p can be as small
            colUb[toIndex(t,i)] = osiInterface.getInfinity();      // p can be as big as we like
        }
        // Set coefficients and bounds on error variables
        for (int i = 0; i < varPerFrame; i++) {
            objectiveCoefficients[toSlackIndex(t,i)] = 1; // Minimise the sum of the slacks
            colLb[toSlackIndex(t,i)] = 0;                 // slacks must be positive
            colUb[toSlackIndex(t,i)] = osiInterface.getInfinity(); // Slacks can be as big as we like
        }
        // Set coefficients and bounds on salient slack Variables
        for (int corner = 0; corner < 2; corner++) {
            for (char comp = 'x'; comp <= 'y'; comp++) {
                objectiveCoefficients[toSalientSlackIndex(t,corner,comp)] = 10; //
                colLb[toSalientSlackIndex(t,corner,comp)] = 0;                 // slacks must be positive
                colUb[toSalientSlackIndex(t,corner,comp)] = osiInterface.getInfinity();  // slacks can be as big as we like
            }
        }
    }

    // Anchor first movement to be at location of crop window
    for (char i = 'a'; i <= 'f' ; i++) {
        if (i == 'a' || i == 'd') {
            colLb[toIndex(0,i)] = 1;
            colUb[toIndex(0,i)] = 1;
        } else {
            colLb[toIndex(0,i)] = 0;
            colUb[toIndex(0,i)] = 0;
        }
    }
}

int L1SalientModel::toSalientSlackIndex(int t, int corner, char component)
{
    assert(component == 'x' || component == 'y');
    int salientVarOffset = 2*corner+(component-'x');
    return t*(varPerFrame+slackVarPerFrame)+varPerFrame+varPerFrame+salientVarOffset;
}

void L1SalientModel::setSalientConstraints(Video* video, bool centered) {
    qDebug() << "L1SalientModel::setSalientConstraints - Forcing salient feature to stay within crop window";
    int numConstraints = maxT * 4;
    constraints.reserve(constraints.size()+numConstraints);
    constraintsLb.reserve(constraintsLb.size()+numConstraints);
    constraintsUb.reserve(constraintsUb.size()+numConstraints);
    Rect cropbox = video->getCropBox();
    double tlX = centered ? cropbox.x + (0.5 * cropbox.width) - 2 : cropbox.x ;
    double tlY = centered ? cropbox.y + (0.5 * cropbox.height) - 2 : cropbox.y ;
    double brX = centered ? cropbox.x + (0.5 * cropbox.width) + 2 : cropbox.x + cropbox.width - 1;
    double brY = centered ? cropbox.y + (0.5 * cropbox.height) + 2 : cropbox.y + cropbox.height - 1;
    // For each pt
    for (int t = 0; t < maxT; t++) {
        Point2f* salientPoint = video->accessFrameAt(t)->getFeature();
        // Add constraint set for TOP LEFT CORNER
        CoinPackedVector const1;
        const1.insert(toIndex(t, 'a'), salientPoint->x);
        const1.insert(toIndex(t, 'b'), salientPoint->y);
        const1.insert(toIndex(t, 'e'), 1);
        const1.insert(toSalientSlackIndex(t,0,'x'), 1);
        constraintsLb.push_back(tlX);
        constraintsUb.push_back(osiInterface.getInfinity());
        constraints.push_back(const1);
        CoinPackedVector const2;
        const2.insert(toIndex(t, 'c'), salientPoint->x);
        const2.insert(toIndex(t, 'd'), salientPoint->y);
        const2.insert(toIndex(t,'f'), 1);
        const2.insert(toSalientSlackIndex(t,0,'y'), 1);
        constraintsLb.push_back(tlY);
        constraintsUb.push_back(osiInterface.getInfinity());
        constraints.push_back(const2);
        // Add constraint set for BOTTOM RIGHT CORNER
        CoinPackedVector const3;
        const3.insert(toIndex(t, 'a'), salientPoint->x);
        const3.insert(toIndex(t, 'b'), salientPoint->y);
        const3.insert(toIndex(t, 'e'), 1);
        const3.insert(toSalientSlackIndex(t,1,'x'), -1);
        constraintsLb.push_back(-1*osiInterface.getInfinity());
        constraintsUb.push_back(brX);
        constraints.push_back(const3);
        CoinPackedVector const4;
        const4.insert(toIndex(t, 'c'), salientPoint->x);
        const4.insert(toIndex(t, 'd'), salientPoint->y);
        const4.insert(toIndex(t,'f'), 1);
        const4.insert(toSalientSlackIndex(t,1,'y'), -1);
        constraintsLb.push_back(-osiInterface.getInfinity());
        constraintsUb.push_back(brY);
        constraints.push_back(const4);
    }
}

// Ensures the frame transformation always contains the Cropbox
void L1SalientModel::setInclusionConstraints(Rect cropbox, int frameWidth, int frameHeight)
{
    qDebug() << "L1SalientModel::setInclusionConstraints - Forcing frame transformations to always include Cropbox";
    int numFrames = maxT;
    int numCorners = 4;
    int numConstraintsPerFramePerCorner = 2;
    int numConstraints = numFrames * numCorners * numConstraintsPerFramePerCorner;
    constraints.reserve(constraints.size()+numConstraints);
    constraintsLb.reserve(constraintsLb.size()+numConstraints);
    constraintsUb.reserve(constraintsUb.size()+numConstraints);
    int ct = cropbox.y;
    int cb = cropbox.y+cropbox.height-1;
    int cl = cropbox.x;
    int cr = cropbox.x+cropbox.width-1;
    // For each pt
    for (int t = 0; t < maxT; t++) {
        // For each corner
        for (int corner = 0; corner < 4; corner++) {
            int frameCornerX = (corner % 2 == 0) ? 0 : frameWidth-1;
            int frameCornerY = (corner < 2) ? 0 : frameHeight-1;
            CoinPackedVector const1;
            const1.insert(toIndex(t, 'a'),frameCornerX);
            const1.insert(toIndex(t,'b'), frameCornerY);
            const1.insert(toIndex(t,'e'),1);
            constraints.push_back(const1);
            CoinPackedVector const2;
            const2.insert(toIndex(t, 'c'),frameCornerX);
            const2.insert(toIndex(t,'d'), frameCornerY);
            const2.insert(toIndex(t,'f'),1);
            constraints.push_back(const2);
            switch(corner) {
            case 0:
                constraintsLb.push_back(-1*osiInterface.getInfinity());
                constraintsUb.push_back(cl);
                constraintsLb.push_back(-1*osiInterface.getInfinity());
                constraintsUb.push_back(ct);
                break;
            case 1:
                constraintsLb.push_back(cr);
                constraintsUb.push_back(osiInterface.getInfinity());
                constraintsLb.push_back(-1*osiInterface.getInfinity());
                constraintsUb.push_back(ct);
                break;
            case 2:
                constraintsLb.push_back(-1*osiInterface.getInfinity());
                constraintsUb.push_back(cl);
                constraintsLb.push_back(cb);
                constraintsUb.push_back(osiInterface.getInfinity());
                break;
            case 3:
                constraintsLb.push_back(cr);
                constraintsUb.push_back(osiInterface.getInfinity());
                constraintsLb.push_back(cb);
                constraintsUb.push_back(osiInterface.getInfinity());
                break;
            }
        }

    }
}
