#ifndef INVALIDARGUMENTSEXCEPTION_H
#define INVALIDARGUMENTSEXCEPTION_H

#include <QObject>

class InvalidArgumentsException : public QObject
{
    Q_OBJECT
public:
    explicit InvalidArgumentsException(QObject *parent = 0);
    
signals:
    
public slots:
    
};

#endif // INVALIDARGUMENTSEXCEPTION_H
