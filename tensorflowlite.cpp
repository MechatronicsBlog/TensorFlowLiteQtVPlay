#include "tensorflowlite.h"

#include <QFile>
#include <QElapsedTimer>
#include <QDebug>

#include "tensorflow/lite/builtin_op_data.h"
#include "tensorflow/lite/kernels/internal/tensor.h"
#include "tensorflow/lite/kernels/internal/tensor_utils.h"
using namespace tflite;

#include "get_top_n.h"

TensorflowLite::TensorflowLite()
{
    initialized = false;
}

double TensorflowLite::getThreshold() const
{
    return threshold;
}

void TensorflowLite::setThreshold(double value)
{
    threshold = value;
}

QStringList TensorflowLite::getResults()
{
    return rCaption;
}

QList<double> TensorflowLite::getConfidence()
{
    return rConfidence;
}

QList<QRectF> TensorflowLite::getBoxes()
{
    return rBox;
}

int TensorflowLite::getKindNetwork()
{
    return kind_network;
}

QString TensorflowLite::getModelFilename() const
{
    return modelFilename;
}

void TensorflowLite::setModelFilename(const QString &value)
{
    modelFilename = value;
}

QString TensorflowLite::getLabelsFilename() const
{
    return labelsFilename;
}

void TensorflowLite::setLabelsFilename(const QString &value)
{
    labelsFilename = value;
}

int TensorflowLite::getImgHeight() const
{
    return img_height;
}

int TensorflowLite::getImgWidth() const
{
    return img_width;
}

double TensorflowLite::getInfTime() const
{
    return infTime;
}

int TensorflowLite::getNThreads() const
{
    return nThreads;
}

void TensorflowLite::setNThreads(int value)
{
    nThreads = value;
}

bool TensorflowLite::getAcceleration() const
{
    return acceleration;
}

void TensorflowLite::setAcceleration(bool value)
{
    acceleration = value;
}

bool TensorflowLite::init()
{   
    try{
        // Open model & assign error reporter
        model = getModelFilename().trimmed().isEmpty() && getLabelsFilename().trimmed().isEmpty() ? nullptr :
                FlatBufferModel::BuildFromFile(getModelFilename().toStdString().c_str(),&error_reporter);

        if(model == nullptr)
        {
            qDebug() << "TensorFlow model loading: ERROR";
            return false;
        }

        // Link model & resolver
        InterpreterBuilder builder(*model, resolver);

        // Check interpreter
        if(builder(&interpreter) != kTfLiteOk)
        {
            qDebug() << "Interpreter: ERROR";
            return false;
        }

        // Apply accelaration (Neural Network Android)
        interpreter->UseNNAPI(acceleration);

        // Set number of threads
        if (nThreads > 1) interpreter->SetNumThreads(nThreads);

        qDebug() << "NNAPI:" << acceleration;
        qDebug() << "Num. Threads:" << nThreads;

        if(interpreter->AllocateTensors() != kTfLiteOk)
        {
            qDebug() << "Allocate tensors: ERROR";
            return false;
        }

        // Set kind of network
        kind_network = interpreter->outputs().size()>1 ? knOBJECT_DETECTION : knIMAGE_CLASSIFIER;

        // Get input dimension from the input tensor metadata
        // Assuming one input only
        int input = interpreter->inputs()[0];
        TfLiteIntArray* dims = interpreter->tensor(input)->dims;

        // Save outputs
        outputs.clear();
        for(unsigned int i=0;i<interpreter->outputs().size();i++)
            outputs.push_back(interpreter->tensor(interpreter->outputs()[i]));

        wanted_height   = dims->data[1];
        wanted_width    = dims->data[2];
        wanted_channels = dims->data[3];

        // Read labels
        if (readLabels()) qDebug() << "There are" << labels.count() << "labels.";
        else qDebug() << "There are NO labels";

        qDebug() << "Tensorflow initialization: OK";
        initialized = true;

    }catch(...)
    {
        qDebug() << "Exception loading model";
    }

    return initialized;
}

bool TensorflowLite::readLabels()
{
    if (!labelsFilename.trimmed().isEmpty())
    {
        QFile textFile(labelsFilename);

        if (textFile.exists())
        {
            QByteArray line;

            labels.clear();
            textFile.open(QIODevice::ReadOnly);

            line = textFile.readLine().trimmed();
            while(!line.isEmpty()) // !textFile.atEnd() &&
            {
                labels.append(line);
                line = textFile.readLine().trimmed();
            }

            textFile.close();
        }
        return true;
    }
    return false;
}

