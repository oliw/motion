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
    L1Model(OsiSolverInterface* osi, Video* v);

    void setObjectiveCoefficients();
    void setLowerBounds();
    void setUpperBounds();

    void setConstraints(vector<Mat>& originalTransformations);

    int getWidth();
    int getConstraintsHeight();

    bool solve();
    double getVariableSolution(int frame, char ch);

    static int toRow(char c);
    static int toCol(char c);

    void print();


private:
    OsiSolverInterface* si;

    int numVariablesPerFrame;
    int numSlackVariablesPerFrame;

    int maxT;

    double * objectiveCoefficients;
    double * variableLowerBounds;
    double * variableUpperBounds;

    CoinPackedMatrix * matrix;
    double * constraintsLowerBounds;
    double * constraintsUpperBounds;
    double getAffineElement(Mat& affine, char c);

    //p1,p2
    int getVarIndex(int frame, char variable);
    int getSlackVarIndex(int frame, char variable);

    // Sets the bounds for the two rows representing -e <= xxxx <= e
    void setConstraintBounds(int frame, int parameter, double bound);

};

#endif // L1MODEL_H
