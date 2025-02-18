//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//

#include "mainwindow.h"
#include <QApplication>
#include "rendering.h"
#include <stdio.h>
#include "showimage.h"
#include "popwindow.h"
#include "tiling.h"
#include "controlpanel.h"
#include "database.h"
#include "targetmodel.h"
#include "loaddatalist.h"
#include "vtkmodel.h"
#include "showdbinfo.h"
#include "calibration.h"
#include "calibrationgraph.h"
#include "controlgraphpanel.h"
#include "showfitsinfo.h"
#include "project.h"
#include "dataset.h"
#include "readtxt.h"    //2022.12 ono

#include "calibration_tiri.h"
#include "calibrationgraph_tiri.h"

QSqlDatabase Global_DB; //2022.12 ono
int main(int argc, char *argv[])
{
    project pro;
    QApplication a(argc, argv);
    MainWindow w;
    PopWindow p;
    Tiling tt;
    Controlpanel c;
    Database d;
    TargetModel t;
    LoadDataList ls;
    VtkModel v;
    ShowDBInfo i;
    Calibration cali;
    CalibrationGraph cg;
    ControlGraphPanel cgp;
    ShowFITSInfo sfi;
    readtxt rt;
    Calibration_tiri cali_tiri;
    CalibrationGraph_tiri cg_tiri;

    qDebug()<<"Seikou  --->>  "+xmldata.Camera;
    QObject::connect(&w, SIGNAL(exportDataSignal(QString)), &tt, SLOT(tilingWindow(QString)));
    QObject::connect(&w, SIGNAL(tilingWindowSignal()), &tt, SLOT(show()));

    QObject::connect(&c, SIGNAL(showImageSignal()), &ls, SLOT(show()));
    QObject::connect(&c, SIGNAL(showDatabaseSignal()), &d, SLOT(show()));
    QObject::connect(&c, SIGNAL(showTargetModelSignal()), &t, SLOT(show()));
    QObject::connect(&c, SIGNAL(quitSystemSignal()), &a, SLOT(quit()));
    QObject::connect(&c, SIGNAL(showVtkSignal()), &v, SLOT(show()));
    QObject::connect(&c, SIGNAL(showCaliSignal()), &cali, SLOT(show()));

    QObject::connect(&c, SIGNAL(showControlGraphPanel()), &cgp, SLOT(show()));
    QObject::connect(&c, SIGNAL(showReadTxtSignal()), &rt, SLOT(show()));
    QObject::connect(&c, SIGNAL(showCali_tiriSignal()), &cali_tiri, SLOT(show()));
    QObject::connect(&ls, SIGNAL(loadDataSignal_TIRI(QString, int)), &p,SLOT(getFileName_TIRI(QString, int))); // TIRIRyuji
    QObject::connect(&cali_tiri,SIGNAL(showCalibrationGraphSignal_tiri(QVector<QVector<QString>>, QString,QString, QString, int,bool)),&cg_tiri,SLOT(popCalibrationGraph_tiri(QVector<QVector<QString>>, QString, QString,QString, int,bool)));


    QObject::connect(&ls, SIGNAL(loadDataSignal(QString,int)), &p, SLOT(getFileName(QString,int)));
    QObject::connect(&ls, SIGNAL(SelectDataSignal(QString,int)), &p, SLOT(getFileName(QString,int)));
    QObject::connect(&ls, SIGNAL(tilingWindowSignal()), &p, SLOT(tilingWindow()));
    QObject::connect(&ls, SIGNAL(closeWindowSignal()), &p, SLOT(closeAll()));
    QObject::connect(&ls, SIGNAL(changeParameterSignal(double,double,int)), &p, SLOT(changeParameter(double,double,int)));
    QObject::connect(&ls, SIGNAL(substractSignal()),&p, SLOT(substract()));

    QObject::connect(&d, SIGNAL(infoSignal(QString*)), &i, SLOT(getInfo(QString*)));
    QObject::connect(&d, SIGNAL(getFilePathSignal(QString)), &cali, SLOT(getDataPath(QString)));
    QObject::connect(&d, SIGNAL(getFilePathSignal(QString)), &cg, SLOT(getDataPath(QString)));
    QObject::connect(&d, SIGNAL(getFilePathSignal(QString)), &cgp, SLOT(getDataPath(QString)));
    QObject::connect(&cali, SIGNAL(showCalibrationGraphSignal(QVector< QVector<QString> >, QString, QString, QString, int, bool)), &cg, SLOT(popCalibrationGraph(QVector< QVector<QString> >, QString, QString, QString , int, bool)));

    QObject::connect(&cgp, SIGNAL( changeY(QString) ), &cali, SLOT( setY(QString)) );
    QObject::connect(&cgp, SIGNAL( changeX(QString) ), &cali, SLOT( setX(QString)) );
    c.show();
    return a.exec();

}
