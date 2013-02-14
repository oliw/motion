#include "l1model.h"
#include "video.h"
#include <coin/CoinModel.hpp>
#include <coin/CoinPackedMatrix.hpp>
#include <coin/CoinPackedVector.hpp>
#include <QDebug>


L1Model::L1Model(OsiSolverInterface* osi, Video* v)
{
    si = osi;
    numVariablesPerFrame = 6;
    numSlackVariablesPerFrame = numVariablesPerFrame;
    // 0,1,2,3,...,maxT
    maxT = v->getFrameCount()-1; // TODO
    setObjectiveCoefficients();
    setLowerBounds();
    setUpperBounds();
    vector<Mat> affs = v->getAffineTransforms();
    setConstraints(affs);
}

int L1Model::getConstraintsHeight()
{
    return (maxT-1)*2*numVariablesPerFrame;
}

int L1Model::getWidth()
{
    return (numVariablesPerFrame + numSlackVariablesPerFrame)*maxT;
}

void L1Model::setObjectiveCoefficients()
{
    int width = getWidth();
    objectiveCoefficients = new double[width];
    for (int t = 0; t < maxT; t++) {
        for (int i = 0; i < numVariablesPerFrame; i++) {
            int index = getVarIndex(t,i);
            objectiveCoefficients[index] = 0;
        }
        for (int s = 0; s < numSlackVariablesPerFrame; s++) {
            // We want to minimise the sum of the slack variables
            int index = getSlackVarIndex(t,s);
            objectiveCoefficients[index] = 1;
        }
    }
}

void L1Model::setLowerBounds()
{
    int width = getWidth();
    variableLowerBounds = new double[width];
    for (int t = 0; t < maxT; t++) {
        for (int i = 0; i < numVariablesPerFrame; i++) {
            int index = getVarIndex(t,i);
            variableLowerBounds[index] = -1*si->getInfinity();
        }
        for (int s = 0; s < numSlackVariablesPerFrame; s++) {
            int index = getSlackVarIndex(t,s);
            variableLowerBounds[index] = 0;
        }
    }
}

void L1Model::setUpperBounds()
{
    int width = getWidth();
    variableUpperBounds = new double[width];
    for (int t = 0; t < maxT; t++) {
        for (int i = 0; i < numVariablesPerFrame; i++) {
            int index = getVarIndex(t,i);
            variableUpperBounds[index] = si->getInfinity();
        }
        for (int s = 0; s < numSlackVariablesPerFrame; s++) {
            int index = getSlackVarIndex(t,s);
            variableUpperBounds[index] = si->getInfinity();
        }
    }
}

double L1Model::getAffineElement(Mat& affine, char c)
{
    return affine.at<double>(toRow(c),toCol(c));
}

void L1Model::setConstraintBounds(int frame, int parameter, double bound)
{
    constraintsUpperBounds[frame*numVariablesPerFrame*2 + parameter*2] = bound;
    constraintsLowerBounds[frame*numVariablesPerFrame*2 + parameter*2] = -1*si->getInfinity();

    constraintsUpperBounds[frame*numVariablesPerFrame*2 + parameter*2 + 1] = si->getInfinity();
    constraintsLowerBounds[frame*numVariablesPerFrame*2 + parameter*2 + 1] = bound;

}


