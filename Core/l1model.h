#ifndef L1MODEL_H
#define L1MODEL_H
#include "video.h"
#include <coin/CoinPackedMatrix.hpp>
#include <coin/CoinModel.hpp>
#include <coin/OsiClpSolverInterface.hpp>
#include <opencv2/core/core.hpp>

using namespace std;
using namespace cv;


class L1Model
{
public:
    L1Model(int dof = 4);
    ~L1Model();

    int getWidth();

    bool solve();
    double getVariableSolution(int frame, char ch);

    static int toRow(char c);
    static int toCol(char c);

    void print();

    void enableDebug();
    void setDOF(int dof = 6);
    void writeToFile();
    void prepare(Video* video);

protected:
    ClpSimplex si;
    OsiClpSolverInterface osiInterface;

    // Objectives
    vector<double> objectiveCoefficients, colLb, colUb;

    // Constraints
    CoinPackedMatrix matrix;
    vector<CoinPackedVector> constraints;
    vector<double> constraintsLb, constraintsUb;

    int varPerFrame;
    int slackVarPerFrame;
    int maxT;
    bool isSimilarityTransform;
    bool problemLoaded;

    double getElem(const Mat& affine, char c);

    int toIndex(int t, char variable);
    int toIndex(int t, int variable);
    int toSlackIndex(int t, char variable);
    int toSlackIndex(int t, int variable);

    void setObjectives();
    void setSmoothnessConstraints(vector<Mat>& originalTransformations);
    void setInclusionConstraints(Rect cropbox, int videoWidth, int videoHeight);
    void setProximityConstraints();
    void setSimilarityConstraints();

};

#endif // L1MODEL_H
