#include "l1model.h"
#include "video.h"
#include <fstream>
#include <coin/CoinModel.hpp>
#include <coin/CoinPackedMatrix.hpp>
#include <coin/CoinPackedVector.hpp>
#include <QDebug>


L1Model::L1Model(Video* v)
{
    varPerFrame = 6;
    slackVarPerFrame = varPerFrame;
    // 0,1,2,3,...,maxT
    maxT = v->getFrameCount()-1;
    setObjectiveCoefficients();
    vector<Mat> affs = v->getAffineTransforms();
    qDebug() << "L1Model - Setting Smoothness Constraints";
    setSmoothnessConstraints(affs);
    qDebug() << "L1Model - Setting Inclusion Constraints";
    setInclusionConstraints(v->getCropBox(), v->getWidth(), v->getHeight());
    qDebug() << "L1Model - Setting Proximity Constraints";
    setProximityConstraints();
    si.setLogLevel(10000);
}

L1Model::~L1Model()
{

}


int L1Model::getWidth()
{
    return (varPerFrame + slackVarPerFrame)*maxT;
}

void L1Model::setObjectiveCoefficients()
{
    int width = getWidth();
    objectiveCoefficients.reserve(width);
    colLb.reserve(width);
    colUb.reserve(width);
    for (int t = 0; t < maxT; t++) {
        for (int i = 0; i < varPerFrame; i++) {
            objectiveCoefficients[toIndex(t,i)] = 0;      // We don't mind about what p is
            objectiveCoefficients[toSlackIndex(t,i)] = 1; // Minimise the sum of the slacks
            colLb[toIndex(t,i)] = -1 * si.getInfinity(); // p can be as small
            colLb[toSlackIndex(t,i)] = 0;                 // slacks must be positive
            colUb[toIndex(t,i)] = si.getInfinity();      // p can be as big as we like
            colUb[toSlackIndex(t,i)] = si.getInfinity(); // Slacks can be as big as we like
        }
    }
}

double L1Model::getElem(const Mat& affine, char c)
{
    return affine.at<double>(toRow(c),toCol(c));
}


