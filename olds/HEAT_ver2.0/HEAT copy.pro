#-------------------------------------------------
#
# Project created by QtCreator 2015-11-26T11:51:51
#
#-------------------------------------------------

### for windows 

## Qt5
QT += core gui sql opengl
## Qt6
##QT += core gui sql opengl openglwidgets widgets core5compat

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

## Qt6
## greaterThan(QT_MAJOR_VERSION, 4): CONFIG += c++11
## lessThan(QT_MAJOR_VERSION, 5): QMAKE_CXXFLAGS += -std=c++11
greaterThan(QT_MAJOR_VERSION, 5): QMAKE_CXXFLAGS += -std=c++11 -stdlib=libc++



//!host_build:QMAKE_MAC_SDK = macosx10.10

TARGET = HEAT
TEMPLATE = app
###QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.10
## HEAT 202301 (Qt 5.12?)
## QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15
## HEAT 202304 (Qt 6.2)
QMAKE_MACOSX_DEPLOYMENT_TARGET = 13.0


##ifdef Q_OS_WIN32
##// Windows
DEFINES +=_TCHAR_DEFINED
#endif

##ifdef Q_OS_MAC
##// Mac OS (= Darwin)
##endif

##ifdef Q_OS_LINUX
##// Linux
##endif




//QMAKE_CXX = g++-4.9
### QMAKE_CC = gcc-4.9

//QMAKE_LINK = g++-4.9
//QMAKE_CXXFLAGS += -std=c++11

//QMAKE_CXXFLAGS+= -fopenmp
//QMAKE_LFLAGS +=  -fopenmp

SOURCES += main.cpp\
        mainwindow.cpp \
        rendering.cpp \
        showimage.cpp \
        popwindow.cpp \
        tiling.cpp \
    controlpanel.cpp \
    database.cpp \
    targetmodel.cpp \
    loaddatalist.cpp \
    vtkrendering.cpp \
    vtkmodel.cpp \
    showdbinfo.cpp \
    qcustomplot.cpp \
    pixcelgraph.cpp \
    calibration.cpp \
    calibrationgraph.cpp \
    controlgraphpanel.cpp \
    showfitsinfo.cpp \
    readtxt.cpp


HEADERS  += mainwindow.h \
    rendering.h \
    showimage.h \
    popwindow.h \
    tiling.h \
    controlpanel.h \
    database.h \
    targetmodel.h \
    loaddatalist.h \
    vtkrendering.h \
    vtkmodel.h \
    showdbinfo.h \
    qcustomplot.h \
    pixcelgraph.h \
    calibration.h \
    calibrationgraph.h \
    controlgraphpanel.h \
    showfitsinfo.h \
    readtxt.h


FORMS    += mainwindow.ui \
    popwindow.ui \
    tiling.ui \
    controlpanel.ui \
    database.ui \
    targetmodel.ui \
    loaddatalist.ui \
    vtkmodel.ui \
    showdbinfo.ui \
    pkghkgraph.ui \
    pixcelgraph.ui \
    calibration.ui \
    calibrationgraph.ui \
    controlgraphpanel.ui \
    showfitsinfo.ui \
    readtxt.ui

# QT += widgets

QT += concurrent

win32{
LIBS += "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.19041.0/um/x64/OpenGL32.Lib"
LIBS += "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.19041.0/um/x64/GlU32.Lib"
QMAKE_CXXFLAGS += /F 32000000
QMAKE_LFLAGS   += /STACK:32000000

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/'../../../../Program Files (x86)/CFITSIO/lib/' -lcfitsio
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/'../../../../Program Files (x86)/CFITSIO/lib/' -lcfitsio
else:unix: LIBS += -L$$PWD/'../../../../Program Files (x86)/CFITSIO/lib/' -lcfitsio

INCLUDEPATH += $$PWD/'../../../../Program Files (x86)/CFITSIO/include'
DEPENDPATH += $$PWD/'../../../../Program Files (x86)/CFITSIO/include'
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/'../../../../Program Files (x86)/CCfits/lib/' -lCCfits
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/'../../../../Program Files (x86)/CCfits/lib/' -lCCfits
else:unix: LIBS += -L$$PWD/'../../../../Program Files (x86)/CCfits/lib/' -lCCfits

INCLUDEPATH += $$PWD/'../../../../Program Files (x86)/CCfits/include'
DEPENDPATH += $$PWD/'../../../../Program Files (x86)/CCfits/include'

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../Users/S_hir/AppData/Local/pthread/lib/x64/ -lpthreadVC2
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../Users/S_hir/AppData/Local/pthread/lib/x64/ -lpthreadVC2
else:unix: LIBS += -L$$PWD/../../../../Users/S_hir/AppData/Local/pthread/lib/x64/ -lpthreadVC2

INCLUDEPATH += $$PWD/../../../../Users/S_hir/AppData/Local/pthread/lib/x64
DEPENDPATH += $$PWD/../../../../Users/S_hir/AppData/Local/pthread/lib/x64
}

