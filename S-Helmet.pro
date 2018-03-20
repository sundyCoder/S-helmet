
QT       += core gui sensors
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = S-Helmet
TEMPLATE = app
CONFIG += c++11

#CONFIG += console

SOURCES += main.cpp\
    mainwindow.cpp \
    qcustomplot.cpp \
    accelerometerfilter.cpp \
    Serial/src/Serial.cpp \
    Serial/src/TimeOut.cpp \
    Serial/src/cic.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    accelerometerfilter.h \
    Serial/inc/TimeOut.h \
    Serial/inc/cic.h \
    Serial/inc/Serial.h

FORMS    += mainwindow.ui

winrt {
    WINRT_MANIFEST.version = 1.0.0.0
    WINRT_MANIFEST.name = S-Helmet
    WINRT_MANIFEST.publisher = sundycoder@gmail.com.
    WINRT_MANIFEST.description = Accelerometer Chart
}
