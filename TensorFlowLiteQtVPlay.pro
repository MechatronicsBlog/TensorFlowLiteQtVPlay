# allows to add DEPLOYMENTFOLDERS and links to the V-Play library and QtCreator auto-completion
CONFIG += v-play

# uncomment this line to add the Live Client Module and use live reloading with your custom C++ code
# for the remaining steps to build a custom Live Code Reload app see here: https://v-play.net/custom-code-reload-app/
#CONFIG += v-play-live

# configure the bundle identifier for iOS
PRODUCT_IDENTIFIER = com.yourcompany.wizardEVAP.TensorFlowLiteQtVPlay

qmlFolder.source = qml

assetsFolder.source = assets
DEPLOYMENTFOLDERS += assetsFolder

# NOTE: for PUBLISHING, perform the following steps:
# 1. comment the DEPLOYMENTFOLDERS += qmlFolder line above, to avoid shipping your qml files with the application (instead they get compiled to the app binary)
# 2. uncomment the resources.qrc file inclusion and add any qml subfolders to the .qrc file; this compiles your qml files and js files to the app binary and protects your source code
# 3. change the setMainQmlFile() call in main.cpp to the one starting with "qrc:/" - this loads the qml files from the resources
# for more details see the "Deployment Guides" in the V-Play Documentation

# during development, use the qmlFolder deployment because you then get shorter compilation times (the qml files do not need to be compiled to the binary but are just copied)
# also, for quickest deployment on Desktop disable the "Shadow Build" option in Projects/Builds - you can then select "Run Without Deployment" from the Build menu in Qt Creator if you only changed QML files; this speeds up application start, because your app is not copied & re-compiled but just re-interpreted

# Debug or release mode

# This works for Linux and macOS, appropiate copy command must be set for Windows
CONFIG(debug,debug|release):system($$quote(cp "$$PWD/qml/config_debug.json" "$$PWD/qml/config.json"))
CONFIG(release,debug|release):system($$quote(cp "$$PWD/qml/config_release.json" "$$PWD/qml/config.json"))

CONFIG(debug,debug|release):DEPLOYMENTFOLDERS += qmlFolder # comment for publishing
CONFIG(release,debug|release):RESOURCES += resources.qrc # uncomment for publishing

CONFIG(release,debug|release):CONFIG += qtquickcompiler # Qt Quick compiler

QT += multimedia multimedia-private

# The .cpp file which was generated for your project. Feel free to hack it.
SOURCES += main.cpp \
    objectsrecogfilter.cpp \
    tensorflowthread.cpp \
    auxutils.cpp \
    colormanager.cpp \
    tensorflowlite.cpp

android {
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
    OTHER_FILES += android/AndroidManifest.xml \
                   android/build.gradle
}

ios {
    QMAKE_INFO_PLIST = ios/Project-Info.plist
    OTHER_FILES += $$QMAKE_INFO_PLIST
}

# set application icons for win and macx
win32 {
    RC_FILE += win/app_icon.rc
}
macx {
    ICON = macx/app_icon.icns
}

# TensorFlow Lite - Global
TENSORFLOW_PATH = $$PWD/tensorflow/
TFLITE_MAKE_PATH = $$TENSORFLOW_PATH/tensorflow/lite/tools/make
INCLUDEPATH += $$TENSORFLOW_PATH \
               $$TFLITE_MAKE_PATH/downloads/ \
               $$TFLITE_MAKE_PATH/downloads/eigen \
               $$TFLITE_MAKE_PATH/downloads/gemmlowp \
               $$TFLITE_MAKE_PATH/downloads/neon_2_sse \
               $$TFLITE_MAKE_PATH/downloads/farmhash/src \
               $$TFLITE_MAKE_PATH/downloads/flatbuffers/include

# TensorFlow Lite - Linux
linux:!android {
    INCLUDEPATH += -L$$TFLITE_MAKE_PATH/gen/linux_x86_64/obj
    LIBS += -L$$TFLITE_MAKE_PATH/gen/linux_x86_64/lib/ \
            -ltensorflow-lite -ldl
}

# TensorFlow Lite - Android - armv7a
android {
    QT += androidextras
    LIBS += -L$$TENSORFLOW_PATH/bazel-bin/tensorflow/lite \
            -L$$TENSORFLOW_PATH/bazel-bin/tensorflow/lite/c \
            -L$$TENSORFLOW_PATH/bazel-bin/tensorflow/lite/core/api \
            -L$$TENSORFLOW_PATH/bazel-bin/tensorflow/lite/kernels \
            -L$$TENSORFLOW_PATH/bazel-bin/tensorflow/lite/kernels/internal \
            -L$$TENSORFLOW_PATH/bazel-bin/external/androidndk \
            -L$$TENSORFLOW_PATH/bazel-bin/external/farmhash_archive \
            -L$$TENSORFLOW_PATH/bazel-bin/external/fft2d \
            -L$$TENSORFLOW_PATH/bazel-bin/external/flatbuffers \
            -lframework -larena_planner -lsimple_memory_arena -lutil -lapi -lc_api_internal \
            -lbuiltin_ops -lbuiltin_op_kernels -lkernel_util -leigen_support -lgemm_support \
            -laudio_utils -lkernel_utils -ltensor_utils -lneon_tensor_utils -lquantization_util \
            -llstm_eval -lstring_util -lcpufeatures -lfarmhash -lfft2d -lflatbuffers
}

# TensorFlow Lite - iOS - Universal library
ios {
        LIBS += -L$$TFLITE_MAKE_PATH/gen/lib/ \
                -framework Accelerate \
                -ltensorflow-lite
}

DISTFILES += \
    qml/VideoPage.qml \
    AppSettingsPage.qml \
    qml/PopupMessage.qml

HEADERS += \
    objectsrecogfilter.h \
    tensorflowthread.h \
    auxutils.h \
    colormanager.h \
    get_top_n_impl.h \
    get_top_n.h \
    tensorflowlite.h
