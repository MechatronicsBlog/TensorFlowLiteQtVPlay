#ifndef TENSORFLOWTHREAD_H
#define TENSORFLOWTHREAD_H

#include <QObject>
#include <QImage>
#include <QRectF>
#include <QThread>
#include "tensorflowlite.h"

class WorkerTF: public QObject
{
    Q_OBJECT

        QImage imgTF;
        TensorflowLite *tf;

    public:
        void setImgTF(const QImage &value);
        void setTf(TensorflowLite *value);

    public slots:
        void work();

    signals:
        void results(int network, QStringList captions, QList<double> confidences, QList<QRectF> boxes, double time);
        void finished();
};

class TensorflowThread: public QObject
{
    Q_OBJECT

    public:
        TensorflowThread();
        void run(QImage imgTF);
        void setTf(TensorflowLite *value);
        void stop();

    signals:
        void results(int network, QStringList captions, QList<double> confidences, QList<QRectF> boxes, double time);

    public slots:
        void propagateResults(int network, QStringList captions, QList<double> confidences, QList<QRectF> boxes, double time);

    private:
        QThread threadTF;
        WorkerTF worker;
};

#endif // TENSORFLOWTHREAD_H