void L1Model::setConstraints(vector<Mat>& originalTransformations)
{
    qDebug() << originalTransformations.size();
    matrix = new CoinPackedMatrix(false,0,0);
    int height = getConstraintsHeight();
    matrix->setDimensions(height, getWidth());
    constraintsLowerBounds = new double[height];
    constraintsUpperBounds = new double[height];
    for (int t = 0; t < maxT; t++) {
        Mat& origTrans = originalTransformations.at(t+1);
        for (int p = 0; p < numVariablesPerFrame; p++) {
            CoinPackedVector* row1 = new CoinPackedVector();
            CoinPackedVector* row2;
            switch (p) {
                case 0:
                row1->insert(getVarIndex(t+1,'a'), getAffineElement(origTrans, 'a'));
                row1->insert(getVarIndex(t+1,'c'), getAffineElement(origTrans, 'b'));
                row1->insert(getVarIndex(t,'a'), -1);
                row2 = new CoinPackedVector(row1);
                row1->insert(getSlackVarIndex(t,'a'), -1);
                row2->insert(getSlackVarIndex(t,'a'),1);
                setConstraintBounds(t,p,0);
                    break;
                case 1:
                row1->insert(getVarIndex(t+1, 'b'), getAffineElement(origTrans,'a'));
                row1->insert(getVarIndex(t+1, 'd'), getAffineElement(origTrans,'b'));
                row1->insert(getVarIndex(t, 'b'),-1);
                row2 = new CoinPackedVector(row1);
                row1->insert(getSlackVarIndex(t,'b'), -1);
                row2->insert(getSlackVarIndex(t,'b'),1);
                setConstraintBounds(t,p,0);
                    break;
                case 2:
                row1->insert(getVarIndex(t+1, 'a'), getAffineElement(origTrans,'c'));
                row1->insert(getVarIndex(t+1, 'c'), getAffineElement(origTrans,'d'));
                row1->insert(getVarIndex(t, 'c'),-1);
                row2 = new CoinPackedVector(row1);
                row1->insert(getSlackVarIndex(t,'c'), -1);
                row2->insert(getSlackVarIndex(t,'c'),1);
                setConstraintBounds(t,p,0);
                    break;
                case 3:
                row1->insert(getVarIndex(t+1, 'b'), getAffineElement(origTrans,'c'));
                row1->insert(getVarIndex(t+1, 'd'), getAffineElement(origTrans,'d'));
                row1->insert(getVarIndex(t, 'd'),-1);
                row2 = new CoinPackedVector(row1);
                row1->insert(getSlackVarIndex(t,'d'), -1);
                row2->insert(getSlackVarIndex(t,'d'),1);
                setConstraintBounds(t,p,0);
                    break;
                case 4:
                row1->insert(getVarIndex(t+1, 'e'), getAffineElement(origTrans,'a'));
                row1->insert(getVarIndex(t+1, 'f'), getAffineElement(origTrans,'b'));
                row1->insert(getVarIndex(t, 'e'),-1);
                row2 = new CoinPackedVector(row1);
                row1->insert(getSlackVarIndex(t,'e'), -1);
                row2->insert(getSlackVarIndex(t,'e'),1);
                setConstraintBounds(t,p,-1*getAffineElement(origTrans,'e'));
                    break;
                case 5:
                row1->insert(getVarIndex(t+1, 'e'), getAffineElement(origTrans,'c'));
                row1->insert(getVarIndex(t+1, 'f'), getAffineElement(origTrans,'d'));
                row1->insert(getVarIndex(t, 'f'),-1);
                row2 = new CoinPackedVector(row1);
                row1->insert(getSlackVarIndex(t,'f'), -1);
                row2->insert(getSlackVarIndex(t,'f'),1);
                setConstraintBounds(t,p,-1*getAffineElement(origTrans,'f'));
                    break;
            }
            matrix->appendRow(*row1);
            matrix->appendRow(*row2);
            delete row1;
            delete row2;
        }
    }
}


int L1Model::toRow(char c)
{
    assert(c >= 'a' && c <= 'f');
    if (c == 'a' || c == 'b' || c == 'e') {
        return 0;
    } else {
        return 1;
    }
}

int L1Model::toCol(char c)
{
    assert(c >= 'a' && c <= 'f');
    if (c == 'a' || c == 'c') {
        return 0;
    } else if (c == 'b' || c == 'd') {
        return 1;
    } else {
        return 2;
    }
}

int L1Model::getVarIndex(int frame, char v)
{
    int var = v - 'a';
    return frame*(numVariablesPerFrame+numSlackVariablesPerFrame)+var;
}

int L1Model::getSlackVarIndex(int frame, char v)
{
    int var = v - 'a';
    return frame*(numVariablesPerFrame+numSlackVariablesPerFrame)+numVariablesPerFrame+var;
}

bool L1Model::solve()
{
    // Load Problem
    si->loadProblem(*matrix, variableLowerBounds, variableUpperBounds, objectiveCoefficients, constraintsLowerBounds, constraintsUpperBounds);
    // Solve Problem
    si->initialSolve();
    // Check Is Optimal
    if (si->isProvenOptimal()) {
        qDebug() << "VideoProcessor::calculateIdealPath - Found optimal solution";
    } else {
        qDebug() << "VideoProcessor::calculateIdealPath - Did not find optimal solution";
    }
    return si->isProvenOptimal();
}

double L1Model::getVariableSolution(int frame, char ch)
{
    const double * sols = si->getColSolution();
    return sols[getVarIndex(frame,ch)];
}



