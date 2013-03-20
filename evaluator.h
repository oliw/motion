#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <QObject>
#include "graphdrawer.h"

struct DataSet {
    vector<int> time;
    vector<Point2f> location;
};

class Evaluator : public QObject
{
    Q_OBJECT
public:
    explicit Evaluator(QObject *parent = 0);

    void compareMotions(DataSet oldMotion, DataSet newMotion);
    void calculateStillness(DataSet motion);

private:
    //GraphDrawer gd;


signals:
    
public slots:
    
};

#endif // EVALUATOR_H
