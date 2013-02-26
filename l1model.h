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
    L1Model(Video* v);
    ~L1Model();

    int getWidth();

    bool solve();
    double getVariableSolution(int frame, char ch);

    static int toRow(char c);
    static int toCol(char c);

    void print();


private:
    OsiClpSolverInterface si;

    // Objectives
    vector<double> objectiveCoefficients, colLb, colUb;

    // Constraints
    vector<CoinPackedVector> smoothnessConstraints;
    vector<double> smoothnessLb,smoothnessUb;

    vector<CoinPackedVector> inclusionConstraints;
    vector<double> inclusionLb, inclusionUb;

    int varPerFrame;
    int slackVarPerFrame;
    int maxT;

    double getElem(const Mat& affine, char c);

    int toIndex(int t, char variable);
    int toIndex(int t, int variable);
    int toSlackIndex(int t, char variable);
    int toSlackIndex(int t, int variable);

    void setObjectiveCoefficients();
    void setSmoothnessConstraints(vector<Mat>& originalTransformations);
    void setInclusionConstraints(Rect cropbox, int videoWidth, int videoHeight);

};

#endif // L1MODEL_H
