#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <QObject>
#include <QDebug>
#include "engine.h"
#include "tools.h"

typedef QMap<int, Point2f> DataSet;
typedef QList<Mat> TransformData;

class Evaluator : public QObject
{
    Q_OBJECT
public:
    explicit Evaluator(QObject *parent = 0);
    ~Evaluator();

    void drawData(const DataSet& data);
    void drawData(const DataSet& origData, const DataSet& newData);

    void drawOriginalPath(const QList<Mat>& transforms, bool showX, bool showY);
    void drawNewPath(const QList<Mat>& originalTransforms, QList<Mat>& updateTransforms, bool showOriginal, bool showX, bool showY);

    mxArray* convertToMatlab(const QList<Mat>& transforms);

    void exportMatrices(QList<Mat> matrices, QString filePath, QString name);

private:
    Engine* mEngine;
    bool functionLocationSet;

signals:
    
public slots:
    void addFunctionLocationToPath(QString path);
    
};

#endif // EVALUATOR_H
