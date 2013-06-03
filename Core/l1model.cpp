#include "l1model.h"
#include "video.h"
#include <fstream>
#include <coin/CoinModel.hpp>
#include <coin/CoinPackedMatrix.hpp>
#include <coin/CoinPackedVector.hpp>
#include <coin/ClpDualRowSteepest.hpp>
#include <coin/ClpPrimalColumnSteepest.hpp>
#include <coin/ClpPresolve.hpp>
#include <QDebug>


L1Model::L1Model(int dof):si(ClpSimplex(false))
{
    varPerFrame = 6;
    slackVarPerFrame = varPerFrame;
    isSimilarityTransform = false;
    problemLoaded = false;
    setDOF(dof);
}

L1Model::~L1Model()
{

}

void L1Model::enableDebug()
{
    si.setLogLevel(10000);
    FILE * file = fopen("coin.log","w");
    assert(file);
    CoinMessageHandler* handler = new CoinMessageHandler(file);
    si.passInMessageHandler(handler);
}

void L1Model::setDOF(int dof) {
    assert(dof == 4 || dof == 6);
    isSimilarityTransform = dof == 4;
}

void L1Model::writeToFile()
{
    if (problemLoaded) {
        si.writeMps("coin.mps");
    }
}

void L1Model::prepare(Video* video)
{
    vector<Mat> frameMotions = video->getAffineTransforms();
    Rect cropBox = video->getCropBox();
    int vidWidth = video->getWidth();
    int vidHeight = video->getHeight();
    maxT = video->getFrameCount()-1;

    if (problemLoaded) {
        si = ClpSimplex(false);
        problemLoaded = false;
    }

    setObjectives();

    matrix.setDimensions(0,getWidth());
    setSmoothnessConstraints(frameMotions); // Define what motion is permissible
    setInclusionConstraints(cropBox, vidWidth, vidHeight);   // Prevent crop window from leaving frame
    setProximityConstraints();  // Prevent the crop window from varying too greatly

    if (isSimilarityTransform) {
        setSimilarityConstraints(); // Restrict transformation to 4 DOF
    }

    for (uint i = 0; i < constraints.size(); i++) {
        matrix.appendRow(constraints[i]);
    }

}

bool L1Model::solve()
{
    qDebug() << "L1Model::solve() - Solving ";
    si.loadProblem(matrix,&colLb[0],&colUb[0],&objectiveCoefficients[0],&constraintsLb[0],&constraintsUb[0]);

    ClpDualRowSteepest dualSteep(1);
    si.setDualRowPivotAlgorithm(dualSteep);

    ClpPrimalColumnSteepest primalSteep(1);
    si.setPrimalColumnPivotAlgorithm(primalSteep);

    si.scaling(1);

    ClpPresolve presolveInfo;
    Ptr<ClpSimplex> presolvedModel = presolveInfo.presolvedModel(si);

    if (!presolvedModel.empty())
    {
        presolvedModel->dual();
        presolveInfo.postsolve(true);
        si.checkSolution();
        si.primal(1);
    }
    else
    {
        si.dual();
        si.checkSolution();
        si.primal(1);
    }

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
    return si.isProvenOptimal();
}

// ACCESS RESULTS
double L1Model::getVariableSolution(int t, char ch)
{
    const double * sols = si.getColSolution();
    return sols[toIndex(t-1,ch)];
}

