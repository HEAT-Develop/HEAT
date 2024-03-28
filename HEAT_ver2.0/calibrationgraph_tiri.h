//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//
#ifndef CALIBRATIONGRAPH_tiri_H
#define CALIBRATIONGRAPH_tiri_H

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
class CalibrationGraph_tiri;
}




double planck4_tiri(double);


class CalibrationGraph_tiri : public QDialog
{
    Q_OBJECT

public:
    explicit CalibrationGraph_tiri(QWidget *parent = 0);
    ~CalibrationGraph_tiri();
    CalibrationGraph_tiri *cg[10000];

    double DN[768][1024];
    QVector<QVector<QString> > pixelList;
    QVector<QVector<QString> > pixelList2;
    QVector<QVector<QString> > diff;
    QVector<QVector<QString> > replot;
    QString xAxis, yAxis;
    QString databPath;
    QVector<double> vx, vy;
    bool axisFlag=false;
    bool RorT=false;
    int infoNum=0;
    int previousNum=0;
    int plotNum=0;
    int regressionGraphCount=1;
    double xMax,xMin,yMax,yMin, xRangeMax, xRangeMin, yRangeMax, yRangeMin;
    double tgtMax,tgtMin, boloMax, boloMin, pkgMax, pkgMin, caseMax, caseMin, shMax, shMin, lensMax, lensMin;


    double fw1Max,fw1Min,fw2Max,fw2Min,dcMax,dcMin,hodMax,hodMin,rad1Max,rad1Min,rad2Max,rad2Min,len1Max,len1Min,len2Max,len2Min,fpa1Max,fpa1Min,fpa2Max,fpa2Min;





    double a, b, c, d, e, f, g, h;
    double planck(double T);
    double planck(double T, int filter_num);
    void OutputSliderValue(QString);
    void OutputUsedImage(QString);
    double OutputSliderValuearray[20];

    QString initialFileDirectory;





private:
    Ui::CalibrationGraph_tiri *ui;
    QVector<double> getAxisValue(QString axis, QVector<QVector<QString> > info, int itemNum);
    QVector<double> getAxisValue2(QString axis, QVector<QVector<QString> > info, int itemNum);
    QSqlDatabase db;
    QSqlQuery query;



    void drawGraph(QVector<double> x, QVector<double> y, QString lineType);
    int judgeAxis(QString x, QString y);
    void setRegressionCoefficient(QVector<double> vx, QVector<double> vy);
    void setRegressionCoefficientforBlack(QVector<double> vx, QVector<double> vy);
    void setRegressionCoefficientforBlack(QVector<double> vx, QVector<double> vy, int filter_num);
    void closeEvent(QCloseEvent *e);
    void setInitializeUI();
    void setMax();
    void setMin();
    void connectDBtiri();
    QDir filterpath;
    QString judgeTableName(int x, int y);
        int judgeTableNameint(int x, int y);
    QString getRegressionCoefficientInRegisterRegression(QVector<double> vx, QVector<double> vy, int xc, int yc);
    QString getRegressionCoefficientInRegisterRegressionforBlackbody(QVector<double> vx, QVector<double> vy, int xc, int yc);
    QString dataPath;
    void saveFileInRegisterRegression(QString fileName, QString folder);



    void makefitssample(std::string filename);
    void makefitssamplerad(std::string filename);
    double round1_tiri(double dIn);
    double gettemperature_tiri(double FT1);




    void loadFilter();
    double h_planck;
    double kB;
    double c_speed;
    double c1;
    double c2;
    double SIGMA;
    double epsilon=1;

    int Outputplotnumberini;
    int Outputplotnumber;
    int numfortxt;


public slots:
    void checkAction();
    void popCalibrationGraph_tiri(QVector< QVector<QString> > p, QString xAxisName, QString yAxisName, QString lineType, int n, bool ismodified);
private slots:
    void on_regressionButton_clicked();




    void on_boloMaxSlider_valueChanged(int value);
    void on_boloMinSlider_valueChanged(int value);
    void on_pkgMaxSlider_valueChanged(int value);
    void on_pkgMinSlider_valueChanged(int value);
    void on_shMaxSlider_valueChanged(int value);
    void on_shMinSlider_valueChanged(int value);
    void on_lensMaxSlider_valueChanged(int value);
    void on_lensMinSlider_valueChanged(int value);



    void on_tgtMaxSlider_valueChanged(int value);
    void on_tgtMinSlider_valueChanged(int value);
    void on_fpa1MaxSlider_valueChanged(int value);
    void on_fpa1MinSlider_valueChanged(int value);
    void on_fpa2MaxSlider_valueChanged(int value);
    void on_fpa2MinSlider_valueChanged(int value);
    void on_caseMaxSlider_valueChanged(int value);
    void on_caseMinSlider_valueChanged(int value);
    void on_len1MaxSlider_valueChanged(int value);
    void on_len1MinSlider_valueChanged(int value);
    void on_len2MaxSlider_valueChanged(int value);
    void on_len2MinSlider_valueChanged(int value);
    void on_fw1MaxSlider_valueChanged(int value);
    void on_fw1MinSlider_valueChanged(int value);
    void on_fw2MaxSlider_valueChanged(int value);
    void on_fw2MinSlider_valueChanged(int value);
    void on_hodMaxSlider_valueChanged(int value);
    void on_hodMinSlider_valueChanged(int value);
    void on_dcMaxSlider_valueChanged(int value);
    void on_dcMinSlider_valueChanged(int value);
    void on_rad1MaxSlider_valueChanged(int value);
    void on_rad1MinSlider_valueChanged(int value);
    void on_rad2MaxSlider_valueChanged(int value);
    void on_rad2MinSlider_valueChanged(int value);






    void on_fpa1MaxLineEdit_textChanged(const QString &arg1);
    void on_fpa1MinLineEdit_textChanged(const QString &arg1);
    void on_fpa2MaxLineEdit_textChanged(const QString &arg1);
    void on_fpa2MinLineEdit_textChanged(const QString &arg1);
    void on_len1MaxLineEdit_textChanged(const QString &arg1);
    void on_len1MinLineEdit_textChanged(const QString &arg1);
    void on_len2MaxLineEdit_textChanged(const QString &arg1);
    void on_len2MinLineEdit_textChanged(const QString &arg1);
    void on_fw1MaxLineEdit_textChanged(const QString &arg1);
    void on_fw1MinLineEdit_textChanged(const QString &arg1);
    void on_fw2MaxLineEdit_textChanged(const QString &arg1);
    void on_fw2MinLineEdit_textChanged(const QString &arg1);
    void on_hodMaxLineEdit_textChanged(const QString &arg1);
    void on_hodMinLineEdit_textChanged(const QString &arg1);
    void on_dcMaxLineEdit_textChanged(const QString &arg1);
    void on_dcMinLineEdit_textChanged(const QString &arg1);
    void on_rad1MaxLineEdit_textChanged(const QString &arg1);
    void on_rad1MinLineEdit_textChanged(const QString &arg1);
    void on_rad2MaxLineEdit_textChanged(const QString &arg1);
    void on_rad2MinLineEdit_textChanged(const QString &arg1);





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