bool TensorflowLite::run(QImage img)
{
    QElapsedTimer timer;

    if (initialized)
    {
        // Start timer
        //timer.start();

        // Transform image format & copy data
        QImage image = img.format() == format ? img : img.convertToFormat(format);

        // Store original image properties
        img_width    = image.width();
        img_height   = image.height();
        img_channels = numChannels;

        // Set inputs
        if (!setInputs(image)) return false;

        // Perform inference
        timer.start();
        if (!inference()) return false;
        infTime = timer.elapsed();

        // -------------------------------------
        // Outputs depend on the kind of network
        // -------------------------------------
        rCaption.clear();
        rConfidence.clear();
        rBox.clear();

        // Image classifier
        if (kind_network == knIMAGE_CLASSIFIER)
        {
            std::vector<std::pair<float, int>> top_results;

            if (!getClassfierOutputs(&top_results)) return false;

            for (const auto& result : top_results)
            {
                rConfidence.append(result.first);
                rCaption.append(getLabel(result.second));
            }
        }
        // Object detection
        else if (kind_network == knOBJECT_DETECTION)
        {
            if (!getObjectOutputs(rCaption,rConfidence,rBox)) return false;
        }

        return true;
    }

    return false;
}

QString TensorflowLite::getLabel(int index)
{
    if(index>=0 && index<labels.count())
    {
        QString label = labels[index];

        // Capitalize label
        return  label.left(1).toUpper()+label.mid(1);
    }
    return "";
}
bool TensorflowLite::inference()
{
    // Invoke interpreter
    if (interpreter->Invoke() != kTfLiteOk)
    {
        qDebug() << "Failed to invoke interpreter";
        return false;
    }
    return true;
}

template<class T>
bool formatImageTF(T* out, QImage image, int image_channels, bool input_floating)
{
    const float input_mean = 127.5f;
    const float input_std  = 127.5f;

    // Number of pixels
    const int numberPixels = image.height()*image.width()*image_channels;

    // Pointer to image data
    const uint8_t *output = image.bits();

    // Boolean to [0,1]
    const int inputFloat = input_floating ? 1 : 0;
    const int inputInt   = input_floating ? 0 : 1;

    // Transform to [-128,128]
    for (int i = 0; i < numberPixels; i++)
    {
      out[i] = inputFloat*((output[i] - input_mean) / input_std) +
               inputInt*(uint8_t)output[i];
    }

    return true;
}

// -----------------------------------------------------------------------------------------------------------------------
// https://github.com/tensorflow/tensorflow/blob/master/tensorflow/contrib/lite/examples/label_image/bitmap_helpers_impl.h
// -----------------------------------------------------------------------------------------------------------------------
template <class T>
void formatImageTFLite(T* out, const uint8_t* in, int image_height, int image_width, int image_channels, int wanted_height, int wanted_width, int wanted_channels, bool input_floating)
{
   const float input_mean = 127.5f;
   const float input_std  = 127.5f;

  int number_of_pixels = image_height * image_width * image_channels;
  std::unique_ptr<Interpreter> interpreter(new Interpreter);

  int base_index = 0;

  // two inputs: input and new_sizes
  interpreter->AddTensors(2, &base_index);

  // one output
  interpreter->AddTensors(1, &base_index);

  // set input and output tensors
  interpreter->SetInputs({0, 1});
  interpreter->SetOutputs({2});

  // set parameters of tensors
  TfLiteQuantizationParams quant;
  interpreter->SetTensorParametersReadWrite(0, kTfLiteFloat32, "input",    {1, image_height, image_width, image_channels}, quant);
  interpreter->SetTensorParametersReadWrite(1, kTfLiteInt32,   "new_size", {2},quant);
  interpreter->SetTensorParametersReadWrite(2, kTfLiteFloat32, "output",   {1, wanted_height, wanted_width, wanted_channels}, quant);

  ops::builtin::BuiltinOpResolver resolver;
  const TfLiteRegistration *resize_op = resolver.FindOp(BuiltinOperator_RESIZE_BILINEAR,1);
  auto* params = reinterpret_cast<TfLiteResizeBilinearParams*>(malloc(sizeof(TfLiteResizeBilinearParams)));
  params->align_corners = false;
  interpreter->AddNodeWithParameters({0, 1}, {2}, nullptr, 0, params, resize_op, nullptr);
  interpreter->AllocateTensors();


  // fill input image
  // in[] are integers, cannot do memcpy() directly
  auto input = interpreter->typed_tensor<float>(0);
  for (int i = 0; i < number_of_pixels; i++)
    input[i] = in[i];

  // fill new_sizes
  interpreter->typed_tensor<int>(1)[0] = wanted_height;
  interpreter->typed_tensor<int>(1)[1] = wanted_width;

  interpreter->Invoke();

  auto output = interpreter->typed_tensor<float>(2);
  auto output_number_of_pixels = wanted_height * wanted_height * wanted_channels;

  for (int i = 0; i < output_number_of_pixels; i++)
  {
    if (input_floating)
      out[i] = (output[i] - input_mean) / input_std;
    else
      out[i] = (uint8_t)output[i];
  }

//QImage img(image_width,image_height,QImage::Format_RGB888);
//memcpy(img.bits(),in,number_of_pixels);

//  QImage img(wanted_width,wanted_height,QImage::Format_RGB888);
//  uint8_t iOut[output_number_of_pixels];
//  for(int i=0; i<output_number_of_pixels;i++)
//      iOut[i] = (uint8_t)output[i];
//  memcpy(img.bits(),iOut,output_number_of_pixels);
//  img.save("/home/javi/test.png");
}