// SET OBJECTIVE
void L1Model::setObjectives()
{
    qDebug() << "L1Model::setObjectives - Setting Objectives";
    int width = getWidth();
    objectiveCoefficients.reserve(width);
    colLb.reserve(width);
    colUb.reserve(width);
    for (int t = 0; t < maxT; t++) {
        for (int i = 0; i < varPerFrame; i++) {
            objectiveCoefficients[toIndex(t,i)] = 0;      // We don't mind about what p is
            objectiveCoefficients[toSlackIndex(t,i)] = 1; // Minimise the sum of the slacks
            colLb[toIndex(t,i)] = -1 * osiInterface.getInfinity(); // p can be as small
            colLb[toSlackIndex(t,i)] = 0;                 // slacks must be positive
            colUb[toIndex(t,i)] = osiInterface.getInfinity();      // p can be as big as we like
            colUb[toSlackIndex(t,i)] = osiInterface.getInfinity(); // Slacks can be as big as we like
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

// CONSTRAINT SETTERS
void L1Model::setSmoothnessConstraints(vector<Mat>& fs)
{
    qDebug() << "L1Model::setSmoothnessConstraints - Setting Smoothness Constraints";
    constraints.reserve(constraints.size()+((maxT-1)*2*6));
    constraintsLb.reserve(constraintsLb.size()+((maxT-1)*2*6));
    constraintsUb.reserve(constraintsUb.size()+((maxT-1)*2*6));
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
        constraints.push_back(a);
        constraints.push_back(a2);
        constraints.push_back(b);
        constraints.push_back(b2);
        constraints.push_back(c);
        constraints.push_back(c2);
        constraints.push_back(d);
        constraints.push_back(d2);
        constraints.push_back(e);
        constraints.push_back(e2);
        constraints.push_back(f);
        constraints.push_back(f2);

        for (int i = 0; i < 4; i++) {
            constraintsLb.push_back(-1 * osiInterface.getInfinity());
            constraintsUb.push_back(0);
            constraintsLb.push_back(0);
            constraintsUb.push_back(osiInterface.getInfinity());
        }
        constraintsLb.push_back(-1 * osiInterface.getInfinity());
        constraintsUb.push_back(-1 * getElem(aff,'e'));
        constraintsLb.push_back(-1 * getElem(aff,'e'));
        constraintsUb.push_back(osiInterface.getInfinity());

        constraintsLb.push_back(-1 * osiInterface.getInfinity());
        constraintsUb.push_back(-1 * getElem(aff,'f'));
        constraintsLb.push_back(-1 * getElem(aff,'f'));
        constraintsUb.push_back(osiInterface.getInfinity());
    }
}

void L1Model::setProximityConstraints()
{
    qDebug() << "L1Model::setProximityConstraints";
    constraints.reserve(constraints.size()+((maxT-1)*6));
    constraintsLb.reserve(constraintsLb.size()+((maxT-1)*6));
    constraintsUb.reserve(constraintsUb.size()+((maxT-1)*6));
    for (int t = 0; t < maxT-1; t++) {
         CoinPackedVector v;
         v.insert(toIndex(t, 'a'), 1);
         constraintsLb.push_back(0.9);
         constraintsUb.push_back(1.1);
         constraints.push_back(v);
         CoinPackedVector v2;
         v2.insert(toIndex(t, 'd'), 1);
         constraintsLb.push_back(0.9);
         constraintsUb.push_back(1.1);
         constraints.push_back(v2);
         CoinPackedVector v3;
         v3.insert(toIndex(t, 'b'), 1);
         constraintsLb.push_back(-0.1);
         constraintsUb.push_back(0.1);
         constraints.push_back(v3);
         CoinPackedVector v4;
         v4.insert(toIndex(t, 'c'), 1);
         constraintsLb.push_back(-0.1);
         constraintsUb.push_back(0.1);
         constraints.push_back(v4);
         CoinPackedVector v5;
         v5.insert(toIndex(t, 'b'), 1);
         v5.insert(toIndex(t, 'c'), 1);
         constraintsLb.push_back(-0.05);
         constraintsUb.push_back(0.05);
         constraints.push_back(v5);
         CoinPackedVector v6;
         v6.insert(toIndex(t, 'a'), 1);
         v6.insert(toIndex(t, 'd'), -1);
         constraintsLb.push_back(-0.1);
         constraintsUb.push_back(0.1);
         constraints.push_back(v6);
    }
}

void L1Model::setInclusionConstraints(Rect cropBox, int videoWidth, int videoHeight) {
    qDebug() << "L1Model::setInclusionConstraints - Ensuring cropbox stays within frame";
    // Number of Constraints
    int numConstraints = 2 * 4 * maxT;
    constraints.reserve(constraints.size()+numConstraints);
    constraintsLb.reserve(constraintsLb.size()+numConstraints);
    constraintsUb.reserve(constraintsUb.size()+numConstraints);
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
                constraintsLb.push_back(0);
                constraintsUb.push_back(videoWidth);
                constraints.push_back(const1);
                CoinPackedVector const2;
                const2.insert(toIndex(t, 'c'), x);
                const2.insert(toIndex(t, 'd'), y);
                const2.insert(toIndex(t,'f'), 1);
                constraintsLb.push_back(0);
                constraintsUb.push_back(videoHeight);
                constraints.push_back(const2);

            }
        }
    }
}

void L1Model::setSimilarityConstraints() {
    qDebug() << "L1Model::setSimilarityConstraints - Ensuring only 4 DOF";
    int numConstraints = 2 * maxT;
    constraints.reserve(constraints.size()+numConstraints);
    constraintsLb.reserve(constraintsLb.size()+numConstraints);
    constraintsUb.reserve(constraintsUb.size()+numConstraints);
    for (int i = 0; i < maxT; i++) {
        CoinPackedVector s1;
        s1.insert(toIndex(i,'a'),1);
        s1.insert(toIndex(i,'d'),-1);
        constraints.push_back(s1);
        constraintsLb.push_back(0);
        constraintsUb.push_back(0);
        CoinPackedVector s2;
        s2.insert(toIndex(i,'b'),1);
        s2.insert(toIndex(i,'c'),1);
        constraints.push_back(s2);
        constraintsLb.push_back(0);
        constraintsUb.push_back(0);
    }
}

// HELPERS
int L1Model::getWidth()
{
    return (varPerFrame + slackVarPerFrame)*maxT;
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

double L1Model::getElem(const Mat& affine, char c)
{
    return affine.at<double>(toRow(c),toCol(c));
}
