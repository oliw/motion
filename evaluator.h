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

    void drawOriginal();
    void drawNew();
    void drawBoth();

    void exportMatrices(QList<Mat> matrices, QString filePath, QString name);

private:
    Engine* mEngine;
    bool functionLocationSet;

signals:
    
public slots:
    void addFunctionLocationToPath(QString path);
    
};

#endif // EVALUATOR_H
