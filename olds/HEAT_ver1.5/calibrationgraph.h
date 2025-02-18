//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//
#ifndef CALIBRATIONGRAPH_H
#define CALIBRATIONGRAPH_H

#include "qcustomplot.h"
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QtSql>
#include <algorithm>
#include <math.h>
#include <sstream>

#define PI (4.0*atan(1.0))

namespace Ui {
class CalibrationGraph;
}

class ForThread1 : public QThread
{
    Q_OBJECT
public:
    void run();
    QString name;

private:
    QSqlQuery query_1;
 void saveFileInRegisterRegression(QString fileName, QString folder);



public slots:
};

class ForThread2 : public QThread
{
    Q_OBJECT
public:
    void run();
    QString name;
private:
    QSqlQuery query_2;
 void saveFileInRegisterRegression(QString fileName, QString folder);

public slots:
};

class ForThread3 : public QThread
{
    Q_OBJECT
public:
    void run();
    QString name;

private:
    QSqlQuery query_3;
 void saveFileInRegisterRegression(QString fileName, QString folder);


public slots:
};

class ForThread4 : public QThread
{
    Q_OBJECT
public:
    void run();
    QString name;
private:
    QSqlQuery query_4;
 void saveFileInRegisterRegression(QString fileName, QString folder);

public slots:
};

class ForThread5 : public QThread
{
    Q_OBJECT
public:
    void run();
    QString name;
private:
    QSqlQuery query_5;
 void saveFileInRegisterRegression(QString fileName, QString folder);

public slots:
};

class ForThread6 : public QThread
{
    Q_OBJECT
public:
    void run();
    QString name;
private:
    QSqlQuery query_6;
 void saveFileInRegisterRegression(QString fileName, QString folder);

public slots:
};



double planck4(double);


class CalibrationGraph : public QDialog
{
    Q_OBJECT

public:
    explicit CalibrationGraph(QWidget *parent = 0);
    ~CalibrationGraph();
    CalibrationGraph *cg[10000];
    QVector<QVector<QString> > pixelList; 
    QVector<QVector<QString> > pixelList2;
    QVector<QVector<QString> > diff; 
    QVector<QVector<QString> > replot; 
    QString xAxis, yAxis; 
    QString databPath;
    QVector<double> vx, vy;  
    bool axisFlag=false; 
    int infoNum=0; 
    int previousNum=0;
    int plotNum=0; 
    int regressionGraphCount=1; 
    double xMax,xMin,yMax,yMin, xRangeMax, xRangeMin, yRangeMax, yRangeMin;
    double tgtMax,tgtMin, boloMax, boloMin, pkgMax, pkgMin, caseMax, caseMin, shMax, shMin, lensMax, lensMin;
    double a, b, c, d, e, f, g, h; 
    double planck(double T);
    void OutputSliderValue(QString);
    void OutputUsedImage(QString);
    double OutputSliderValuearray[20];

    QString initialFileDirectory;

    QString **arra, **arrb, **arrc, **arrd, **arre, **arrf, **arrg, **arrh;
    QString **residual;

   
private:
    Ui::CalibrationGraph *ui;
    QVector<double> getAxisValue(QString axis, QVector<QVector<QString> > info, int itemNum);
    QVector<double> getAxisValue2(QString axis, QVector<QVector<QString> > info, int itemNum);
    QSqlDatabase db;
    QSqlQuery query;

    void drawGraph(QVector<double> x, QVector<double> y, QString lineType);
    int judgeAxis(QString x, QString y);
    void setRegressionCoefficient(QVector<double> vx, QVector<double> vy);
    void setRegressionCoefficientforBlack(QVector<double> vx, QVector<double> vy);
    void closeEvent(QCloseEvent *e);
    void setInitializeUI();
    void setMax();
    void setMin();
    void connectDB();
    QDir filterpath;
    QString judgeTableName(int x, int y);
        int judgeTableNameint(int x, int y);
    QString getRegressionCoefficientInRegisterRegression(QVector<double> vx, QVector<double> vy, int xc, int yc);
    QString getRegressionCoefficientInRegisterRegressionforBlackbody(QVector<double> vx, QVector<double> vy, int xc, int yc);
    QString dataPath;
    void saveFileInRegisterRegression(QString fileName, QString folder);



    void makefitssample(std::string filename);




    void loadFilter();
    double h_planck;
    double kB;
    double c_speed;
    double c1;
    double c2;
    double SIGMA;
    
    int Outputplotnumberini;
    int Outputplotnumber;
    int numfortxt;


public slots:
    void popCalibrationGraph(QVector< QVector<QString> > p, QString xAxisName, QString yAxisName, QString lineType, int n, bool ismodified);

private slots:
    void on_regressionButton_clicked();
    void on_tgtMaxSlider_valueChanged(int value);
    void on_tgtMinSlider_valueChanged(int value);
    void on_boloMaxSlider_valueChanged(int value);
    void on_boloMinSlider_valueChanged(int value);
    void on_pkgMaxSlider_valueChanged(int value);
    void on_pkgMinSlider_valueChanged(int value);
    void on_caseMaxSlider_valueChanged(int value);
    void on_caseMinSlider_valueChanged(int value);
    void on_shMaxSlider_valueChanged(int value);
    void on_shMinSlider_valueChanged(int value);
    void on_lensMaxSlider_valueChanged(int value);
    void on_lensMinSlider_valueChanged(int value);
    void on_replotButton_clicked();
    // void on_plotFormulaButton_clicked();
    void on_tgtMaxLineEdit_textChanged(const QString &arg1);
    void on_tgtMinLineEdit_textChanged(const QString &arg1);
    void on_boloMaxLineEdit_textChanged(const QString &arg1);
    void on_boloMinLineEdit_textChanged(const QString &arg1);
    void on_pkgMaxLineEdit_textChanged(const QString &arg1);
    void on_pkgMinLineEdit_textChanged(const QString &arg1);
    void on_caseMaxLineEdit_textChanged(const QString &arg1);
    void on_caseMinLineEdit_textChanged(const QString &arg1);
    void on_shMaxLineEdit_textChanged(const QString &arg1);
    void on_shMinLineEdit_textChanged(const QString &arg1);
    void on_lensMaxLineEdit_textChanged(const QString &arg1);
    void on_lensMinLineEdit_textChanged(const QString &arg1);
    void on_outputCSVFileButton_clicked();
    void on_outputGraphImageButton_clicked();
    void contextMenuRequest(QPoint pos);
    void graphClicked(QCPAbstractPlottable *);
    void removeSelectedGraph();
    void removeRegressionAllGraphs();
    void mousePress();
    void on_exportFormulaButton_clicked();
    void on_loadFileButton_clicked();
    void getDataPath(QString);
};

#endif
