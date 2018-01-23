#-------------------------------------------------
#
# Project created by QtCreator 2017-09-20T21:44:37
#
#-------------------------------------------------

QT       += core gui
#RC_ICONS += wali.ico
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lprx
TEMPLATE = app


#QMAKE_CXXFLAGS += -fopenmp
#LIBS += -fopenmp


SOURCES += main.cpp\
        lpr.cpp \
    mser/mser2.cpp \
    plate_detect.cpp \
    definition/chars_identify.cpp \
    definition/chars_recognise.cpp \
    definition/chars_segment.cpp \
    definition/core_func.cpp \
    definition/feature.cpp \
    definition/plate_locate.cpp

HEADERS  += lpr.h \
    mser/mser2.h \
    header/plate.h \
    plate_detect.h \
    header/character.h \
    header/chars_identify.h \
    header/chars_recognise.h \
    header/chars_segment.h \
    header/core_func.h \
    header/feature.h \
    header/plate_locate.h


FORMS    += lpr.ui


INCLUDEPATH += /usr/local/include/          \
               /usr/local/include/opencv    \
               /usr/local/include/opencv2

LIBS += /usr/local/lib/libopencv_core.so        \
        /usr/local/lib/libopencv_calib3d.so     \
        /usr/local/lib/libopencv_features2d.so  \
        /usr/local/lib/libopencv_flann.so       \
        /usr/local/lib/libopencv_highgui.so     \
        /usr/local/lib/libopencv_imgcodecs.so   \
        /usr/local/lib/libopencv_imgproc.so     \
        /usr/local/lib/libopencv_ml.so          \
        /usr/local/lib/libopencv_objdetect.so   \
        /usr/local/lib/libopencv_photo.so       \
        /usr/local/lib/libopencv_shape.so       \
        /usr/local/lib/libopencv_stitching.so   \
        /usr/local/lib/libopencv_superres.so    \
        /usr/local/lib/libopencv_video.so       \
        /usr/local/lib/libopencv_videoio.so     \
        /usr/local/lib/libopencv_videostab.so   \
        /usr/local/lib/libopencv_viz.so         \
        /usr/local/lib/libwiringPi.so           \
        /usr/local/lib/libwiringPiDev.so        \
        /usr/local/lib/libdmtx.so


RESOURCES += \
    image_label.qrc