void L1Model::setSmoothnessConstraints(vector<Mat>& fs)
{
    // F(t) = mapping from It to It-1
    for (int t = 0; t < maxT-1; t++) {
        const Mat& aff = fs[t+1]; // F = F(t+1)
        // 1 = B(t+1)
        // 2 = F(t+1)
        // 3 = B(t)
        // slack a: a2a1 + b2c1 - a3
        CoinPackedVector a;
        a.insert(toIndex(t+1,'a'), getElem(aff,'a'));
        a.insert(toIndex(t+1, 'c'), getElem(aff, 'b'));
        a.insert(toIndex(t, 'a'), -1);
        CoinPackedVector a2(a);
        a.insert(toSlackIndex(t, 'a'), -1);
        a2.insert(toSlackIndex(t, 'a'), 1);
        // slack b: a2b1 + b2d1 - b3
        CoinPackedVector b;
        b.insert(toIndex(t+1,'b'), getElem(aff,'a'));
        b.insert(toIndex(t+1, 'd'), getElem(aff, 'b'));
        b.insert(toIndex(t, 'b'), -1);
        CoinPackedVector b2(b);
        b.insert(toSlackIndex(t, 'b'), -1);
        b2.insert(toSlackIndex(t, 'b'), 1);
        // slack c: c2a1 + d2c1 - c3
        CoinPackedVector c;
        c.insert(toIndex(t+1,'a'), getElem(aff,'c'));
        c.insert(toIndex(t+1, 'c'), getElem(aff, 'd'));
        c.insert(toIndex(t, 'c'), -1);
        CoinPackedVector c2(c);
        c.insert(toSlackIndex(t, 'c'), -1);
        c2.insert(toSlackIndex(t, 'c'), 1);
        // slack d: c2b1 + d2d1 - d3
        CoinPackedVector d;
        d.insert(toIndex(t+1,'b'), getElem(aff,'c'));
        d.insert(toIndex(t+1, 'd'), getElem(aff, 'd'));
        d.insert(toIndex(t, 'd'), -1);
        CoinPackedVector d2(d);
        d.insert(toSlackIndex(t, 'd'), -1);
        d2.insert(toSlackIndex(t, 'd'), 1);
        // slack e: a2e1 + b2f1 + e2 - e3
        CoinPackedVector e;
        e.insert(toIndex(t+1,'e'), getElem(aff,'a'));
        e.insert(toIndex(t+1, 'f'), getElem(aff, 'b'));
        e.insert(toIndex(t, 'e'), -1);
        CoinPackedVector e2(e);
        e.insert(toSlackIndex(t, 'e'), -1);
        e2.insert(toSlackIndex(t, 'e'), 1);
        // slack f: c2e1 + d2f1 + f2 - f3
        CoinPackedVector f;
        f.insert(toIndex(t+1,'e'), getElem(aff,'c'));
        f.insert(toIndex(t+1, 'f'), getElem(aff, 'd'));
        f.insert(toIndex(t, 'f'), -1);
        CoinPackedVector f2(f);
        f.insert(toSlackIndex(t, 'f'), -1);
        f2.insert(toSlackIndex(t, 'f'), 1);
        smoothnessConstraints.push_back(a);
        smoothnessConstraints.push_back(a2);
        smoothnessConstraints.push_back(b);
        smoothnessConstraints.push_back(b2);
        smoothnessConstraints.push_back(c);
        smoothnessConstraints.push_back(c2);
        smoothnessConstraints.push_back(d);
        smoothnessConstraints.push_back(d2);
        smoothnessConstraints.push_back(e);
        smoothnessConstraints.push_back(e2);
        smoothnessConstraints.push_back(f);
        smoothnessConstraints.push_back(f2);

        for (int i = 0; i < 4; i++) {
            smoothnessLb.push_back(-1 * si.getInfinity());
            smoothnessUb.push_back(0);
            smoothnessLb.push_back(0);
            smoothnessUb.push_back(si.getInfinity());
        }
        smoothnessLb.push_back(-1 * si.getInfinity());
        smoothnessUb.push_back(-1 * getElem(aff,'e'));
        smoothnessLb.push_back(-1 * getElem(aff,'e'));
        smoothnessUb.push_back(si.getInfinity());

        smoothnessLb.push_back(-1 * si.getInfinity());
        smoothnessUb.push_back(-1 * getElem(aff,'f'));
        smoothnessLb.push_back(-1 * getElem(aff,'f'));
        smoothnessUb.push_back(si.getInfinity());
    }
}

