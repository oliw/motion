#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <QObject>
#include <QDebug>
#include "engine.h"
#include "tools.h"

typedef QMap<int, Point2f> DataSet;

class Evaluator : public QObject
{
    Q_OBJECT
public:
    explicit Evaluator(QObject *parent = 0);
    ~Evaluator();

    void drawData(const DataSet& data);
    void drawData(const DataSet& origData, const DataSet& newData);

    void exportMatrices(QList<Mat> matrices, QString filePath, QString name);

private:
    Engine* mEngine;

signals:
    
public slots:

    
};

#endif // EVALUATOR_H
