#ifndef WORK_H
#define WORK_H
#include <QObject>
#include <QImage>
#include <QThread>

class Work : public QObject
{
    Q_OBJECT
public:
    explicit Work(QObject *parent = nullptr);

public slots:
    void doWork(QImage img,QThread *childThread);

signals:
    void resultReady(QByteArray postData,QThread *childThread);

};

#endif // WORK_H
