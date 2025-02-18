##
## (c) 2022 The University of Aizu
## This software is released under the GNU General Public License.
##
QT += core gui sql opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

//!host_build:QMAKE_MAC_SDK = macosx10.10

VERSION = 1.0
QMAKE_TARGET_COMPANY = University of Aizu
QMAKE_TARGET_PRODUCT = HEAT

TARGET = HEAT
TEMPLATE = app
### QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.10


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
    showfitsinfo.cpp


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
    showfitsinfo.h


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
    showfitsinfo.ui
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
QT += widgets

##If you use WindowsOS, you should set the path.
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
##If you use MacOS, you should set the path.
macx{
INCLUDEPATH +=
INCLUDEPATH += /usr/local/include
INCLUDEPATH += /usr/local/Cellar/cfitsio/3.490/include
INCLUDEPATH += /usr/local/Cellar/ccfits/2.5_2/include/CCfits
INCLUDEPATH += /usr/local/Cellar/cspice/66/include
DISTFILES += \
    HEAT.icns
LIBS += -L/usr/local/Cellar/ccfits/2.5_2/lib -lCCfits
LIBS += -L/usr/local/Cellar/cfitsio/3.490/lib -lcfitsio
LIBS += /usr/local/Cellar/cspice/67/lib/cspice.a
QMAKE_CXXFLAGS += -Xpreprocessor -fopenmp -lomp -I/usr/local/include
QMAKE_LFLAGS += -lomp
ICON = HEAT.icns
LIBS += -L /usr/local/lib /usr/local/lib/libomp.dylib
}
CONFIG += opengl console