macx{
INCLUDEPATH +=
INCLUDEPATH += /usr/local/include
## INCLUDEPATH += /usr/local/lib
#INCLUDEPATH += /usr/local/Cellar/cfitsio/3.490/include
#INCLUDEPATH += /usr/local/Cellar/ccfits/2.5_2/include/CCfits
#INCLUDEPATH += /usr/local/Cellar/cspice/66/include
### success bigsur
### INCLUDEPATH += /usr/local/Cellar/cfitsio/3.470/include
### INCLUDEPATH += /usr/local/Cellar/ccfits/2.5_1/include/CCfits
### INCLUDEPATH += /usr/local/Cellar/cspice/66/include

## 2022/11/25
INCLUDEPATH += /usr/local/Cellar/ccfits/2.6/include/CCfits
INCLUDEPATH += /usr/local/Cellar/cfitsio/4.1.0/include
INCLUDEPATH += /usr/local/Cellar/cspice/66/include
## heat2022 (mysql5.7)
## INCLUDEPATH += /usr/local/Cellar/libomp/15.0.7/include
## HEAT2023: Is it caused by mysql8 (homebrew) installation?
INCLUDEPATH += /usr/local/Cellar/libomp/13.0.0/include
## ß

DISTFILES += \
    HEAT.icns
#LIBS += -L/usr/local/Cellar/ccfits/2.5_2/lib -lCCfits
#LIBS += -L/usr/local/Cellar/cfitsio/3.490/lib -lcfitsio
#LIBS += /usr/local/Cellar/cspice/67/lib/cspice.a

## 2022/11/25
LIBS += -L/usr/local/Cellar/ccfits/2.6/lib -lCCfits
LIBS += -L/usr/local/Cellar/cfitsio/4.1.0/lib -lcfitsio
LIBS += /usr/local/Cellar/cspice/66/lib/cspice.a
## ß

QMAKE_CXXFLAGS += -Xpreprocessor -fopenmp -lomp -I/usr/local/include

#QMAKE_LFLAGS += -lomp

#LIBS += -L /usr/local/lib /usr/local/lib/libomp.dylib

## 2022/11/25
#LIBS += -L /usr/local/Cellar/libomp/15.0.4/lib -llibomp
## heat2022 (mysql5.7)
## LIBS += /usr/local/Cellar/libomp/15.0.7/lib/libomp.a
## HEAT2023: Is it caused by mysql8 (homebrew) installation?
LIBS += /usr/local/Cellar/libomp/13.0.0/lib/libomp.a
##
}
CONFIG += opengl console
#INCLUDEPATH += C:/Project/setup/cfitsio/cfitsio
#INCLUDEPATH += C:/Project/setup/ccfits/CCfits
#INCLUDEPATH += C:/Project/setup/pthreads/include
#INCLUDEPATH += C:/Project/setup/freeglut-3.0.0/include/GL
### INCLUDEPATH += /usr/local/include
### INCLUDEPATH += /usr/local/lib
### INCLUDEPATH += /usr/local/include/CCfits
### INCLUDEPATH += /usr/local/opt/ccfits/include/CCfits


## x64
DISTFILES += \
    HEAT.icns \
    src/ItokawaPoly.txt \
    src/ItokawaPolyID.txt \
    src/ItokawaPolyID4.txt \
    src/ItokawaTemp0.txt \
    src/ItokawaTemp1.txt \
    src/ItokawaTemp2.txt \
    src/RyuguPolygonID_02140919.txt \
    src/RyuguPolygon_02140919.txt \
    src/temp_radiance_table.csv \
    src/tir_response.txt
#LIBS += C:/Project/setup/pthreads/lib/x64/pthreadVC2.lib
#LIBS += C:/Project/setup/pthreads/lib/x64/libpthreadGC2.a
## LIBS += C:/Project/setup/cfitsio/cfitsio.build/Release/cfitsio.lib
## LIBS += C:/Project/setup/ccfits/CCfits.build/Release/CCfits.lib
#LIBS += C:/Project/setup/cfitsio/cfitsio.build/Debug/cfitsio.lib
#LIBS += C:/Project/setup/ccfits/CCfits.build/Debug/CCfits.lib

## QMAKE_LFLAGS_WINDOWS += /F,64000000

ICON = HEAT.icns

##QMAKE_CXXFLAGS += -Xpreprocessor -fopenmp -lomp -I/

## x64
#### LIBS += C:/Project/setup/cfitsio/cfitsio.build/Release/cfitsio.lib
#### LIBS += C:/Project/setup/cfitsio/cfitsio.build/Release/cfitsio.dll
## LIBS += C:/Project/setup/cfitsio/cfitsio.build/Debug/cfitsio.lib
#### LIBS += C:/Project/setup/cfitsio/cfitsio.build/Debug/cfitsio.dll
## LIBS += C:/Project/setup/ccfits/CCfits.build/Debug/CCfits.lib
#### LIBS += C:/Project/setup/pthreads/dll/x64/pthreadGC2.dll
#### LIBS += C:/Project/setup/pthreads/dll/x64/pthreadVC2.dll
## LIBS += C:/Project/setup/pthreads/lib/x64/pthreadVC2.lib
## LIBS += C:/Project/setup/pthreads/lib/x64/libpthreadGC2.a
## LIBS += "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.19041.0/um/x64/OpenGL32.Lib"
## LIBS += "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.19041.0/um/x64/GlU32.Lib"  
### LIBS += /usr/local/lib/libcfitsio.a
### LIBS += /usr/local/lib/libCCfits.dylib
### LIBS += /usr/local/lib/cspice.a
### LIBS += -lpthread