void L1Model::setProximityConstraints()
{
    for (int t = 0; t < maxT-1; t++) {
         CoinPackedVector v;
         v.insert(toIndex(t, 'a'), 1);
         proximityLb.push_back(0.9);
         proximityUb.push_back(1.1);
         proximityConstraints.push_back(v);
         CoinPackedVector v2;
         v2.insert(toIndex(t, 'd'), 1);
         proximityLb.push_back(0.9);
         proximityUb.push_back(1.1);
         proximityConstraints.push_back(v2);
         CoinPackedVector v3;
         v3.insert(toIndex(t, 'b'), 1);
         proximityLb.push_back(-0.1);
         proximityUb.push_back(0.1);
         proximityConstraints.push_back(v3);
         CoinPackedVector v4;
         v4.insert(toIndex(t, 'c'), 1);
         proximityLb.push_back(-0.1);
         proximityUb.push_back(0.1);
         proximityConstraints.push_back(v4);
         CoinPackedVector v5;
         v5.insert(toIndex(t, 'b'), 1);
         v5.insert(toIndex(t, 'c'), 1);
         proximityLb.push_back(-0.05);
         proximityUb.push_back(0.05);
         proximityConstraints.push_back(v5);
         CoinPackedVector v6;
         v6.insert(toIndex(t, 'a'), 1);
         v6.insert(toIndex(t, 'd'), -1);
         proximityLb.push_back(-0.1);
         proximityUb.push_back(0.1);
         proximityConstraints.push_back(v6);
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

int L1Model::toIndex(int t, char v)
{
    int var = v - 'a';
    return toIndex(t,var);
}

int L1Model::toIndex(int t, int var)
{
    return t*(varPerFrame+slackVarPerFrame)+var;
}

int L1Model::toSlackIndex(int t, char v)
{
    int var = v - 'a';
    return toSlackIndex(t, var);
}

int L1Model::toSlackIndex(int t, int var)
{
    return t*(varPerFrame+slackVarPerFrame)+varPerFrame+var;
}

bool L1Model::solve()
{
    qDebug() << "L1Model::solve() - Solving ";
    double *objectives = &objectiveCoefficients[0];
    double *columnLb = &colLb[0];
    double *columnUb = &colUb[0];

    CoinPackedMatrix matrix;
    matrix.setDimensions(0, getWidth());
    for (uint i = 0; i < smoothnessConstraints.size(); i++) {
        matrix.appendRow(smoothnessConstraints[i]);
    }
    for (uint i = 0; i < inclusionConstraints.size(); i++) {
        matrix.appendRow(inclusionConstraints[i]);
    }
    for (uint i = 0; i < proximityConstraints.size(); i++) {
        matrix.appendRow(proximityConstraints[i]);
    }

    vector<double> rowLb;
    rowLb.reserve(smoothnessConstraints.size()+inclusionConstraints.size()+proximityConstraints.size());
    rowLb.insert(rowLb.end(), smoothnessLb.begin(), smoothnessLb.end());
    rowLb.insert(rowLb.end(), inclusionLb.begin(), inclusionLb.end());
    rowLb.insert(rowLb.end(), proximityLb.begin(), proximityLb.end());

    vector<double> rowUb;
    rowUb.reserve(smoothnessConstraints.size()+inclusionConstraints.size()+proximityConstraints.size());
    rowUb.insert(rowUb.end(), smoothnessUb.begin(), smoothnessUb.end());
    rowUb.insert(rowUb.end(), inclusionUb.begin(), inclusionUb.end());
    rowUb.insert(rowUb.end(), proximityUb.begin(), proximityUb.end());

    si.loadProblem(matrix,columnLb,columnUb,objectives,&rowLb[0],&rowUb[0]);

    qDebug() << "Problem Rows: " << si.getNumRows() << ", Cols:" << si.getNumCols();
    // Consolidate all the constraints
    FILE * file = fopen("coin.log","w");
    assert(file);
    CoinMessageHandler* handler = new CoinMessageHandler(file);
    si.passInMessageHandler(handler);
    // Load Problem
    si.writeMps("coin.mps");
    // Solve Problem
    qDebug() << "L1Model::solve - Solving";
    si.initialSolve();
    // Check Is Optimal
    if (si.isProvenOptimal()) {
        qDebug() << "L1Model::solve - Found optimal solution";
    } else {
        qDebug() << "L1Model::solve - Did not find optimal solution";
        if (si.isProvenPrimalInfeasible())
        qDebug() << "Problem is proven to be infeasible.";
        if (si.isProvenDualInfeasible())
        qDebug() << "Problem is proven dual infeasible.";
        if (si.isIterationLimitReached())
            qDebug() << "Iteration limit reached";
    }
    if (si.isProvenPrimalInfeasible()) {
        qDebug() << "Infeasible";
    }
    fclose(file);
    return si.isProvenOptimal();
}

// t >= 1
double L1Model::getVariableSolution(int t, char ch)
{
    const double * sols = si.getColSolution();
    return sols[toIndex(t-1,ch)];
}

void L1Model::setInclusionConstraints(Rect cropBox, int videoWidth, int videoHeight) {
    // Number of Constraints
    int numConstraints = 2 * 4 * maxT;
    inclusionLb.reserve(numConstraints);
    inclusionUb.reserve(numConstraints);
    // For each pt
    for (int t = 0; t < maxT; t++) {
        // For each corner
        for (int x = cropBox.x; x <= cropBox.x+cropBox.width; x+=cropBox.width) {
            for (int y = cropBox.y; y <= cropBox.y+cropBox.height; y+=cropBox.height) {
                //Set inclusion constraints
                CoinPackedVector const1;
                const1.insert(toIndex(t, 'a'), x);
                const1.insert(toIndex(t, 'b'), y);
                const1.insert(toIndex(t, 'e'), 1);
                inclusionLb.push_back(0);
                inclusionUb.push_back(videoWidth);
                inclusionConstraints.push_back(const1);
                CoinPackedVector const2;
                const2.insert(toIndex(t, 'c'), x);
                const2.insert(toIndex(t, 'd'), y);
                const2.insert(toIndex(t,'f'), 1);
                inclusionLb.push_back(0);
                inclusionUb.push_back(videoHeight);
                inclusionConstraints.push_back(const2);

            }
        }
    }
}

