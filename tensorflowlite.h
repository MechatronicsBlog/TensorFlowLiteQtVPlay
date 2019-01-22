#ifndef TENSORFLOW_H
#define TENSORFLOW_H

#include <QStringList>
#include <QImage>
#include <QRectF>

#include "tensorflow/lite/error_reporter.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/graph_info.h"
#include "tensorflow/lite/kernels/register.h"

using namespace tflite;

class TensorflowLite
{
public:
    static const int knIMAGE_CLASSIFIER = 1;
    static const int knOBJECT_DETECTION = 2;

    TensorflowLite();

    bool init();
    double getThreshold() const;
    void setThreshold(double value);
    QStringList getResults();
    QList<double> getConfidence();
    QList<QRectF> getBoxes();
    int getKindNetwork();
    bool run(QImage img);
    QString getModelFilename() const;
    void setModelFilename(const QString &value);
    QString getLabelsFilename() const;
    void setLabelsFilename(const QString &value);
    int getImgHeight() const;
    int getImgWidth() const;
    double getInfTime() const;
    int getNThreads() const;
    void setNThreads(int value);
    bool getAcceleration() const;
    void setAcceleration(bool value);

private:
    bool initialized;
    double threshold;
    int nThreads;
    bool acceleration;

    // Results
    QStringList rCaption;
    QList<double> rConfidence;
    QList<QRectF> rBox;
    double infTime;

    int kind_network;
    std::vector<TfLiteTensor *> outputs;
    std::unique_ptr<Interpreter> interpreter;
    std::unique_ptr<FlatBufferModel> model;
    ops::builtin::BuiltinOpResolver resolver;
    StderrReporter error_reporter;

    int wanted_height, wanted_width, wanted_channels;

    bool inference();
    bool setInputs(QImage image);
    bool getClassfierOutputs(std::vector<std::pair<float, int> > *top_results);
    bool getObjectOutputs(QStringList &captions, QList<double> &confidences, QList<QRectF> &locations);
    bool readLabels();

    QString input_name;
    TfLiteType input_dtype;
    std::unique_ptr<TfLiteTensor> input_tensor;
    QString modelFilename;
    QString labelsFilename;
    QStringList labels;
    QString getLabel(int index);
    int img_height, img_width, img_channels;
    const QImage::Format format = QImage::Format_RGB888;
    const int numChannels = 3;
};

#endif // TENSORFLOW_H