bool TensorflowLite::setInputs(QImage image)
{
    // Get inputs
    std::vector<int> inputs = interpreter->inputs();

    // Set inputs
    for(unsigned int i=0;i<interpreter->inputs().size();i++)
    {
        int input = inputs[i];

        // Convert input
        switch (interpreter->tensor(input)->type)
        {
            case kTfLiteFloat32:
            {
                formatImageTFLite<float>(interpreter->typed_tensor<float>(input),image.bits(), image.height(),
                                         image.width(), img_channels, wanted_height, wanted_width,wanted_channels, true);
                break;
            }
            case kTfLiteUInt8:
            {
                formatImageTFLite<uint8_t>(interpreter->typed_tensor<uint8_t>(input),image.bits(),
                                           img_height, img_width, img_channels, wanted_height,
                                           wanted_width, wanted_channels, false);
                break;
            }
            default:
            {
                qDebug() << "Cannot handle input type" << interpreter->tensor(input)->type << "yet";
                return false;
            }
        }
    }

    return true;
}

// --------------------------------------------------------------------------------------
// Code from: https://github.com/YijinLiu/tf-cpu/blob/master/benchmark/obj_detect_lite.cc
// --------------------------------------------------------------------------------------
template<typename T>
T* TensorData(TfLiteTensor* tensor, int batch_index);

template<>
float* TensorData(TfLiteTensor* tensor, int batch_index) {
    int nelems = 1;
    for (int i = 1; i < tensor->dims->size; i++) nelems *= tensor->dims->data[i];
    switch (tensor->type) {
        case kTfLiteFloat32:
            return tensor->data.f + nelems * batch_index;
        default:
            qDebug() << "Should not reach here!";
    }
    return nullptr;
}

template<>
uint8_t* TensorData(TfLiteTensor* tensor, int batch_index) {
    int nelems = 0;
    for (int i = 1; i < tensor->dims->size; i++) nelems *= tensor->dims->data[i];
    switch (tensor->type) {
        case kTfLiteUInt8:
            return tensor->data.uint8 + nelems * batch_index;
        default:
            qDebug() << "Should not reach here!";
    }
    return nullptr;
}

bool TensorflowLite::getClassfierOutputs(std::vector<std::pair<float, int>> *top_results)
{
    const int    output_size = 1000;
    const size_t num_results = 5;

    // Assume one output
    if (interpreter->outputs().size()>0)
    {
        int output = interpreter->outputs()[0];

        switch (interpreter->tensor(output)->type)
        {
            case kTfLiteFloat32:
            {
                get_top_n<float>(interpreter->typed_output_tensor<float>(0), output_size,
                                 num_results, threshold, top_results, true);
                break;
            }
            case kTfLiteUInt8:
            {
                get_top_n<uint8_t>(interpreter->typed_output_tensor<uint8_t>(0),
                                   output_size, num_results, threshold, top_results,false);
                break;
            }
            default:
            {
                qDebug() << "Cannot handle output type" << interpreter->tensor(output)->type << "yet";
                return false;
            }
        }
        return true;
    }
    return false;
}

bool TensorflowLite::getObjectOutputs(QStringList &captions, QList<double> &confidences, QList<QRectF> &locations)
{
    if (outputs.size() >= 4)
    {
        const int    num_detections    = *TensorData<float>(outputs[3], 0);
        const float* detection_classes =  TensorData<float>(outputs[1], 0);
        const float* detection_scores  =  TensorData<float>(outputs[2], 0);
        const float* detection_boxes   =  TensorData<float>(outputs[0], 0);

        for (int i=0; i<num_detections; i++)
        {
            // Get class
            const int cls = detection_classes[i] + 1;

            // Ignore first one
            if (cls == 0) continue;

            // Get score
            float score = detection_scores[i];

            // Check minimum score
            if (score < getThreshold()) break;

            // Get class label
            const QString label = getLabel(cls);

            // Get coordinates
            const float top    = detection_boxes[4 * i]     * img_height;
            const float left   = detection_boxes[4 * i + 1] * img_width;
            const float bottom = detection_boxes[4 * i + 2] * img_height;
            const float right  = detection_boxes[4 * i + 3] * img_width;

            // Save coordinates
            QRectF box(left,top,right-left,bottom-top);

            // Save remaining data
            captions.append(label);
            confidences.append(score);
            locations.append(box);
        }
        return true;
    }
    return false;
}
