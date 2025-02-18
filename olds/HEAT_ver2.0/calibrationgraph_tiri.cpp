
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//
#include "calibrationgraph_tiri.h"
#include "ui_calibrationgraph_tiri.h"
#include <iostream>
#include <fstream>
#include <QDateTime>
#include <showimage.h>
#include <calibration_tiri.h>
#include <controlgraphpanel.h>
#include <exception>
#include <thread>
#include <exception>
#include <QtConcurrent>
#include <omp.h>
#include<FITS.h>
#include <cmath>
#include <CCfits/CCfits>

using namespace CCfits;

static int counting=1;
static int xPosition[10000], yPosition[10000], isActive[10000];
static int n = 0, sum = 0;
static QString Usedimage[4000];
static double tirifilter[1301][8];

/*
// ryuji
static double lambda[1301];
static double integral = 0, epsilon = 0.925;
static double h_planck = 6.62606957e-34;
static double kB = 1.3806488e-23;
static double c_speed = 299792458;
static double c1 = 2 * M_PI * h_planck * pow(c_speed, 2);
static double c2 = (h_planck * c_speed) / kB;
static double SIGMA = 5.670373e-8;
static double c3=(2 * h_planck * c_speed * c_speed);
static double c4[1301];
*/

static double bol_temp;
static double pkg_temp;
static double cas_temp;
static double sht_temp;
static double len_temp;

int filter_num;



static double len1_temp;
static double len2_temp;
static double fw1_temp;
static double fw2_temp;
static double fpa1_temp;
static double fpa2_temp;
static double dc_temp;
static double hod_temp;
static double rad1_temp;
static double rad2_temp;
static QString fw_num;



static bool ismodifiedgl;

static QString fitsfilename;
static QString ImageFilefilepath;

#define Width 1024
#define Height 768

using namespace std;
static int pixelxy_thread;
static int Outputplotnumber_thread;
static QVector<QVector<QString>> tmp2_thread;
static QString xAxis_thread;
static QString yAxis_thread;
static QString initialFileDirectory_thread;
static int xaxis_thread[64*64];
static int yaxis_thread[64*64];
static QString num2_thread;
static QString arrg[768][1024], arrh[768][1024];



CalibrationGraph_tiri::CalibrationGraph_tiri(QWidget *parent) : QDialog(parent),
    ui(new Ui::CalibrationGraph_tiri)
{

    ui->setupUi(this);

    ui->widget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->widget->setInteraction(QCP::iSelectPlottables);

    connect(ui->widget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
    connect(ui->widget, SIGNAL(plottableClick(QCPAbstractPlottable *, QMouseEvent *)), this, SLOT(graphClicked(QCPAbstractPlottable *)));
    connect(ui->widget, SIGNAL(mousePress(QMouseEvent *)), this, SLOT(mousePress()));
    connect(ui->LadorTemp, SIGNAL(clicked()), this, SLOT(checkAction()));

    filterpath.cd("../../../");

    h_planck = 6.62606957e-34;
    kB = 1.3806488e-23;
    c_speed = 299792458;
    c1 = 2 * M_PI * h_planck * pow(c_speed, 2);
    c2 = (h_planck * c_speed) / kB;
    SIGMA = 5.670373e-8;

    Outputplotnumberini = 0;
    Outputplotnumber = 0;
    numfortxt = 0;

    loadFilter();
}

CalibrationGraph_tiri::~CalibrationGraph_tiri()
{
    delete ui;
}

void CalibrationGraph_tiri::popCalibrationGraph_tiri(QVector<QVector<QString>> p, QString xAxisName, QString yAxisName, QString lineType, int itemNum, bool ismodified)
{
    ismodifiedgl = ismodified;

    if (sum < 10000)
    {
        cg[n] = new CalibrationGraph_tiri;
    }
    else
    {
        for (int i = 0; i < 10000; i++)
        {
            if (cg[i]->isHidden())
            {
                n = i;
            }
        }
        if (n == 10000)
        {
            n = 0;
        }
    }

    cg[n]->vx.clear();
    cg[n]->vy.clear();
    cg[n]->pixelList.clear();
    cg[n]->pixelList2.clear();
    cg[n]->diff.clear();
    cg[n]->replot.clear();
    cg[n]->regressionGraphCount = 1;
    cg[n]->axisFlag = false;
    cg[n]->infoNum = 0;
    cg[n]->previousNum = 0;
    cg[n]->plotNum = 0;
    cg[n]->regressionGraphCount = 1;

    if (p[0][0] == "empty")
    {
        return;
    }

    isActive[n] = 1;

    cg[n]->previousNum = itemNum;

    cg[n]->pixelList = p; // p は　引数としてpixelListが渡されている by ryuji
    cg[n]->replot = p;

    cg[n]->xAxis = xAxisName;
    cg[n]->yAxis = yAxisName;

    setMax();
    setMin();

    xPosition[n] = cg[n]->pixelList[0][1].toInt();
    yPosition[n] = cg[n]->pixelList[0][2].toInt();
    cg[n]->pixelList[0][0];
    qDebug()<<cg[n]->pixelList[0][0];
    qDebug()<<sum;


    cg[n]->setWindowTitle("Position : (" + QString::number(xPosition[n]) + "," + QString::number(yPosition[n]) + ")" + "   X : " + xAxisName + "   Y : " + yAxisName);

    int f = judgeAxis(xAxisName, yAxisName);
    //qDebug()<<f<<"konnnitiwaaaaaaa"; //f=2

    if (f == -1)
    {
        cg[n]->vx = getAxisValue(xAxisName, cg[n]->pixelList, itemNum);
        cg[n]->vy = getAxisValue(yAxisName, cg[n]->pixelList, itemNum);
    }
    else if (f == 0)
    {
        cg[n]->vx = getAxisValue2(xAxisName, cg[n]->pixelList, itemNum);
        cg[n]->vy = getAxisValue2(yAxisName, cg[n]->pixelList, itemNum);
    }
    else if (f == 1)
    {
        cg[n]->vx = getAxisValue2(xAxisName, cg[n]->pixelList, itemNum);
        cg[n]->vy = getAxisValue(yAxisName, pixelList2, infoNum);
    }
    else if (f == 2)
    {
        cg[n]->vy = getAxisValue2(yAxisName, cg[n]->pixelList, itemNum);
        cg[n]->vx = getAxisValue(xAxisName, pixelList2, infoNum);
    }


    cg[n]->pixelList2 = pixelList2;


    cg[n]->infoNum = infoNum;
    cg[n]->plotNum = infoNum;


    Outputplotnumberini = cg[n]->plotNum;


    cg[n]->ui->plotNumberBrowser->clear();
    if (f == -1)
    {
        cg[n]->ui->plotNumberBrowser->setText(QString::number(itemNum) + " / " + QString::number(itemNum));
    }
    else
    {
        cg[n]->ui->plotNumberBrowser->setText(QString::number(infoNum) + " / " + QString::number(infoNum));
    }

    if ((f == 1 || f == 2) && (xAxisName == "diff DN" || yAxisName == "diff DN"))
    {
        cg[n]->diff = pixelList2;
        cg[n]->replot = pixelList2;
    }

    setInitializeUI();


    cg[n]->ui->widget->addGraph();
    cg[n]->drawGraph(cg[n]->vx, cg[n]->vy, lineType);

    cg[n]->move(300, 50);
    cg[n]->show();
    cg[n]->raise();
    cg[n]->activateWindow();

    sum++;
    n++;
}

void CalibrationGraph_tiri::setInitializeUI()
{


    cg[n]->ui->tgtMaxLineEdit->clear();
    cg[n]->ui->tgtMinLineEdit->clear();
    cg[n]->ui->fpa1MaxLineEdit->clear();
    cg[n]->ui->fpa1MinLineEdit->clear();
    cg[n]->ui->fpa2MaxLineEdit->clear();
    cg[n]->ui->fpa2MinLineEdit->clear();
    cg[n]->ui->caseMaxLineEdit->clear();
    cg[n]->ui->caseMinLineEdit->clear();
    cg[n]->ui->len1MaxLineEdit->clear();
    cg[n]->ui->len1MinLineEdit->clear();
    cg[n]->ui->len2MaxLineEdit->clear();
    cg[n]->ui->len2MinLineEdit->clear();
    cg[n]->ui->fw1MaxLineEdit->clear();
    cg[n]->ui->fw1MinLineEdit->clear();
    cg[n]->ui->fw2MaxLineEdit->clear();
    cg[n]->ui->fw2MinLineEdit->clear();
    cg[n]->ui->dcMaxLineEdit->clear();
    cg[n]->ui->dcMinLineEdit->clear();
    cg[n]->ui->hodMaxLineEdit->clear();
    cg[n]->ui->hodMinLineEdit->clear();
    cg[n]->ui->rad1MaxLineEdit->clear();
    cg[n]->ui->rad1MinLineEdit->clear();
    cg[n]->ui->rad2MaxLineEdit->clear();
    cg[n]->ui->rad2MinLineEdit->clear();

    cg[n]->ui->regressionFormulaBrowser->clear();
    QDoubleValidator *v = new QDoubleValidator(this);



    cg[n]->ui->tgtMaxSlider->setRange(tgtMin, tgtMax);
    cg[n]->ui->tgtMinSlider->setRange(tgtMin, tgtMax);

    cg[n]->ui->fpa1MaxSlider->setRange(fpa1Min, fpa1Max);
    cg[n]->ui->fpa1MinSlider->setRange(fpa1Min, fpa1Max);

    cg[n]->ui->fpa2MaxSlider->setRange(fpa2Min, fpa2Max);
    cg[n]->ui->fpa2MinSlider->setRange(fpa2Min, fpa2Max);

    cg[n]->ui->caseMaxSlider->setRange(caseMin, caseMax);
    cg[n]->ui->caseMinSlider->setRange(caseMin, caseMax);

    cg[n]->ui->len1MaxSlider->setRange(len1Min, len1Max);
    cg[n]->ui->len1MinSlider->setRange(len1Min, len1Max);

    cg[n]->ui->len2MaxSlider->setRange(len2Min, len2Max);
    cg[n]->ui->len2MinSlider->setRange(len2Min, len2Max);

    cg[n]->ui->fw1MaxSlider->setRange(fw1Min, fw1Max);
    cg[n]->ui->fw1MinSlider->setRange(fw1Min, fw1Max);

    cg[n]->ui->fw2MaxSlider->setRange(fw2Min, fw2Max);
    cg[n]->ui->fw2MinSlider->setRange(fw2Min, fw2Max);

    cg[n]->ui->dcMaxSlider->setRange(dcMin, dcMax);
    cg[n]->ui->dcMinSlider->setRange(dcMin, dcMax);

    cg[n]->ui->hodMaxSlider->setRange(hodMin, hodMax);
    cg[n]->ui->hodMinSlider->setRange(hodMin, hodMax);

    cg[n]->ui->rad1MaxSlider->setRange(rad1Min, rad1Max);
    cg[n]->ui->rad1MinSlider->setRange(rad1Min, rad1Max);

    cg[n]->ui->rad2MaxSlider->setRange(rad2Min, rad2Max);
    cg[n]->ui->rad2MinSlider->setRange(rad2Min, rad2Max);






    cg[n]->ui->tgtMaxSlider->setTickInterval((tgtMax - tgtMin) / 10);
    cg[n]->ui->tgtMinSlider->setTickInterval((tgtMax - tgtMin) / 10);

    cg[n]->ui->fpa1MaxSlider->setTickInterval((fpa1Max - fpa1Min) / 10);
    cg[n]->ui->fpa1MinSlider->setTickInterval((fpa1Max - fpa1Min) / 10);

    cg[n]->ui->fpa2MaxSlider->setTickInterval((fpa2Max - fpa2Min) / 10);
    cg[n]->ui->fpa2MinSlider->setTickInterval((fpa2Max - fpa2Min) / 10);

    cg[n]->ui->caseMaxSlider->setTickInterval((caseMax - caseMin) / 10);
    cg[n]->ui->caseMinSlider->setTickInterval((caseMax - caseMin) / 10);

    cg[n]->ui->len1MaxSlider->setTickInterval((len1Max - len1Min) / 10);
    cg[n]->ui->len1MinSlider->setTickInterval((len1Max - len1Min) / 10);

    cg[n]->ui->len2MaxSlider->setTickInterval((len2Max - len2Min) / 10);
    cg[n]->ui->len2MinSlider->setTickInterval((len2Max - len2Min) / 10);

    cg[n]->ui->fw1MaxSlider->setTickInterval((fw1Max - fw1Min) / 10);
    cg[n]->ui->fw1MinSlider->setTickInterval((fw1Max - fw1Min) / 10);

    cg[n]->ui->dcMaxSlider->setTickInterval((dcMax - dcMin) / 10);
    cg[n]->ui->dcMinSlider->setTickInterval((dcMax - dcMin) / 10);

    cg[n]->ui->hodMaxSlider->setTickInterval((hodMax - hodMin) / 10);
    cg[n]->ui->hodMinSlider->setTickInterval((hodMax - pkgMin) / 10);

    cg[n]->ui->fw2MaxSlider->setTickInterval((fw2Max - fw2Min) / 10);
    cg[n]->ui->fw2MinSlider->setTickInterval((fw2Max - fw2Min) / 10);

    cg[n]->ui->rad1MaxSlider->setTickInterval((rad1Max - rad1Min) / 10);
    cg[n]->ui->rad1MinSlider->setTickInterval((rad1Max - rad1Min) / 10);

    cg[n]->ui->rad2MaxSlider->setTickInterval((rad2Max - rad2Min) / 10);
    cg[n]->ui->rad2MinSlider->setTickInterval((rad2Max - rad2Min) / 10);




    cg[n]->ui->tgtMaxSlider->setValue(tgtMax);
    cg[n]->ui->tgtMinSlider->setValue(tgtMin);

    cg[n]->ui->fpa1MaxSlider->setValue(fpa1Max);
    cg[n]->ui->fpa1MinSlider->setValue(fpa1Min);

    cg[n]->ui->fpa2MaxSlider->setValue(fpa1Max);
    cg[n]->ui->fpa2MinSlider->setValue(fpa1Min);

    cg[n]->ui->caseMaxSlider->setValue(caseMax);
    cg[n]->ui->caseMinSlider->setValue(caseMin);

    cg[n]->ui->len1MaxSlider->setValue(len1Max);
    cg[n]->ui->len1MinSlider->setValue(len1Min);

    cg[n]->ui->len2MaxSlider->setValue(len2Max);
    cg[n]->ui->len2MinSlider->setValue(len2Min);

    cg[n]->ui->fw1MaxSlider->setValue(fw1Max);
    cg[n]->ui->fw1MinSlider->setValue(fw1Min);

    cg[n]->ui->fw2MaxSlider->setValue(fw2Max);
    cg[n]->ui->fw2MinSlider->setValue(fw2Min);

    cg[n]->ui->dcMaxSlider->setValue(dcMax);
    cg[n]->ui->dcMinSlider->setValue(dcMin);

    cg[n]->ui->hodMaxSlider->setValue(hodMax);
    cg[n]->ui->hodMinSlider->setValue(hodMin);

    cg[n]->ui->rad1MaxSlider->setValue(rad1Max);
    cg[n]->ui->rad1MinSlider->setValue(rad1Min);

    cg[n]->ui->rad2MaxSlider->setValue(rad2Max);
    cg[n]->ui->rad2MinSlider->setValue(rad2Min);



}

int CalibrationGraph_tiri::judgeAxis(QString x, QString y)
{
    if (x == "open DN" || x == "close DN" || x == "diff DN")
    {
        if (y == "open DN" || y == "close DN" || y == "diff DN")
        {
            return 0;
        }
        return 1;
    }

    if (y == "open DN" || y == "close DN" || y == "diff DN")
    {
        if (x == "open DN" || x == "close DN" || x == "diff DN")
        {
            return 0;
        }
        return 2;
    }

    return -1;
}

QVector<double> CalibrationGraph_tiri::getAxisValue(QString axis, QVector<QVector<QString>> info, int num)
{

    QVector<double> v(num);

    if (axis == "No.")
    {
        for (int i = 0; i < num; i++)
        {
            v[i] = i + 1;
        }
        return v;
    }
    else if (axis == "pkg T(degC)")
    {
        for (int i = 0; i < num; i++)
        {
            v[i] = info[i][5].toDouble();
        }
        return v;
    }
    else if (axis == "case T(degC)")
    {
        for (int i = 0; i < num; i++)
        {
            v[i] = info[i][6].toDouble();
        }
        return v;
    }
    else if (axis == "shtr T(degC)")
    {
        for (int i = 0; i < num; i++)
        {
            v[i] = info[i][7].toDouble();
        }
        return v;
    }
    else if (axis == "lens T(degC)")
    {
        for (int i = 0; i < num; i++)
        {
            v[i] = info[i][8].toDouble();
        }
        return v;
    }
    else if (axis == "open DN" || axis == "close DN" || axis == "diff DN")
    {
        for (int i = 0; i < num; i++)
        {
            v[i] = info[i][3].toDouble();
        }
        return v;
    }
    else if (axis == "target T(degC)")
    {
        for (int i = 0; i < num; i++)
        {
            v[i] = info[i][5].toDouble();
        }
        return v;
    }
    return v;
}

QVector<double> CalibrationGraph_tiri::getAxisValue2(QString axis, QVector<QVector<QString>> info, int itemNum)
{

    infoNum = 0;
    pixelList2.clear();

    if (axis == "open DN")
    {
        QVector<QString> tmp1;
        infoNum = 0;
        for (int i = 0; i < itemNum; i++)
        {
            if (info[i][0].section('.', 1, 1) == "open")
            {
                tmp1.clear();
                for (int j = 0; j < 25; j++)
                {
                    tmp1.append(info[i][j]);
                }

                pixelList2.append(tmp1);
                infoNum++;
            }
        }

        return getAxisValue(axis, pixelList2, infoNum);
    }
    else if (axis == "close DN")
    {
        QVector<QString> tmp1;
        infoNum = 0;
        for (int i = 0; i < itemNum; i++)
        {
            if (info[i][0].section('.', 2, 2) == "close")
            {
                tmp1.clear();
                for (int j = 0; j < 25; j++)
                {
                    tmp1.append(info[i][j]);
                }
                pixelList2.append(tmp1);
                infoNum++;
            }
        }
        return getAxisValue(axis, pixelList2, infoNum);
    }
    else
    {
        QProgressDialog p;
        p.setLabelText("Search Pair Progress");
        p.setRange(0, itemNum);
        p.setCancelButton(0);

        QVector<QString> tmp1;
        int searchID, pairID;



        infoNum = 0;
        for (int i = 0; i < itemNum; i++)
        {
            //if(info[i][19].toInt()==12)
            //{
                //if(info[i][20]=="full")
                //{
                    if (info[i][0].section('.', 1, 1) == "open")
                    {

                        searchID = info[i][6].toInt(0, 16);


                        if (info[i][18]=="g")
                        {
                            pairID = searchID - 1;
                        }
                        else if(info[i][18]=="a")
                        {

                            pairID = searchID - 2;
                        }
                        else if(info[i][18]=="b")
                        {

                            pairID = searchID - 3;
                        }
                        else if(info[i][18]=="c")
                        {

                            pairID = searchID - 4;
                        }
                        else if(info[i][18]=="d")
                        {

                            pairID = searchID - 5;
                        }
                        else if(info[i][18]=="e")
                        {

                            pairID = searchID - 6;
                        }
                        else if(info[i][18]=="f")
                        {

                            pairID = searchID - 7;
                        }

                        for (int j = 0; j < itemNum; j++)
                        {
                            if (info[j][6].toInt(0, 16) == pairID && info[i][21] == info[j][21] && info[j][0].section('.', 1, 1) == "close") //  && info[i][12] == info[j][12]
                            {
                                if(((info[i][3].toDouble() - info[j][3].toDouble()<150)&&(info[i][21].toStdString()=="Oil_bath_BB")&&info[i][5].toInt()==50)|| // ((info[i][3].toDouble() - info[j][3].toDouble()<150)&&(info[i][21].toStdString()=="BB")&&info[i][5].toInt()==50)||
                                        ((info[i][3].toDouble() - info[j][3].toDouble()<325)&&(info[i][21].toStdString()=="Oil_bath_BB")&&info[i][5].toInt()==75)||
                                        ((info[i][3].toDouble() - info[j][3].toDouble()<550)&&(info[i][21].toStdString()=="Oil_bath_BB")&&info[i][5].toInt()==100)||
                                        ((info[i][3].toDouble() - info[j][3].toDouble()<750)&&(info[i][21].toStdString()=="Oil_bath_BB")&&info[i][5].toInt()==125)){


                                }


                                else{
                                    tmp1.clear();
                                    for (int m = 0; m < 23; m++)
                                    {

                                        if (m == 3)
                                        {
                                            tmp1.append(QString::number(info[i][3].toDouble() - info[j][3].toDouble())); // /16
                                            counting++;
                                        }
                                        else
                                        {
                                            tmp1.append(info[i][m]);
                                        }
                                    }
                                    pixelList2.append(tmp1);
                                    infoNum++;
                                    break;
                                }
                            }
                        }
                    }

                    p.setValue(i);
                    p.show();
                    QCoreApplication::processEvents();
                //}
            //}
        }

        return getAxisValue(axis, pixelList2, infoNum);

    }
}

void CalibrationGraph_tiri::drawGraph(QVector<double> x, QVector<double> y, QString lineType)
{

    ui->widget->graph(0)->clearData();

    ui->widget->xAxis->setLabel(xAxis);
    ui->widget->yAxis->setLabel(yAxis);

    if (lineType == "solid line")
    {
        ui->widget->graph(0)->setLineStyle(QCPGraph::lsLine);
    }
    else
    {
        ui->widget->graph(0)->setLineStyle(QCPGraph::lsNone);
    }
    ui->widget->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 2));

    ui->widget->graph(0)->setData(x, y);
    ui->widget->graph(0)->setPen(QPen(Qt::black));
    ui->widget->rescaleAxes(true);

    xMax = *std::max_element(x.constBegin(), x.constEnd());
    xMin = *std::min_element(x.constBegin(), x.constEnd());
    yMax = *std::max_element(y.constBegin(), y.constEnd());
    yMin = *std::min_element(y.constBegin(), y.constEnd());

    if (axisFlag == false)
    {
        xRangeMin = xMin - ((xMax - xMin) / 10);
        xRangeMax = xMax + ((xMax - xMin) / 10);
        yRangeMin = yMin - ((yMax - yMin) / 10);
        yRangeMax = yMax + ((yMax - yMin) / 10);

        axisFlag = true;
    }

    ui->widget->xAxis->setRange(xRangeMin, xRangeMax);
    ui->widget->yAxis->setRange(yRangeMin, yRangeMax);

    ui->widget->replot();
}

void CalibrationGraph_tiri::on_regressionButton_clicked()
{

    int size = 10000, num = ui->degreeComboBox->currentText().toInt();
    double min = xRangeMin;
    QVector<double> x(size), y(size);
    QString formula = "y = ";
    QString coefficient;
    QString num2 = ui->degreeComboBox->currentText();


    if (vx.size() < 1 || vy.size() < 1)
    {
        ui->regressionFormulaBrowser->setText("The graph is not appropriate.");
        return;
    }

/*
    for (int i = 0; i < replot.size(); i++)
    {
        if(replot[i][18]=='a') filter_num.append(1);
        if(replot[i][18]=='b') filter_num.append(2);
        if(replot[i][18]=='c') filter_num.append(3);
        if(replot[i][18]=='d') filter_num.append(4);
        if(replot[i][18]=='e') filter_num.append(5);
        if(replot[i][18]=='f') filter_num.append(6);
        if(replot[i][18]=='g') filter_num.append(7);
    }
*/
    if (num2 == "Black_Body")
        setRegressionCoefficientforBlack(vx, vy,filter_num);
    else
        setRegressionCoefficient(vx, vy);
    if (7 <= num)
    {
        formula.append(QString::number(a) + "x^7 ");
    }

    coefficient.append(QString::number(a) + ",");
    coefficient.append(QString::number(b) + ",");
    coefficient.append(QString::number(c) + ",");
    coefficient.append(QString::number(d) + ",");
    coefficient.append(QString::number(e) + ",");
    coefficient.append(QString::number(f) + ",");
    coefficient.append(QString::number(g) + ",");
    coefficient.append(QString::number(h) + ",");

    if (6 == num)
    {
        formula.append(QString::number(b) + "x^6 ");
    }
    else if (6 <= num)
    {
        if (b < 0)
        {
            formula.append("- " + QString::number(-1 * b) + "x^6 ");
        }
        else
        {
            formula.append("+ " + QString::number(b) + "x^6 ");
        }
    }

    if (5 == num)
    {
        formula.append(QString::number(c) + "x^5 ");
    }
    else if (5 <= num)
    {
        if (c < 0)
        {
            formula.append("- " + QString::number(-1 * c) + "x^5 ");
        }
        else
        {
            formula.append("+ " + QString::number(c) + "x^5 ");
        }
    }

    if (4 == num)
    {
        formula.append(QString::number(d) + "x^4 ");
    }
    else if (4 <= num)
    {
        if (d < 0)
        {
            formula.append("- " + QString::number(-1 * d) + "x^4 ");
        }
        else
        {
            formula.append("+ " + QString::number(d) + "x^4 ");
        }
    }

    if (3 == num)
    {
        formula.append(QString::number(e) + "x^3 ");
    }
    else if (3 <= num)
    {
        if (e < 0)
        {
            formula.append("- " + QString::number(-1 * e) + "x^3 ");
        }
        else
        {
            formula.append("+ " + QString::number(e) + "x^3 ");
        }
    }

    if (2 == num)
    {
        formula.append(QString::number(f) + "x^2 ");
    }
    else if (2 <= num)
    {
        if (f < 0)
        {
            formula.append("- " + QString::number(-1 * f) + "x^2 ");
        }
        else
        {
            formula.append("+ " + QString::number(f) + "x^2 ");
        }
    }

    if (1 == num || num2 == "Black_Body")
    {
        if (num2 == "Black_Body")
            formula.append(QString::number(g) + "*F( T + 273.15) ");
        else
            formula.append(QString::number(g) + "x ");
    }
    else if (1 <= num)
    {
        if (g < 0)
        {
            if (num2 == "Black_Body")
                formula.append("- " + QString::number(-1 * g) + "*F( T + 273.15) ");
            else
                formula.append("- " + QString::number(-1 * g) + "x ");
        }
        else
        {
            if (num2 == "Black_Body")
                formula.append("+ " + QString::number(g) + "*F( T + 273.15) ");
            else
                formula.append("+ " + QString::number(g) + "x ");
        }
    }
    if (h < 0)
    {
        formula.append("- " + QString::number(-1 * h));
    }
    else
    {
        formula.append("+ " + QString::number(h));
    }

    if (num2 == "Black_Body")
    {
        for (int i = 0; i < size; i++)
        {
            x[i] = min;
            y[i] = g * planck(min + 273.15,filter_num) + h;
            min += (xRangeMax - xRangeMin) / 10000;
        }
    }
    else
    {
        for (int i = 0; i < size; i++)
        {
            x[i] = min;
            y[i] = a * pow(min, 7) + b * pow(min, 6) + c * pow(min, 5) + d * pow(min, 4) + e * pow(min, 3) + f * pow(min, 2) + g * min + h;
            min += (xRangeMax - xRangeMin) / 10000;
        }
    }

    QVector<QString> tmp1;
    int searchID, pairID;

    if ((xAxis == "diff DN") || (yAxis == "diff DN"))
    {
        for (int i = 0; i < replot.size(); i++)
        {
            searchID = replot[i][6].toInt(0, 16);


            if (replot[i][18]=="g")
            {
                pairID = searchID - 1;
            }
            else if(replot[i][18]=="a")
            {

                pairID = searchID - 2;
            }
            else if(replot[i][18]=="b")
            {

                pairID = searchID - 3;
            }
            else if(replot[i][18]=="c")
            {

                pairID = searchID - 4;
            }
            else if(replot[i][18]=="d")
            {

                pairID = searchID - 5;
            }
            else if(replot[i][18]=="e")
            {

                pairID = searchID - 6;
            }
            else if(replot[i][18]=="f")
            {

                pairID = searchID - 7;
            }

            for (int j = 0; j < pixelList.size(); j++)
            {
                if (pixelList[j][0] == replot[i][0])
                {
                    tmp1.append(pixelList[j][0]);
                    break;
                }
            }

            for (int j = 0; j < pixelList.size(); j++)
            {
                if (pixelList[j][6].toInt(0, 16) == pairID && replot[i][21] == pixelList[j][21])
                {
                    tmp1.append(pixelList[j][0]);
                    break;
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < replot.size(); i++)
        {
            if (xAxis == "open DN" || yAxis == "open DN")
            {
                if (replot[i][0].section('.', 1, 1) == "open")
                {
                    tmp1.append(replot[i][0]);
                }
            }
            else if (yAxis == "close DN" || xAxis == "close DN")
            {
                if (replot[i][0].section('.', 1, 1) == "close")
                {
                    tmp1.append(replot[i][0]);
                }
            }
            else
            {
                tmp1.append(replot[i][0]);
            }
        }
    }

    QString searchName;
    searchName.clear();

    for (int l = 0; l < tmp1.size(); l++)
    {
        searchName.append("tiriimageinfo_mask.img_file='" + tmp1[l] + "'");
        if (l + 1 != tmp1.size())
        {
            searchName.append(" or ");
        }
    }


    ui->widget->addGraph();
    ui->widget->graph(regressionGraphCount)->setData(x, y);
    ui->widget->graph(regressionGraphCount)->setPen(QPen(Qt::red));
    ui->widget->graph(regressionGraphCount)->setName(QString::number(num) + "," + coefficient + formula + "," + searchName);
    ui->widget->replot();

    regressionGraphCount++;

}

void CalibrationGraph_tiri::setRegressionCoefficient(QVector<double> vx, QVector<double> vy)
{

    int num;
    double x = 0, x2 = 0, x3 = 0, x4 = 0, x5 = 0, x6 = 0, x7 = 0, x8 = 0, x9 = 0, x10 = 0, x11 = 0, x12 = 0, x13 = 0, x14 = 0;
    double x7y = 0, x6y = 0, x5y = 0, x4y = 0, x3y = 0, x2y = 0, xy = 0, y = 0;
    double pivot, mul;

    num = ui->degreeComboBox->currentText().toInt() + 1;

    a = b = c = d = e = f = g = h = 0;

    for (int i = 0; i < vx.size(); i++)
    {
        x += vx[i];
        x2 += pow(vx[i], 2);
        x3 += pow(vx[i], 3);
        x4 += pow(vx[i], 4);
        x5 += pow(vx[i], 5);
        x6 += pow(vx[i], 6);
        x7 += pow(vx[i], 7);
        x8 += pow(vx[i], 8);
        x9 += pow(vx[i], 9);
        x10 += pow(vx[i], 10);
        x11 += pow(vx[i], 11);
        x12 += pow(vx[i], 12);
        x13 += pow(vx[i], 13);
        x14 += pow(vx[i], 14);
        x7y += (pow(vx[i], 7) * vy[i]);
        x6y += (pow(vx[i], 6) * vy[i]);
        x5y += (pow(vx[i], 5) * vy[i]);
        x4y += (pow(vx[i], 4) * vy[i]);
        x3y += (pow(vx[i], 3) * vy[i]);
        x2y += (pow(vx[i], 2) * vy[i]);
        xy += (vx[i] * vy[i]);
        y += vy[i];
    }

    double M[8][8 + 1] =
    {
        {x14, x13, x12, x11, x10, x9, x8, x7, x7y},
        {x13, x12, x11, x10, x9, x8, x7, x6, x6y},
        {x12, x11, x10, x9, x8, x7, x6, x5, x5y},
        {x11, x10, x9, x8, x7, x6, x5, x4, x4y},
        {x10, x9, x8, x7, x6, x5, x4, x3, x3y},
        {x9, x8, x7, x6, x5, x4, x3, x2, x2y},
        {x8, x7, x6, x5, x4, x3, x2, x, xy},
        {x7, x6, x5, x4, x3, x2, x, double(vx.size()), y}};

    double **M2 = new double *[num];
    for (int i = 0; i < num; i++)
    {
        M2[i] = new double[num + 1];
    }

    for (int i = 0; i < num; i++)
    {
        for (int j = 0; j < num + 1; j++)
        {
            M2[i][j] = M[(8 - num) + i][(8 - num) + j];
        }
    }

    for (int i = 0; i < num; ++i)
    {
        pivot = M2[i][i];
        for (int j = 0; j < num + 1; ++j)
        {
            M2[i][j] = (1 / pivot) * M2[i][j];
        }

        for (int k = i + 1; k < num; ++k)
        {
            mul = M2[k][i];
            for (int count = i; count < num + 1; ++count)
            {
                M2[k][count] = M2[k][count] - mul * M2[i][count];
            }
        }
    }

    for (int i = num - 1; i > 0; --i)
    {
        for (int k = i - 1; k >= 0; --k)
        {
            mul = M2[k][i];
            for (int count = i; count < num + 1; ++count)
            {
                M2[k][count] = M2[k][count] - mul * M2[i][count];
            }
        }
    }

    if (1 < num)
    {
        h = M2[num - 1][num];
        g = M2[num - 2][num];
    }
    if (2 < num)
    {
        f = M2[num - 3][num];
    }
    if (3 < num)
    {
        e = M2[num - 4][num];
    }
    if (4 < num)
    {
        d = M2[num - 5][num];
    }
    if (5 < num)
    {
        c = M2[num - 6][num];
    }
    if (6 < num)
    {
        b = M2[num - 7][num];
    }
    if (7 < num)
    {
        a = M2[num - 8][num];
    }

    for (int i = 0; i < num; i++)
    {
        delete[] M2[i];
    }
    delete[] M2;
}

void CalibrationGraph_tiri::setRegressionCoefficientforBlack(QVector<double> vx, QVector<double> vy)
{

    int num;
    double x = 0, x2 = 0, x3 = 0, x4 = 0, x5 = 0, x6 = 0, x7 = 0, x8 = 0, x9 = 0, x10 = 0, x11 = 0, x12 = 0, x13 = 0, x14 = 0;
    double x7y = 0, x6y = 0, x5y = 0, x4y = 0, x3y = 0, x2y = 0, xy = 0, y = 0;
    double pivot, mul;

    num = 2;

    a = b = c = d = e = f = g = h = 0;

    for (int i = 0; i < vx.size(); i++)
    {
        x += planck(vx[i] + 273.15);
        x2 += pow(planck(vx[i] + 273.15), 2);
        x3 += pow(planck(vx[i] + 273.15), 3);
        x4 += pow(planck(vx[i] + 273.15), 4);
        x5 += pow(planck(vx[i] + 273.15), 5);
        x6 += pow(planck(vx[i] + 273.15), 6);
        x7 += pow(planck(vx[i] + 273.15), 7);
        x8 += pow(planck(vx[i] + 273.15), 8);
        x9 += pow(planck(vx[i] + 273.15), 9);
        x10 += pow(planck(vx[i] + 273.15), 10);
        x11 += pow(planck(vx[i] + 273.15), 11);
        x12 += pow(planck(vx[i] + 273.15), 12);
        x13 += pow(planck(vx[i] + 273.15), 13);
        x14 += pow(planck(vx[i] + 273.15), 14);
        x7y += (pow(planck(vx[i] + 273.15), 7) * vy[i]);
        x6y += (pow(planck(vx[i] + 273.15), 6) * vy[i]);
        x5y += (pow(planck(vx[i] + 273.15), 5) * vy[i]);
        x4y += (pow(planck(vx[i] + 273.15), 4) * vy[i]);
        x3y += (pow(planck(vx[i] + 273.15), 3) * vy[i]);
        x2y += (pow(planck(vx[i] + 273.15), 2) * vy[i]);
        xy += (planck(vx[i] + 273.15) * vy[i]);
        y += vy[i];
    }

    double M[8][8 + 1] =
    {
        {x14, x13, x12, x11, x10, x9, x8, x7, x7y},
        {x13, x12, x11, x10, x9, x8, x7, x6, x6y},
        {x12, x11, x10, x9, x8, x7, x6, x5, x5y},
        {x11, x10, x9, x8, x7, x6, x5, x4, x4y},
        {x10, x9, x8, x7, x6, x5, x4, x3, x3y},
        {x9, x8, x7, x6, x5, x4, x3, x2, x2y},
        {x8, x7, x6, x5, x4, x3, x2, x, xy},
        {x7, x6, x5, x4, x3, x2, x, double(vx.size()), y}};

    double **M2 = new double *[num];
    for (int i = 0; i < num; i++)
    {
        M2[i] = new double[num + 1];
    }

    for (int i = 0; i < num; i++)
    {
        for (int j = 0; j < num + 1; j++)
        {
            M2[i][j] = M[(8 - num) + i][(8 - num) + j];
        }
    }

    for (int i = 0; i < num; ++i)
    {
        pivot = M2[i][i];
        for (int j = 0; j < num + 1; ++j)
        {
            M2[i][j] = (1 / pivot) * M2[i][j];
        }

        for (int k = i + 1; k < num; ++k)
        {
            mul = M2[k][i];
            for (int count = i; count < num + 1; ++count)
            {
                M2[k][count] = M2[k][count] - mul * M2[i][count];
            }
        }
    }

    for (int i = num - 1; i > 0; --i)
    {
        for (int k = i - 1; k >= 0; --k)
        {
            mul = M2[k][i];
            for (int count = i; count < num + 1; ++count)
            {
                M2[k][count] = M2[k][count] - mul * M2[i][count];
            }
        }
    }
    if (1 < num)
    {
        h = M2[num - 1][num];
        g = M2[num - 2][num];
    }
    if (2 < num)
    {
        f = M2[num - 3][num];
    }
    if (3 < num)
    {
        e = M2[num - 4][num];
    }
    if (4 < num)
    {
        d = M2[num - 5][num];
    }
    if (5 < num)
    {
        c = M2[num - 6][num];
    }
    if (6 < num)
    {
        b = M2[num - 7][num];
    }
    if (7 < num)
    {
        a = M2[num - 8][num];
    }

    for (int i = 0; i < num; i++)
    {
        delete[] M2[i];
    }
    delete[] M2;
}









void CalibrationGraph_tiri::setRegressionCoefficientforBlack(QVector<double> vx, QVector<double> vy,int filter_num)
{

    int num;
    double x = 0, x2 = 0, x3 = 0, x4 = 0, x5 = 0, x6 = 0, x7 = 0, x8 = 0, x9 = 0, x10 = 0, x11 = 0, x12 = 0, x13 = 0, x14 = 0;
    double x7y = 0, x6y = 0, x5y = 0, x4y = 0, x3y = 0, x2y = 0, xy = 0, y = 0;
    double pivot, mul;

    num = 2;

    a = b = c = d = e = f = g = h = 0;

    for (int i = 0; i < vx.size(); i++)
    {
        x += planck(vx[i] + 273.15,filter_num);
        x2 += pow(planck(vx[i] + 273.15,filter_num), 2);
        x3 += pow(planck(vx[i] + 273.15,filter_num), 3);
        x4 += pow(planck(vx[i] + 273.15,filter_num), 4);
        x5 += pow(planck(vx[i] + 273.15,filter_num), 5);
        x6 += pow(planck(vx[i] + 273.15,filter_num), 6);
        x7 += pow(planck(vx[i] + 273.15,filter_num), 7);
        x8 += pow(planck(vx[i] + 273.15,filter_num), 8);
        x9 += pow(planck(vx[i] + 273.15,filter_num), 9);
        x10 += pow(planck(vx[i] + 273.15,filter_num), 10);
        x11 += pow(planck(vx[i] + 273.15,filter_num), 11);
        x12 += pow(planck(vx[i] + 273.15,filter_num), 12);
        x13 += pow(planck(vx[i] + 273.15,filter_num), 13);
        x14 += pow(planck(vx[i] + 273.15,filter_num), 14);
        x7y += (pow(planck(vx[i] + 273.15,filter_num), 7) * vy[i]);
        x6y += (pow(planck(vx[i] + 273.15,filter_num), 6) * vy[i]);
        x5y += (pow(planck(vx[i] + 273.15,filter_num), 5) * vy[i]);
        x4y += (pow(planck(vx[i] + 273.15,filter_num), 4) * vy[i]);
        x3y += (pow(planck(vx[i] + 273.15,filter_num), 3) * vy[i]);
        x2y += (pow(planck(vx[i] + 273.15,filter_num), 2) * vy[i]);
        xy += (planck(vx[i] + 273.15,filter_num) * vy[i]);
        y += vy[i];
    }

    double M[8][8 + 1] =
    {
        {x14, x13, x12, x11, x10, x9, x8, x7, x7y},
        {x13, x12, x11, x10, x9, x8, x7, x6, x6y},
        {x12, x11, x10, x9, x8, x7, x6, x5, x5y},
        {x11, x10, x9, x8, x7, x6, x5, x4, x4y},
        {x10, x9, x8, x7, x6, x5, x4, x3, x3y},
        {x9, x8, x7, x6, x5, x4, x3, x2, x2y},
        {x8, x7, x6, x5, x4, x3, x2, x, xy},
        {x7, x6, x5, x4, x3, x2, x, double(vx.size()), y}};

    double **M2 = new double *[num];
    for (int i = 0; i < num; i++)
    {
        M2[i] = new double[num + 1];
    }

    for (int i = 0; i < num; i++)
    {
        for (int j = 0; j < num + 1; j++)
        {
            M2[i][j] = M[(8 - num) + i][(8 - num) + j];
        }
    }

    for (int i = 0; i < num; ++i)
    {
        pivot = M2[i][i];
        for (int j = 0; j < num + 1; ++j)
        {
            M2[i][j] = (1 / pivot) * M2[i][j];
        }

        for (int k = i + 1; k < num; ++k)
        {
            mul = M2[k][i];
            for (int count = i; count < num + 1; ++count)
            {
                M2[k][count] = M2[k][count] - mul * M2[i][count];
            }
        }
    }

    for (int i = num - 1; i > 0; --i)
    {
        for (int k = i - 1; k >= 0; --k)
        {
            mul = M2[k][i];
            for (int count = i; count < num + 1; ++count)
            {
                M2[k][count] = M2[k][count] - mul * M2[i][count];
            }
        }
    }
    if (1 < num)
    {
        h = M2[num - 1][num];
        g = M2[num - 2][num];
    }
    if (2 < num)
    {
        f = M2[num - 3][num];
    }
    if (3 < num)
    {
        e = M2[num - 4][num];
    }
    if (4 < num)
    {
        d = M2[num - 5][num];
    }
    if (5 < num)
    {
        c = M2[num - 6][num];
    }
    if (6 < num)
    {
        b = M2[num - 7][num];
    }
    if (7 < num)
    {
        a = M2[num - 8][num];
    }

    for (int i = 0; i < num; i++)
    {
        delete[] M2[i];
    }
    delete[] M2;
}










void CalibrationGraph_tiri::closeEvent(QCloseEvent *e)
{

    vx.clear();
    vy.clear();
    pixelList.clear();
    pixelList2.clear();
    diff.clear();
    replot.clear();
    regressionGraphCount = 1;
    axisFlag = false;
    infoNum = 0;
    previousNum = 0;
    plotNum = 0;
    regressionGraphCount = 1;

    ui->widget->clearGraphs();

#ifndef QT_NO_WHATSTHIS
    if (isModal() && QWhatsThis::inWhatsThisMode())
        QWhatsThis::leaveWhatsThisMode();
#endif
    if (isVisible())
    {
        QPointer<QObject> that = this;
        reject();
        if (that && isVisible())
            e->ignore();
    }
    else
    {
        e->accept();
    }
}

void CalibrationGraph_tiri::setMax()
{

    tgtMax = boloMax = pkgMax = caseMax = shMax = lensMax = fpa1Max = fpa2Max = fw1Max = fw2Max = dcMax = hodMax = len1Max = len2Max = rad1Max = rad2Max = 0;

    for (int i = 0; i < cg[n]->previousNum; i++)
    {
        if (fpa1Max < cg[n]->pixelList[i][12].toDouble() * 1000)
        {
            fpa1Max = cg[n]->pixelList[i][12].toDouble() * 1000;
        }
        if (fpa2Max < cg[n]->pixelList[i][13].toDouble() * 1000)
        {
            fpa2Max = cg[n]->pixelList[i][13].toDouble() * 1000;
        }
        if (caseMax < cg[n]->pixelList[i][11].toDouble() * 1000)
        {
            caseMax = cg[n]->pixelList[i][11].toDouble() * 1000;
        }
        if (len1Max < cg[n]->pixelList[i][7].toDouble() * 1000)
        {
            len1Max = cg[n]->pixelList[i][7].toDouble() * 1000;
        }
        if (len2Max < cg[n]->pixelList[i][8].toDouble() * 1000)
        {
            len2Max = cg[n]->pixelList[i][8].toDouble() * 1000;
        }
        if (tgtMax < cg[n]->pixelList[i][5].toDouble() * 1000)
        {
            tgtMax = cg[n]->pixelList[i][5].toDouble() * 1000;
        }
        if (fw1Max < cg[n]->pixelList[i][9].toDouble() * 1000)
        {
            fw1Max = cg[n]->pixelList[i][9].toDouble() * 1000;
        }
        if (fw2Max < cg[n]->pixelList[i][10].toDouble() * 1000)
        {
            fw2Max = cg[n]->pixelList[i][10].toDouble() * 1000;
        }
        if (hodMax < cg[n]->pixelList[i][14].toDouble() * 1000)
        {
            hodMax = cg[n]->pixelList[i][14].toDouble() * 1000;
        }
        if (dcMax < cg[n]->pixelList[i][17].toDouble() * 1000)
        {
            dcMax = cg[n]->pixelList[i][17].toDouble() * 1000;
        }
        if (rad1Max < cg[n]->pixelList[i][15].toDouble() * 1000)
        {
            rad1Max = cg[n]->pixelList[i][15].toDouble() * 1000;
        }
        if (rad2Max < cg[n]->pixelList[i][16].toDouble() * 1000)
        {
            rad2Max = cg[n]->pixelList[i][16].toDouble() * 1000;
        }
    }
}

void CalibrationGraph_tiri::setMin()
{

    tgtMin = boloMin = pkgMin = caseMin = shMin = lensMin = fpa1Min = fpa2Min = fw1Min = fw2Min = dcMin = hodMin = len1Min = len2Min = rad1Min = rad2Min =  10000 * 1000;

    for (int i = 0; i < cg[n]->previousNum; i++)
    {
        if (fpa1Min > cg[n]->pixelList[i][12].toDouble() * 1000)
        {
            fpa1Min = cg[n]->pixelList[i][12].toDouble() * 1000;
        }
        if (fpa2Min > cg[n]->pixelList[i][13].toDouble() * 1000)
        {
            fpa2Min = cg[n]->pixelList[i][13].toDouble() * 1000;
        }
        if (caseMin > cg[n]->pixelList[i][11].toDouble() * 1000)
        {
            caseMin = cg[n]->pixelList[i][11].toDouble() * 1000;
        }
        if (len1Min > cg[n]->pixelList[i][7].toDouble() * 1000)
        {
            len1Min = cg[n]->pixelList[i][7].toDouble() * 1000;
        }
        if (len2Min > cg[n]->pixelList[i][8].toDouble() * 1000)
        {
            len2Min = cg[n]->pixelList[i][8].toDouble() * 1000;
        }
        if (tgtMin > cg[n]->pixelList[i][5].toDouble() * 1000)
        {
            tgtMin = cg[n]->pixelList[i][5].toDouble() * 1000;
        }
        if (fw1Min > cg[n]->pixelList[i][9].toDouble() * 1000)
        {
            fw1Min = cg[n]->pixelList[i][9].toDouble() * 1000;
        }
        if (fw2Min > cg[n]->pixelList[i][10].toDouble() * 1000)
        {
            fw2Min = cg[n]->pixelList[i][10].toDouble() * 1000;
        }
        if (hodMin > cg[n]->pixelList[i][14].toDouble() * 1000)
        {
            hodMin = cg[n]->pixelList[i][14].toDouble() * 1000;
        }
        if (dcMin > cg[n]->pixelList[i][17].toDouble() * 1000)
        {
            dcMin = cg[n]->pixelList[i][17].toDouble() * 1000;
        }
        if (rad1Min > cg[n]->pixelList[i][15].toDouble() * 1000)
        {
            rad1Min = cg[n]->pixelList[i][15].toDouble() * 1000;
        }
        if (rad2Min > cg[n]->pixelList[i][16].toDouble() * 1000)
        {
            rad2Min = cg[n]->pixelList[i][16].toDouble() * 1000;
        }
    }
}

void CalibrationGraph_tiri::on_replotButton_clicked()
{
    double tgt_max, tgt_min, bolo_max, bolo_min, pkg_max, pkg_min;
    double case_max, case_min, sh_max, sh_min, lens_max, lens_min;
    int count = 0;

    vx.clear();
    vy.clear();

    tgt_max = ui->tgtMaxLineEdit->text().toDouble();
    tgt_min = ui->tgtMinLineEdit->text().toDouble();

    bolo_max = ui->boloMaxLineEdit->text().toDouble();
    bolo_min = ui->boloMinLineEdit->text().toDouble();

    pkg_max = ui->pkgMaxLineEdit->text().toDouble();
    pkg_min = ui->pkgMinLineEdit->text().toDouble();

    case_max = ui->caseMaxLineEdit->text().toDouble();
    case_min = ui->caseMinLineEdit->text().toDouble();

    sh_max = ui->shMaxLineEdit->text().toDouble();
    sh_min = ui->shMinLineEdit->text().toDouble();

    lens_max = ui->lensMaxLineEdit->text().toDouble();
    lens_min = ui->lensMinLineEdit->text().toDouble();

    QVector<QString> tmp;
    replot.clear();

    int f = judgeAxis(xAxis, yAxis);

    if ((f == 1 || f == 2) && (xAxis == "diff DN" || yAxis == "diff DN"))
    {
        count = 0;
        for (int i = 0; i < infoNum; i++)
        {
            if (tgt_min <= diff[i][9].toDouble() && diff[i][9].toDouble() <= tgt_max)
            {
                if (bolo_min <= diff[i][4].toDouble() && diff[i][4].toDouble() <= bolo_max)
                {
                    if (pkg_min <= diff[i][5].toDouble() && diff[i][5].toDouble() <= pkg_max)
                    {
                        if (case_min <= diff[i][6].toDouble() && diff[i][6].toDouble() <= case_max)
                        {
                            if (sh_min <= diff[i][7].toDouble() && diff[i][7].toDouble() <= sh_max)
                            {
                                if (lens_min <= diff[i][8].toDouble() && diff[i][8].toDouble() <= lens_max)
                                {
                                    tmp.clear();
                                    for (int j = 0; j < 25; j++)
                                    {
                                        tmp.append(diff[i][j]);
                                    }
                                    replot.append(tmp);
                                    count++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        count = 0;
        for (int i = 0; i < previousNum; i++)
        {
            if (tgt_min <= pixelList[i][9].toDouble() && pixelList[i][9].toDouble() <= tgt_max)
            {
                if (bolo_min <= pixelList[i][4].toDouble() && pixelList[i][4].toDouble() <= bolo_max)
                {
                    if (pkg_min <= pixelList[i][5].toDouble() && pixelList[i][5].toDouble() <= pkg_max)
                    {
                        if (case_min <= pixelList[i][6].toDouble() && pixelList[i][6].toDouble() <= case_max)
                        {
                            if (sh_min <= pixelList[i][7].toDouble() && pixelList[i][7].toDouble() <= sh_max)
                            {
                                if (lens_min <= pixelList[i][8].toDouble() && pixelList[i][8].toDouble() <= lens_max)
                                {
                                    tmp.clear();
                                    for (int j = 0; j < 25; j++)
                                    {
                                        tmp.append(pixelList[i][j]);
                                    }
                                    replot.append(tmp);
                                    count++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (f == -1)
    {
        vx = getAxisValue(xAxis, replot, count);
        vy = getAxisValue(yAxis, replot, count);
    }
    else if (f == 0)
    {
        vx = getAxisValue2(xAxis, replot, count);
        vy = getAxisValue2(yAxis, replot, count);
    }
    else if (f == 1)
    {
        if (xAxis == "diff DN")
        {
            vx = getAxisValue(xAxis, replot, count);
            vy = getAxisValue(yAxis, replot, count);
        }
        else
        {
            vx = getAxisValue2(xAxis, replot, count);
            vy = getAxisValue(yAxis, pixelList2, infoNum);
        }
    }
    else if (f == 2)
    {
        if (yAxis == "diff DN")
        {
            vy = getAxisValue(yAxis, replot, count);
            vx = getAxisValue(xAxis, replot, count);
        }
        else
        {
            vy = getAxisValue2(yAxis, replot, count);
            vx = getAxisValue(xAxis, pixelList2, infoNum);
        }
    }

    ui->plotNumberBrowser->clear();
    if (f == -1)
    {
        ui->plotNumberBrowser->setText(QString::number(count) + " / " + QString::number(previousNum));
    }
    else if (xAxis == "diff DN" || yAxis == "diff DN")
    {
        ui->plotNumberBrowser->setText(QString::number(count) + " / " + QString::number(plotNum));
    }
    else
    {
        ui->plotNumberBrowser->setText(QString::number(infoNum) + " / " + QString::number(plotNum));
    }
    Outputplotnumber = count;
    qDebug()<<vy;
    drawGraph(vx, vy, "");
}

void CalibrationGraph_tiri::on_outputCSVFileButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save CSV"), "", tr("CSV (*.csv);;TXT (*.txt)"));

    if (fileName == "")
    {
        return;
    }

    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::information(this, tr("Unable to open file"), file.errorString());
        return;
    }

    QTextStream out(&file);

    out << "img_file" << ',' << "x" << ',' << "y" << ',' << "pixel" << ',';
    out << "mask" << ',' << "thumbnail" << ',' << "path" << ',' << "date_time" << ',';
    out << "m" << ',' << "place" << ',' << "target_name" << ',' << "phi" << ',';
    out << "hood_t" << ',' << "target_t" << ',' << "len_t" << ',' << "plt_t" << ',';
    out << "bol_t" << ',' << "img_id" << ',' << "plt_t_set" << ',' << "bol_t_mon" << ',';
    out << "pkg_t_mon" << ',' << "case_t_mon" << ',' << "sh_t_mon" << ',' << "lens_t_mon" << endl;

    int f = judgeAxis(xAxis, yAxis);

    if (f == -1 || f == 0)
    {
        for (int i = 0; i < replot.size(); i++)
        {
            out << replot[i][0] << ',' << replot[i][1] << ',' << replot[i][2] << ',' << replot[i][3] << ',';
            out << replot[i][13] << ',' << replot[i][15] << ',' << replot[i][12] << ',' << replot[i][16] << ',';
            out << replot[i][17] << ',' << replot[i][18] << ',' << replot[i][11] << ',' << replot[i][19] << ',';
            out << replot[i][20] << ',' << replot[i][9] << ',' << replot[i][21] << ',' << replot[i][22] << ',';
            out << replot[i][23] << ',' << replot[i][10] << ',' << replot[i][24] << ',' << replot[i][4] << ',';
            out << replot[i][5] << ',' << replot[i][6] << ',' << replot[i][7] << ',' << replot[i][8] << endl;
        }
    }
    else
    {
        if (xAxis == "diff DN" || yAxis == "diff DN")
        {
            QProgressDialog p;
            p.setLabelText("Search Pair Progress");
            p.setRange(0, replot.size());
            p.setCancelButton(0);

            QVector<QString> tmp1;
            QVector<QVector<QString>> tmp2;
            int searchID, pairID;

            for (int i = 0; i < replot.size(); i++)
            {
                searchID = replot[i][10].toInt(0, 16);

                if (searchID % 2)
                {
                    pairID = searchID + 1;
                }
                else
                {
                    pairID = searchID - 1;
                }

                for (int j = 0; j < pixelList.size(); j++)
                {
                    if (pixelList[j][0] == replot[i][0])
                    {
                        tmp1.clear();
                        for (int m = 0; m < 25; m++)
                        {
                            tmp1.append(pixelList[j][m]);
                        }

                        tmp2.append(tmp1);
                        break;
                    }
                }

                for (int j = 0; j < pixelList.size(); j++)
                {
                    if (pixelList[j][10].toInt(0, 16) == pairID && replot[i][11] == pixelList[j][11] && replot[i][12] == pixelList[j][12])
                    {
                        tmp1.clear();
                        for (int m = 0; m < 25; m++)
                        {
                            tmp1.append(pixelList[j][m]);
                        }

                        tmp2.append(tmp1);
                        break;
                    }
                }

                p.setValue(i);
                p.show();
                QCoreApplication::processEvents();
            }

            for (int i = 0; i < tmp2.size(); i++)
            {
                out << tmp2[i][0] << ',' << tmp2[i][1] << ',' << tmp2[i][2] << ',' << tmp2[i][3] << ',';
                out << tmp2[i][13] << ',' << tmp2[i][15] << ',' << tmp2[i][12] << ',' << tmp2[i][16] << ',';
                out << tmp2[i][17] << ',' << tmp2[i][18] << ',' << tmp2[i][11] << ',' << tmp2[i][19] << ',';
                out << tmp2[i][20] << ',' << tmp2[i][9] << ',' << tmp2[i][21] << ',' << tmp2[i][22] << ',';
                out << tmp2[i][23] << ',' << tmp2[i][10] << ',' << tmp2[i][24] << ',' << tmp2[i][4] << ',';
                out << tmp2[i][5] << ',' << tmp2[i][6] << ',' << tmp2[i][7] << ',' << tmp2[i][8] << endl;
            }
        }
        else
        {
            for (int i = 0; i < pixelList2.size(); i++)
            {
                out << pixelList2[i][0] << ',' << pixelList2[i][1] << ',' << pixelList2[i][2] << ',' << pixelList2[i][3] << ',';
                out << pixelList2[i][13] << ',' << pixelList2[i][15] << ',' << pixelList2[i][12] << ',' << pixelList2[i][16] << ',';
                out << pixelList2[i][17] << ',' << pixelList2[i][18] << ',' << pixelList2[i][11] << ',' << pixelList2[i][19] << ',';
                out << pixelList2[i][20] << ',' << pixelList2[i][9] << ',' << pixelList2[i][21] << ',' << pixelList2[i][22] << ',';
                out << pixelList2[i][23] << ',' << pixelList2[i][10] << ',' << pixelList2[i][24] << ',' << pixelList2[i][4] << ',';
                out << pixelList2[i][5] << ',' << pixelList2[i][6] << ',' << pixelList2[i][7] << ',' << pixelList2[i][8] << endl;
            }
        }
    }
}

void CalibrationGraph_tiri::OutputSliderValue(QString fileName)
{
    QString date, time, str;

    date = QDateTime::currentDateTime().date().toString(Qt::ISODate);
    time = QDateTime::currentDateTime().time().toString(Qt::ISODate);

    str = date + "_" + time + "_Slidervalue";

    if (fileName == "")
    {
        return;
    }

    QFile file(fileName + "/" + str + ".txt");
    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);

    out << "Target Temperature" << endl;
    out << "Max: " << OutputSliderValuearray[0] << endl;
    out << "Min: " << OutputSliderValuearray[1] << endl;
    out << endl;

    out << "Bolometer Temperature" << endl;
    out << "Max: " << OutputSliderValuearray[2] << endl;
    out << "Min: " << OutputSliderValuearray[3] << endl;
    out << endl;

    out << "Package Temperature" << endl;
    out << "Max: " << OutputSliderValuearray[4] << endl;
    out << "Min: " << OutputSliderValuearray[5] << endl;
    out << endl;

    out << "Case Temperature" << endl;
    out << "Max: " << OutputSliderValuearray[6] << endl;
    out << "Min: " << OutputSliderValuearray[7] << endl;
    out << endl;

    out << "Shutter Temperature" << endl;
    out << "Max: " << OutputSliderValuearray[8] << endl;
    out << "Min: " << OutputSliderValuearray[9] << endl;
    out << endl;

    out << "Lens Temperature" << endl;
    out << "Max: " << OutputSliderValuearray[10] << endl;
    out << "Min: " << OutputSliderValuearray[11] << endl;
    out << endl;
    out << "********************************" << endl;

    out << "Plot Number" << endl;
    out << QString::number(vx.size());
    out << "/";
    out << Outputplotnumberini << endl;

    out << endl;


    out << "Used image file" << endl;
    for (int i = 0; i < numfortxt; i++)
    {
        out << Usedimage[i] << endl;
    }
    file.close();
}

void CalibrationGraph_tiri::OutputUsedImage(QString fileName)
{

    if (fileName == "")
    {
        return;
    }

    QFile file(fileName + "/UsedImage.txt");
    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);


    for (int i = 0; i < numfortxt; i++)
    {
        out << Usedimage[i] << endl;
    }
    file.close();
}













void CalibrationGraph_tiri::on_tgtMaxSlider_valueChanged(int value)
{
    ui->tgtMaxLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[0] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_tgtMinSlider_valueChanged(int value)
{
    ui->tgtMinLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[1] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_fpa1MaxSlider_valueChanged(int value)
{
    ui->fpa1MaxLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[2] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_fpa1MinSlider_valueChanged(int value)
{
    ui->fpa1MinLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[3] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_fpa2MaxSlider_valueChanged(int value)
{
    ui->fpa2MaxLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[4] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_fpa2MinSlider_valueChanged(int value)
{
    ui->fpa2MinLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[5] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_caseMaxSlider_valueChanged(int value)
{
    ui->caseMaxLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[6] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_caseMinSlider_valueChanged(int value)
{
    ui->caseMinLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[7] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_len1MaxSlider_valueChanged(int value)
{
    ui->len1MaxLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[8] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_len1MinSlider_valueChanged(int value)
{
    ui->len1MinLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[9] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_len2MaxSlider_valueChanged(int value)
{
    ui->len2MaxLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[10] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_len2MinSlider_valueChanged(int value)
{
    ui->len2MinLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[11] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_fw1MaxSlider_valueChanged(int value)
{
    ui->fw1MaxLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[0] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_fw1MinSlider_valueChanged(int value)
{
    ui->fw1MinLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[1] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_fw2MaxSlider_valueChanged(int value)
{
    ui->fw2MaxLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[2] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_fw2MinSlider_valueChanged(int value)
{
    ui->fw2MinLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[3] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_dcMaxSlider_valueChanged(int value)
{
    ui->dcMaxLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[4] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_dcMinSlider_valueChanged(int value)
{
    ui->dcMinLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[5] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_hodMaxSlider_valueChanged(int value)
{
    ui->hodMaxLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[6] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_hodMinSlider_valueChanged(int value)
{
    ui->hodMinLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[7] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_rad1MaxSlider_valueChanged(int value)
{
    ui->rad1MaxLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[8] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_rad1MinSlider_valueChanged(int value)
{
    ui->rad1MinLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[9] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_rad2MaxSlider_valueChanged(int value)
{
    ui->rad2MaxLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[10] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_rad2MinSlider_valueChanged(int value)
{
    ui->rad2MinLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[11] = (double)value / 1000;
}









void CalibrationGraph_tiri::on_tgtMaxLineEdit_textChanged(const QString &arg1)
{
    ui->tgtMaxSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_tgtMinLineEdit_textChanged(const QString &arg1)
{
    ui->tgtMinSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_fpa1MaxLineEdit_textChanged(const QString &arg1)
{
    ui->fpa1MaxSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_fpa1MinLineEdit_textChanged(const QString &arg1)
{
    ui->fpa1MinSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_fpa2MaxLineEdit_textChanged(const QString &arg1)
{
    ui->fpa2MaxSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_fpa2MinLineEdit_textChanged(const QString &arg1)
{
    ui->fpa2MinSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_caseMaxLineEdit_textChanged(const QString &arg1)
{
    ui->caseMaxSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_caseMinLineEdit_textChanged(const QString &arg1)
{
    ui->caseMinSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_len1MaxLineEdit_textChanged(const QString &arg1)
{
    ui->len1MaxSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_len1MinLineEdit_textChanged(const QString &arg1)
{
    ui->len1MinSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_len2MaxLineEdit_textChanged(const QString &arg1)
{
    ui->len2MaxSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_len2MinLineEdit_textChanged(const QString &arg1)
{
    ui->len2MinSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_fw1MaxLineEdit_textChanged(const QString &arg1)
{
    ui->fw1MaxSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_fw1MinLineEdit_textChanged(const QString &arg1)
{
    ui->fw1MinSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_fw2MaxLineEdit_textChanged(const QString &arg1)
{
    ui->fw2MaxSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_fw2MinLineEdit_textChanged(const QString &arg1)
{
    ui->fw2MinSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_dcMaxLineEdit_textChanged(const QString &arg1)
{
    ui->dcMaxSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_dcMinLineEdit_textChanged(const QString &arg1)
{
    ui->dcMinSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_hodMaxLineEdit_textChanged(const QString &arg1)
{
    ui->hodMaxSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_hodMinLineEdit_textChanged(const QString &arg1)
{
    ui->hodMinSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_rad1MaxLineEdit_textChanged(const QString &arg1)
{
    ui->rad1MaxSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_rad1MinLineEdit_textChanged(const QString &arg1)
{
    ui->rad1MinSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_rad2MaxLineEdit_textChanged(const QString &arg1)
{
    ui->rad2MaxSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_rad2MinLineEdit_textChanged(const QString &arg1)
{
    ui->rad2MinSlider->setValue(arg1.toDouble() * 1000);
}





void CalibrationGraph_tiri::on_boloMaxSlider_valueChanged(int value)
{
    ui->boloMaxLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[2] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_boloMinSlider_valueChanged(int value)
{
    ui->boloMinLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[3] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_pkgMaxSlider_valueChanged(int value)
{
    ui->pkgMaxLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[4] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_pkgMinSlider_valueChanged(int value)
{
    ui->pkgMinLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[5] = (double)value / 1000;
}



void CalibrationGraph_tiri::on_shMaxSlider_valueChanged(int value)
{
    ui->shMaxLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[8] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_shMinSlider_valueChanged(int value)
{
    ui->shMinLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[9] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_lensMaxSlider_valueChanged(int value)
{
    ui->lensMaxLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[10] = (double)value / 1000;
}

void CalibrationGraph_tiri::on_lensMinSlider_valueChanged(int value)
{
    ui->lensMinLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[11] = (double)value / 1000;
}


void CalibrationGraph_tiri::on_boloMaxLineEdit_textChanged(const QString &arg1)
{
    ui->boloMaxSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_boloMinLineEdit_textChanged(const QString &arg1)
{
    ui->boloMinSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_pkgMaxLineEdit_textChanged(const QString &arg1)
{
    ui->pkgMaxSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_pkgMinLineEdit_textChanged(const QString &arg1)
{
    ui->pkgMinSlider->setValue(arg1.toDouble() * 1000);
}


void CalibrationGraph_tiri::on_shMaxLineEdit_textChanged(const QString &arg1)
{
    ui->shMaxSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_shMinLineEdit_textChanged(const QString &arg1)
{
    ui->shMinSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_lensMaxLineEdit_textChanged(const QString &arg1)
{
    ui->lensMaxSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph_tiri::on_lensMinLineEdit_textChanged(const QString &arg1)
{
    ui->lensMinSlider->setValue(arg1.toDouble() * 1000);
}




















void CalibrationGraph_tiri::on_outputGraphImageButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image"), "", tr("PNG (*.png);;JPG (*.jpg);;PDF (*.pdf)"));

    if (fileName == "")
    {
        return;
    }

    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::information(this, tr("Unable to open file"), file.errorString());
        return;
    }

    if (fileName.section(".", -1, -1) == "png")
    {
        ui->widget->savePng(fileName);
    }
    else if (fileName.section(".", -1, -1) == "jpg")
    {
        ui->widget->saveJpg(fileName);
    }
    else if (fileName.section(".", -1, -1) == "pdf")
    {
        ui->widget->savePdf(fileName);
    }
}

void CalibrationGraph_tiri::contextMenuRequest(QPoint pos)
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    if (ui->widget->selectedGraphs().size() > 0 && !ui->widget->selectedGraphs().contains(ui->widget->graph(0)))
    {
        menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
    }

    if (ui->widget->graphCount() > 0)
    {
        menu->addAction("Remove all regression formula graphs", this, SLOT(removeRegressionAllGraphs()));
    }

    menu->popup(ui->widget->mapToGlobal(pos));
}

void CalibrationGraph_tiri::graphClicked(QCPAbstractPlottable *plottable)
{
    if (plottable->name() == "Graph 1")
    {
        return;
    }

    ui->regressionFormulaBrowser->setText(plottable->name().section(",", -2, -2));



    QString ra, rb, rc, rd, re, rf, rg, rh;

    ra = plottable->name().section(",", -10, -10);
    rb = plottable->name().section(",", -9, -9);
    rc = plottable->name().section(",", -8, -8);
    rd = plottable->name().section(",", -7, -7);
    re = plottable->name().section(",", -6, -6);
    rf = plottable->name().section(",", -5, -5);
    rg = plottable->name().section(",", -4, -4);
    rh = plottable->name().section(",", -3, -3);

}

void CalibrationGraph_tiri::removeSelectedGraph()
{
    if (ui->widget->selectedGraphs().size() > 0)
    {
        ui->widget->removeGraph(ui->widget->selectedGraphs().first());
        ui->widget->replot();
        regressionGraphCount--;
    }
}

void CalibrationGraph_tiri::removeRegressionAllGraphs()
{
    QMessageBox msgBox;
    msgBox.setText("Is it really okay to delete all regression formulas?");
    msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    int ret = msgBox.exec();
    if (ret == QMessageBox::Ok)
    {
        for (int i = regressionGraphCount - 1; 1 <= i; i--)
        {
            ui->widget->removeGraph(i);
        }

        regressionGraphCount = 1;

        ui->widget->replot();
    }
}

void CalibrationGraph_tiri::mousePress()
{
    ui->regressionFormulaBrowser->clear();

}

/* void CalibrationGraph_tiri::on_plotFormulaButton_clicked()
{
    double da, db, dc, dd, de, df, dg, dh;



    int size = 10000, num = 7;
    double min = xRangeMin;
    QVector<double> x(size), y(size);
    QString formula = "y = ";
    QString coefficient;
    QString num2 = ui->degreeComboBox->currentText();

    coefficient.append(QString::number(da) + ",");
    coefficient.append(QString::number(db) + ",");
    coefficient.append(QString::number(dc) + ",");
    coefficient.append(QString::number(dd) + ",");
    coefficient.append(QString::number(de) + ",");
    coefficient.append(QString::number(df) + ",");
    coefficient.append(QString::number(dg) + ",");
    coefficient.append(QString::number(dh) + ",");

    if (7 <= num)
    {
        formula.append(QString::number(da) + "x^7 ");
    }

    if (6 == num)
    {
        formula.append(QString::number(db) + "x^6 ");
    }
    else if (6 <= num)
    {
        if (db < 0)
        {
            formula.append("- " + QString::number(-1 * db) + "x^6 ");
        }
        else
        {
            formula.append("+ " + QString::number(db) + "x^6 ");
        }
    }

    if (5 == num)
    {
        formula.append(QString::number(dc) + "x^5 ");
    }
    else if (5 <= num)
    {
        if (dc < 0)
        {
            formula.append("- " + QString::number(-1 * dc) + "x^5 ");
        }
        else
        {
            formula.append("+ " + QString::number(dc) + "x^5 ");
        }
    }

    if (4 == num)
    {
        formula.append(QString::number(dd) + "x^4 ");
    }
    else if (4 <= num)
    {
        if (dd < 0)
        {
            formula.append("- " + QString::number(-1 * dd) + "x^4 ");
        }
        else
        {
            formula.append("+ " + QString::number(dd) + "x^4 ");
        }
    }

    if (3 == num)
    {
        formula.append(QString::number(de) + "x^3 ");
    }
    else if (3 <= num)
    {
        if (de < 0)
        {
            formula.append("- " + QString::number(-1 * de) + "x^3 ");
        }
        else
        {
            formula.append("+ " + QString::number(de) + "x^3 ");
        }
    }

    if (2 == num)
    {
        formula.append(QString::number(df) + "x^2 ");
    }
    else if (2 <= num)
    {
        if (df < 0)
        {
            formula.append("- " + QString::number(-1 * df) + "x^2 ");
        }
        else
        {
            formula.append("+ " + QString::number(df) + "x^2 ");
        }
    }


    if (1 == num || num2 == "Black_Body")
    {
        if (num2 == "Black_Body")
            formula.append(QString::number(dg) + "*F( T + 273.15) ");
        else
            formula.append(QString::number(dg) + "x ");
    }
    else if (1 <= num)
    {
        if (dg < 0)
        {
            if (num2 == "Black_Body")
                formula.append("- " + QString::number(-1 * dg) + "*F( T + 273.15) ");
            else
                formula.append("- " + QString::number(-1 * dg) + "x ");
        }
        else
        {
            if (num2 == "Black_Body")
                formula.append("+ " + QString::number(dg) + "*F( T + 273.15) ");
            else
                formula.append("+ " + QString::number(dg) + "x ");
        }
    }
    if (dh < 0)
    {
        formula.append("- " + QString::number(-1 * dh));
    }
    else
    {
        formula.append("+ " + QString::number(dh));
    }

    if (num2 == "Black_Body")
    {
        for (int i = 0; i < size; i++)
        {
            x[i] = min;
            y[i] = dg * planck(min + 273.15) + dg * planck(min + 273.15) - dh;
            min += (xRangeMax - xRangeMin) / 10000;
        }
    }
    else
    {
        for (int i = 0; i < size; i++)
        {
            x[i] = min;
            y[i] = da * pow(min, 7) + db * pow(min, 6) + dc * pow(min, 5) + dd * pow(min, 4) + de * pow(min, 3) + df * pow(min, 2) + dg * min + dh;
            min += (xRangeMax - xRangeMin) / 10000;
        }
    }

    QVector<QString> tmp1;
    int searchID, pairID;

    for (int i = 0; i < replot.size(); i++)
    {

        searchID = replot[i][10].toInt(0, 16);

        if (searchID % 2)
        {
            pairID = searchID + 1;
        }
        else
        {
            pairID = searchID - 1;
        }

        for (int j = 0; j < pixelList.size(); j++)
        {
            if (pixelList[j][0] == replot[i][0])
            {
                tmp1.append(pixelList[j][0]);
                break;
            }
        }

        for (int j = 0; j < pixelList.size(); j++)
        {
            if (pixelList[j][10].toInt(0, 16) == pairID &&
                    replot[i][11] == pixelList[j][11] &&
                    replot[i][12] == pixelList[j][12])
            {
                tmp1.append(pixelList[j][0]);
                break;
            }
        }
    }

    QString searchName;
    searchName.clear();
    for (int l = 0; l < tmp1.size(); l++)
    {
        searchName.append("tirimageinfo.img_file='" + tmp1[l] + "'");
        if (l + 1 != tmp1.size())
        {
            searchName.append(" or ");
        }
    }

    ui->widget->addGraph();
    ui->widget->graph(regressionGraphCount)->setData(x, y);
    ui->widget->graph(regressionGraphCount)->setPen(QPen(Qt::red));
    ui->widget->graph(regressionGraphCount)->setName(QString::number(num) + "," + coefficient + formula + "," + searchName);
    ui->widget->replot();

    regressionGraphCount++;
}
*/
void CalibrationGraph_tiri::on_exportFormulaButton_clicked()
{


    int xmin, xmax, ymin, ymax;
    xmin = ui->MinxlineEdit->text().toInt();
    xmax = ui->MaxxlineEdit->text().toInt();
    ymin = ui->MinylineEdit->text().toInt();
    ymax = ui->MaxylineEdit->text().toInt();

    int upperleft=judgeTableNameint(xmin,ymin);
    int upperright=judgeTableNameint(xmax,ymin);
    int lowerleft=judgeTableNameint(xmin,ymax);
    int lowerright=judgeTableNameint(xmax,ymax);
    int tablecounter=upperleft;
    QVector <QString>table;
    int width=upperright-upperleft;
    int tablecounter3=upperleft;
    while(1){
        if(lowerleft==tablecounter3)
        {
            for(int p=lowerleft;p<=lowerright;p++){
                if(tablecounter<10){
                    table.append("pix0"+QString::number(tablecounter));
                }
                else{
                    table.append("pix"+QString::number(tablecounter));
                }
                tablecounter++;
                qDebug()<<lowerright-lowerleft<<" "<<tablecounter;
            }
            break;
        }

        int tablecounter2=1;
        while(1){
            if(tablecounter<10){
                table.append("pix0"+QString::number(tablecounter));
            }
            else{
                table.append("pix"+QString::number(tablecounter));
            }
            if(tablecounter2 == width+1) break;

            tablecounter++;
            tablecounter2++;
            qDebug()<<width<<" "<<tablecounter;
        }

        tablecounter=tablecounter3+16;
        tablecounter3=tablecounter3+16;
    }
    QString date, time;
    date = QDateTime::currentDateTime().date().toString(Qt::ISODate);
    time = QDateTime::currentDateTime().time().toString(Qt::ISODate);
    QString basedir = QDir::homePath()+"/HEATcalibration/";
    QString subdir =
            date + "_" + time + "_" + QString::number(vx.size()) + "grounddata";
    QDir dir=QDir::home();
    QDir dir1(basedir);
    if(!dir1.exists()) {
        dir.mkdir("HEATcalibration");
        qDebug()<<"Make HEATcalibration Directory.";
    }

    if (dir1.exists(subdir))
    {
        QMessageBox msgBox;
        msgBox.setText("This calibration formula already exists. Is it okay to export anyway?");
        msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Cancel)
            return;
    }

    else
    {
        QMessageBox msgBox;
        msgBox.setText("Is it okay to export this formula?");
        msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);

        int ret = msgBox.exec();
        if (ret != QMessageBox::Ok)
        {
            return;
        }
    }
    if (ui->regressionFormulaBrowser->toPlainText() == "" || ui->regressionFormulaBrowser->toPlainText() == "Graph 1" || ui->regressionFormulaBrowser->toPlainText() == "The graph is not appropriate.")
    {
        return;
    }

    if (xAxis == yAxis)
    {
        return;
    }

    if (xAxis == "No." || yAxis == "No.")
    {
        return;
    }

    QProgressDialog p;
    p.setLabelText("Registration Progress");
    p.setRange(0, Height * Width);
    p.show();
    QCoreApplication::processEvents();

   for (int i = 0; i < Height; i++)
    {
        for (int j = 0; j < Width; j++)
        {

            arrg[i][j] = "non";
            arrh[i][j] = "non";
        }
    }


    QVector<QString> tmpfortxt;
    int searchIDfortxt, pairIDfortxt;
    for (int i = 0; i < replot.size(); i++)
    {
        searchIDfortxt = replot[i][6].toInt(0, 16);

        if(filter_num==7) pairIDfortxt = searchIDfortxt - 1;
        else pairIDfortxt=searchIDfortxt-filter_num-1;

        for (int j = 0; j < pixelList.size(); j++)
        {
            if (pixelList[j][0] == replot[i][0])
            {
                tmpfortxt.append(pixelList[j][0]);
                break;
            }
        }

        for (int j = 0; j < pixelList.size(); j++)
        {
            if (pixelList[j][6].toInt(0, 16) == pairIDfortxt && replot[i][21] == pixelList[j][21])
            {
                tmpfortxt.append(pixelList[j][0]);
                break;
            }
        }
    }
    numfortxt = tmpfortxt.size();
    for (int l = 0; l < tmpfortxt.size(); l++)
    {
        Usedimage[l] = tmpfortxt[l];
    }


    QString searchName = ui->widget->selectedGraphs().first()->name().section(",", -1, -1);
    qDebug()<<searchName<<"koredayo--n";

    connectDBtiri();
    QDir tmp;
    QDir dir2(basedir);
    if (!dir2.exists(subdir)){
        dir2.mkdir(subdir);
    }

    initialFileDirectory = basedir + subdir;

    if (initialFileDirectory == "")
    {
        return;
    }


    /*
    for(int i = 1; i < 1301; i++)
    {
        lambda[i-1] = (double)i * 1e-8;
        c4[i-1] = pow(lambda[i-1], 5);
    }
    */
    initialFileDirectory_thread = initialFileDirectory;
    OutputSliderValue(initialFileDirectory);
    OutputUsedImage(initialFileDirectory);
    QString searchObject = "";
    QString tableName = "";
    QVector<QVector<QString>> tmp2;
    QVector<QVector<QString>> tmp3;
    QVector<QString> t;
    int x, y;
    int searchID, pairID;

    if (xAxis == "No.")
    {
        searchObject.append("img_file, ");
    }
    else if (xAxis == "pkg T(degC)")
    {
        searchObject.append("pkg_t_mon, ");
    }
    else if (xAxis == "case T(degC)")
    {
        searchObject.append("case_t_mon, ");
    }
    else if (xAxis == "shtr T(degC)")
    {
        searchObject.append("sh_t_mon, ");
    }
    else if (xAxis == "lens T(degC)")
    {
        searchObject.append("lens_t_mon, ");
    }
    else if ((xAxis == "open DN" || xAxis == "close DN" || xAxis == "diff DN") && ismodifiedgl == false)
    {
        searchObject.append("pixel, ");

    }
    else if ((xAxis == "open DN" || xAxis == "close DN" || xAxis == "diff DN") && ismodifiedgl == true)
    {
        searchObject.append("pixel_modified, ");

    }
    else if (xAxis == "target T(degC)")
    {
        searchObject.append("target_t, ");
    }

    if (yAxis == "No.")
    {
        searchObject.append("img_file, ");
    }
    else if (yAxis == "pkg T(degC)")
    {
        searchObject.append("pkg_t_mon, ");
    }
    else if (yAxis == "case T(degC)")
    {
        searchObject.append("case_t_mon, ");
    }
    else if (yAxis == "shtr T(degC)")
    {
        searchObject.append("sh_t_mon, ");
    }
    else if (yAxis == "lens T(degC)")
    {
        searchObject.append("lens_t_mon, ");
    }
    else if ((yAxis == "open DN" || yAxis == "close DN" || yAxis == "diff DN") && ismodifiedgl == false)
    {
        searchObject.append("pixel, ");
    }
    else if ((yAxis == "open DN" || yAxis == "close DN" || yAxis == "diff DN") && ismodifiedgl == true)
    {
        searchObject.append("pixel_modified, ");

    }
    else if (yAxis == "target T(degC)")
    {
        searchObject.append("target_t, ");
    }

    QString querystring;
    int i = 0;
    // for(i=0;i<table.length();i++)
    // {
    //     qDebug()<<"table["<<i<<"]="<<table[i];
    // }
    //qDebug()<<"aaaaaa "<<fw_num;
    for(i=0;i<table.length();i++)
    {
        tmp2.clear();
        tmp2_thread.clear();
        if (1)
        {
            querystring = "SELECT " + table[i] + ".img_file, x, y," + searchObject + "img_id, target_name,  m, mask FROM " + table[i] + ", tiriimageinfo_mask " +
                    "WHERE tiriimageinfo_mask.img_file=" + table[i] + ".img_file AND (" + searchName + ") order by img_id asc,x asc, y asc"; // AND x>=16 AND x<=343 AND y>=6 AND y<=253";

            query.clear();
            query.exec(querystring);
            int pixelxy;
            //qDebug()<<querystring;


            pixelxy = 64*64;


            pixelxy_thread = pixelxy;
            xAxis_thread = xAxis;
            yAxis_thread = yAxis;
            num2_thread = ui->degreeComboBox->currentText();
            int counter1 = 0;
            int xaxis[pixelxy];
            int yaxis[pixelxy];

            query.first();
            do
            {
                t.clear();
                t.append(query.value(0).toString());

                if(query.value(7).toInt() > 1 && (xAxis == "diff DN" || xAxis == "open DN" || xAxis == "close DN"))
                {
                    t.append(QString::number(query.value(3).toDouble()));// /8
                }
                else
                {
                    t.append(query.value(3).toString());
                }

                if(query.value(7).toInt() > 1 && (yAxis == "diff DN" || yAxis == "open DN" || yAxis == "close DN"))
                {
                    t.append(QString::number(query.value(4).toDouble()));// /16
                }
                else
                {
                    t.append(query.value(4).toString());
                }

                t.append(query.value(5).toString());
                t.append(query.value(6).toString());

                t.append(QString::number(query.value(8).toDouble()));//5 /16

                //tmp2.append(t);
                tmp2_thread.append(t);

                if (counter1 < pixelxy)
                {
                    xaxis[counter1] = query.value(1).toInt();
                    yaxis[counter1] = query.value(2).toInt();
                    xaxis_thread[counter1] = query.value(1).toInt();
                    yaxis_thread[counter1] = query.value(2).toInt();
                    //if(query.value(1).toInt() == 41&query.value(2).toInt()==41) qDebug()<<"("<<query.value(1).toInt()<<","<<query.value(2).toInt()<<") counter = "<<counter;
                }

                counter1++;

                //qDebug()<<"("<<query.value(1).toInt()<<","<<query.value(2).toInt()<<")";

            } while (query.next());
            t.clear();
            //qDebug()<<"counter = "<<counter;
            Outputplotnumber_thread = Outputplotnumber_thread = tmpfortxt.length()/2;
            //qDebug()<<"tmpfortxt="<<tmpfortxt.length(); //絞り込まれた地上試験データの数




            if(tmp2_thread.size()>1){
#pragma omp parallel for
                for (int pix = 0; pix <pixelxy_thread; pix++)
                {
                    int xycounter = pix;

                    int maskcount=0;

                    int ryu=0;
                    int searchID, pairID;
                    QVector<QString> t;
                    QVector<QVector<QString>> tmp3;
                    QDir tmp5;

                    for (int plotn = 0; plotn < Outputplotnumber_thread * 2; plotn++)
                    {
                        int currentpixel = pixelxy_thread * plotn + pix;
                        // if(ryu==0) qDebug()<<tmp2_thread.size();
                        if (tmp2_thread[currentpixel][0].section('.', 1, 1) == "open")
                        {

                            searchID = tmp2_thread[currentpixel][3].toInt(0, 16);
                            if(filter_num==7) pairID = searchID - 1;
                            else pairID=searchID-filter_num-1;
                            for (int plotn2 = 0; plotn2 < Outputplotnumber_thread * 2; plotn2++)
                            {
                                int currentpixel2 = pixelxy_thread * plotn2 + pix;
                                if (tmp2_thread[currentpixel2][3].toInt(0, 16) == pairID && tmp2_thread[currentpixel][4] == tmp2_thread[currentpixel2][4])
                                {



                                    // QString a="47f";
                                    // if(tmp2_thread[currentpixel][3].toInt(0, 16) == a.toInt(0, 16)) {
                                    //     DN[yaxis_thread[xycounter]][xaxis_thread[xycounter]]=tmp2_thread[currentpixel][2].toDouble() - tmp2_thread[currentpixel2][2].toDouble();
                                    //     break;
                                    // }
                                    // else continue;

                                    if((tmp2_thread[currentpixel][2].toDouble() - tmp2_thread[currentpixel2][2].toDouble())>(tmp2_thread[currentpixel][5].toDouble()*0.8) && tmp2_thread[currentpixel][1].toDouble()<20) {continue;}
                                    if((tmp2_thread[currentpixel][2].toDouble() - tmp2_thread[currentpixel2][2].toDouble())<(tmp2_thread[currentpixel][5].toDouble()*0.8) && tmp2_thread[currentpixel][1].toDouble()>20) {continue;}
                                    if (tmp2_thread[currentpixel][1].toDouble()==20) {
                                        //maskcount--;
                                        continue;
                                    }



                                    t.clear();
                                    t.append(tmp2_thread[pixelxy_thread * plotn + pix][0]);
                                    if (xAxis_thread == "diff DN")
                                    {
                                        t.append(QString::number(tmp2_thread[currentpixel][1].toDouble() - tmp2_thread[currentpixel2][1].toDouble()));
                                        t.append(tmp2_thread[pixelxy_thread * plotn + pix][2]);

                                    }
                                    else if (yAxis_thread == "diff DN")
                                    {
                                        t.append(tmp2_thread[pixelxy_thread * plotn + pix][1]);
                                        t.append(QString::number((tmp2_thread[currentpixel][2].toDouble() - tmp2_thread[currentpixel2][2].toDouble())/16));
                                        //qDebug()<<"hatudou";
                                    }
                                    tmp3.append(t);
                                    t.clear();
                                    maskcount++;
                                    //qDebug()<<"asgfdagdgsag";
                                    break;
                                }
                            }
                        }
                        /*
                        else
                        {
                            if(tmp2_thread[currentpixel][3].toInt(0, 16) == 1936) {


                                DN[yaxis_thread[xycounter]][xaxis_thread[xycounter]]=(DN[yaxis_thread[xycounter]][xaxis_thread[xycounter]]-tmp2_thread[currentpixel][2].toDouble())/4;
                                if(yaxis_thread[xycounter]==500 && xaxis_thread[xycounter]==400) qDebug()<<"aaaaaaaaaaa"<<DN[500][400];


                            }
                        }
                        */
                    }

                    QVector<double> vx;
                    QVector<double> vy;
                    if (yAxis_thread == "diff DN" || xAxis_thread == "diff DN")
                    {
                        for (int l = 0; l < tmp3.size(); l++)
                        {
                            vx.append(tmp3[l][1].toDouble());
                            vy.append(tmp3[l][2].toDouble());
                        }
                    }
                    else
                    {
                        for (int l = 0; l < tmp2_thread.size(); l++)
                        {
                            vx.append(tmp2_thread[l][1].toDouble());
                            vy.append(tmp2_thread[l][2].toDouble());
                        }
                    }



                    QString s;
                    int num;
                    double x = 0, x2 = 0, xy = 0, y = 0;
                    double pivot, mul;
                    double aa, bb, cc, dd, ee, ff, gg, hh;
                    QString coefficient, formula;
                    num = 2;
                    aa = bb = cc = dd = ee = ff = gg = hh = 0;



                    for (int i = 0; i < vx.size(); i++)
                    {

                        double pla4=planck4_tiri(vx[i] + 273.15);
                        // double pla4=1;
                        // maskcount=0;


                        //x2 += pow(planck4(vx[i] + 273.15), 2);
                        //xy += (planck4(vx[i] + 273.15) * vy[i]);
                        x2 += pow(pla4, 2);
                        xy += (pla4 * vy[i]);
                        y += vy[i];
                        x += pla4;
                    }
                    double M2[2][3]=
                    {
                        {x2,x,xy},
                        {x,double(vx.size()), y}
                    };





                    for (int i = 0; i < num; ++i)
                    {
                        pivot = M2[i][i];
                        for (int j = 0; j < num + 1; ++j)
                        {
                            M2[i][j] = (1 / pivot) * M2[i][j];
                        }

                        for (int k = i + 1; k < num; ++k)
                        {
                            mul = M2[k][i];
                            for (int count = i; count < num + 1; ++count)
                            {
                                M2[k][count] = M2[k][count] - mul * M2[i][count];
                            }
                        }
                    }



                    for (int i = num - 1; i > 0; --i)
                    {
                        for (int k = i - 1; k >= 0; --k)
                        {
                            mul = M2[k][i];
                            for (int count = i; count < num + 1; ++count)
                            {
                                M2[k][count] = M2[k][count] - mul * M2[i][count];
                            }
                        }
                    }

                    /*
                        for (int i = 0; i < vx.size(); i++)
                        {
                            if(i==0 || vx[i] != vx[i-1])
                            {
                                pla4=planck4(vx[i] + 273.15);
                            }
                            //x2 += pow(planck4(vx[i] + 273.15), 2);
                            //xy += (planck4(vx[i] + 273.15) * vy[i]);
                            x2 += pow(pla4, 2);
                            xy += (pla4 * vy[i]);
                            y += vy[i];
                        }

                        double M2[2][2]=
                        {
                            {x2,xy},
                            {double(vx.size()), y}
                        };

                        // comentout
                        double **M2 = new double *[num];
                        for (int i = 0; i < num; i++)
                        {
                            M2[i] = new double[num];
                        }

                        for (int i = 0; i < num; i++)
                        {
                            for (int j = 0; j < num; j++)
                            {
                                // M2[i][j] = M[(8 - num) + i][(8 - num) + j]; ryuji
                                M2[i][j] = M[i][j];
                            }
                        }
                        // comentout

                        for (int i = 0; i < num; ++i) // num=2
                        {
                            pivot = M2[i][0];
                            for (int j = 0; j < num; ++j)// num=2
                            {
                                M2[i][j] = (1 / pivot) * M2[i][j];
                            }
                        }
                        mul = (1 / pivot)*planck4(vx[i] + 273.15);

                        M2[1][1] = M2[1][1] - mul * M2[0][1];

                        M2[0][1] = M2[0][1] - mul * M2[1][1];
                        */

                    if (1 < num)
                    {
                        hh = M2[1][2];
                        gg = M2[0][2];
                    }


                    /*
                    for (int i = 0; i < num; i++)
                    {
                        delete[] M2[i];
                    }
                    delete[] M2;
                    */

                    double d = 0;
                    double tmp;
                    for (int i = 0; i < vx.size(); i++)
                    {
                        tmp = vy[i] - (gg * vx[i] + hh);
                        if (tmp < 0)
                        {
                            d += tmp * -1;
                        }
                        else
                        {
                            d += tmp;
                        }
                    }
                    d = d / vx.size();
                    //qDebug()<<"ここまでOK"<<yaxis_thread[xycounter]<<","<<xaxis_thread[xycounter]<<"table:"<<table[i];

                    //arrg[tmp_thread][xaxis_thread[xycounter]] = QString::number(gg);
                    //qDebug()<<maskcount;
                    if(maskcount==0) {
                        hh=0;
                        gg=0;
                    }
                    if(!isfinite(gg)) {
                        qDebug()<<maskcount<<" maskcountdayo---n";
                        gg=0;
                    }
                    if(!isfinite(hh)) {
                        qDebug()<<maskcount<<" maskcountdayo---n";
                        hh=0;
                    }
                    if(gg<0) {
                        qDebug()<<maskcount<<" mainasudayyo-n";
                        gg=0;
                    }
                    if(5000<hh) {
                        qDebug()<<maskcount<<" mainasudayyo-n";
                        hh=0;
                    }
                    //gg=300+((gg-70)*8);
                    arrh[yaxis_thread[xycounter]][xaxis_thread[xycounter]] = QString::number(hh);
                    arrg[yaxis_thread[xycounter]][xaxis_thread[xycounter]] = QString::number(gg);
                    // arrh[yaxis_thread[xycounter]][xaxis_thread[xycounter]] = QString::number(gg);
                    // arrg[yaxis_thread[xycounter]][xaxis_thread[xycounter]] = QString::number(hh);



                    //arrh[yaxis_thread[xycounter]][xaxis_thread[xycounter]] = QString::number(hh);

                    //if((yaxis_thread[xycounter] - 6) ==0 && (xaxis_thread[xycounter] - 16)==0) qDebug()<<"[0][0]="<<arrg[0][0]<<" "<<arrh[0][0];

                    coefficient.append(QString::number(gg) + ",");
                    coefficient.append(QString::number(hh) + ",");

                    num = num - 1;

                    if (2 == num)
                    {
                        formula.append(QString::number(ff) + "x^2 ");
                    }
                    else if (2 <= num)
                    {
                        if (ff < 0)
                        {
                            formula.append("- " + QString::number(-1 * ff) + "x^2 ");
                        }
                        else
                        {
                            formula.append("+ " + QString::number(ff) + "x^2 ");
                        }
                    }

                    if (1 == num || num2_thread == "Black_Body")
                    {
                        if (num2_thread == "Black_Body")
                            formula.append(QString::number(gg) + "*F( T + 273.15) ");
                        else
                            formula.append(QString::number(gg) + "x ");
                    }
                    else if (1 <= num)
                    {
                        if (gg < 0)
                        {
                            if (num2_thread == "Black_Body")
                                formula.append("- " + QString::number(-1 * gg) + "*F( T + 273.15) ");
                            else
                                formula.append("- " + QString::number(-1 * gg) + "x ");
                        }
                        else
                        {
                            if (num2_thread == "Black_Body")
                                formula.append("+ " + QString::number(gg) + "*F( T + 273.15) ");
                            else
                                formula.append("+ " + QString::number(gg) + "x ");
                        }
                    }


                    if (hh < 0)
                    {
                        formula.append("- " + QString::number(-1 * hh));
                    }

                    else
                    {
                        formula.append("+ " + QString::number(hh));
                    }

                    s = QString::number(num) + "," + coefficient + formula;

                    vx.clear();
                    vy.clear();
                    tmp3.clear();


                }
            }


            if (p.wasCanceled())
            {

                QProgressDialog p;
                p.setLabelText("Save Registration Information File Process");
                p.setRange(0, 12);
                p.show();
                QCoreApplication::processEvents();

                if (initialFileDirectory == "")
                {
                    return;
                }
            }
        }
    }





    qDebug()<<"calculation complete";
    makefitssample("filter_f_g.fit");
    //if(RorT==true) makefitssamplerad("0_a.fit");
    makefitssamplerad("filter_f_h.fit");

    for (int iii = 1; iii < regressionGraphCount; iii++)
    {
        ui->widget->graph(iii)->setPen(QPen(Qt::red));
    }
    ui->widget->selectedGraphs().first()->setPen(QPen(Qt::darkGreen));

    db.close();
    date = QDateTime::currentDateTime().date().toString(Qt::ISODate);
    time = QDateTime::currentDateTime().time().toString(Qt::ISODate);
    qDebug()<<date<<":"<<time;
    QMessageBox msgBox2;
    msgBox2.setText("This formula registration have been complete.");
    msgBox2.setStandardButtons(QMessageBox::Ok);
    msgBox2.exec();
}



QString CalibrationGraph_tiri::getRegressionCoefficientInRegisterRegression(
        QVector<double> vx, QVector<double> vy, int xc, int yc) {

    int num;
    double x = 0, x2 = 0, x3 = 0, x4 = 0, x5 = 0, x6 = 0, x7 = 0, x8 = 0, x9 = 0,
            x10 = 0, x11 = 0, x12 = 0, x13 = 0, x14 = 0;
    double x7y = 0, x6y = 0, x5y = 0, x4y = 0, x3y = 0, x2y = 0, xy = 0, y = 0;
    double pivot, mul;
    double aa, bb, cc, dd, ee, ff, gg, hh;
    QString coefficient, formula;

    num = ui->widget->selectedGraphs().at(0)->name().section(",", 0, 0).toInt() + 1;

    aa = bb = cc = dd = ee = ff = gg = hh = 0;


    for (int i = 0; i < vx.size(); i++)
    {
        x += vx[i];
        x2 += pow(vx[i], 2);
        x3 += pow(vx[i], 3);
        x4 += pow(vx[i], 4);
        x5 += pow(vx[i], 5);
        x6 += pow(vx[i], 6);
        x7 += pow(vx[i], 7);
        x8 += pow(vx[i], 8);
        x9 += pow(vx[i], 9);
        x10 += pow(vx[i], 10);
        x11 += pow(vx[i], 11);
        x12 += pow(vx[i], 12);
        x13 += pow(vx[i], 13);
        x14 += pow(vx[i], 14);
        x7y += (pow(vx[i], 7) * vy[i]);
        x6y += (pow(vx[i], 6) * vy[i]);
        x5y += (pow(vx[i], 5) * vy[i]);
        x4y += (pow(vx[i], 4) * vy[i]);
        x3y += (pow(vx[i], 3) * vy[i]);
        x2y += (pow(vx[i], 2) * vy[i]);
        xy += (vx[i] * vy[i]);
        y += vy[i];
    }

    double M[8][8 + 1] =
    {
        {x14, x13, x12, x11, x10, x9, x8, x7, x7y},
        {x13, x12, x11, x10, x9, x8, x7, x6, x6y},
        {x12, x11, x10, x9, x8, x7, x6, x5, x5y},
        {x11, x10, x9, x8, x7, x6, x5, x4, x4y},
        {x10, x9, x8, x7, x6, x5, x4, x3, x3y},
        {x9, x8, x7, x6, x5, x4, x3, x2, x2y},
        {x8, x7, x6, x5, x4, x3, x2, x, xy},
        {x7, x6, x5, x4, x3, x2, x, double(vx.size()), y}};

    double **M2 = new double *[num];
    for (int i = 0; i < num; i++)
    {
        M2[i] = new double[num + 1];
    }

    for (int i = 0; i < num; i++)
    {
        for (int j = 0; j < num + 1; j++)
        {
            M2[i][j] = M[(8 - num) + i][(8 - num) + j];
        }
    }

    for (int i = 0; i < num; ++i)
    {
        pivot = M2[i][i];
        for (int j = 0; j < num + 1; ++j)
        {
            M2[i][j] = (1 / pivot) * M2[i][j];
        }

        for (int k = i + 1; k < num; ++k)
        {
            mul = M2[k][i];
            for (int count = i; count < num + 1; ++count)
            {
                M2[k][count] = M2[k][count] - mul * M2[i][count];
            }
        }
    }

    for (int i = num - 1; i > 0; --i)
    {
        for (int k = i - 1; k >= 0; --k)
        {
            mul = M2[k][i];
            for (int count = i; count < num + 1; ++count)
            {
                M2[k][count] = M2[k][count] - mul * M2[i][count];
            }
        }
    }

    if (1 < num)
    {
        hh = M2[num - 1][num];
        gg = M2[num - 2][num];
    }
    if (2 < num)
    {
        ff = M2[num - 3][num];
    }
    if (3 < num)
    {
        ee = M2[num - 4][num];
    }
    if (4 < num)
    {
        dd = M2[num - 5][num];
    }
    if (5 < num)
    {
        cc = M2[num - 6][num];
    }
    if (6 < num)
    {
        bb = M2[num - 7][num];
    }
    if (7 < num)
    {
        aa = M2[num - 8][num];
    }

    for (int i = 0; i < num; i++)
    {
        delete[] M2[i];
    }
    delete[] M2;

    double d = 0;
    double tmp;
    for (int i = 0; i < vx.size(); i++)
    {
        tmp = vy[i] - (aa * pow(vx[i], 7) + bb * pow(vx[i], 6) + cc * pow(vx[i], 5) + dd * pow(vx[i], 4) + ee * pow(vx[i], 3) + ff * pow(vx[i], 2) + gg * vx[i] + hh);
        if (tmp < 0)
        {
            d += tmp * -1;
        }
        else
        {
            d += tmp;
        }
    }

    d = d / vx.size();
    arrg[yc][xc] = QString::number(gg);
    arrh[yc][xc] = QString::number(hh);


    coefficient.append(QString::number(aa) + ",");
    coefficient.append(QString::number(bb) + ",");
    coefficient.append(QString::number(cc) + ",");
    coefficient.append(QString::number(dd) + ",");
    coefficient.append(QString::number(ee) + ",");
    coefficient.append(QString::number(ff) + ",");
    coefficient.append(QString::number(gg) + ",");
    coefficient.append(QString::number(hh) + ",");

    num = num - 1;

    if (7 <= num)
    {
        formula.append(QString::number(aa) + "x^7 ");
    }

    if (6 == num)
    {
        formula.append(QString::number(bb) + "x^6 ");
    }
    else if (6 <= num)
    {
        if (bb < 0)
        {
            formula.append("- " + QString::number(-1 * bb) + "x^6 ");
        }
        else
        {
            formula.append("+ " + QString::number(bb) + "x^6 ");
        }
    }

    if (5 == num)
    {
        formula.append(QString::number(cc) + "x^5 ");
    }
    else if (5 <= num)
    {
        if (cc < 0)
        {
            formula.append("- " + QString::number(-1 * cc) + "x^5 ");
        }
        else
        {
            formula.append("+ " + QString::number(cc) + "x^5 ");
        }
    }

    if (4 == num)
    {
        formula.append(QString::number(dd) + "x^4 ");
    }
    else if (4 <= num)
    {
        if (dd < 0)
        {
            formula.append("- " + QString::number(-1 * dd) + "x^4 ");
        }
        else
        {
            formula.append("+ " + QString::number(dd) + "x^4 ");
        }
    }

    if (3 == num)
    {
        formula.append(QString::number(ee) + "x^3 ");
    }
    else if (3 <= num)
    {
        if (ee < 0)
        {
            formula.append("- " + QString::number(-1 * ee) + "x^3 ");
        }
        else
        {
            formula.append("+ " + QString::number(ee) + "x^3 ");
        }
    }

    if (2 == num)
    {
        formula.append(QString::number(ff) + "x^2 ");
    }
    else if (2 <= num)
    {
        if (ff < 0)
        {
            formula.append("- " + QString::number(-1 * ff) + "x^2 ");
        }
        else
        {
            formula.append("+ " + QString::number(ff) + "x^2 ");
        }
    }

    if (1 == num)
    {
        formula.append(QString::number(gg) + "x ");
    }
    else if (1 <= num)
    {
        if (gg < 0)
        {
            formula.append("- " + QString::number(-1 * gg) + "x ");
        }
        else
        {
            formula.append("+ " + QString::number(gg) + "x ");
        }
    }

    if (hh < 0)
    {
        formula.append("- " + QString::number(-1 * hh));
    }
    else
    {
        formula.append("+ " + QString::number(hh));
    }

    return QString::number(num) + "," + coefficient + formula;
}

QString CalibrationGraph_tiri::getRegressionCoefficientInRegisterRegressionforBlackbody(QVector<double> vx, QVector<double> vy, int xc, int yc)
{

    int num;
    double x = 0, x2 = 0, x3 = 0, x4 = 0, x5 = 0, x6 = 0, x7 = 0, x8 = 0, x9 = 0, x10 = 0, x11 = 0, x12 = 0, x13 = 0, x14 = 0;
    double x7y = 0, x6y = 0, x5y = 0, x4y = 0, x3y = 0, x2y = 0, xy = 0, y = 0;
    double pivot, mul;
    double aa, bb, cc, dd, ee, ff, gg, hh;
    QString coefficient, formula;
    QString num2 = ui->degreeComboBox->currentText();
    num = 2;
    aa = bb = cc = dd = ee = ff = gg = hh = 0;

    for (int i = 0; i < vx.size(); i++)
    {
        x += planck(vx[i] + 273.15);
        x2 += pow(planck(vx[i] + 273.15), 2);
        x3 += pow(planck(vx[i] + 273.15), 3);
        x4 += pow(planck(vx[i] + 273.15), 4);
        x5 += pow(planck(vx[i] + 273.15), 5);
        x6 += pow(planck(vx[i] + 273.15), 6);
        x7 += pow(planck(vx[i] + 273.15), 7);
        x8 += pow(planck(vx[i] + 273.15), 8);
        x9 += pow(planck(vx[i] + 273.15), 9);
        x10 += pow(planck(vx[i] + 273.15), 10);
        x11 += pow(planck(vx[i] + 273.15), 11);
        x12 += pow(planck(vx[i] + 273.15), 12);
        x13 += pow(planck(vx[i] + 273.15), 13);
        x14 += pow(planck(vx[i] + 273.15), 14);
        x7y += (pow(planck(vx[i] + 273.15), 7) * vy[i]);
        x6y += (pow(planck(vx[i] + 273.15), 6) * vy[i]);
        x5y += (pow(planck(vx[i] + 273.15), 5) * vy[i]);
        x4y += (pow(planck(vx[i] + 273.15), 4) * vy[i]);
        x3y += (pow(planck(vx[i] + 273.15), 3) * vy[i]);
        x2y += (pow(planck(vx[i] + 273.15), 2) * vy[i]);
        xy += (planck(vx[i] + 273.15) * vy[i]);
        y += vy[i];

    }

    double M[8][8 + 1] =
    {
        {x14, x13, x12, x11, x10, x9, x8, x7, x7y},
        {x13, x12, x11, x10, x9, x8, x7, x6, x6y},
        {x12, x11, x10, x9, x8, x7, x6, x5, x5y},
        {x11, x10, x9, x8, x7, x6, x5, x4, x4y},
        {x10, x9, x8, x7, x6, x5, x4, x3, x3y},
        {x9, x8, x7, x6, x5, x4, x3, x2, x2y},
        {x8, x7, x6, x5, x4, x3, x2, x, xy},
        {x7, x6, x5, x4, x3, x2, x, double(vx.size()), y}};

    double **M2 = new double *[num];
    for (int i = 0; i < num; i++)
    {
        M2[i] = new double[num + 1];
    }

    for (int i = 0; i < num; i++)
    {
        for (int j = 0; j < num + 1; j++)
        {
            M2[i][j] = M[(8 - num) + i][(8 - num) + j];
        }
    }

    for (int i = 0; i < num; ++i)
    {
        pivot = M2[i][i];
        for (int j = 0; j < num + 1; ++j)
        {
            M2[i][j] = (1 / pivot) * M2[i][j];
        }

        for (int k = i + 1; k < num; ++k)
        {
            mul = M2[k][i];
            for (int count = i; count < num + 1; ++count)
            {
                M2[k][count] = M2[k][count] - mul * M2[i][count];
            }
        }
    }
    for (int i = num - 1; i > 0; --i)
    {
        for (int k = i - 1; k >= 0; --k)
        {
            mul = M2[k][i];
            for (int count = i; count < num + 1; ++count)
            {
                M2[k][count] = M2[k][count] - mul * M2[i][count];
            }
        }
    }

    if (1 < num)
    {
        hh = M2[num - 1][num];
        gg = M2[num - 2][num];
    }
    if (2 < num)
    {
        ff = M2[num - 3][num];
    }
    if (3 < num)
    {
        ee = M2[num - 4][num];
    }
    if (4 < num)
    {
        dd = M2[num - 5][num];
    }
    if (5 < num)
    {
        cc = M2[num - 6][num];
    }
    if (6 < num)
    {
        bb = M2[num - 7][num];
    }
    if (7 < num)
    {
        aa = M2[num - 8][num];
    }

    for (int i = 0; i < num; i++)
    {
        delete[] M2[i];
    }
    delete[] M2;

    double d = 0;
    double tmp;
    for (int i = 0; i < vx.size(); i++)
    {
        tmp = vy[i] - (gg * vx[i] + hh);
        if (tmp < 0)
        {
            d += tmp * -1;
        }
        else
        {
            d += tmp;
        }
    }
    d = d / vx.size();
    arrg[yc][xc] = QString::number(gg);
    arrh[yc][xc] = QString::number(hh);


    coefficient.append(QString::number(aa) + ",");
    coefficient.append(QString::number(bb) + ",");
    coefficient.append(QString::number(cc) + ",");
    coefficient.append(QString::number(dd) + ",");
    coefficient.append(QString::number(ee) + ",");
    coefficient.append(QString::number(ff) + ",");
    coefficient.append(QString::number(gg) + ",");
    coefficient.append(QString::number(hh) + ",");

    num = num - 1;

    if (7 <= num)
    {
        formula.append(QString::number(aa) + "x^7 ");
    }

    if (6 == num)
    {
        formula.append(QString::number(bb) + "x^6 ");
    }
    else if (6 <= num)
    {
        if (bb < 0)
        {
            formula.append("- " + QString::number(-1 * bb) + "x^6 ");
        }
        else
        {
            formula.append("+ " + QString::number(bb) + "x^6 ");
        }
    }

    if (5 == num)
    {
        formula.append(QString::number(cc) + "x^5 ");
    }
    else if (5 <= num)
    {
        if (cc < 0)
        {
            formula.append("- " + QString::number(-1 * cc) + "x^5 ");
        }
        else
        {
            formula.append("+ " + QString::number(cc) + "x^5 ");
        }
    }

    if (4 == num)
    {
        formula.append(QString::number(dd) + "x^4 ");
    }
    else if (4 <= num)
    {
        if (dd < 0)
        {
            formula.append("- " + QString::number(-1 * dd) + "x^4 ");
        }
        else
        {
            formula.append("+ " + QString::number(dd) + "x^4 ");
        }
    }

    if (3 == num)
    {
        formula.append(QString::number(ee) + "x^3 ");
    }
    else if (3 <= num)
    {
        if (ee < 0)
        {
            formula.append("- " + QString::number(-1 * ee) + "x^3 ");
        }
        else
        {
            formula.append("+ " + QString::number(ee) + "x^3 ");
        }
    }

    if (2 == num)
    {
        formula.append(QString::number(ff) + "x^2 ");
    }
    else if (2 <= num)
    {
        if (ff < 0)
        {
            formula.append("- " + QString::number(-1 * ff) + "x^2 ");
        }
        else
        {
            formula.append("+ " + QString::number(ff) + "x^2 ");
        }
    }

    if (1 == num || num2 == "Black_Body")
    {
        if (num2 == "Black_Body")
            formula.append(QString::number(gg) + "*F( T + 273.15) ");
        else
            formula.append(QString::number(gg) + "x ");
    }
    else if (1 <= num)
    {
        if (gg < 0)
        {
            if (num2 == "Black_Body")
                formula.append("- " + QString::number(-1 * gg) + "*F( T + 273.15) ");
            else
                formula.append("- " + QString::number(-1 * gg) + "x ");
        }
        else
        {
            if (num2 == "Black_Body")
                formula.append("+ " + QString::number(gg) + "*F( T + 273.15) ");
            else
                formula.append("+ " + QString::number(gg) + "x ");
        }
    }

    if (hh < 0)
    {
        formula.append("- " + QString::number(-1 * hh));
    }
    else
    {
        formula.append("+ " + QString::number(hh));
    }

    return QString::number(num) + "," + coefficient + formula;
}


void CalibrationGraph_tiri::connectDBtiri()
{

    db = QSqlDatabase::addDatabase(QString("QMYSQL"),"konkontiri");

    db.setHostName("localhost");

    db.setUserName(QString("root"));

    db.setPassword(QString("kontake825"));

    db.setDatabaseName(QString("TIRI_pre"));

    db.open();

    query = QSqlQuery(db);

    if (query.isActive())
    {
        query.first();
    }


    /*db.open();
    query = QSqlQuery(db);
    if (query.isActive())
    {
        query.first();
    }
    */



}

QString CalibrationGraph_tiri::judgeTableName(int x, int y)
{
    if (0 <= y && y <= 63)
    {
        if (0 <= x && x <= 63)
            return "pix01";
        else if (64 <= x && x <= 127)
            return "pix02";
        else if (128 <= x && x <= 191)
            return "pix03";
        else if (192 <= x && x <= 255)
            return "pix04";
        else if (256 <= x && x <= 319)
            return "pix05";
        else if (320 <= x && x <= 383)
            return "pix06";
        else if (384 <= x && x <= 447)
            return "pix07";
        else if (448 <= x && x <= 511)
            return "pix08";
        else if (512 <= x && x <= 575)
            return "pix09";
        else if (576 <= x && x <= 639)
            return "pix10";
        else if (640 <= x && x <= 703)
            return "pix11";
        else if (704 <= x && x <= 767)
            return "pix12";
        else if (768 <= x && x <= 831)
            return "pix13";
        else if (832 <= x && x <= 895)
            return "pix14";
        else if (896 <= x && x <= 959)
            return "pix15";
        else if (960 <= x && x <= 1024)
            return "pix16";
    }
    else if (64 <= y && y <= 127)
    {
        if (0 <= x && x <= 63)
            return "pix17";
        else if (64 <= x && x <= 127)
            return "pix18";
        else if (128 <= x && x <= 191)
            return "pix19";
        else if (192 <= x && x <= 255)
            return "pix20";
        else if (256 <= x && x <= 319)
            return "pix21";
        else if (320 <= x && x <= 383)
            return "pix22";
        else if (384 <= x && x <= 447)
            return "pix23";
        else if (448 <= x && x <= 511)
            return "pix24";
        else if (512 <= x && x <= 575)
            return "pix25";
        else if (576 <= x && x <= 639)
            return "pix26";
        else if (640 <= x && x <= 703)
            return "pix27";
        else if (704 <= x && x <= 767)
            return "pix28";
        else if (768 <= x && x <= 831)
            return "pix29";
        else if (832 <= x && x <= 895)
            return "pix30";
        else if (896 <= x && x <= 959)
            return "pix31";
        else if (960 <= x && x <= 1024)
            return "pix32";
    }
    else if (128 <= y && y <= 191)
    {
        if (0 <= x && x <= 63)
            return "pix33";
        else if (64 <= x && x <= 127)
            return "pix34";
        else if (128 <= x && x <= 191)
            return "pix35";
        else if (192 <= x && x <= 255)
            return "pix36";
        else if (256 <= x && x <= 319)
            return "pix37";
        else if (320 <= x && x <= 383)
            return "pix38";
        else if (384 <= x && x <= 447)
            return "pix39";
        else if (448 <= x && x <= 511)
            return "pix40";
        else if (512 <= x && x <= 575)
            return "pix41";
        else if (576 <= x && x <= 639)
            return "pix42";
        else if (640 <= x && x <= 703)
            return "pix43";
        else if (704 <= x && x <= 767)
            return "pix44";
        else if (768 <= x && x <= 831)
            return "pix45";
        else if (832 <= x && x <= 895)
            return "pix46";
        else if (896 <= x && x <= 959)
            return "pix47";
        else if (960 <= x && x <= 1024)
            return "pix48";
    }
    else if (192 <= y && y <= 255)
    {
        if (0 <= x && x <= 63)
            return "pix49";
        else if (64 <= x && x <= 127)
            return "pix50";
        else if (128 <= x && x <= 191)
            return "pix51";
        else if (192 <= x && x <= 255)
            return "pix52";
        else if (256 <= x && x <= 319)
            return "pix53";
        else if (320 <= x && x <= 383)
            return "pix54";
        else if (384 <= x && x <= 447)
            return "pix55";
        else if (448 <= x && x <= 511)
            return "pix56";
        else if (512 <= x && x <= 575)
            return "pix57";
        else if (576 <= x && x <= 639)
            return "pix58";
        else if (640 <= x && x <= 703)
            return "pix59";
        else if (704 <= x && x <= 767)
            return "pix60";
        else if (768 <= x && x <= 831)
            return "pix61";
        else if (832 <= x && x <= 895)
            return "pix62";
        else if (896 <= x && x <= 959)
            return "pix63";
        else if (960 <= x && x <= 1024)
            return "pix64";
    }
    else if (256 <= y && y <= 319)
    {
        if (0 <= x && x <= 63)
            return "pix65";
        else if (64 <= x && x <= 127)
            return "pix66";
        else if (128 <= x && x <= 191)
            return "pix67";
        else if (192 <= x && x <= 255)
            return "pix68";
        else if (256 <= x && x <= 319)
            return "pix69";
        else if (320 <= x && x <= 383)
            return "pix70";
        else if (384 <= x && x <= 447)
            return "pix71";
        else if (448 <= x && x <= 511)
            return "pix72";
        else if (512 <= x && x <= 575)
            return "pix73";
        else if (576 <= x && x <= 639)
            return "pix74";
        else if (640 <= x && x <= 703)
            return "pix75";
        else if (704 <= x && x <= 767)
            return "pix76";
        else if (768 <= x && x <= 831)
            return "pix77";
        else if (832 <= x && x <= 895)
            return "pix78";
        else if (896 <= x && x <= 959)
            return "pix79";
        else if (960 <= x && x <= 1024)
            return "pix80";
    }
    else if (320 <= y && y <= 383)
    {
        if (0 <= x && x <= 63)
            return "pix81";
        else if (64 <= x && x <= 127)
            return "pix82";
        else if (128 <= x && x <= 191)
            return "pix83";
        else if (192 <= x && x <= 255)
            return "pix84";
        else if (256 <= x && x <= 319)
            return "pix85";
        else if (320 <= x && x <= 383)
            return "pix86";
        else if (384 <= x && x <= 447)
            return "pix87";
        else if (448 <= x && x <= 511)
            return "pix88";
        else if (512 <= x && x <= 575)
            return "pix89";
        else if (576 <= x && x <= 639)
            return "pix90";
        else if (640 <= x && x <= 703)
            return "pix91";
        else if (704 <= x && x <= 767)
            return "pix92";
        else if (768 <= x && x <= 831)
            return "pix93";
        else if (832 <= x && x <= 895)
            return "pix94";
        else if (896 <= x && x <= 959)
            return "pix95";
        else if (960 <= x && x <= 1024)
            return "pix96";
    }
    else if (384 <= y && y <= 447)
    {
        if (0 <= x && x <= 63)
            return "pix97";
        else if (64 <= x && x <= 127)
            return "pix98";
        else if (128 <= x && x <= 191)
            return "pix99";
        else if (192 <= x && x <= 255)
            return "pix100";
        else if (256 <= x && x <= 319)
            return "pix101";
        else if (320 <= x && x <= 383)
            return "pix102";
        else if (384 <= x && x <= 447)
            return "pix103";
        else if (448 <= x && x <= 511)
            return "pix104";
        else if (512 <= x && x <= 575)
            return "pix105";
        else if (576 <= x && x <= 639)
            return "pix106";
        else if (640 <= x && x <= 703)
            return "pix107";
        else if (704 <= x && x <= 767)
            return "pix108";
        else if (768 <= x && x <= 831)
            return "pix109";
        else if (832 <= x && x <= 895)
            return "pix110";
        else if (896 <= x && x <= 959)
            return "pix111";
        else if (960 <= x && x <= 1024)
            return "pix112";
    }
    else if (448 <= y && y <= 511)
    {
        if (0 <= x && x <= 63)
            return "pix113";
        else if (64 <= x && x <= 127)
            return "pix114";
        else if (128 <= x && x <= 191)
            return "pix115";
        else if (192 <= x && x <= 255)
            return "pix116";
        else if (256 <= x && x <= 319)
            return "pix117";
        else if (320 <= x && x <= 383)
            return "pix118";
        else if (384 <= x && x <= 447)
            return "pix119";
        else if (448 <= x && x <= 511)
            return "pix120";
        else if (512 <= x && x <= 575)
            return "pix121";
        else if (576 <= x && x <= 639)
            return "pix122";
        else if (640 <= x && x <= 703)
            return "pix123";
        else if (704 <= x && x <= 767)
            return "pix124";
        else if (768 <= x && x <= 831)
            return "pix125";
        else if (832 <= x && x <= 895)
            return "pix126";
        else if (896 <= x && x <= 959)
            return "pix127";
        else if (960 <= x && x <= 1024)
            return "pix128";
    }
    else if (512 <= y && y <= 575)
    {
        if (0 <= x && x <= 63)
            return "pix129";
        else if (64 <= x && x <= 127)
            return "pix130";
        else if (128 <= x && x <= 191)
            return "pix131";
        else if (192 <= x && x <= 255)
            return "pix132";
        else if (256 <= x && x <= 319)
            return "pix133";
        else if (320 <= x && x <= 383)
            return "pix134";
        else if (384 <= x && x <= 447)
            return "pix135";
        else if (448 <= x && x <= 511)
            return "pix136";
        else if (512 <= x && x <= 575)
            return "pix137";
        else if (576 <= x && x <= 639)
            return "pix138";
        else if (640 <= x && x <= 703)
            return "pix139";
        else if (704 <= x && x <= 767)
            return "pix140";
        else if (768 <= x && x <= 831)
            return "pix141";
        else if (832 <= x && x <= 895)
            return "pix142";
        else if (896 <= x && x <= 959)
            return "pix143";
        else if (960 <= x && x <= 1024)
            return "pix144";
    }
    else if (576 <= y && y <= 639)
    {
        if (0 <= x && x <= 63)
            return "pix145";
        else if (64 <= x && x <= 127)
            return "pix146";
        else if (128 <= x && x <= 191)
            return "pix147";
        else if (192 <= x && x <= 255)
            return "pix148";
        else if (256 <= x && x <= 319)
            return "pix149";
        else if (320 <= x && x <= 383)
            return "pix150";
        else if (384 <= x && x <= 447)
            return "pix151";
        else if (448 <= x && x <= 511)
            return "pix152";
        else if (512 <= x && x <= 575)
            return "pix153";
        else if (576 <= x && x <= 639)
            return "pix154";
        else if (640 <= x && x <= 703)
            return "pix155";
        else if (704 <= x && x <= 767)
            return "pix156";
        else if (768 <= x && x <= 831)
            return "pix157";
        else if (832 <= x && x <= 895)
            return "pix158";
        else if (896 <= x && x <= 959)
            return "pix159";
        else if (960 <= x && x <= 1024)
            return "pix160";
    }
    else if (640 <= y && y <= 703)
    {
        if (0 <= x && x <= 63)
            return "pix161";
        else if (64 <= x && x <= 127)
            return "pix162";
        else if (128 <= x && x <= 191)
            return "pix163";
        else if (192 <= x && x <= 255)
            return "pix164";
        else if (256 <= x && x <= 319)
            return "pix165";
        else if (320 <= x && x <= 383)
            return "pix166";
        else if (384 <= x && x <= 447)
            return "pix167";
        else if (448 <= x && x <= 511)
            return "pix168";
        else if (512 <= x && x <= 575)
            return "pix169";
        else if (576 <= x && x <= 639)
            return "pix170";
        else if (640 <= x && x <= 703)
            return "pix171";
        else if (704 <= x && x <= 767)
            return "pix172";
        else if (768 <= x && x <= 831)
            return "pix173";
        else if (832 <= x && x <= 895)
            return "pix174";
        else if (896 <= x && x <= 959)
            return "pix175";
        else if (960 <= x && x <= 1024)
            return "pix176";
    }
    else if (704<= y && y <= 768)
    {
        if (0 <= x && x <= 63)
            return "pix177";
        else if (64 <= x && x <= 127)
            return "pix178";
        else if (128 <= x && x <= 191)
            return "pix179";
        else if (192 <= x && x <= 255)
            return "pix180";
        else if (256 <= x && x <= 319)
            return "pix181";
        else if (320 <= x && x <= 383)
            return "pix182";
        else if (384 <= x && x <= 447)
            return "pix183";
        else if (448 <= x && x <= 511)
            return "pix184";
        else if (512 <= x && x <= 575)
            return "pix185";
        else if (576 <= x && x <= 639)
            return "pix186";
        else if (640 <= x && x <= 703)
            return "pix187";
        else if (704 <= x && x <= 767)
            return "pix188";
        else if (768 <= x && x <= 831)
            return "pix189";
        else if (832 <= x && x <= 895)
            return "pix190";
        else if (896 <= x && x <= 959)
            return "pix191";
        else if (960 <= x && x <= 1024)
            return "pix192";
    }

    return "";
}

int CalibrationGraph_tiri::judgeTableNameint(int x, int y)
{
    if (0 <= y && y <= 63)
    {
        if (0 <= x && x <= 63)
            return 1;
        else if (64 <= x && x <= 127)
            return 2;
        else if (128 <= x && x <= 191)
            return 3;
        else if (192 <= x && x <= 255)
            return 4;
        else if (256 <= x && x <= 319)
            return 5;
        else if (320 <= x && x <= 383)
            return 6;
        else if (384 <= x && x <= 447)
            return 7;
        else if (448 <= x && x <= 511)
            return 8;
        else if (512 <= x && x <= 575)
            return 9;
        else if (576 <= x && x <= 639)
            return 10;
        else if (640 <= x && x <= 703)
            return 11;
        else if (704 <= x && x <= 767)
            return 12;
        else if (768 <= x && x <= 831)
            return 13;
        else if (832 <= x && x <= 895)
            return 14;
        else if (896 <= x && x <= 959)
            return 15;
        else if (960 <= x && x <= 1024)
            return 16;
    }
    else if (64 <= y && y <= 127)
    {
        if (0 <= x && x <= 63)
            return 17;
        else if (64 <= x && x <= 127)
            return 18;
        else if (128 <= x && x <= 191)
            return 19;
        else if (192 <= x && x <= 255)
            return 20;
        else if (256 <= x && x <= 319)
            return 21;
        else if (320 <= x && x <= 383)
            return 22;
        else if (384 <= x && x <= 447)
            return 23;
        else if (448 <= x && x <= 511)
            return 24;
        else if (512 <= x && x <= 575)
            return 25;
        else if (576 <= x && x <= 639)
            return 26;
        else if (640 <= x && x <= 703)
            return 27;
        else if (704 <= x && x <= 767)
            return 28;
        else if (768 <= x && x <= 831)
            return 29;
        else if (832 <= x && x <= 895)
            return 30;
        else if (896 <= x && x <= 959)
            return 31;
        else if (960 <= x && x <= 1024)
            return 32;
    }
    else if (128 <= y && y <= 191)
    {
        if (0 <= x && x <= 63)
            return 33;
        else if (64 <= x && x <= 127)
            return 34;
        else if (128 <= x && x <= 191)
            return 35;
        else if (192 <= x && x <= 255)
            return 36;
        else if (256 <= x && x <= 319)
            return 37;
        else if (320 <= x && x <= 383)
            return 38;
        else if (384 <= x && x <= 447)
            return 39;
        else if (448 <= x && x <= 511)
            return 40;
        else if (512 <= x && x <= 575)
            return 41;
        else if (576 <= x && x <= 639)
            return 42;
        else if (640 <= x && x <= 703)
            return 43;
        else if (704 <= x && x <= 767)
            return 44;
        else if (768 <= x && x <= 831)
            return 45;
        else if (832 <= x && x <= 895)
            return 46;
        else if (896 <= x && x <= 959)
            return 47;
        else if (960 <= x && x <= 1024)
            return 48;
    }
    else if (192 <= y && y <= 255)
    {
        if (0 <= x && x <= 63)
            return 49;
        else if (64 <= x && x <= 127)
            return 50;
        else if (128 <= x && x <= 191)
            return 51;
        else if (192 <= x && x <= 255)
            return 52;
        else if (256 <= x && x <= 319)
            return 53;
        else if (320 <= x && x <= 383)
            return 54;
        else if (384 <= x && x <= 447)
            return 55;
        else if (448 <= x && x <= 511)
            return 56;
        else if (512 <= x && x <= 575)
            return 57;
        else if (576 <= x && x <= 639)
            return 58;
        else if (640 <= x && x <= 703)
            return 59;
        else if (704 <= x && x <= 767)
            return 60;
        else if (768 <= x && x <= 831)
            return 61;
        else if (832 <= x && x <= 895)
            return 62;
        else if (896 <= x && x <= 959)
            return 63;
        else if (960 <= x && x <= 1024)
            return 64;
    }
    else if (256 <= y && y <= 319)
    {
        if (0 <= x && x <= 63)
            return 65;
        else if (64 <= x && x <= 127)
            return 66;
        else if (128 <= x && x <= 191)
            return 67;
        else if (192 <= x && x <= 255)
            return 68;
        else if (256 <= x && x <= 319)
            return 69;
        else if (320 <= x && x <= 383)
            return 70;
        else if (384 <= x && x <= 447)
            return 71;
        else if (448 <= x && x <= 511)
            return 72;
        else if (512 <= x && x <= 575)
            return 73;
        else if (576 <= x && x <= 639)
            return 74;
        else if (640 <= x && x <= 703)
            return 75;
        else if (704 <= x && x <= 767)
            return 76;
        else if (768 <= x && x <= 831)
            return 77;
        else if (832 <= x && x <= 895)
            return 78;
        else if (896 <= x && x <= 959)
            return 79;
        else if (960 <= x && x <= 1024)
            return 80;
    }
    else if (320 <= y && y <= 383)
    {
        if (0 <= x && x <= 63)
            return 81;
        else if (64 <= x && x <= 127)
            return 82;
        else if (128 <= x && x <= 191)
            return 83;
        else if (192 <= x && x <= 255)
            return 84;
        else if (256 <= x && x <= 319)
            return 85;
        else if (320 <= x && x <= 383)
            return 86;
        else if (384 <= x && x <= 447)
            return 87;
        else if (448 <= x && x <= 511)
            return 88;
        else if (512 <= x && x <= 575)
            return 89;
        else if (576 <= x && x <= 639)
            return 90;
        else if (640 <= x && x <= 703)
            return 91;
        else if (704 <= x && x <= 767)
            return 92;
        else if (768 <= x && x <= 831)
            return 93;
        else if (832 <= x && x <= 895)
            return 94;
        else if (896 <= x && x <= 959)
            return 95;
        else if (960 <= x && x <= 1024)
            return 96;
    }
    else if (384 <= y && y <= 447)
    {
        if (0 <= x && x <= 63)
            return 97;
        else if (64 <= x && x <= 127)
            return 98;
        else if (128 <= x && x <= 191)
            return 99;
        else if (192 <= x && x <= 255)
            return 100;
        else if (256 <= x && x <= 319)
            return 101;
        else if (320 <= x && x <= 383)
            return 102;
        else if (384 <= x && x <= 447)
            return 103;
        else if (448 <= x && x <= 511)
            return 104;
        else if (512 <= x && x <= 575)
            return 105;
        else if (576 <= x && x <= 639)
            return 106;
        else if (640 <= x && x <= 703)
            return 107;
        else if (704 <= x && x <= 767)
            return 108;
        else if (768 <= x && x <= 831)
            return 109;
        else if (832 <= x && x <= 895)
            return 110;
        else if (896 <= x && x <= 959)
            return 111;
        else if (960 <= x && x <= 1024)
            return 112;
    }
    else if (448 <= y && y <= 511)
    {
        if (0 <= x && x <= 63)
            return 113;
        else if (64 <= x && x <= 127)
            return 114;
        else if (128 <= x && x <= 191)
            return 115;
        else if (192 <= x && x <= 255)
            return 116;
        else if (256 <= x && x <= 319)
            return 117;
        else if (320 <= x && x <= 383)
            return 118;
        else if (384 <= x && x <= 447)
            return 119;
        else if (448 <= x && x <= 511)
            return 120;
        else if (512 <= x && x <= 575)
            return 121;
        else if (576 <= x && x <= 639)
            return 122;
        else if (640 <= x && x <= 703)
            return 123;
        else if (704 <= x && x <= 767)
            return 124;
        else if (768 <= x && x <= 831)
            return 125;
        else if (832 <= x && x <= 895)
            return 126;
        else if (896 <= x && x <= 959)
            return 127;
        else if (960 <= x && x <= 1024)
            return 128;
    }
    else if (512 <= y && y <= 575)
    {
        if (0 <= x && x <= 63)
            return 129;
        else if (64 <= x && x <= 127)
            return 130;
        else if (128 <= x && x <= 191)
            return 131;
        else if (192 <= x && x <= 255)
            return 132;
        else if (256 <= x && x <= 319)
            return 133;
        else if (320 <= x && x <= 383)
            return 134;
        else if (384 <= x && x <= 447)
            return 135;
        else if (448 <= x && x <= 511)
            return 136;
        else if (512 <= x && x <= 575)
            return 137;
        else if (576 <= x && x <= 639)
            return 138;
        else if (640 <= x && x <= 703)
            return 139;
        else if (704 <= x && x <= 767)
            return 140;
        else if (768 <= x && x <= 831)
            return 141;
        else if (832 <= x && x <= 895)
            return 142;
        else if (896 <= x && x <= 959)
            return 143;
        else if (960 <= x && x <= 1024)
            return 144;
    }
    else if (576 <= y && y <= 639)
    {
        if (0 <= x && x <= 63)
            return 145;
        else if (64 <= x && x <= 127)
            return 146;
        else if (128 <= x && x <= 191)
            return 147;
        else if (192 <= x && x <= 255)
            return 148;
        else if (256 <= x && x <= 319)
            return 149;
        else if (320 <= x && x <= 383)
            return 150;
        else if (384 <= x && x <= 447)
            return 151;
        else if (448 <= x && x <= 511)
            return 152;
        else if (512 <= x && x <= 575)
            return 153;
        else if (576 <= x && x <= 639)
            return 154;
        else if (640 <= x && x <= 703)
            return 155;
        else if (704 <= x && x <= 767)
            return 156;
        else if (768 <= x && x <= 831)
            return 157;
        else if (832 <= x && x <= 895)
            return 158;
        else if (896 <= x && x <= 959)
            return 159;
        else if (960 <= x && x <= 1024)
            return 160;
    }
    else if (640 <= y && y <= 703)
    {
        if (0 <= x && x <= 63)
            return 161;
        else if (64 <= x && x <= 127)
            return 162;
        else if (128 <= x && x <= 191)
            return 163;
        else if (192 <= x && x <= 255)
            return 164;
        else if (256 <= x && x <= 319)
            return 165;
        else if (320 <= x && x <= 383)
            return 166;
        else if (384 <= x && x <= 447)
            return 167;
        else if (448 <= x && x <= 511)
            return 168;
        else if (512 <= x && x <= 575)
            return 169;
        else if (576 <= x && x <= 639)
            return 170;
        else if (640 <= x && x <= 703)
            return 171;
        else if (704 <= x && x <= 767)
            return 172;
        else if (768 <= x && x <= 831)
            return 173;
        else if (832 <= x && x <= 895)
            return 174;
        else if (896 <= x && x <= 959)
            return 175;
        else if (960 <= x && x <= 1024)
            return 176;
    }
    else if (704<= y && y <= 768)
    {
        if (0 <= x && x <= 63)
            return 177;
        else if (64 <= x && x <= 127)
            return 178;
        else if (128 <= x && x <= 191)
            return 179;
        else if (192 <= x && x <= 255)
            return 180;
        else if (256 <= x && x <= 319)
            return 181;
        else if (320 <= x && x <= 383)
            return 182;
        else if (384 <= x && x <= 447)
            return 183;
        else if (448 <= x && x <= 511)
            return 184;
        else if (512 <= x && x <= 575)
            return 185;
        else if (576 <= x && x <= 639)
            return 186;
        else if (640 <= x && x <= 703)
            return 187;
        else if (704 <= x && x <= 767)
            return 188;
        else if (768 <= x && x <= 831)
            return 189;
        else if (832 <= x && x <= 895)
            return 190;
        else if (896 <= x && x <= 959)
            return 191;
        else if (960 <= x && x <= 1024)
            return 192;
    }
}


void CalibrationGraph_tiri::getDataPath(QString path)
{
    databPath = path;
}



void CalibrationGraph_tiri::makefitssample(std::string filename){

    QString date, time;
    date = QDateTime::currentDateTime().date().toString(Qt::ISODate);
    time = QDateTime::currentDateTime().time().toString(Qt::ISODate);


    filename=initialFileDirectory.toStdString()+"/"+filename;

    long naxis=2;
    int xsize=Width;
    int ysize=Height;
    long naxes[2] = {xsize,ysize};
    int nelements=xsize*ysize;
    vector<long> extAx;
    extAx.push_back(xsize) ;
    extAx.push_back(ysize) ;
    string names = "kokodousimasu";
    std::auto_ptr<FITS> pFits;
        try{
            pFits.reset(new FITS(filename,DOUBLE_IMG,naxis,naxes));
        }
        catch (FITS::CantCreate){
        return;
        }
    static std::valarray<double> konkong(nelements);
    static std::valarray<double> konkonh(nelements);
    for(int i=0;i<ysize;i++){ //248
        for(int j=0;j<xsize;j++){ //328
            konkong[i*xsize+j]=arrg[i][j].toDouble();
            konkonh[i*xsize+j]=arrh[i][j].toDouble();
        }
    }
    pFits->pHDU().addKey("MADE_BY","HEAT","june28");
    pFits->pHDU().write(1,nelements,konkong);
    ExtHDU* addh = pFits->addImage(names,DOUBLE_IMG,extAx);
    addh->write(1,nelements,konkonh);
}

void CalibrationGraph_tiri::makefitssamplerad(std::string filename){

    QString date, time;
    date = QDateTime::currentDateTime().date().toString(Qt::ISODate);
    time = QDateTime::currentDateTime().time().toString(Qt::ISODate);

    double tmp2=0;
    double FT1;
    double Radiance=0;
    //double epsilon=1;
    long naxis=2;
    int xsize=Width;
    int ysize=Height;
    long naxes[2] = {xsize,ysize};
    int nelements=xsize*ysize;
    vector<long> extAx;
    extAx.push_back(xsize);
    extAx.push_back(ysize);
    string names = "kokodousimasu";
    static std::valarray<double> konkong(nelements);
    static std::valarray<double> konkonh(nelements);
    // cout<<darkimage[y+6][x+16]<<endl;


    for(int i=0;i<ysize;i++){ //248
        for(int j=0;j<xsize;j++){ //328
            // double FT0=((DN[i][j]-arrh[i][j].toDouble())/(arrg[i][j].toDouble()));
            // FT1=round1_tiri(FT0);
            //if(i==300&&j==300) qDebug()<<"300,300 = "<<FT1;
            //if(i==400&&j==500) qDebug()<<"400,500 = "<<FT1;
            // tmp2 = gettemperature_tiri(FT1);

            // if(i==300&&j==300) qDebug()<<"tmp2,300,300 = "<<tmp2;
            // if(i==400&&j==500) qDebug()<<"tmp2,400,500 = "<<tmp2;
            // if(DN[i][j]<0) tmp2=150-273.15;
            // Radiance = ((tmp2+273.15)*(tmp2+273.15)*(tmp2+273.15)*(tmp2+273.15))/PI;
            //konkong[i*xsize+j]=Radiance;
            // konkonh[i*xsize+j]=;
            konkong[i*xsize+j]=konkonh[i*xsize+j]=arrh[i][j].toDouble();
;

        }
    }


    filename=initialFileDirectory.toStdString()+"/"+filename;

    std::auto_ptr<FITS> pFits;
        try{
            pFits.reset(new FITS(filename,DOUBLE_IMG,naxis,naxes));
        }
        catch (FITS::CantCreate){
        return;
        }

    pFits->pHDU().addKey("MADE_BY","HEAT","june28");
    pFits->pHDU().write(1,nelements,konkong);

    // filename=initialFileDirectory.toStdString()+"/konkontemp.fit";

    // std::auto_ptr<FITS> pFits2;
    //     try{
    //         pFits2.reset(new FITS(filename,DOUBLE_IMG,naxis,naxes));
    //     }
    //     catch (FITS::CantCreate){
    //     return;
    //     }

    // pFits2->pHDU().addKey("MADE_BY","HEAT","june28");
    // pFits2->pHDU().write(1,nelements,konkonh);
}

double CalibrationGraph_tiri::round1_tiri(double dIn) {
    double dOut;

    dOut = dIn * pow(10.0, 5);
    if (dIn >= 0) {
        dOut = (double)(int)(dOut + 0.5);
    } else
        dOut = (double)(int)(dOut - 0.5);
    return dOut * pow(10.0, -5);
}

void CalibrationGraph_tiri::saveFileInRegisterRegression(QString fileName, QString folder)
{

    QFile file(folder + "/" + fileName);

    if (file.exists())
    {
        file.open(QIODevice::WriteOnly);
        QTextStream out(&file);

        for (int k = 0; k < 248; k++)
        {
            for (int m = 0; m < 328; m++)
            {

                if (fileName == "g.csv")
                {
                    out << arrg[k][m] << ",";
                }
                else
                {
                    out << arrh[k][m] << ",";
                }
            }
            out << endl;
        }

        file.close();
    }
    else
    {
        QFile ini(fileName);

        ini.open(QIODevice::WriteOnly);

        QTextStream out(&ini);

        for (int k = 0; k < 248; k++)
        {
            for (int m = 0; m < 328; m++)
            {

                if (fileName == "g.csv")
                {
                    out << arrg[k][m] << ",";
                }
                else
                {
                    out << arrh[k][m] << ",";
                }
            }
            out << endl;
        }

        ini.close();

        ini.rename(fileName, folder + "/" + fileName);
    }
}


void CalibrationGraph_tiri::on_loadFileButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open Image"), QDir::homePath()+"/Desktop", tr("Image Files (*.fit *.fits *.fts *.inf)"));
    ImageFilefilepath = fileName;
    ui->fileNameBrowser->clear();
    ui->fileNameBrowser->setText(fileName.section('/', -1, -1));

    fstream ifs;
    ifs.open(&fileName.toStdString()[0], ios::in | ios::binary);
    QFileInfo fileinfo;
    fileinfo.setFile(fileName);
    QString ext = fileinfo.suffix();
    ext = ext.toLower();

    if (ext == "fit" || ext == "fits" || ext == "fts")
    {
        valarray<long> contents;
        auto_ptr<FITS> pInfile(0);
        try
        {
            pInfile.reset(new FITS(fileName.toStdString().c_str(), Read, true));
            PHDU &fitsImage = pInfile->pHDU();
            fitsImage.read(contents);
            fitsImage.readAllKeys();
            int counter=0;
            double DNtmp1=0;
            double tmp1=0;

            double fitsave=0;

            for(int i=0; i<Height; i++){
                for(int j=0; j<Width; j++){
                    fitsave+=DNtmp1;
                }
            }

            fitsave=fitsave/(Height*Width);

            for(int i=0; i<Height; i++){
                for(int j=0; j<Width; j++){
                    tmp1=contents[counter];

                    if(j==500&&i==400) qDebug()<<"("<<j<<","<<i<<")"<<"="<<tmp1;

                    if(fitsave<-700){
                        tmp1=tmp1/8;
                    }

                    //image[Height-1-i][j]=tmp1;
                    DN[i][j]=tmp1;

                    counter++;
                }
            }


            string fw_num_s;

            pInfile->pHDU().readKey("LS1_TEMP", len1_temp);
            pInfile->pHDU().readKey("LS2_TEMP", len2_temp);
            pInfile->pHDU().readKey("CAS_TEMP", cas_temp);
            pInfile->pHDU().readKey("FW1_TEMP", fw1_temp);
            pInfile->pHDU().readKey("FW2_TEMP", fw2_temp);
            pInfile->pHDU().readKey("FPA1TEMP", fpa1_temp);
            pInfile->pHDU().readKey("FPA2TEMP", fpa2_temp);
            pInfile->pHDU().readKey("HOD_TEMP", hod_temp);
            pInfile->pHDU().readKey("DC_TEMP", dc_temp);
            pInfile->pHDU().readKey("RD1_TEMP", rad1_temp);
            pInfile->pHDU().readKey("RD2_TEMP", rad2_temp);
            pInfile->pHDU().readKey("FW_NUM", fw_num_s);

            fw_num = QString::fromStdString(fw_num_s);
            if(fw_num=="CLOSE") fw_num="close";
            else {
                fw_num=fw_num.section(' ',1,1);
            }

            if(fw_num=="g") filter_num=7;
            else if(fw_num=="a") filter_num=1;
            else if(fw_num=="b") filter_num=2;
            else if(fw_num=="c") filter_num=3;
            else if(fw_num=="d") filter_num=4;
            else if(fw_num=="e") filter_num=5;
            else if(fw_num=="f") filter_num=6;

            fitsfilename = "File Name: " + QFileInfo(fileName).fileName();

            ui->fpa1MaxSlider->setValue(fpa1_temp * 1000 + 2000);
            ui->fpa1MinSlider->setValue(fpa1_temp * 1000 - 1000);

            ui->fpa2MaxSlider->setValue(fpa2_temp * 1000 + 2000);
            ui->fpa2MinSlider->setValue(fpa2_temp * 1000 - 1000);

            ui->len1MaxSlider->setValue(len1_temp * 1000 + 2000);
            ui->len1MinSlider->setValue(len1_temp * 1000 - 1000);

            ui->len2MaxSlider->setValue(len2_temp * 1000 + 2000);
            ui->len2MinSlider->setValue(len2_temp * 1000 - 1000);

            ui->fw1MaxSlider->setValue(fw1_temp * 1000 + 2000);
            ui->fw1MinSlider->setValue(fw1_temp * 1000 - 1000);

            ui->fw2MaxSlider->setValue(fw2_temp * 1000 + 2000);
            ui->fw2MinSlider->setValue(fw2_temp * 1000 - 1000);

            ui->dcMaxSlider->setValue(dc_temp * 1000 + 2000);
            ui->dcMinSlider->setValue(dc_temp * 1000 - 1000);

            ui->hodMaxSlider->setValue(hod_temp * 1000 + 2000);
            ui->hodMinSlider->setValue(hod_temp * 1000 - 1000);

            ui->rad1MaxSlider->setValue(rad1_temp * 1000 + 2000);
            ui->rad1MinSlider->setValue(rad1_temp * 1000 - 1000);

            ui->caseMaxSlider->setValue(cas_temp * 1000 + 2000);
            ui->caseMinSlider->setValue(cas_temp * 1000 - 1000);

            ui->rad2MaxSlider->setValue(rad2_temp * 1000 + 2000);
            ui->rad2MinSlider->setValue(rad2_temp * 1000 - 1000);






            ui->fpa1_tempBrowser->clear();
            ui->fpa1_tempBrowser->setText(QString::number(fpa1_temp));
            ui->cas_tempBrowser->clear();
            ui->cas_tempBrowser->setText(QString::number(cas_temp));
            ui->fpa2_tempBrowser->clear();
            ui->fpa2_tempBrowser->setText(QString::number(fpa2_temp));
            ui->len1_tempBrowser->clear();
            ui->len1_tempBrowser->setText(QString::number(len1_temp));
            ui->len2_tempBrowser->clear();
            ui->len2_tempBrowser->setText(QString::number(len2_temp));
            ui->fw1_tempBrowser->clear();
            ui->fw1_tempBrowser->setText(QString::number(fw1_temp));
            ui->fw2_tempBrowser->clear();
            ui->fw2_tempBrowser->setText(QString::number(fw2_temp));
            ui->rad1_tempBrowser->clear();
            ui->rad1_tempBrowser->setText(QString::number(rad1_temp));
            ui->rad2_tempBrowser->clear();
            ui->rad2_tempBrowser->setText(QString::number(rad2_temp));
            ui->dc_tempBrowser->clear();
            ui->dc_tempBrowser->setText(QString::number(dc_temp));
            ui->hod_tempBrowser->clear();
            ui->hod_tempBrowser->setText(QString::number(hod_temp));



        }
        catch (...)
        {
            qDebug()<<"error";
        };
    }







    if (ext == "inf")
    {
        double imagetmp[50][2];
        QFile file1(fileinfo.filePath());
        if (!file1.open(QIODevice::ReadOnly))
        {
            printf("txt dat open error\n");
            return;
        }
        QTextStream in1(&file1);
        QString str1;
        for (int i = 0; !in1.atEnd(); i++)
        {
            for (int j = 0; j < 2; j++)
            {
                in1 >> str1;
                imagetmp[i][j] = str1.toDouble();
            }
        }

        bol_temp = imagetmp[38][1];
        pkg_temp = imagetmp[42][1];
        cas_temp = imagetmp[43][1];
        sht_temp = imagetmp[44][1];
        len_temp = imagetmp[45][1];

        ui->pkgMaxSlider->setValue(pkg_temp * 1000 + 1000);
        ui->pkgMinSlider->setValue(pkg_temp * 1000 - 1000);

        ui->caseMaxSlider->setValue(cas_temp * 1000 + 1000);
        ui->caseMinSlider->setValue(cas_temp * 1000 - 1000);

        ui->shMaxSlider->setValue(sht_temp * 1000 + 1000);
        ui->shMinSlider->setValue(sht_temp * 1000 - 1000);

        ui->lensMaxSlider->setValue(len_temp * 1000 + 1000);
        ui->lensMinSlider->setValue(len_temp * 1000 - 1000);

        ui->pkg_tempBrowser->clear();
        ui->pkg_tempBrowser->setText(QString::number(pkg_temp));

        ui->cas_tempBrowser->clear();
        ui->cas_tempBrowser->setText(QString::number(cas_temp));
        ui->sht_tempBrowser->clear();
        ui->sht_tempBrowser->setText(QString::number(sht_temp));
        ui->len_tempBrowser->clear();
        ui->len_tempBrowser->setText(QString::number(len_temp));
    }








    double bolo_max, bolo_min, pkg_max, pkg_min;
    double sh_max, sh_min, lens_max, lens_min;


    double tgt_max, tgt_min, fpa1_max, fpa1_min, fpa2_max, fpa2_min;
    double case_max, case_min, len1_max, len1_min, len2_max, len2_min;
    double fw1_max, fw1_min, fw2_max, fw2_min, dc_max, dc_min;
    double hod_max, hod_min, rad1_max, rad1_min, rad2_max, rad2_min;
    int count = 0;

    vx.clear();
    vy.clear();

    tgt_max = ui->tgtMaxLineEdit->text().toDouble();
    tgt_min = ui->tgtMinLineEdit->text().toDouble();

    fpa1_max = ui->fpa1MaxLineEdit->text().toDouble();
    fpa1_min = ui->fpa1MinLineEdit->text().toDouble();

    fpa2_max = ui->fpa2MaxLineEdit->text().toDouble();
    fpa2_min = ui->fpa2MinLineEdit->text().toDouble();

    case_max = ui->caseMaxLineEdit->text().toDouble();
    case_min = ui->caseMinLineEdit->text().toDouble();

    len1_max = ui->len1MaxLineEdit->text().toDouble();
    len1_min = ui->len1MinLineEdit->text().toDouble();

    len2_max = ui->len2MaxLineEdit->text().toDouble();
    len2_min = ui->len2MinLineEdit->text().toDouble();

    fw1_max = ui->fw1MaxLineEdit->text().toDouble();
    fw1_min = ui->fw1MinLineEdit->text().toDouble();

    fw2_max = ui->fw2MaxLineEdit->text().toDouble();
    fw2_min = ui->fw2MinLineEdit->text().toDouble();

    dc_max = ui->dcMaxLineEdit->text().toDouble();
    dc_min = ui->dcMinLineEdit->text().toDouble();

    hod_max = ui->hodMaxLineEdit->text().toDouble();
    hod_min = ui->hodMinLineEdit->text().toDouble();

    rad1_max = ui->rad1MaxLineEdit->text().toDouble();
    rad1_min = ui->rad1MinLineEdit->text().toDouble();

    rad2_max = ui->rad2MaxLineEdit->text().toDouble();
    rad2_min = ui->rad2MinLineEdit->text().toDouble();



    QVector<QString> tmp;
    replot.clear();

    int f = judgeAxis(xAxis, yAxis);

    if ((f == 1 || f == 2) && (xAxis == "diff DN" || yAxis == "diff DN"))
    {
        count = 0;
        for (int i = 0; i < infoNum; i++)
        {
            if (tgt_min <= diff[i][5].toDouble() && diff[i][5].toDouble() <= tgt_max)
            {
                if (len1_min <= diff[i][7].toDouble() && diff[i][7].toDouble() <= len1_max)
                {
                    if (len2_min <= diff[i][8].toDouble() && diff[i][8].toDouble() <= len2_max)
                    {
                        if (case_min <= diff[i][11].toDouble() && diff[i][11].toDouble() <= case_max)
                        {
                            if (fw1_min <= diff[i][9].toDouble() && diff[i][9].toDouble() <= fw1_max)
                            {
                                if (fw2_min <= diff[i][10].toDouble() && diff[i][10].toDouble() <= fw2_max)
                                {
                                    if (fpa1_min <= diff[i][12].toDouble() && diff[i][12].toDouble() <= fpa1_max)
                                    {
                                        if (fpa2_min <= diff[i][13].toDouble() && diff[i][13].toDouble() <= fpa2_max)
                                        {
                                            if (dc_min <= diff[i][17].toDouble() && diff[i][17].toDouble() <= dc_max)
                                            {
                                                if (hod_min <= diff[i][14].toDouble() && diff[i][14].toDouble() <= hod_max)
                                                {
                                                    if (rad1_min <= diff[i][15].toDouble() && diff[i][15].toDouble() <= rad1_max)
                                                    {
                                                        if (rad2_min <= diff[i][16].toDouble() && diff[i][16].toDouble() <= rad2_max)
                                                        {
                                                            if(diff[i][18]==fw_num){
                                                                 if(!(diff[i][3].toDouble()>(diff[i][22].toDouble()*0.9)/16&&diff[i][5].toDouble()<20)){
                                                                     if(!(diff[i][3].toDouble()<(diff[i][22].toDouble()*0.9)/16&&diff[i][5].toDouble()>20)){
                                                                        if(diff[i][20]=="2.8deg"){
                                                                            tmp.clear();
                                                                            for (int j = 0; j < 23; j++)
                                                                            {
                                                                                tmp.append(diff[i][j]);
                                                                            }
                                                                            replot.append(tmp);
                                                                            count++;
                                                                        }
                                                                     }
                                                                 }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        count = 0;
        for (int i = 0; i < previousNum; i++)
        {
            if (tgt_min <= pixelList[i][9].toDouble() && pixelList[i][9].toDouble() <= tgt_max)
            {
                if (bolo_min <= pixelList[i][4].toDouble() && pixelList[i][4].toDouble() <= bolo_max)
                {
                    if (pkg_min <= pixelList[i][5].toDouble() && pixelList[i][5].toDouble() <= pkg_max)
                    {
                        if (case_min <= pixelList[i][6].toDouble() && pixelList[i][6].toDouble() <= case_max)
                        {
                            if (sh_min <= pixelList[i][7].toDouble() && pixelList[i][7].toDouble() <= sh_max)
                            {
                                if (lens_min <= pixelList[i][8].toDouble() && pixelList[i][8].toDouble() <= lens_max)
                                {
                                    tmp.clear();
                                    for (int j = 0; j < 25; j++)
                                    {
                                        tmp.append(pixelList[i][j]);
                                    }
                                    replot.append(tmp);
                                    count++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (f == -1)
    {
        vx = getAxisValue(xAxis, replot, count);
        vy = getAxisValue(yAxis, replot, count);
    }
    else if (f == 0)
    {
        vx = getAxisValue2(xAxis, replot, count);
        vy = getAxisValue2(yAxis, replot, count);
    }
    else if (f == 1)
    {
        if (xAxis == "diff DN")
        {
            vx = getAxisValue(xAxis, replot, count);
            vy = getAxisValue(yAxis, replot, count);
        }
        else
        {
            vx = getAxisValue2(xAxis, replot, count);
            vy = getAxisValue(yAxis, pixelList2, infoNum);
        }
    }
    else if (f == 2)
    {
        if (yAxis == "diff DN")
        {
            vy = getAxisValue(yAxis, replot, count);
            vx = getAxisValue(xAxis, replot, count);
        }
        else
        {
            vy = getAxisValue2(yAxis, replot, count);
            vx = getAxisValue(xAxis, pixelList2, infoNum);
        }
    }

    ui->plotNumberBrowser->clear();
    if (f == -1)
    {
        ui->plotNumberBrowser->setText(QString::number(count) + " / " + QString::number(previousNum));
    }
    else if (xAxis == "diff DN" || yAxis == "diff DN")
    {
        ui->plotNumberBrowser->setText(QString::number(count) + " / " + QString::number(plotNum));
    }
    else
    {
        ui->plotNumberBrowser->setText(QString::number(infoNum) + " / " + QString::number(plotNum));
    }
    Outputplotnumber = count;

    drawGraph(vx, vy, "");
}

double CalibrationGraph_tiri::planck(double T)
{
    double lambda, Bt, integral = 0, epsilon = 0.925;

    for (int i = 1; i < 1301; i++)
    {
        lambda = (double)i * 1e-8;
        Bt = ((2 * h_planck * c_speed * c_speed) / (pow(lambda, 5)) / (pow(M_E, c2 / (lambda * T)) - 1.0) * tirifilter[i-1][filter_num] * epsilon);
        integral += (Bt);
    }
    integral *= 1e-8;
    return integral;
}


double CalibrationGraph_tiri::planck(double T,int filter_num)
{
    double lambda, Bt, integral = 0, epsilon = 0.925;

    for (int i = 1; i < 1301; i++)
    {
        lambda = (double)i * 1e-8;
        Bt = ((2 * h_planck * c_speed * c_speed) / (pow(lambda, 5)) / (pow(M_E, c2 / (lambda * T)) - 1.0) * tirifilter[i-1][filter_num] * epsilon);
        integral += (Bt);
    }
    integral *= 1e-8;
    return integral;
}


double planck4_tiri(double T)
{
    /*
    double lambda, Bt, integral = 0, epsilon = 0.925;
    double h_planck = 6.62606957e-34;
    double kB = 1.3806488e-23;
    double c_speed = 299792458;
    double c1 = 2 * M_PI * h_planck * pow(c_speed, 2);
    double c2 = (h_planck * c_speed) / kB;
    double SIGMA = 5.670373e-8;
    double c3=(2 * h_planck * c_speed * c_speed);
    */
    double lambda, Bt, Bt1,integral = 0, epsilon = 0.925;
    double h_planck = 6.62606957e-34;
    double kB = 1.3806488e-23;
    double c_speed = 299792458;
    double c1 = 2 * M_PI * h_planck * pow(c_speed, 2);
    double c2 = (h_planck * c_speed) / kB;
    double SIGMA = 5.670373e-8;
    int count=0;
    int i;

    // while(1)
    // {
    //     if(filter_num==7){
    //         i=700+count;
    //         if(i==1401) break;
    //     }
    //     else if(filter_num==1){
    //         i=760+count;
    //         if(i==801) break;
    //     }
    //     else if(filter_num==2){
    //         i=840+count;
    //         if(i==881) break;
    //     }
    //     else if(filter_num==3){
    //         i=940+count;
    //         if(i==981) break;
    //     }
    //     else if(filter_num==4){
    //         i=1040+count;
    //         if(i==1081) break;
    //     }
    //     else if(filter_num==5){
    //         i=1140+count;
    //         if(i==1181) break;
    //     }
    //     else if(filter_num==6){
    //         i=1280+count;
    //         if(i==1321) break;
    //     }
    //     lambda = (double)i * 1e-8;
    //     Bt = ((2 * h_planck * c_speed * c_speed) / (pow(lambda, 5)) / (pow(M_E, c2 / (lambda * T)) - 1.0) * tirifilter[i-301][filter_num] * epsilon);
    //     integral += (Bt);
    //     count++;
    // }




    if(filter_num==7){
        i=700;
        while(1){
        lambda = (double)i * 1e-8;
        Bt = ((2 * h_planck * c_speed * c_speed) / (pow(lambda, 5)) / (pow(M_E, c2 / (lambda * T)) - 1.0) * tirifilter[i-301][filter_num] * epsilon);
        Bt1=Bt;
        if(i==0) continue;
        Bt=
        integral += (Bt);
        i++;
        if(i==1401) break;
        }
    }
    else if(filter_num==1){
        i=760;
        while(1){
        lambda = (double)i * 1e-8;
        Bt = ((2 * h_planck * c_speed * c_speed) / (pow(lambda, 5)) / (pow(M_E, c2 / (lambda * T)) - 1.0) * tirifilter[i-301][filter_num] * epsilon);
        integral += (Bt);
        i++;
        if(i==801) break;
        }
    }
    else if(filter_num==2){
        i=840;
        while(1){
        lambda = (double)i * 1e-8;
        Bt = ((2 * h_planck * c_speed * c_speed) / (pow(lambda, 5)) / (pow(M_E, c2 / (lambda * T)) - 1.0) * tirifilter[i-301][filter_num] * epsilon);
        integral += (Bt);
        i++;
        if(i==881) break;
        }
    }
    else if(filter_num==3){
        i=940;
        while(1){
        lambda = (double)i * 1e-8;
        Bt = ((2 * h_planck * c_speed * c_speed) / (pow(lambda, 5)) / (pow(M_E, c2 / (lambda * T)) - 1.0) * tirifilter[i-301][filter_num] * epsilon);
        integral += (Bt);
        i++;

        if(i==981) break;
        }
    }
    else if(filter_num==4){
        i=1040;
        while(1){
        lambda = (double)i * 1e-8;
        Bt = ((2 * h_planck * c_speed * c_speed) / (pow(lambda, 5)) / (pow(M_E, c2 / (lambda * T)) - 1.0) * tirifilter[i-301][filter_num] * epsilon);
        integral += (Bt);
        i++;
        if(i==1081) break;
        }
    }
    else if(filter_num==5){
        i=1140;
        while(1){
        lambda = (double)i * 1e-8;
        Bt = ((2 * h_planck * c_speed * c_speed) / (pow(lambda, 5)) / (pow(M_E, c2 / (lambda * T)) - 1.0) * tirifilter[i-301][filter_num] * epsilon);
        integral += (Bt);
        i++;
        if(i==1181) break;
        }
    }
    else if(filter_num==6){
        i=1280;
        while(1){
        lambda = (double)i * 1e-8;
        Bt = ((2 * h_planck * c_speed * c_speed) / (pow(lambda, 5)) / (pow(M_E, c2 / (lambda * T)) - 1.0) * tirifilter[i-301][filter_num] * epsilon);
        integral += (Bt);
        i++;
        if(i==1321) break;
        }
    }

    integral *= 1e-8;
    return integral;




    // if(filter_num==7){
    //     i=700;
    // }
    // else if(filter_num==1){
    //     i=760;
    // }
    // else if(filter_num==2){
    //     i=840;
    // }
    // else if(filter_num==3){
    //     i=940;

    // }
    // else if(filter_num==4){
    //     i=1040;
    // }
    // else if(filter_num==5){
    //     i=1140;
    // }
    // else if(filter_num==6){
    //     i=1280;
    // }

    // while(1)
    // {
    //     if(filter_num==7 && i==1401) break;

    //     else if(filter_num==1 && i==801) break;

    //     else if(filter_num==2 && i==881) break;

    //     else if(filter_num==3 && i==981) break;

    //     else if(filter_num==4 && i==1081) break;

    //     else if(filter_num==5 && i==1181) break;

    //     else if(filter_num==6 && i==1321) break;
    //     lambda = (double)i * 1e-8;
    //     Bt = ((2 * h_planck * c_speed * c_speed) / (pow(lambda, 5)) / (pow(M_E, c2 / (lambda * T)) - 1.0) * tirifilter[i-301][filter_num] * epsilon);
    //     integral += (Bt);
    //     i++;
    // }




    /*
    for (int i = 1; i < 1301; i++)
    {
        lambda = (double)i * 1e-8;
        Bt = ((2 * h_planck * c_speed * c_speed) / (pow(lambda, 5)) / (pow(M_E, c2 / (lambda * T)) - 1.0) * tirifilter[i-1][filter_num] * epsilon);
        integral += (Bt);
    }
    integral *= 1e-8;
    return integral;
    */
}

void CalibrationGraph_tiri::loadFilter()
{

    QString str;
    QString appPath;
    appPath = QCoreApplication::applicationDirPath();

    //qDebug()<<appPath;
    QFile file(appPath + "/tiri_response.txt");

    if (!file.open(QIODevice::ReadOnly))
    {
        printf("tiri_response.txt open error\n");
        return;
    }

    QTextStream in(&file);

    for (int i = 0; !in.atEnd(); i++)
    {
        for (int j = 0; j < 8; j++)
        {
            in >> str;
            tirifilter[i][j] = str.toDouble();
        }
    }
}

void CalibrationGraph_tiri::checkAction(){
    RorT=true;
}


double CalibrationGraph_tiri::gettemperature_tiri(double FT1){

    double tmp1=0;

    if(FT1<=0)tmp1=-123.15;
    else if(FT1<=0.03578){
        if(FT1==0.03304)tmp1=-150;
        else if(FT1<=0.03333)tmp1=-149.9;
        else if(FT1<=0.03363)tmp1=-149.8;
        else if(FT1<=0.03393)tmp1=-149.7;
        else if(FT1<=0.03423)tmp1=-149.6;
        else if(FT1<=0.03454)tmp1=-149.5;
        else if(FT1<=0.03485)tmp1=-149.4;
        else if(FT1<=0.03515)tmp1=-149.3;
        else if(FT1<=0.03547)tmp1=-149.2;
        else if(FT1<=0.03578)tmp1=-149.1;
        else if(FT1<=0.03609)tmp1=-149;
    }
    else if(FT1<=0.03904){
        if(FT1==0.03609)tmp1=-149;
        else if(FT1<=0.03641)tmp1=-148.9;
        else if(FT1<=0.03673)tmp1=-148.8;
        else if(FT1<=0.03706)tmp1=-148.7;
        else if(FT1<=0.03738)tmp1=-148.6;
        else if(FT1<=0.03771)tmp1=-148.5;
        else if(FT1<=0.03804)tmp1=-148.4;
        else if(FT1<=0.03837)tmp1=-148.3;
        else if(FT1<=0.0387)tmp1=-148.2;
        else if(FT1<=0.03904)tmp1=-148.1;
        else if(FT1<=0.03938)tmp1=-148;
    }
    else if(FT1<=0.04255){
        if(FT1==0.03938)tmp1=-148;
        else if(FT1<=0.03972)tmp1=-147.9;
        else if(FT1<=0.04007)tmp1=-147.8;
        else if(FT1<=0.04041)tmp1=-147.7;
        else if(FT1<=0.04076)tmp1=-147.6;
        else if(FT1<=0.04111)tmp1=-147.5;
        else if(FT1<=0.04147)tmp1=-147.4;
        else if(FT1<=0.04182)tmp1=-147.3;
        else if(FT1<=0.04218)tmp1=-147.2;
        else if(FT1<=0.04255)tmp1=-147.1;
        else if(FT1<=0.04291)tmp1=-147;
    }
    else if(FT1<=0.0463){
        if(FT1==0.04291)tmp1=-147;
        else if(FT1<=0.04328)tmp1=-146.9;
        else if(FT1<=0.04365)tmp1=-146.8;
        else if(FT1<=0.04402)tmp1=-146.7;
        else if(FT1<=0.04439)tmp1=-146.6;
        else if(FT1<=0.04477)tmp1=-146.5;
        else if(FT1<=0.04515)tmp1=-146.4;
        else if(FT1<=0.04553)tmp1=-146.3;
        else if(FT1<=0.04592)tmp1=-146.2;
        else if(FT1<=0.0463)tmp1=-146.1;
        else if(FT1<=0.0467)tmp1=-146;
    }
    else if(FT1<=0.05033){
        if(FT1==0.0467)tmp1=-146;
        else if(FT1<=0.04709)tmp1=-145.9;
        else if(FT1<=0.04748)tmp1=-145.8;
        else if(FT1<=0.04788)tmp1=-145.7;
        else if(FT1<=0.04828)tmp1=-145.6;
        else if(FT1<=0.04869)tmp1=-145.5;
        else if(FT1<=0.0491)tmp1=-145.4;
        else if(FT1<=0.04951)tmp1=-145.3;
        else if(FT1<=0.04992)tmp1=-145.2;
        else if(FT1<=0.05033)tmp1=-145.1;
        else if(FT1<=0.05075)tmp1=-145;
    }
    else if(FT1<=0.05464){
        if(FT1==0.05075)tmp1=-145;
        else if(FT1<=0.05117)tmp1=-144.9;
        else if(FT1<=0.0516)tmp1=-144.8;
        else if(FT1<=0.05202)tmp1=-144.7;
        else if(FT1<=0.05245)tmp1=-144.6;
        else if(FT1<=0.05289)tmp1=-144.5;
        else if(FT1<=0.05332)tmp1=-144.4;
        else if(FT1<=0.05376)tmp1=-144.3;
        else if(FT1<=0.0542)tmp1=-144.2;
        else if(FT1<=0.05464)tmp1=-144.1;
        else if(FT1<=0.05509)tmp1=-144;
    }
    else if(FT1<=0.05925){
        if(FT1==0.05509)tmp1=-144;
        else if(FT1<=0.05554)tmp1=-143.9;
        else if(FT1<=0.056)tmp1=-143.8;
        else if(FT1<=0.05645)tmp1=-143.7;
        else if(FT1<=0.05691)tmp1=-143.6;
        else if(FT1<=0.05737)tmp1=-143.5;
        else if(FT1<=0.05784)tmp1=-143.4;
        else if(FT1<=0.05831)tmp1=-143.3;
        else if(FT1<=0.05878)tmp1=-143.2;
        else if(FT1<=0.05925)tmp1=-143.1;
        else if(FT1<=0.05973)tmp1=-143;
    }
    else if(FT1<=0.06418){
        if(FT1==0.05973)tmp1=-143;
        else if(FT1<=0.06021)tmp1=-142.9;
        else if(FT1<=0.0607)tmp1=-142.8;
        else if(FT1<=0.06119)tmp1=-142.7;
        else if(FT1<=0.06168)tmp1=-142.6;
        else if(FT1<=0.06217)tmp1=-142.5;
        else if(FT1<=0.06267)tmp1=-142.4;
        else if(FT1<=0.06317)tmp1=-142.3;
        else if(FT1<=0.06367)tmp1=-142.2;
        else if(FT1<=0.06418)tmp1=-142.1;
        else if(FT1<=0.06469)tmp1=-142;
    }
    else if(FT1<=0.06943){
        if(FT1==0.06469)tmp1=-142;
        else if(FT1<=0.0652)tmp1=-141.9;
        else if(FT1<=0.06572)tmp1=-141.8;
        else if(FT1<=0.06624)tmp1=-141.7;
        else if(FT1<=0.06676)tmp1=-141.6;
        else if(FT1<=0.06729)tmp1=-141.5;
        else if(FT1<=0.06782)tmp1=-141.4;
        else if(FT1<=0.06835)tmp1=-141.3;
        else if(FT1<=0.06889)tmp1=-141.2;
        else if(FT1<=0.06943)tmp1=-141.1;
        else if(FT1<=0.06998)tmp1=-141;
    }
    else if(FT1<=0.07503){
        if(FT1==0.06998)tmp1=-141;
        else if(FT1<=0.07052)tmp1=-140.9;
        else if(FT1<=0.07107)tmp1=-140.8;
        else if(FT1<=0.07163)tmp1=-140.7;
        else if(FT1<=0.07219)tmp1=-140.6;
        else if(FT1<=0.07275)tmp1=-140.5;
        else if(FT1<=0.07331)tmp1=-140.4;
        else if(FT1<=0.07388)tmp1=-140.3;
        else if(FT1<=0.07445)tmp1=-140.2;
        else if(FT1<=0.07503)tmp1=-140.1;
        else if(FT1<=0.07561)tmp1=-140;
    }
    else if(FT1<=0.08099){
        if(FT1==0.07561)tmp1=-140;
        else if(FT1<=0.07619)tmp1=-139.9;
        else if(FT1<=0.07678)tmp1=-139.8;
        else if(FT1<=0.07737)tmp1=-139.7;
        else if(FT1<=0.07797)tmp1=-139.6;
        else if(FT1<=0.07856)tmp1=-139.5;
        else if(FT1<=0.07916)tmp1=-139.4;
        else if(FT1<=0.07977)tmp1=-139.3;
        else if(FT1<=0.08038)tmp1=-139.2;
        else if(FT1<=0.08099)tmp1=-139.1;
        else if(FT1<=0.08161)tmp1=-139;
    }
    else if(FT1<=0.08733){
        if(FT1==0.08161)tmp1=-139;
        else if(FT1<=0.08223)tmp1=-138.9;
        else if(FT1<=0.08285)tmp1=-138.8;
        else if(FT1<=0.08348)tmp1=-138.7;
        else if(FT1<=0.08411)tmp1=-138.6;
        else if(FT1<=0.08475)tmp1=-138.5;
        else if(FT1<=0.08539)tmp1=-138.4;
        else if(FT1<=0.08603)tmp1=-138.3;
        else if(FT1<=0.08668)tmp1=-138.2;
        else if(FT1<=0.08733)tmp1=-138.1;
        else if(FT1<=0.08799)tmp1=-138;
    }
    else if(FT1<=0.09407){
        if(FT1==0.08799)tmp1=-138;
        else if(FT1<=0.08865)tmp1=-137.9;
        else if(FT1<=0.08931)tmp1=-137.8;
        else if(FT1<=0.08998)tmp1=-137.7;
        else if(FT1<=0.09065)tmp1=-137.6;
        else if(FT1<=0.09133)tmp1=-137.5;
        else if(FT1<=0.09201)tmp1=-137.4;
        else if(FT1<=0.09269)tmp1=-137.3;
        else if(FT1<=0.09338)tmp1=-137.2;
        else if(FT1<=0.09407)tmp1=-137.1;
        else if(FT1<=0.09477)tmp1=-137;
    }
    else if(FT1<=0.10123){
        if(FT1==0.09477)tmp1=-137;
        else if(FT1<=0.09547)tmp1=-136.9;
        else if(FT1<=0.09618)tmp1=-136.8;
        else if(FT1<=0.09688)tmp1=-136.7;
        else if(FT1<=0.0976)tmp1=-136.6;
        else if(FT1<=0.09832)tmp1=-136.5;
        else if(FT1<=0.09904)tmp1=-136.4;
        else if(FT1<=0.09976)tmp1=-136.3;
        else if(FT1<=0.10049)tmp1=-136.2;
        else if(FT1<=0.10123)tmp1=-136.1;
        else if(FT1<=0.10197)tmp1=-136;
    }
    else if(FT1<=0.10882){
        if(FT1==0.10197)tmp1=-136;
        else if(FT1<=0.10271)tmp1=-135.9;
        else if(FT1<=0.10346)tmp1=-135.8;
        else if(FT1<=0.10421)tmp1=-135.7;
        else if(FT1<=0.10497)tmp1=-135.6;
        else if(FT1<=0.10573)tmp1=-135.5;
        else if(FT1<=0.10649)tmp1=-135.4;
        else if(FT1<=0.10726)tmp1=-135.3;
        else if(FT1<=0.10804)tmp1=-135.2;
        else if(FT1<=0.10882)tmp1=-135.1;
        else if(FT1<=0.1096)tmp1=-135;
    }
    else if(FT1<=0.11686){
        if(FT1==0.1096)tmp1=-135;
        else if(FT1<=0.11039)tmp1=-134.9;
        else if(FT1<=0.11118)tmp1=-134.8;
        else if(FT1<=0.11198)tmp1=-134.7;
        else if(FT1<=0.11278)tmp1=-134.6;
        else if(FT1<=0.11359)tmp1=-134.5;
        else if(FT1<=0.1144)tmp1=-134.4;
        else if(FT1<=0.11521)tmp1=-134.3;
        else if(FT1<=0.11604)tmp1=-134.2;
        else if(FT1<=0.11686)tmp1=-134.1;
        else if(FT1<=0.11769)tmp1=-134;
    }
    else if(FT1<=0.12538){
        if(FT1==0.11769)tmp1=-134;
        else if(FT1<=0.11853)tmp1=-133.9;
        else if(FT1<=0.11937)tmp1=-133.8;
        else if(FT1<=0.12021)tmp1=-133.7;
        else if(FT1<=0.12106)tmp1=-133.6;
        else if(FT1<=0.12191)tmp1=-133.5;
        else if(FT1<=0.12277)tmp1=-133.4;
        else if(FT1<=0.12364)tmp1=-133.3;
        else if(FT1<=0.1245)tmp1=-133.2;
        else if(FT1<=0.12538)tmp1=-133.1;
        else if(FT1<=0.12626)tmp1=-133;
    }
    else if(FT1<=0.13439){
        if(FT1==0.12626)tmp1=-133;
        else if(FT1<=0.12714)tmp1=-132.9;
        else if(FT1<=0.12803)tmp1=-132.8;
        else if(FT1<=0.12892)tmp1=-132.7;
        else if(FT1<=0.12982)tmp1=-132.6;
        else if(FT1<=0.13072)tmp1=-132.5;
        else if(FT1<=0.13163)tmp1=-132.4;
        else if(FT1<=0.13254)tmp1=-132.3;
        else if(FT1<=0.13346)tmp1=-132.2;
        else if(FT1<=0.13439)tmp1=-132.1;
        else if(FT1<=0.13532)tmp1=-132;
    }
    else if(FT1<=0.14391){
        if(FT1==0.13532)tmp1=-132;
        else if(FT1<=0.13625)tmp1=-131.9;
        else if(FT1<=0.13719)tmp1=-131.8;
        else if(FT1<=0.13813)tmp1=-131.7;
        else if(FT1<=0.13908)tmp1=-131.6;
        else if(FT1<=0.14004)tmp1=-131.5;
        else if(FT1<=0.141)tmp1=-131.4;
        else if(FT1<=0.14196)tmp1=-131.3;
        else if(FT1<=0.14294)tmp1=-131.2;
        else if(FT1<=0.14391)tmp1=-131.1;
        else if(FT1<=0.14489)tmp1=-131;
    }
    else if(FT1<=0.15397){
        if(FT1==0.14489)tmp1=-131;
        else if(FT1<=0.14588)tmp1=-130.9;
        else if(FT1<=0.14687)tmp1=-130.8;
        else if(FT1<=0.14787)tmp1=-130.7;
        else if(FT1<=0.14887)tmp1=-130.6;
        else if(FT1<=0.14988)tmp1=-130.5;
        else if(FT1<=0.1509)tmp1=-130.4;
        else if(FT1<=0.15191)tmp1=-130.3;
        else if(FT1<=0.15294)tmp1=-130.2;
        else if(FT1<=0.15397)tmp1=-130.1;
        else if(FT1<=0.15501)tmp1=-130;
    }
    else if(FT1<=0.16459){
        if(FT1==0.15501)tmp1=-130;
        else if(FT1<=0.15605)tmp1=-129.9;
        else if(FT1<=0.1571)tmp1=-129.8;
        else if(FT1<=0.15815)tmp1=-129.7;
        else if(FT1<=0.15921)tmp1=-129.6;
        else if(FT1<=0.16027)tmp1=-129.5;
        else if(FT1<=0.16134)tmp1=-129.4;
        else if(FT1<=0.16242)tmp1=-129.3;
        else if(FT1<=0.1635)tmp1=-129.2;
        else if(FT1<=0.16459)tmp1=-129.1;
        else if(FT1<=0.16568)tmp1=-129;
    }
    else if(FT1<=0.17578){
        if(FT1==0.16568)tmp1=-129;
        else if(FT1<=0.16678)tmp1=-128.9;
        else if(FT1<=0.16788)tmp1=-128.8;
        else if(FT1<=0.16899)tmp1=-128.7;
        else if(FT1<=0.17011)tmp1=-128.6;
        else if(FT1<=0.17123)tmp1=-128.5;
        else if(FT1<=0.17236)tmp1=-128.4;
        else if(FT1<=0.17349)tmp1=-128.3;
        else if(FT1<=0.17463)tmp1=-128.2;
        else if(FT1<=0.17578)tmp1=-128.1;
        else if(FT1<=0.17693)tmp1=-128;
    }
    else if(FT1<=0.18757){
        if(FT1==0.17693)tmp1=-128;
        else if(FT1<=0.17809)tmp1=-127.9;
        else if(FT1<=0.17925)tmp1=-127.8;
        else if(FT1<=0.18042)tmp1=-127.7;
        else if(FT1<=0.1816)tmp1=-127.6;
        else if(FT1<=0.18278)tmp1=-127.5;
        else if(FT1<=0.18397)tmp1=-127.4;
        else if(FT1<=0.18517)tmp1=-127.3;
        else if(FT1<=0.18637)tmp1=-127.2;
        else if(FT1<=0.18757)tmp1=-127.1;
        else if(FT1<=0.18879)tmp1=-127;
    }
    else if(FT1<=0.19999){
        if(FT1==0.18879)tmp1=-127;
        else if(FT1<=0.19001)tmp1=-126.9;
        else if(FT1<=0.19123)tmp1=-126.8;
        else if(FT1<=0.19247)tmp1=-126.7;
        else if(FT1<=0.1937)tmp1=-126.6;
        else if(FT1<=0.19495)tmp1=-126.5;
        else if(FT1<=0.1962)tmp1=-126.4;
        else if(FT1<=0.19746)tmp1=-126.3;
        else if(FT1<=0.19872)tmp1=-126.2;
        else if(FT1<=0.19999)tmp1=-126.1;
        else if(FT1<=0.20127)tmp1=-126;
    }
    else if(FT1<=0.21306){
        if(FT1==0.20127)tmp1=-126;
        else if(FT1<=0.20255)tmp1=-125.9;
        else if(FT1<=0.20384)tmp1=-125.8;
        else if(FT1<=0.20514)tmp1=-125.7;
        else if(FT1<=0.20644)tmp1=-125.6;
        else if(FT1<=0.20775)tmp1=-125.5;
        else if(FT1<=0.20907)tmp1=-125.4;
        else if(FT1<=0.21039)tmp1=-125.3;
        else if(FT1<=0.21172)tmp1=-125.2;
        else if(FT1<=0.21306)tmp1=-125.1;
        else if(FT1<=0.2144)tmp1=-125;
    }
    else if(FT1<=0.22679){
        if(FT1==0.2144)tmp1=-125;
        else if(FT1<=0.21575)tmp1=-124.9;
        else if(FT1<=0.21711)tmp1=-124.8;
        else if(FT1<=0.21847)tmp1=-124.7;
        else if(FT1<=0.21984)tmp1=-124.6;
        else if(FT1<=0.22122)tmp1=-124.5;
        else if(FT1<=0.2226)tmp1=-124.4;
        else if(FT1<=0.22399)tmp1=-124.3;
        else if(FT1<=0.22539)tmp1=-124.2;
        else if(FT1<=0.22679)tmp1=-124.1;
        else if(FT1<=0.22821)tmp1=-124;
    }
    else if(FT1<=0.24123){
        if(FT1==0.22821)tmp1=-124;
        else if(FT1<=0.22962)tmp1=-123.9;
        else if(FT1<=0.23105)tmp1=-123.8;
        else if(FT1<=0.23248)tmp1=-123.7;
        else if(FT1<=0.23392)tmp1=-123.6;
        else if(FT1<=0.23537)tmp1=-123.5;
        else if(FT1<=0.23682)tmp1=-123.4;
        else if(FT1<=0.23828)tmp1=-123.3;
        else if(FT1<=0.23975)tmp1=-123.2;
        else if(FT1<=0.24123)tmp1=-123.1;
        else if(FT1<=0.24271)tmp1=-123;
    }
    else if(FT1<=0.25638){
        if(FT1==0.24271)tmp1=-123;
        else if(FT1<=0.2442)tmp1=-122.9;
        else if(FT1<=0.24569)tmp1=-122.8;
        else if(FT1<=0.2472)tmp1=-122.7;
        else if(FT1<=0.24871)tmp1=-122.6;
        else if(FT1<=0.25023)tmp1=-122.5;
        else if(FT1<=0.25175)tmp1=-122.4;
        else if(FT1<=0.25329)tmp1=-122.3;
        else if(FT1<=0.25483)tmp1=-122.2;
        else if(FT1<=0.25638)tmp1=-122.1;
        else if(FT1<=0.25793)tmp1=-122;
    }
    else if(FT1<=0.27227){
        if(FT1==0.25793)tmp1=-122;
        else if(FT1<=0.25949)tmp1=-121.9;
        else if(FT1<=0.26106)tmp1=-121.8;
        else if(FT1<=0.26264)tmp1=-121.7;
        else if(FT1<=0.26423)tmp1=-121.6;
        else if(FT1<=0.26582)tmp1=-121.5;
        else if(FT1<=0.26742)tmp1=-121.4;
        else if(FT1<=0.26903)tmp1=-121.3;
        else if(FT1<=0.27065)tmp1=-121.2;
        else if(FT1<=0.27227)tmp1=-121.1;
        else if(FT1<=0.2739)tmp1=-121;
    }
    else if(FT1<=0.28893){
        if(FT1==0.2739)tmp1=-121;
        else if(FT1<=0.27554)tmp1=-120.9;
        else if(FT1<=0.27719)tmp1=-120.8;
        else if(FT1<=0.27884)tmp1=-120.7;
        else if(FT1<=0.2805)tmp1=-120.6;
        else if(FT1<=0.28217)tmp1=-120.5;
        else if(FT1<=0.28385)tmp1=-120.4;
        else if(FT1<=0.28554)tmp1=-120.3;
        else if(FT1<=0.28723)tmp1=-120.2;
        else if(FT1<=0.28893)tmp1=-120.1;
        else if(FT1<=0.29064)tmp1=-120;
    }
    else if(FT1<=0.30639){
        if(FT1==0.29064)tmp1=-120;
        else if(FT1<=0.29236)tmp1=-119.9;
        else if(FT1<=0.29409)tmp1=-119.8;
        else if(FT1<=0.29582)tmp1=-119.7;
        else if(FT1<=0.29756)tmp1=-119.6;
        else if(FT1<=0.29931)tmp1=-119.5;
        else if(FT1<=0.30107)tmp1=-119.4;
        else if(FT1<=0.30284)tmp1=-119.3;
        else if(FT1<=0.30461)tmp1=-119.2;
        else if(FT1<=0.30639)tmp1=-119.1;
        else if(FT1<=0.30818)tmp1=-119;
    }
    else if(FT1<=0.32467){
        if(FT1==0.30818)tmp1=-119;
        else if(FT1<=0.30998)tmp1=-118.9;
        else if(FT1<=0.31179)tmp1=-118.8;
        else if(FT1<=0.31361)tmp1=-118.7;
        else if(FT1<=0.31543)tmp1=-118.6;
        else if(FT1<=0.31726)tmp1=-118.5;
        else if(FT1<=0.3191)tmp1=-118.4;
        else if(FT1<=0.32095)tmp1=-118.3;
        else if(FT1<=0.32281)tmp1=-118.2;
        else if(FT1<=0.32467)tmp1=-118.1;
        else if(FT1<=0.32655)tmp1=-118;
    }
    else if(FT1<=0.3438){
        if(FT1==0.32655)tmp1=-118;
        else if(FT1<=0.32843)tmp1=-117.9;
        else if(FT1<=0.33032)tmp1=-117.8;
        else if(FT1<=0.33222)tmp1=-117.7;
        else if(FT1<=0.33413)tmp1=-117.6;
        else if(FT1<=0.33605)tmp1=-117.5;
        else if(FT1<=0.33797)tmp1=-117.4;
        else if(FT1<=0.33991)tmp1=-117.3;
        else if(FT1<=0.34185)tmp1=-117.2;
        else if(FT1<=0.3438)tmp1=-117.1;
        else if(FT1<=0.34576)tmp1=-117;
    }
    else if(FT1<=0.36381){
        if(FT1==0.34576)tmp1=-117;
        else if(FT1<=0.34773)tmp1=-116.9;
        else if(FT1<=0.34971)tmp1=-116.8;
        else if(FT1<=0.3517)tmp1=-116.7;
        else if(FT1<=0.35369)tmp1=-116.6;
        else if(FT1<=0.3557)tmp1=-116.5;
        else if(FT1<=0.35771)tmp1=-116.4;
        else if(FT1<=0.35973)tmp1=-116.3;
        else if(FT1<=0.36176)tmp1=-116.2;
        else if(FT1<=0.36381)tmp1=-116.1;
        else if(FT1<=0.36585)tmp1=-116;
    }
    else if(FT1<=0.38471){
        if(FT1==0.36585)tmp1=-116;
        else if(FT1<=0.36791)tmp1=-115.9;
        else if(FT1<=0.36998)tmp1=-115.8;
        else if(FT1<=0.37206)tmp1=-115.7;
        else if(FT1<=0.37414)tmp1=-115.6;
        else if(FT1<=0.37624)tmp1=-115.5;
        else if(FT1<=0.37834)tmp1=-115.4;
        else if(FT1<=0.38046)tmp1=-115.3;
        else if(FT1<=0.38258)tmp1=-115.2;
        else if(FT1<=0.38471)tmp1=-115.1;
        else if(FT1<=0.38685)tmp1=-115;
    }
    else if(FT1<=0.40655){
        if(FT1==0.38685)tmp1=-115;
        else if(FT1<=0.389)tmp1=-114.9;
        else if(FT1<=0.39116)tmp1=-114.8;
        else if(FT1<=0.39333)tmp1=-114.7;
        else if(FT1<=0.39551)tmp1=-114.6;
        else if(FT1<=0.3977)tmp1=-114.5;
        else if(FT1<=0.3999)tmp1=-114.4;
        else if(FT1<=0.4021)tmp1=-114.3;
        else if(FT1<=0.40432)tmp1=-114.2;
        else if(FT1<=0.40655)tmp1=-114.1;
        else if(FT1<=0.40878)tmp1=-114;
    }
    else if(FT1<=0.42934){
        if(FT1==0.40878)tmp1=-114;
        else if(FT1<=0.41103)tmp1=-113.9;
        else if(FT1<=0.41328)tmp1=-113.8;
        else if(FT1<=0.41555)tmp1=-113.7;
        else if(FT1<=0.41782)tmp1=-113.6;
        else if(FT1<=0.4201)tmp1=-113.5;
        else if(FT1<=0.4224)tmp1=-113.4;
        else if(FT1<=0.4247)tmp1=-113.3;
        else if(FT1<=0.42702)tmp1=-113.2;
        else if(FT1<=0.42934)tmp1=-113.1;
        else if(FT1<=0.43167)tmp1=-113;
    }
    else if(FT1<=0.45312){
        if(FT1==0.43167)tmp1=-113;
        else if(FT1<=0.43401)tmp1=-112.9;
        else if(FT1<=0.43637)tmp1=-112.8;
        else if(FT1<=0.43873)tmp1=-112.7;
        else if(FT1<=0.4411)tmp1=-112.6;
        else if(FT1<=0.44349)tmp1=-112.5;
        else if(FT1<=0.44588)tmp1=-112.4;
        else if(FT1<=0.44828)tmp1=-112.3;
        else if(FT1<=0.45069)tmp1=-112.2;
        else if(FT1<=0.45312)tmp1=-112.1;
        else if(FT1<=0.45555)tmp1=-112;
    }
    else if(FT1<=0.47791){
        if(FT1==0.45555)tmp1=-112;
        else if(FT1<=0.45799)tmp1=-111.9;
        else if(FT1<=0.46045)tmp1=-111.8;
        else if(FT1<=0.46291)tmp1=-111.7;
        else if(FT1<=0.46538)tmp1=-111.6;
        else if(FT1<=0.46787)tmp1=-111.5;
        else if(FT1<=0.47036)tmp1=-111.4;
        else if(FT1<=0.47287)tmp1=-111.3;
        else if(FT1<=0.47538)tmp1=-111.2;
        else if(FT1<=0.47791)tmp1=-111.1;
        else if(FT1<=0.48045)tmp1=-111;
    }
    else if(FT1<=0.50375){
        if(FT1==0.48045)tmp1=-111;
        else if(FT1<=0.48299)tmp1=-110.9;
        else if(FT1<=0.48555)tmp1=-110.8;
        else if(FT1<=0.48812)tmp1=-110.7;
        else if(FT1<=0.4907)tmp1=-110.6;
        else if(FT1<=0.49328)tmp1=-110.5;
        else if(FT1<=0.49588)tmp1=-110.4;
        else if(FT1<=0.49849)tmp1=-110.3;
        else if(FT1<=0.50111)tmp1=-110.2;
        else if(FT1<=0.50375)tmp1=-110.1;
        else if(FT1<=0.50639)tmp1=-110;
    }
    else if(FT1<=0.53065){
        if(FT1==0.50639)tmp1=-110;
        else if(FT1<=0.50904)tmp1=-109.9;
        else if(FT1<=0.5117)tmp1=-109.8;
        else if(FT1<=0.51438)tmp1=-109.7;
        else if(FT1<=0.51706)tmp1=-109.6;
        else if(FT1<=0.51976)tmp1=-109.5;
        else if(FT1<=0.52247)tmp1=-109.4;
        else if(FT1<=0.52519)tmp1=-109.3;
        else if(FT1<=0.52791)tmp1=-109.2;
        else if(FT1<=0.53065)tmp1=-109.1;
        else if(FT1<=0.53341)tmp1=-109;
    }
    else if(FT1<=0.55867){
        if(FT1==0.53341)tmp1=-109;
        else if(FT1<=0.53617)tmp1=-108.9;
        else if(FT1<=0.53894)tmp1=-108.8;
        else if(FT1<=0.54173)tmp1=-108.7;
        else if(FT1<=0.54452)tmp1=-108.6;
        else if(FT1<=0.54733)tmp1=-108.5;
        else if(FT1<=0.55015)tmp1=-108.4;
        else if(FT1<=0.55297)tmp1=-108.3;
        else if(FT1<=0.55581)tmp1=-108.2;
        else if(FT1<=0.55867)tmp1=-108.1;
        else if(FT1<=0.56153)tmp1=-108;
    }
    else if(FT1<=0.58781){
        if(FT1==0.56153)tmp1=-108;
        else if(FT1<=0.5644)tmp1=-107.9;
        else if(FT1<=0.56729)tmp1=-107.8;
        else if(FT1<=0.57019)tmp1=-107.7;
        else if(FT1<=0.57309)tmp1=-107.6;
        else if(FT1<=0.57601)tmp1=-107.5;
        else if(FT1<=0.57895)tmp1=-107.4;
        else if(FT1<=0.58189)tmp1=-107.3;
        else if(FT1<=0.58484)tmp1=-107.2;
        else if(FT1<=0.58781)tmp1=-107.1;
        else if(FT1<=0.59079)tmp1=-107;
    }
    else if(FT1<=0.61811){
        if(FT1==0.59079)tmp1=-107;
        else if(FT1<=0.59378)tmp1=-106.9;
        else if(FT1<=0.59678)tmp1=-106.8;
        else if(FT1<=0.59979)tmp1=-106.7;
        else if(FT1<=0.60281)tmp1=-106.6;
        else if(FT1<=0.60585)tmp1=-106.5;
        else if(FT1<=0.6089)tmp1=-106.4;
        else if(FT1<=0.61196)tmp1=-106.3;
        else if(FT1<=0.61503)tmp1=-106.2;
        else if(FT1<=0.61811)tmp1=-106.1;
        else if(FT1<=0.62121)tmp1=-106;
    }
    else if(FT1<=0.64961){
        if(FT1==0.62121)tmp1=-106;
        else if(FT1<=0.62432)tmp1=-105.9;
        else if(FT1<=0.62744)tmp1=-105.8;
        else if(FT1<=0.63057)tmp1=-105.7;
        else if(FT1<=0.63371)tmp1=-105.6;
        else if(FT1<=0.63687)tmp1=-105.5;
        else if(FT1<=0.64004)tmp1=-105.4;
        else if(FT1<=0.64322)tmp1=-105.3;
        else if(FT1<=0.64641)tmp1=-105.2;
        else if(FT1<=0.64961)tmp1=-105.1;
        else if(FT1<=0.65283)tmp1=-105;
    }
    else if(FT1<=0.68233){
        if(FT1==0.65283)tmp1=-105;
        else if(FT1<=0.65606)tmp1=-104.9;
        else if(FT1<=0.6593)tmp1=-104.8;
        else if(FT1<=0.66255)tmp1=-104.7;
        else if(FT1<=0.66582)tmp1=-104.6;
        else if(FT1<=0.6691)tmp1=-104.5;
        else if(FT1<=0.67239)tmp1=-104.4;
        else if(FT1<=0.67569)tmp1=-104.3;
        else if(FT1<=0.679)tmp1=-104.2;
        else if(FT1<=0.68233)tmp1=-104.1;
        else if(FT1<=0.68567)tmp1=-104;
    }
    else if(FT1<=0.71631){
        if(FT1==0.68567)tmp1=-104;
        else if(FT1<=0.68903)tmp1=-103.9;
        else if(FT1<=0.69239)tmp1=-103.8;
        else if(FT1<=0.69577)tmp1=-103.7;
        else if(FT1<=0.69916)tmp1=-103.6;
        else if(FT1<=0.70256)tmp1=-103.5;
        else if(FT1<=0.70598)tmp1=-103.4;
        else if(FT1<=0.70941)tmp1=-103.3;
        else if(FT1<=0.71285)tmp1=-103.2;
        else if(FT1<=0.71631)tmp1=-103.1;
        else if(FT1<=0.71977)tmp1=-103;
    }
    else if(FT1<=0.75157){
        if(FT1==0.71977)tmp1=-103;
        else if(FT1<=0.72326)tmp1=-102.9;
        else if(FT1<=0.72675)tmp1=-102.8;
        else if(FT1<=0.73026)tmp1=-102.7;
        else if(FT1<=0.73377)tmp1=-102.6;
        else if(FT1<=0.73731)tmp1=-102.5;
        else if(FT1<=0.74085)tmp1=-102.4;
        else if(FT1<=0.74441)tmp1=-102.3;
        else if(FT1<=0.74798)tmp1=-102.2;
        else if(FT1<=0.75157)tmp1=-102.1;
        else if(FT1<=0.75516)tmp1=-102;
    }
    else if(FT1<=0.78814){
        if(FT1==0.75516)tmp1=-102;
        else if(FT1<=0.75878)tmp1=-101.9;
        else if(FT1<=0.7624)tmp1=-101.8;
        else if(FT1<=0.76604)tmp1=-101.7;
        else if(FT1<=0.76969)tmp1=-101.6;
        else if(FT1<=0.77335)tmp1=-101.5;
        else if(FT1<=0.77703)tmp1=-101.4;
        else if(FT1<=0.78072)tmp1=-101.3;
        else if(FT1<=0.78442)tmp1=-101.2;
        else if(FT1<=0.78814)tmp1=-101.1;
        else if(FT1<=0.79187)tmp1=-101;
    }
    else if(FT1<=0.82607){
        if(FT1==0.79187)tmp1=-101;
        else if(FT1<=0.79562)tmp1=-100.9;
        else if(FT1<=0.79938)tmp1=-100.8;
        else if(FT1<=0.80315)tmp1=-100.7;
        else if(FT1<=0.80693)tmp1=-100.6;
        else if(FT1<=0.81073)tmp1=-100.5;
        else if(FT1<=0.81455)tmp1=-100.4;
        else if(FT1<=0.81837)tmp1=-100.3;
        else if(FT1<=0.82221)tmp1=-100.2;
        else if(FT1<=0.82607)tmp1=-100.1;
        else if(FT1<=0.82993)tmp1=-100;
    }
    else if(FT1<=0.86537){
        if(FT1==0.82993)tmp1=-100;
        else if(FT1<=0.83382)tmp1=-99.9;
        else if(FT1<=0.83771)tmp1=-99.8;
        else if(FT1<=0.84162)tmp1=-99.7;
        else if(FT1<=0.84554)tmp1=-99.6;
        else if(FT1<=0.84948)tmp1=-99.5;
        else if(FT1<=0.85343)tmp1=-99.4;
        else if(FT1<=0.8574)tmp1=-99.3;
        else if(FT1<=0.86138)tmp1=-99.2;
        else if(FT1<=0.86537)tmp1=-99.1;
        else if(FT1<=0.86938)tmp1=-99;
    }
    else if(FT1<=0.90608){
        if(FT1==0.86938)tmp1=-99;
        else if(FT1<=0.8734)tmp1=-98.9;
        else if(FT1<=0.87743)tmp1=-98.8;
        else if(FT1<=0.88148)tmp1=-98.7;
        else if(FT1<=0.88555)tmp1=-98.6;
        else if(FT1<=0.88963)tmp1=-98.5;
        else if(FT1<=0.89372)tmp1=-98.4;
        else if(FT1<=0.89783)tmp1=-98.3;
        else if(FT1<=0.90195)tmp1=-98.2;
        else if(FT1<=0.90608)tmp1=-98.1;
        else if(FT1<=0.91023)tmp1=-98;
    }
    else if(FT1<=0.94824){
        if(FT1==0.91023)tmp1=-98;
        else if(FT1<=0.9144)tmp1=-97.9;
        else if(FT1<=0.91858)tmp1=-97.8;
        else if(FT1<=0.92277)tmp1=-97.7;
        else if(FT1<=0.92698)tmp1=-97.6;
        else if(FT1<=0.9312)tmp1=-97.5;
        else if(FT1<=0.93544)tmp1=-97.4;
        else if(FT1<=0.93969)tmp1=-97.3;
        else if(FT1<=0.94396)tmp1=-97.2;
        else if(FT1<=0.94824)tmp1=-97.1;
        else if(FT1<=0.95254)tmp1=-97;
    }
    else if(FT1<=0.99188){
        if(FT1==0.95254)tmp1=-97;
        else if(FT1<=0.95685)tmp1=-96.9;
        else if(FT1<=0.96118)tmp1=-96.8;
        else if(FT1<=0.96552)tmp1=-96.7;
        else if(FT1<=0.96987)tmp1=-96.6;
        else if(FT1<=0.97424)tmp1=-96.5;
        else if(FT1<=0.97863)tmp1=-96.4;
        else if(FT1<=0.98303)tmp1=-96.3;
        else if(FT1<=0.98745)tmp1=-96.2;
        else if(FT1<=0.99188)tmp1=-96.1;
        else if(FT1<=0.99632)tmp1=-96;
    }
    else if(FT1<=1.03702){
        if(FT1==0.99632)tmp1=-96;
        else if(FT1<=1.00078)tmp1=-95.9;
        else if(FT1<=1.00526)tmp1=-95.8;
        else if(FT1<=1.00975)tmp1=-95.7;
        else if(FT1<=1.01426)tmp1=-95.6;
        else if(FT1<=1.01878)tmp1=-95.5;
        else if(FT1<=1.02332)tmp1=-95.4;
        else if(FT1<=1.02787)tmp1=-95.3;
        else if(FT1<=1.03244)tmp1=-95.2;
        else if(FT1<=1.03702)tmp1=-95.1;
        else if(FT1<=1.04162)tmp1=-95;
    }
    else if(FT1<=1.0837){
        if(FT1==1.04162)tmp1=-95;
        else if(FT1<=1.04623)tmp1=-94.9;
        else if(FT1<=1.05086)tmp1=-94.8;
        else if(FT1<=1.05551)tmp1=-94.7;
        else if(FT1<=1.06017)tmp1=-94.6;
        else if(FT1<=1.06484)tmp1=-94.5;
        else if(FT1<=1.06954)tmp1=-94.4;
        else if(FT1<=1.07424)tmp1=-94.3;
        else if(FT1<=1.07897)tmp1=-94.2;
        else if(FT1<=1.0837)tmp1=-94.1;
        else if(FT1<=1.08846)tmp1=-94;
    }
    else if(FT1<=1.13196){
        if(FT1==1.08846)tmp1=-94;
        else if(FT1<=1.09323)tmp1=-93.9;
        else if(FT1<=1.09802)tmp1=-93.8;
        else if(FT1<=1.10282)tmp1=-93.7;
        else if(FT1<=1.10763)tmp1=-93.6;
        else if(FT1<=1.11247)tmp1=-93.5;
        else if(FT1<=1.11732)tmp1=-93.4;
        else if(FT1<=1.12218)tmp1=-93.3;
        else if(FT1<=1.12706)tmp1=-93.2;
        else if(FT1<=1.13196)tmp1=-93.1;
        else if(FT1<=1.13688)tmp1=-93;
    }
    else if(FT1<=1.18183){
        if(FT1==1.13688)tmp1=-93;
        else if(FT1<=1.14181)tmp1=-92.9;
        else if(FT1<=1.14675)tmp1=-92.8;
        else if(FT1<=1.15171)tmp1=-92.7;
        else if(FT1<=1.15669)tmp1=-92.6;
        else if(FT1<=1.16169)tmp1=-92.5;
        else if(FT1<=1.1667)tmp1=-92.4;
        else if(FT1<=1.17172)tmp1=-92.3;
        else if(FT1<=1.17677)tmp1=-92.2;
        else if(FT1<=1.18183)tmp1=-92.1;
        else if(FT1<=1.1869)tmp1=-92;
    }
    else if(FT1<=1.23333){
        if(FT1==1.1869)tmp1=-92;
        else if(FT1<=1.192)tmp1=-91.9;
        else if(FT1<=1.1971)tmp1=-91.8;
        else if(FT1<=1.20223)tmp1=-91.7;
        else if(FT1<=1.20737)tmp1=-91.6;
        else if(FT1<=1.21253)tmp1=-91.5;
        else if(FT1<=1.21771)tmp1=-91.4;
        else if(FT1<=1.2229)tmp1=-91.3;
        else if(FT1<=1.22811)tmp1=-91.2;
        else if(FT1<=1.23333)tmp1=-91.1;
        else if(FT1<=1.23857)tmp1=-91;
    }
    else if(FT1<=1.28651){
        if(FT1==1.23857)tmp1=-91;
        else if(FT1<=1.24383)tmp1=-90.9;
        else if(FT1<=1.24911)tmp1=-90.8;
        else if(FT1<=1.2544)tmp1=-90.7;
        else if(FT1<=1.25971)tmp1=-90.6;
        else if(FT1<=1.26503)tmp1=-90.5;
        else if(FT1<=1.27038)tmp1=-90.4;
        else if(FT1<=1.27574)tmp1=-90.3;
        else if(FT1<=1.28111)tmp1=-90.2;
        else if(FT1<=1.28651)tmp1=-90.1;
        else if(FT1<=1.29192)tmp1=-90;
    }
    else if(FT1<=1.34139){
        if(FT1==1.29192)tmp1=-90;
        else if(FT1<=1.29735)tmp1=-89.9;
        else if(FT1<=1.30279)tmp1=-89.8;
        else if(FT1<=1.30825)tmp1=-89.7;
        else if(FT1<=1.31373)tmp1=-89.6;
        else if(FT1<=1.31923)tmp1=-89.5;
        else if(FT1<=1.32475)tmp1=-89.4;
        else if(FT1<=1.33028)tmp1=-89.3;
        else if(FT1<=1.33583)tmp1=-89.2;
        else if(FT1<=1.34139)tmp1=-89.1;
        else if(FT1<=1.34697)tmp1=-89;
    }
    else if(FT1<=1.39801){
        if(FT1==1.34697)tmp1=-89;
        else if(FT1<=1.35258)tmp1=-88.9;
        else if(FT1<=1.35819)tmp1=-88.8;
        else if(FT1<=1.36383)tmp1=-88.7;
        else if(FT1<=1.36948)tmp1=-88.6;
        else if(FT1<=1.37515)tmp1=-88.5;
        else if(FT1<=1.38084)tmp1=-88.4;
        else if(FT1<=1.38655)tmp1=-88.3;
        else if(FT1<=1.39227)tmp1=-88.2;
        else if(FT1<=1.39801)tmp1=-88.1;
        else if(FT1<=1.40377)tmp1=-88;
    }
    else if(FT1<=1.45641){
        if(FT1==1.40377)tmp1=-88;
        else if(FT1<=1.40955)tmp1=-87.9;
        else if(FT1<=1.41534)tmp1=-87.8;
        else if(FT1<=1.42116)tmp1=-87.7;
        else if(FT1<=1.42699)tmp1=-87.6;
        else if(FT1<=1.43284)tmp1=-87.5;
        else if(FT1<=1.4387)tmp1=-87.4;
        else if(FT1<=1.44459)tmp1=-87.3;
        else if(FT1<=1.45049)tmp1=-87.2;
        else if(FT1<=1.45641)tmp1=-87.1;
        else if(FT1<=1.46235)tmp1=-87;
    }
    else if(FT1<=1.51661){
        if(FT1==1.46235)tmp1=-87;
        else if(FT1<=1.4683)tmp1=-86.9;
        else if(FT1<=1.47428)tmp1=-86.8;
        else if(FT1<=1.48027)tmp1=-86.7;
        else if(FT1<=1.48628)tmp1=-86.6;
        else if(FT1<=1.49231)tmp1=-86.5;
        else if(FT1<=1.49836)tmp1=-86.4;
        else if(FT1<=1.50442)tmp1=-86.3;
        else if(FT1<=1.51051)tmp1=-86.2;
        else if(FT1<=1.51661)tmp1=-86.1;
        else if(FT1<=1.52273)tmp1=-86;
    }
    else if(FT1<=1.57865){
        if(FT1==1.52273)tmp1=-86;
        else if(FT1<=1.52887)tmp1=-85.9;
        else if(FT1<=1.53503)tmp1=-85.8;
        else if(FT1<=1.5412)tmp1=-85.7;
        else if(FT1<=1.5474)tmp1=-85.6;
        else if(FT1<=1.55361)tmp1=-85.5;
        else if(FT1<=1.55984)tmp1=-85.4;
        else if(FT1<=1.56609)tmp1=-85.3;
        else if(FT1<=1.57236)tmp1=-85.2;
        else if(FT1<=1.57865)tmp1=-85.1;
        else if(FT1<=1.58496)tmp1=-85;
    }
    else if(FT1<=1.64256){
        if(FT1==1.58496)tmp1=-85;
        else if(FT1<=1.59128)tmp1=-84.9;
        else if(FT1<=1.59763)tmp1=-84.8;
        else if(FT1<=1.60399)tmp1=-84.7;
        else if(FT1<=1.61037)tmp1=-84.6;
        else if(FT1<=1.61677)tmp1=-84.5;
        else if(FT1<=1.62319)tmp1=-84.4;
        else if(FT1<=1.62963)tmp1=-84.3;
        else if(FT1<=1.63609)tmp1=-84.2;
        else if(FT1<=1.64256)tmp1=-84.1;
        else if(FT1<=1.64906)tmp1=-84;
    }
    else if(FT1<=1.70838){
        if(FT1==1.64906)tmp1=-84;
        else if(FT1<=1.65557)tmp1=-83.9;
        else if(FT1<=1.66211)tmp1=-83.8;
        else if(FT1<=1.66866)tmp1=-83.7;
        else if(FT1<=1.67523)tmp1=-83.6;
        else if(FT1<=1.68183)tmp1=-83.5;
        else if(FT1<=1.68844)tmp1=-83.4;
        else if(FT1<=1.69507)tmp1=-83.3;
        else if(FT1<=1.70172)tmp1=-83.2;
        else if(FT1<=1.70838)tmp1=-83.1;
        else if(FT1<=1.71507)tmp1=-83;
    }
    else if(FT1<=1.77615){
        if(FT1==1.71507)tmp1=-83;
        else if(FT1<=1.72178)tmp1=-82.9;
        else if(FT1<=1.72851)tmp1=-82.8;
        else if(FT1<=1.73525)tmp1=-82.7;
        else if(FT1<=1.74202)tmp1=-82.6;
        else if(FT1<=1.74881)tmp1=-82.5;
        else if(FT1<=1.75561)tmp1=-82.4;
        else if(FT1<=1.76244)tmp1=-82.3;
        else if(FT1<=1.76928)tmp1=-82.2;
        else if(FT1<=1.77615)tmp1=-82.1;
        else if(FT1<=1.78303)tmp1=-82;
    }
    else if(FT1<=1.84588){
        if(FT1==1.78303)tmp1=-82;
        else if(FT1<=1.78993)tmp1=-81.9;
        else if(FT1<=1.79686)tmp1=-81.8;
        else if(FT1<=1.8038)tmp1=-81.7;
        else if(FT1<=1.81076)tmp1=-81.6;
        else if(FT1<=1.81775)tmp1=-81.5;
        else if(FT1<=1.82475)tmp1=-81.4;
        else if(FT1<=1.83177)tmp1=-81.3;
        else if(FT1<=1.83882)tmp1=-81.2;
        else if(FT1<=1.84588)tmp1=-81.1;
        else if(FT1<=1.85296)tmp1=-81;
    }
    else if(FT1<=1.91762){
        if(FT1==1.85296)tmp1=-81;
        else if(FT1<=1.86007)tmp1=-80.9;
        else if(FT1<=1.86719)tmp1=-80.8;
        else if(FT1<=1.87433)tmp1=-80.7;
        else if(FT1<=1.8815)tmp1=-80.6;
        else if(FT1<=1.88868)tmp1=-80.5;
        else if(FT1<=1.89589)tmp1=-80.4;
        else if(FT1<=1.90311)tmp1=-80.3;
        else if(FT1<=1.91036)tmp1=-80.2;
        else if(FT1<=1.91762)tmp1=-80.1;
        else if(FT1<=1.92491)tmp1=-80;
    }
    else if(FT1<=1.99141){
        if(FT1==1.92491)tmp1=-80;
        else if(FT1<=1.93221)tmp1=-79.9;
        else if(FT1<=1.93954)tmp1=-79.8;
        else if(FT1<=1.94689)tmp1=-79.7;
        else if(FT1<=1.95426)tmp1=-79.6;
        else if(FT1<=1.96165)tmp1=-79.5;
        else if(FT1<=1.96905)tmp1=-79.4;
        else if(FT1<=1.97648)tmp1=-79.3;
        else if(FT1<=1.98394)tmp1=-79.2;
        else if(FT1<=1.99141)tmp1=-79.1;
        else if(FT1<=1.9989)tmp1=-79;
    }
    else if(FT1<=2.06727){
        if(FT1==1.9989)tmp1=-79;
        else if(FT1<=2.00641)tmp1=-78.9;
        else if(FT1<=2.01394)tmp1=-78.8;
        else if(FT1<=2.0215)tmp1=-78.7;
        else if(FT1<=2.02907)tmp1=-78.6;
        else if(FT1<=2.03667)tmp1=-78.5;
        else if(FT1<=2.04429)tmp1=-78.4;
        else if(FT1<=2.05193)tmp1=-78.3;
        else if(FT1<=2.05958)tmp1=-78.2;
        else if(FT1<=2.06727)tmp1=-78.1;
        else if(FT1<=2.07497)tmp1=-78;
    }
    else if(FT1<=2.14523){
        if(FT1==2.07497)tmp1=-78;
        else if(FT1<=2.08269)tmp1=-77.9;
        else if(FT1<=2.09043)tmp1=-77.8;
        else if(FT1<=2.0982)tmp1=-77.7;
        else if(FT1<=2.10598)tmp1=-77.6;
        else if(FT1<=2.11379)tmp1=-77.5;
        else if(FT1<=2.12162)tmp1=-77.4;
        else if(FT1<=2.12947)tmp1=-77.3;
        else if(FT1<=2.13734)tmp1=-77.2;
        else if(FT1<=2.14523)tmp1=-77.1;
        else if(FT1<=2.15315)tmp1=-77;
    }
    else if(FT1<=2.22534){
        if(FT1==2.15315)tmp1=-77;
        else if(FT1<=2.16108)tmp1=-76.9;
        else if(FT1<=2.16904)tmp1=-76.8;
        else if(FT1<=2.17702)tmp1=-76.7;
        else if(FT1<=2.18502)tmp1=-76.6;
        else if(FT1<=2.19304)tmp1=-76.5;
        else if(FT1<=2.20108)tmp1=-76.4;
        else if(FT1<=2.20915)tmp1=-76.3;
        else if(FT1<=2.21723)tmp1=-76.2;
        else if(FT1<=2.22534)tmp1=-76.1;
        else if(FT1<=2.23347)tmp1=-76;
    }
    else if(FT1<=2.30763){
        if(FT1==2.23347)tmp1=-76;
        else if(FT1<=2.24162)tmp1=-75.9;
        else if(FT1<=2.2498)tmp1=-75.8;
        else if(FT1<=2.25799)tmp1=-75.7;
        else if(FT1<=2.26621)tmp1=-75.6;
        else if(FT1<=2.27445)tmp1=-75.5;
        else if(FT1<=2.28271)tmp1=-75.4;
        else if(FT1<=2.29099)tmp1=-75.3;
        else if(FT1<=2.2993)tmp1=-75.2;
        else if(FT1<=2.30763)tmp1=-75.1;
        else if(FT1<=2.31598)tmp1=-75;
    }
    else if(FT1<=2.39212){
        if(FT1==2.31598)tmp1=-75;
        else if(FT1<=2.32435)tmp1=-74.9;
        else if(FT1<=2.33274)tmp1=-74.8;
        else if(FT1<=2.34116)tmp1=-74.7;
        else if(FT1<=2.3496)tmp1=-74.6;
        else if(FT1<=2.35806)tmp1=-74.5;
        else if(FT1<=2.36654)tmp1=-74.4;
        else if(FT1<=2.37504)tmp1=-74.3;
        else if(FT1<=2.38357)tmp1=-74.2;
        else if(FT1<=2.39212)tmp1=-74.1;
        else if(FT1<=2.40069)tmp1=-74;
    }
    else if(FT1<=2.47886){
        if(FT1==2.40069)tmp1=-74;
        else if(FT1<=2.40929)tmp1=-73.9;
        else if(FT1<=2.41791)tmp1=-73.8;
        else if(FT1<=2.42654)tmp1=-73.7;
        else if(FT1<=2.43521)tmp1=-73.6;
        else if(FT1<=2.44389)tmp1=-73.5;
        else if(FT1<=2.4526)tmp1=-73.4;
        else if(FT1<=2.46133)tmp1=-73.3;
        else if(FT1<=2.47008)tmp1=-73.2;
        else if(FT1<=2.47886)tmp1=-73.1;
        else if(FT1<=2.48766)tmp1=-73;
    }
    else if(FT1<=2.56787){
        if(FT1==2.48766)tmp1=-73;
        else if(FT1<=2.49648)tmp1=-72.9;
        else if(FT1<=2.50532)tmp1=-72.8;
        else if(FT1<=2.51419)tmp1=-72.7;
        else if(FT1<=2.52308)tmp1=-72.6;
        else if(FT1<=2.53199)tmp1=-72.5;
        else if(FT1<=2.54093)tmp1=-72.4;
        else if(FT1<=2.54988)tmp1=-72.3;
        else if(FT1<=2.55887)tmp1=-72.2;
        else if(FT1<=2.56787)tmp1=-72.1;
        else if(FT1<=2.5769)tmp1=-72;
    }
    else if(FT1<=2.65919){
        if(FT1==2.5769)tmp1=-72;
        else if(FT1<=2.58595)tmp1=-71.9;
        else if(FT1<=2.59502)tmp1=-71.8;
        else if(FT1<=2.60412)tmp1=-71.7;
        else if(FT1<=2.61324)tmp1=-71.6;
        else if(FT1<=2.62238)tmp1=-71.5;
        else if(FT1<=2.63155)tmp1=-71.4;
        else if(FT1<=2.64074)tmp1=-71.3;
        else if(FT1<=2.64996)tmp1=-71.2;
        else if(FT1<=2.65919)tmp1=-71.1;
        else if(FT1<=2.66845)tmp1=-71;
    }
    else if(FT1<=2.75286){
        if(FT1==2.66845)tmp1=-71;
        else if(FT1<=2.67774)tmp1=-70.9;
        else if(FT1<=2.68704)tmp1=-70.8;
        else if(FT1<=2.69638)tmp1=-70.7;
        else if(FT1<=2.70573)tmp1=-70.6;
        else if(FT1<=2.71511)tmp1=-70.5;
        else if(FT1<=2.72451)tmp1=-70.4;
        else if(FT1<=2.73394)tmp1=-70.3;
        else if(FT1<=2.74338)tmp1=-70.2;
        else if(FT1<=2.75286)tmp1=-70.1;
        else if(FT1<=2.76235)tmp1=-70;
    }
    else if(FT1<=2.8489){
        if(FT1==2.76235)tmp1=-70;
        else if(FT1<=2.77187)tmp1=-69.9;
        else if(FT1<=2.78142)tmp1=-69.8;
        else if(FT1<=2.79099)tmp1=-69.7;
        else if(FT1<=2.80058)tmp1=-69.6;
        else if(FT1<=2.81019)tmp1=-69.5;
        else if(FT1<=2.81983)tmp1=-69.4;
        else if(FT1<=2.8295)tmp1=-69.3;
        else if(FT1<=2.83919)tmp1=-69.2;
        else if(FT1<=2.8489)tmp1=-69.1;
        else if(FT1<=2.85863)tmp1=-69;
    }
    else if(FT1<=2.94735){
        if(FT1==2.85863)tmp1=-69;
        else if(FT1<=2.86839)tmp1=-68.9;
        else if(FT1<=2.87818)tmp1=-68.8;
        else if(FT1<=2.88799)tmp1=-68.7;
        else if(FT1<=2.89782)tmp1=-68.6;
        else if(FT1<=2.90768)tmp1=-68.5;
        else if(FT1<=2.91756)tmp1=-68.4;
        else if(FT1<=2.92746)tmp1=-68.3;
        else if(FT1<=2.93739)tmp1=-68.2;
        else if(FT1<=2.94735)tmp1=-68.1;
        else if(FT1<=2.95733)tmp1=-68;
    }
    else if(FT1<=3.04824){
        if(FT1==2.95733)tmp1=-68;
        else if(FT1<=2.96733)tmp1=-67.9;
        else if(FT1<=2.97736)tmp1=-67.8;
        else if(FT1<=2.98741)tmp1=-67.7;
        else if(FT1<=2.99748)tmp1=-67.6;
        else if(FT1<=3.00759)tmp1=-67.5;
        else if(FT1<=3.01771)tmp1=-67.4;
        else if(FT1<=3.02786)tmp1=-67.3;
        else if(FT1<=3.03804)tmp1=-67.2;
        else if(FT1<=3.04824)tmp1=-67.1;
        else if(FT1<=3.05846)tmp1=-67;
    }
    else if(FT1<=3.1516){
        if(FT1==3.05846)tmp1=-67;
        else if(FT1<=3.06871)tmp1=-66.9;
        else if(FT1<=3.07899)tmp1=-66.8;
        else if(FT1<=3.08928)tmp1=-66.7;
        else if(FT1<=3.09961)tmp1=-66.6;
        else if(FT1<=3.10996)tmp1=-66.5;
        else if(FT1<=3.12033)tmp1=-66.4;
        else if(FT1<=3.13073)tmp1=-66.3;
        else if(FT1<=3.14115)tmp1=-66.2;
        else if(FT1<=3.1516)tmp1=-66.1;
        else if(FT1<=3.16208)tmp1=-66;
    }
    else if(FT1<=3.25747){
        if(FT1==3.16208)tmp1=-66;
        else if(FT1<=3.17257)tmp1=-65.9;
        else if(FT1<=3.1831)tmp1=-65.8;
        else if(FT1<=3.19365)tmp1=-65.7;
        else if(FT1<=3.20422)tmp1=-65.6;
        else if(FT1<=3.21482)tmp1=-65.5;
        else if(FT1<=3.22545)tmp1=-65.4;
        else if(FT1<=3.2361)tmp1=-65.3;
        else if(FT1<=3.24677)tmp1=-65.2;
        else if(FT1<=3.25747)tmp1=-65.1;
        else if(FT1<=3.2682)tmp1=-65;
    }
    else if(FT1<=3.36589){
        if(FT1==3.2682)tmp1=-65;
        else if(FT1<=3.27895)tmp1=-64.9;
        else if(FT1<=3.28973)tmp1=-64.8;
        else if(FT1<=3.30053)tmp1=-64.7;
        else if(FT1<=3.31136)tmp1=-64.6;
        else if(FT1<=3.32221)tmp1=-64.5;
        else if(FT1<=3.33309)tmp1=-64.4;
        else if(FT1<=3.344)tmp1=-64.3;
        else if(FT1<=3.35493)tmp1=-64.2;
        else if(FT1<=3.36589)tmp1=-64.1;
        else if(FT1<=3.37687)tmp1=-64;
    }
    else if(FT1<=3.47687){
        if(FT1==3.37687)tmp1=-64;
        else if(FT1<=3.38788)tmp1=-63.9;
        else if(FT1<=3.39891)tmp1=-63.8;
        else if(FT1<=3.40997)tmp1=-63.7;
        else if(FT1<=3.42105)tmp1=-63.6;
        else if(FT1<=3.43217)tmp1=-63.5;
        else if(FT1<=3.4433)tmp1=-63.4;
        else if(FT1<=3.45447)tmp1=-63.3;
        else if(FT1<=3.46566)tmp1=-63.2;
        else if(FT1<=3.47687)tmp1=-63.1;
    }
    else if(FT1<=3.59046){
        if(FT1==3.48811)tmp1=-63;
        else if(FT1<=3.49938)tmp1=-62.9;
        else if(FT1<=3.51067)tmp1=-62.8;
        else if(FT1<=3.52199)tmp1=-62.7;
        else if(FT1<=3.53334)tmp1=-62.6;
        else if(FT1<=3.54471)tmp1=-62.5;
        else if(FT1<=3.55611)tmp1=-62.4;
        else if(FT1<=3.56753)tmp1=-62.3;
        else if(FT1<=3.57898)tmp1=-62.2;
        else if(FT1<=3.59046)tmp1=-62.1;
    }
    else if(FT1<=3.70668){
        if(FT1==3.60196)tmp1=-62;
        else if(FT1<=3.61349)tmp1=-61.9;
        else if(FT1<=3.62505)tmp1=-61.8;
        else if(FT1<=3.63663)tmp1=-61.7;
        else if(FT1<=3.64824)tmp1=-61.6;
        else if(FT1<=3.65988)tmp1=-61.5;
        else if(FT1<=3.67154)tmp1=-61.4;
        else if(FT1<=3.68323)tmp1=-61.3;
        else if(FT1<=3.69494)tmp1=-61.2;
        else if(FT1<=3.70668)tmp1=-61.1;
    }
    else if(FT1<=3.82558){
        if(FT1==3.71845)tmp1=-61;
        else if(FT1<=3.73025)tmp1=-60.9;
        else if(FT1<=3.74207)tmp1=-60.8;
        else if(FT1<=3.75392)tmp1=-60.7;
        else if(FT1<=3.76579)tmp1=-60.6;
        else if(FT1<=3.7777)tmp1=-60.5;
        else if(FT1<=3.78963)tmp1=-60.4;
        else if(FT1<=3.80158)tmp1=-60.3;
        else if(FT1<=3.81357)tmp1=-60.2;
        else if(FT1<=3.82558)tmp1=-60.1;
    }
    else if(FT1<=3.94717){
        if(FT1==3.83761)tmp1=-60;
        else if(FT1<=3.84968)tmp1=-59.9;
        else if(FT1<=3.86177)tmp1=-59.8;
        else if(FT1<=3.87389)tmp1=-59.7;
        else if(FT1<=3.88603)tmp1=-59.6;
        else if(FT1<=3.89821)tmp1=-59.5;
        else if(FT1<=3.91041)tmp1=-59.4;
        else if(FT1<=3.92264)tmp1=-59.3;
        else if(FT1<=3.93489)tmp1=-59.2;
        else if(FT1<=3.94717)tmp1=-59.1;
    }
    else if(FT1<=4.0715){
        if(FT1==3.95948)tmp1=-59;
        else if(FT1<=3.97182)tmp1=-58.9;
        else if(FT1<=3.98418)tmp1=-58.8;
        else if(FT1<=3.99657)tmp1=-58.7;
        else if(FT1<=4.00899)tmp1=-58.6;
        else if(FT1<=4.02144)tmp1=-58.5;
        else if(FT1<=4.03391)tmp1=-58.4;
        else if(FT1<=4.04641)tmp1=-58.3;
        else if(FT1<=4.05894)tmp1=-58.2;
        else if(FT1<=4.0715)tmp1=-58.1;
    }
    else if(FT1<=4.19859){
        if(FT1==4.08408)tmp1=-58;
        else if(FT1<=4.09669)tmp1=-57.9;
        else if(FT1<=4.10933)tmp1=-57.8;
        else if(FT1<=4.122)tmp1=-57.7;
        else if(FT1<=4.1347)tmp1=-57.6;
        else if(FT1<=4.14742)tmp1=-57.5;
        else if(FT1<=4.16017)tmp1=-57.4;
        else if(FT1<=4.17295)tmp1=-57.3;
        else if(FT1<=4.18575)tmp1=-57.2;
        else if(FT1<=4.19859)tmp1=-57.1;
    }
    else if(FT1<=4.32847){
        if(FT1==4.21145)tmp1=-57;
        else if(FT1<=4.22434)tmp1=-56.9;
        else if(FT1<=4.23726)tmp1=-56.8;
        else if(FT1<=4.2502)tmp1=-56.7;
        else if(FT1<=4.26318)tmp1=-56.6;
        else if(FT1<=4.27618)tmp1=-56.5;
        else if(FT1<=4.28921)tmp1=-56.4;
        else if(FT1<=4.30227)tmp1=-56.3;
        else if(FT1<=4.31536)tmp1=-56.2;
        else if(FT1<=4.32847)tmp1=-56.1;
    }
    else if(FT1<=4.46118){
        if(FT1==4.34162)tmp1=-56;
        else if(FT1<=4.35479)tmp1=-55.9;
        else if(FT1<=4.36799)tmp1=-55.8;
        else if(FT1<=4.38122)tmp1=-55.7;
        else if(FT1<=4.39447)tmp1=-55.6;
        else if(FT1<=4.40776)tmp1=-55.5;
        else if(FT1<=4.42107)tmp1=-55.4;
        else if(FT1<=4.43441)tmp1=-55.3;
        else if(FT1<=4.44778)tmp1=-55.2;
        else if(FT1<=4.46118)tmp1=-55.1;
    }
    else if(FT1<=4.59675){
        if(FT1==4.47461)tmp1=-55;
        else if(FT1<=4.48807)tmp1=-54.9;
        else if(FT1<=4.50155)tmp1=-54.8;
        else if(FT1<=4.51507)tmp1=-54.7;
        else if(FT1<=4.52861)tmp1=-54.6;
        else if(FT1<=4.54218)tmp1=-54.5;
        else if(FT1<=4.55578)tmp1=-54.4;
        else if(FT1<=4.56941)tmp1=-54.3;
        else if(FT1<=4.58306)tmp1=-54.2;
        else if(FT1<=4.59675)tmp1=-54.1;
    }
    else if(FT1<=4.73521){
        if(FT1==4.61047)tmp1=-54;
        else if(FT1<=4.62421)tmp1=-53.9;
        else if(FT1<=4.63798)tmp1=-53.8;
        else if(FT1<=4.65178)tmp1=-53.7;
        else if(FT1<=4.66562)tmp1=-53.6;
        else if(FT1<=4.67948)tmp1=-53.5;
        else if(FT1<=4.69336)tmp1=-53.4;
        else if(FT1<=4.70728)tmp1=-53.3;
        else if(FT1<=4.72123)tmp1=-53.2;
        else if(FT1<=4.73521)tmp1=-53.1;
    }
    else if(FT1<=4.87658){
        if(FT1==4.74921)tmp1=-53;
        else if(FT1<=4.76325)tmp1=-52.9;
        else if(FT1<=4.77731)tmp1=-52.8;
        else if(FT1<=4.7914)tmp1=-52.7;
        else if(FT1<=4.80553)tmp1=-52.6;
        else if(FT1<=4.81968)tmp1=-52.5;
        else if(FT1<=4.83386)tmp1=-52.4;
        else if(FT1<=4.84807)tmp1=-52.3;
        else if(FT1<=4.86231)tmp1=-52.2;
        else if(FT1<=4.87658)tmp1=-52.1;
    }
    else if(FT1<=5.0209){
        if(FT1==4.89088)tmp1=-52;
        else if(FT1<=4.90521)tmp1=-51.9;
        else if(FT1<=4.91957)tmp1=-51.8;
        else if(FT1<=4.93395)tmp1=-51.7;
        else if(FT1<=4.94837)tmp1=-51.6;
        else if(FT1<=4.96282)tmp1=-51.5;
        else if(FT1<=4.97729)tmp1=-51.4;
        else if(FT1<=4.9918)tmp1=-51.3;
        else if(FT1<=5.00634)tmp1=-51.2;
        else if(FT1<=5.0209)tmp1=-51.1;
    }
    else if(FT1<=5.1682){
        if(FT1==5.0355)tmp1=-51;
        else if(FT1<=5.05012)tmp1=-50.9;
        else if(FT1<=5.06478)tmp1=-50.8;
        else if(FT1<=5.07946)tmp1=-50.7;
        else if(FT1<=5.09418)tmp1=-50.6;
        else if(FT1<=5.10892)tmp1=-50.5;
        else if(FT1<=5.1237)tmp1=-50.4;
        else if(FT1<=5.1385)tmp1=-50.3;
        else if(FT1<=5.15334)tmp1=-50.2;
        else if(FT1<=5.1682)tmp1=-50.1;
    }
    else if(FT1<=5.31851){
        if(FT1==5.1831)tmp1=-50;
        else if(FT1<=5.19802)tmp1=-49.9;
        else if(FT1<=5.21298)tmp1=-49.8;
        else if(FT1<=5.22796)tmp1=-49.7;
        else if(FT1<=5.24298)tmp1=-49.6;
        else if(FT1<=5.25802)tmp1=-49.5;
        else if(FT1<=5.2731)tmp1=-49.4;
        else if(FT1<=5.28821)tmp1=-49.3;
        else if(FT1<=5.30334)tmp1=-49.2;
        else if(FT1<=5.31851)tmp1=-49.1;
    }
    else if(FT1<=5.47186){
        if(FT1==5.33371)tmp1=-49;
        else if(FT1<=5.34894)tmp1=-48.9;
        else if(FT1<=5.3642)tmp1=-48.8;
        else if(FT1<=5.37948)tmp1=-48.7;
        else if(FT1<=5.3948)tmp1=-48.6;
        else if(FT1<=5.41015)tmp1=-48.5;
        else if(FT1<=5.42553)tmp1=-48.4;
        else if(FT1<=5.44094)tmp1=-48.3;
        else if(FT1<=5.45639)tmp1=-48.2;
        else if(FT1<=5.47186)tmp1=-48.1;
    }
    else if(FT1<=5.62827){
        if(FT1==5.48736)tmp1=-48;
        else if(FT1<=5.5029)tmp1=-47.9;
        else if(FT1<=5.51846)tmp1=-47.8;
        else if(FT1<=5.53405)tmp1=-47.7;
        else if(FT1<=5.54968)tmp1=-47.6;
        else if(FT1<=5.56534)tmp1=-47.5;
        else if(FT1<=5.58103)tmp1=-47.4;
        else if(FT1<=5.59674)tmp1=-47.3;
        else if(FT1<=5.61249)tmp1=-47.2;
        else if(FT1<=5.62827)tmp1=-47.1;
    }
    else if(FT1<=5.78779){
        if(FT1==5.64408)tmp1=-47;
        else if(FT1<=5.65993)tmp1=-46.9;
        else if(FT1<=5.6758)tmp1=-46.8;
        else if(FT1<=5.69171)tmp1=-46.7;
        else if(FT1<=5.70764)tmp1=-46.6;
        else if(FT1<=5.72361)tmp1=-46.5;
        else if(FT1<=5.7396)tmp1=-46.4;
        else if(FT1<=5.75563)tmp1=-46.3;
        else if(FT1<=5.77169)tmp1=-46.2;
        else if(FT1<=5.78779)tmp1=-46.1;
    }
    else if(FT1<=5.95042){
        if(FT1==5.80391)tmp1=-46;
        else if(FT1<=5.82006)tmp1=-45.9;
        else if(FT1<=5.83625)tmp1=-45.8;
        else if(FT1<=5.85246)tmp1=-45.7;
        else if(FT1<=5.86871)tmp1=-45.6;
        else if(FT1<=5.88499)tmp1=-45.5;
        else if(FT1<=5.9013)tmp1=-45.4;
        else if(FT1<=5.91764)tmp1=-45.3;
        else if(FT1<=5.93402)tmp1=-45.2;
        else if(FT1<=5.95042)tmp1=-45.1;
    }
    else if(FT1<=6.11621){
        if(FT1==5.96686)tmp1=-45;
        else if(FT1<=5.98333)tmp1=-44.9;
        else if(FT1<=5.99983)tmp1=-44.8;
        else if(FT1<=6.01636)tmp1=-44.7;
        else if(FT1<=6.03292)tmp1=-44.6;
        else if(FT1<=6.04952)tmp1=-44.5;
        else if(FT1<=6.06614)tmp1=-44.4;
        else if(FT1<=6.0828)tmp1=-44.3;
        else if(FT1<=6.09949)tmp1=-44.2;
        else if(FT1<=6.11621)tmp1=-44.1;
    }
    else if(FT1<=6.28519){
        if(FT1==6.13297)tmp1=-44;
        else if(FT1<=6.14975)tmp1=-43.9;
        else if(FT1<=6.16657)tmp1=-43.8;
        else if(FT1<=6.18342)tmp1=-43.7;
        else if(FT1<=6.2003)tmp1=-43.6;
        else if(FT1<=6.21722)tmp1=-43.5;
        else if(FT1<=6.23416)tmp1=-43.4;
        else if(FT1<=6.25114)tmp1=-43.3;
        else if(FT1<=6.26815)tmp1=-43.2;
        else if(FT1<=6.28519)tmp1=-43.1;
    }
    else if(FT1<=6.45738){
        if(FT1==6.30226)tmp1=-43;
        else if(FT1<=6.31937)tmp1=-42.9;
        else if(FT1<=6.33651)tmp1=-42.8;
        else if(FT1<=6.35368)tmp1=-42.7;
        else if(FT1<=6.37088)tmp1=-42.6;
        else if(FT1<=6.38811)tmp1=-42.5;
        else if(FT1<=6.40538)tmp1=-42.4;
        else if(FT1<=6.42268)tmp1=-42.3;
        else if(FT1<=6.44001)tmp1=-42.2;
        else if(FT1<=6.45738)tmp1=-42.1;
    }
    else if(FT1<=6.6328){
        if(FT1==6.47477)tmp1=-42;
        else if(FT1<=6.4922)tmp1=-41.9;
        else if(FT1<=6.50966)tmp1=-41.8;
        else if(FT1<=6.52716)tmp1=-41.7;
        else if(FT1<=6.54468)tmp1=-41.6;
        else if(FT1<=6.56224)tmp1=-41.5;
        else if(FT1<=6.57983)tmp1=-41.4;
        else if(FT1<=6.59746)tmp1=-41.3;
        else if(FT1<=6.61511)tmp1=-41.2;
        else if(FT1<=6.6328)tmp1=-41.1;
    }
    else if(FT1<=6.81149){
        if(FT1==6.65052)tmp1=-41;
        else if(FT1<=6.66828)tmp1=-40.9;
        else if(FT1<=6.68606)tmp1=-40.8;
        else if(FT1<=6.70388)tmp1=-40.7;
        else if(FT1<=6.72174)tmp1=-40.6;
        else if(FT1<=6.73962)tmp1=-40.5;
        else if(FT1<=6.75754)tmp1=-40.4;
        else if(FT1<=6.77549)tmp1=-40.3;
        else if(FT1<=6.79348)tmp1=-40.2;
        else if(FT1<=6.81149)tmp1=-40.1;
    }
    else if(FT1<=6.99348){
        if(FT1==6.82954)tmp1=-40;
        else if(FT1<=6.84763)tmp1=-39.9;
        else if(FT1<=6.86574)tmp1=-39.8;
        else if(FT1<=6.88389)tmp1=-39.7;
        else if(FT1<=6.90207)tmp1=-39.6;
        else if(FT1<=6.92029)tmp1=-39.5;
        else if(FT1<=6.93854)tmp1=-39.4;
        else if(FT1<=6.95682)tmp1=-39.3;
        else if(FT1<=6.97513)tmp1=-39.2;
        else if(FT1<=6.99348)tmp1=-39.1;
    }
    else if(FT1<=7.17879){
        if(FT1==7.01186)tmp1=-39;
        else if(FT1<=7.03028)tmp1=-38.9;
        else if(FT1<=7.04872)tmp1=-38.8;
        else if(FT1<=7.0672)tmp1=-38.7;
        else if(FT1<=7.08572)tmp1=-38.6;
        else if(FT1<=7.10427)tmp1=-38.5;
        else if(FT1<=7.12285)tmp1=-38.4;
        else if(FT1<=7.14146)tmp1=-38.3;
        else if(FT1<=7.16011)tmp1=-38.2;
        else if(FT1<=7.17879)tmp1=-38.1;
    }
    else if(FT1<=7.36745){
        if(FT1==7.1975)tmp1=-38;
        else if(FT1<=7.21625)tmp1=-37.9;
        else if(FT1<=7.23503)tmp1=-37.8;
        else if(FT1<=7.25385)tmp1=-37.7;
        else if(FT1<=7.2727)tmp1=-37.6;
        else if(FT1<=7.29158)tmp1=-37.5;
        else if(FT1<=7.3105)tmp1=-37.4;
        else if(FT1<=7.32944)tmp1=-37.3;
        else if(FT1<=7.34843)tmp1=-37.2;
        else if(FT1<=7.36745)tmp1=-37.1;
    }
    else if(FT1<=7.55948){
        if(FT1==7.3865)tmp1=-37;
        else if(FT1<=7.40558)tmp1=-36.9;
        else if(FT1<=7.4247)tmp1=-36.8;
        else if(FT1<=7.44385)tmp1=-36.7;
        else if(FT1<=7.46304)tmp1=-36.6;
        else if(FT1<=7.48226)tmp1=-36.5;
        else if(FT1<=7.50151)tmp1=-36.4;
        else if(FT1<=7.5208)tmp1=-36.3;
        else if(FT1<=7.54012)tmp1=-36.2;
        else if(FT1<=7.55948)tmp1=-36.1;
    }
    else if(FT1<=7.75491){
        if(FT1==7.57887)tmp1=-36;
        else if(FT1<=7.59829)tmp1=-35.9;
        else if(FT1<=7.61775)tmp1=-35.8;
        else if(FT1<=7.63724)tmp1=-35.7;
        else if(FT1<=7.65677)tmp1=-35.6;
        else if(FT1<=7.67633)tmp1=-35.5;
        else if(FT1<=7.69592)tmp1=-35.4;
        else if(FT1<=7.71555)tmp1=-35.3;
        else if(FT1<=7.73521)tmp1=-35.2;
        else if(FT1<=7.75491)tmp1=-35.1;
    }
    else if(FT1<=7.95377){
        if(FT1==7.77464)tmp1=-35;
        else if(FT1<=7.79441)tmp1=-34.9;
        else if(FT1<=7.81421)tmp1=-34.8;
        else if(FT1<=7.83404)tmp1=-34.7;
        else if(FT1<=7.85391)tmp1=-34.6;
        else if(FT1<=7.87381)tmp1=-34.5;
        else if(FT1<=7.89375)tmp1=-34.4;
        else if(FT1<=7.91372)tmp1=-34.3;
        else if(FT1<=7.93373)tmp1=-34.2;
        else if(FT1<=7.95377)tmp1=-34.1;
    }
    else if(FT1<=8.15609){
        if(FT1==7.97385)tmp1=-34;
        else if(FT1<=7.99396)tmp1=-33.9;
        else if(FT1<=8.0141)tmp1=-33.8;
        else if(FT1<=8.03428)tmp1=-33.7;
        else if(FT1<=8.0545)tmp1=-33.6;
        else if(FT1<=8.07474)tmp1=-33.5;
        else if(FT1<=8.09503)tmp1=-33.4;
        else if(FT1<=8.11535)tmp1=-33.3;
        else if(FT1<=8.1357)tmp1=-33.2;
        else if(FT1<=8.15609)tmp1=-33.1;
    }
    else if(FT1<=8.36188){
        if(FT1==8.17651)tmp1=-33;
        else if(FT1<=8.19697)tmp1=-32.9;
        else if(FT1<=8.21746)tmp1=-32.8;
        else if(FT1<=8.23798)tmp1=-32.7;
        else if(FT1<=8.25855)tmp1=-32.6;
        else if(FT1<=8.27914)tmp1=-32.5;
        else if(FT1<=8.29978)tmp1=-32.4;
        else if(FT1<=8.32044)tmp1=-32.3;
        else if(FT1<=8.34114)tmp1=-32.2;
        else if(FT1<=8.36188)tmp1=-32.1;
    }
    else if(FT1<=8.57118){
        if(FT1==8.38265)tmp1=-32;
        else if(FT1<=8.40346)tmp1=-31.9;
        else if(FT1<=8.4243)tmp1=-31.8;
        else if(FT1<=8.44518)tmp1=-31.7;
        else if(FT1<=8.46609)tmp1=-31.6;
        else if(FT1<=8.48704)tmp1=-31.5;
        else if(FT1<=8.50802)tmp1=-31.4;
        else if(FT1<=8.52904)tmp1=-31.3;
        else if(FT1<=8.55009)tmp1=-31.2;
        else if(FT1<=8.57118)tmp1=-31.1;
    }
    else if(FT1<=8.78401){
        if(FT1==8.5923)tmp1=-31;
        else if(FT1<=8.61346)tmp1=-30.9;
        else if(FT1<=8.63465)tmp1=-30.8;
        else if(FT1<=8.65588)tmp1=-30.7;
        else if(FT1<=8.67715)tmp1=-30.6;
        else if(FT1<=8.69845)tmp1=-30.5;
        else if(FT1<=8.71979)tmp1=-30.4;
        else if(FT1<=8.74116)tmp1=-30.3;
        else if(FT1<=8.76256)tmp1=-30.2;
        else if(FT1<=8.78401)tmp1=-30.1;
    }
    else if(FT1<=9.00039){
        if(FT1==8.80548)tmp1=-30;
        else if(FT1<=8.827)tmp1=-29.9;
        else if(FT1<=8.84855)tmp1=-29.8;
        else if(FT1<=8.87013)tmp1=-29.7;
        else if(FT1<=8.89175)tmp1=-29.6;
        else if(FT1<=8.91341)tmp1=-29.5;
        else if(FT1<=8.9351)tmp1=-29.4;
        else if(FT1<=8.95682)tmp1=-29.3;
        else if(FT1<=8.97859)tmp1=-29.2;
        else if(FT1<=9.00039)tmp1=-29.1;
    }
    else if(FT1<=9.22035){
        if(FT1==9.02222)tmp1=-29;
        else if(FT1<=9.04409)tmp1=-28.9;
        else if(FT1<=9.066)tmp1=-28.8;
        else if(FT1<=9.08794)tmp1=-28.7;
        else if(FT1<=9.10992)tmp1=-28.6;
        else if(FT1<=9.13193)tmp1=-28.5;
        else if(FT1<=9.15398)tmp1=-28.4;
        else if(FT1<=9.17607)tmp1=-28.3;
        else if(FT1<=9.19819)tmp1=-28.2;
        else if(FT1<=9.22035)tmp1=-28.1;
    }
    else if(FT1<=9.44391){
        if(FT1==9.24254)tmp1=-28;
        else if(FT1<=9.26477)tmp1=-27.9;
        else if(FT1<=9.28704)tmp1=-27.8;
        else if(FT1<=9.30934)tmp1=-27.7;
        else if(FT1<=9.33168)tmp1=-27.6;
        else if(FT1<=9.35405)tmp1=-27.5;
        else if(FT1<=9.37646)tmp1=-27.4;
        else if(FT1<=9.39891)tmp1=-27.3;
        else if(FT1<=9.42139)tmp1=-27.2;
        else if(FT1<=9.44391)tmp1=-27.1;
    }
    else if(FT1<=9.6711){
        if(FT1==9.46646)tmp1=-27;
        else if(FT1<=9.48906)tmp1=-26.9;
        else if(FT1<=9.51168)tmp1=-26.8;
        else if(FT1<=9.53435)tmp1=-26.7;
        else if(FT1<=9.55705)tmp1=-26.6;
        else if(FT1<=9.57979)tmp1=-26.5;
        else if(FT1<=9.60256)tmp1=-26.4;
        else if(FT1<=9.62537)tmp1=-26.3;
        else if(FT1<=9.64822)tmp1=-26.2;
        else if(FT1<=9.6711)tmp1=-26.1;
    }
    else if(FT1<=9.90194){
        if(FT1==9.69402)tmp1=-26;
        else if(FT1<=9.71697)tmp1=-25.9;
        else if(FT1<=9.73997)tmp1=-25.8;
        else if(FT1<=9.76299)tmp1=-25.7;
        else if(FT1<=9.78606)tmp1=-25.6;
        else if(FT1<=9.80916)tmp1=-25.5;
        else if(FT1<=9.8323)tmp1=-25.4;
        else if(FT1<=9.85548)tmp1=-25.3;
        else if(FT1<=9.87869)tmp1=-25.2;
        else if(FT1<=9.90194)tmp1=-25.1;
    }
    else if(FT1<=10.1364){
        if(FT1==9.92522)tmp1=-25;
        else if(FT1<=9.94854)tmp1=-24.9;
        else if(FT1<=9.9719)tmp1=-24.8;
        else if(FT1<=9.9953)tmp1=-24.7;
        else if(FT1<=10.0187)tmp1=-24.6;
        else if(FT1<=10.0422)tmp1=-24.5;
        else if(FT1<=10.0657)tmp1=-24.4;
        else if(FT1<=10.0893)tmp1=-24.3;
        else if(FT1<=10.1128)tmp1=-24.2;
        else if(FT1<=10.1364)tmp1=-24.1;
    }
    else if(FT1<=10.3747){
        if(FT1==10.1601)tmp1=-24;
        else if(FT1<=10.1838)tmp1=-23.9;
        else if(FT1<=10.2075)tmp1=-23.8;
        else if(FT1<=10.2313)tmp1=-23.7;
        else if(FT1<=10.2551)tmp1=-23.6;
        else if(FT1<=10.2789)tmp1=-23.5;
        else if(FT1<=10.3028)tmp1=-23.4;
        else if(FT1<=10.3267)tmp1=-23.3;
        else if(FT1<=10.3507)tmp1=-23.2;
        else if(FT1<=10.3747)tmp1=-23.1;
    }
    else if(FT1<=10.6166){
        if(FT1==10.3987)tmp1=-23;
        else if(FT1<=10.4227)tmp1=-22.9;
        else if(FT1<=10.4468)tmp1=-22.8;
        else if(FT1<=10.471)tmp1=-22.7;
        else if(FT1<=10.4952)tmp1=-22.6;
        else if(FT1<=10.5194)tmp1=-22.5;
        else if(FT1<=10.5436)tmp1=-22.4;
        else if(FT1<=10.5679)tmp1=-22.3;
        else if(FT1<=10.5922)tmp1=-22.2;
        else if(FT1<=10.6166)tmp1=-22.1;
    }
    else if(FT1<=10.8623){
        if(FT1==10.641)tmp1=-22;
        else if(FT1<=10.6654)tmp1=-21.9;
        else if(FT1<=10.6899)tmp1=-21.8;
        else if(FT1<=10.7144)tmp1=-21.7;
        else if(FT1<=10.739)tmp1=-21.6;
        else if(FT1<=10.7635)tmp1=-21.5;
        else if(FT1<=10.7882)tmp1=-21.4;
        else if(FT1<=10.8128)tmp1=-21.3;
        else if(FT1<=10.8375)tmp1=-21.2;
        else if(FT1<=10.8623)tmp1=-21.1;
    }
    else if(FT1<=11.1117){
        if(FT1==10.887)tmp1=-21;
        else if(FT1<=10.9118)tmp1=-20.9;
        else if(FT1<=10.9367)tmp1=-20.8;
        else if(FT1<=10.9616)tmp1=-20.7;
        else if(FT1<=10.9865)tmp1=-20.6;
        else if(FT1<=11.0115)tmp1=-20.5;
        else if(FT1<=11.0365)tmp1=-20.4;
        else if(FT1<=11.0615)tmp1=-20.3;
        else if(FT1<=11.0866)tmp1=-20.2;
        else if(FT1<=11.1117)tmp1=-20.1;
    }
    else if(FT1<=11.3649){
        if(FT1==11.1368)tmp1=-20;
        else if(FT1<=11.162)tmp1=-19.9;
        else if(FT1<=11.1873)tmp1=-19.8;
        else if(FT1<=11.2125)tmp1=-19.7;
        else if(FT1<=11.2378)tmp1=-19.6;
        else if(FT1<=11.2632)tmp1=-19.5;
        else if(FT1<=11.2885)tmp1=-19.4;
        else if(FT1<=11.314)tmp1=-19.3;
        else if(FT1<=11.3394)tmp1=-19.2;
        else if(FT1<=11.3649)tmp1=-19.1;
    }
    else if(FT1<=11.6219){
        if(FT1==11.3904)tmp1=-19;
        else if(FT1<=11.416)tmp1=-18.9;
        else if(FT1<=11.4416)tmp1=-18.8;
        else if(FT1<=11.4673)tmp1=-18.7;
        else if(FT1<=11.4929)tmp1=-18.6;
        else if(FT1<=11.5187)tmp1=-18.5;
        else if(FT1<=11.5444)tmp1=-18.4;
        else if(FT1<=11.5702)tmp1=-18.3;
        else if(FT1<=11.5961)tmp1=-18.2;
        else if(FT1<=11.6219)tmp1=-18.1;
    }
    else if(FT1<=11.8828){
        if(FT1==11.6478)tmp1=-18;
        else if(FT1<=11.6738)tmp1=-17.9;
        else if(FT1<=11.6998)tmp1=-17.8;
        else if(FT1<=11.7258)tmp1=-17.7;
        else if(FT1<=11.7519)tmp1=-17.6;
        else if(FT1<=11.778)tmp1=-17.5;
        else if(FT1<=11.8041)tmp1=-17.4;
        else if(FT1<=11.8303)tmp1=-17.3;
        else if(FT1<=11.8565)tmp1=-17.2;
        else if(FT1<=11.8828)tmp1=-17.1;
    }
    else if(FT1<=12.1475){
        if(FT1==11.9091)tmp1=-17;
        else if(FT1<=11.9354)tmp1=-16.9;
        else if(FT1<=11.9618)tmp1=-16.8;
        else if(FT1<=11.9882)tmp1=-16.7;
        else if(FT1<=12.0146)tmp1=-16.6;
        else if(FT1<=12.0411)tmp1=-16.5;
        else if(FT1<=12.0677)tmp1=-16.4;
        else if(FT1<=12.0942)tmp1=-16.3;
        else if(FT1<=12.1208)tmp1=-16.2;
        else if(FT1<=12.1475)tmp1=-16.1;
    }
    else if(FT1<=12.416){
        if(FT1==12.1742)tmp1=-16;
        else if(FT1<=12.2009)tmp1=-15.9;
        else if(FT1<=12.2276)tmp1=-15.8;
        else if(FT1<=12.2544)tmp1=-15.7;
        else if(FT1<=12.2813)tmp1=-15.6;
        else if(FT1<=12.3082)tmp1=-15.5;
        else if(FT1<=12.3351)tmp1=-15.4;
        else if(FT1<=12.362)tmp1=-15.3;
        else if(FT1<=12.389)tmp1=-15.2;
        else if(FT1<=12.416)tmp1=-15.1;
        else if(FT1<=12.4431)tmp1=-15;
    }
    else if(FT1<=12.6885){
        if(FT1==12.4431)tmp1=-15;
        else if(FT1<=12.4702)tmp1=-14.9;
        else if(FT1<=12.4974)tmp1=-14.8;
        else if(FT1<=12.5246)tmp1=-14.7;
        else if(FT1<=12.5518)tmp1=-14.6;
        else if(FT1<=12.5791)tmp1=-14.5;
        else if(FT1<=12.6064)tmp1=-14.4;
        else if(FT1<=12.6337)tmp1=-14.3;
        else if(FT1<=12.6611)tmp1=-14.2;
        else if(FT1<=12.6885)tmp1=-14.1;
        else if(FT1<=12.716)tmp1=-14;
    }
    else if(FT1<=12.9649){
        if(FT1==12.716)tmp1=-14;
        else if(FT1<=12.7435)tmp1=-13.9;
        else if(FT1<=12.771)tmp1=-13.8;
        else if(FT1<=12.7986)tmp1=-13.7;
        else if(FT1<=12.8262)tmp1=-13.6;
        else if(FT1<=12.8539)tmp1=-13.5;
        else if(FT1<=12.8816)tmp1=-13.4;
        else if(FT1<=12.9093)tmp1=-13.3;
        else if(FT1<=12.9371)tmp1=-13.2;
        else if(FT1<=12.9649)tmp1=-13.1;
        else if(FT1<=12.9927)tmp1=-13;
    }
    else if(FT1<=13.2452){
        if(FT1==12.9927)tmp1=-13;
        else if(FT1<=13.0206)tmp1=-12.9;
        else if(FT1<=13.0485)tmp1=-12.8;
        else if(FT1<=13.0765)tmp1=-12.7;
        else if(FT1<=13.1045)tmp1=-12.6;
        else if(FT1<=13.1326)tmp1=-12.5;
        else if(FT1<=13.1607)tmp1=-12.4;
        else if(FT1<=13.1888)tmp1=-12.3;
        else if(FT1<=13.217)tmp1=-12.2;
        else if(FT1<=13.2452)tmp1=-12.1;
        else if(FT1<=13.2734)tmp1=-12;
    }
    else if(FT1<=13.5294){
        if(FT1==13.2734)tmp1=-12;
        else if(FT1<=13.3017)tmp1=-11.9;
        else if(FT1<=13.33)tmp1=-11.8;
        else if(FT1<=13.3584)tmp1=-11.7;
        else if(FT1<=13.3868)tmp1=-11.6;
        else if(FT1<=13.4152)tmp1=-11.5;
        else if(FT1<=13.4437)tmp1=-11.4;
        else if(FT1<=13.4722)tmp1=-11.3;
        else if(FT1<=13.5008)tmp1=-11.2;
        else if(FT1<=13.5294)tmp1=-11.1;
        else if(FT1<=13.5581)tmp1=-11;
    }
    else if(FT1<=13.8176){
        if(FT1==13.5581)tmp1=-11;
        else if(FT1<=13.5867)tmp1=-10.9;
        else if(FT1<=13.6155)tmp1=-10.8;
        else if(FT1<=13.6442)tmp1=-10.7;
        else if(FT1<=13.673)tmp1=-10.6;
        else if(FT1<=13.7019)tmp1=-10.5;
        else if(FT1<=13.7307)tmp1=-10.4;
        else if(FT1<=13.7597)tmp1=-10.3;
        else if(FT1<=13.7886)tmp1=-10.2;
        else if(FT1<=13.8176)tmp1=-10.1;
        else if(FT1<=13.8467)tmp1=-10;
    }
    else if(FT1<=14.1098){
        if(FT1==13.8467)tmp1=-10;
        else if(FT1<=13.8757)tmp1=-9.9;
        else if(FT1<=13.9049)tmp1=-9.8;
        else if(FT1<=13.934)tmp1=-9.7;
        else if(FT1<=13.9632)tmp1=-9.6;
        else if(FT1<=13.9925)tmp1=-9.5;
        else if(FT1<=14.0217)tmp1=-9.4;
        else if(FT1<=14.0511)tmp1=-9.3;
        else if(FT1<=14.0804)tmp1=-9.2;
        else if(FT1<=14.1098)tmp1=-9.1;
        else if(FT1<=14.1393)tmp1=-9;
    }
    else if(FT1<=14.406){
        if(FT1==14.1393)tmp1=-9;
        else if(FT1<=14.1687)tmp1=-8.9;
        else if(FT1<=14.1983)tmp1=-8.8;
        else if(FT1<=14.2278)tmp1=-8.7;
        else if(FT1<=14.2574)tmp1=-8.6;
        else if(FT1<=14.2871)tmp1=-8.5;
        else if(FT1<=14.3167)tmp1=-8.4;
        else if(FT1<=14.3465)tmp1=-8.3;
        else if(FT1<=14.3762)tmp1=-8.2;
        else if(FT1<=14.406)tmp1=-8.1;
        else if(FT1<=14.4359)tmp1=-8;
    }
    else if(FT1<=14.7063){
        if(FT1==14.4359)tmp1=-8;
        else if(FT1<=14.4657)tmp1=-7.9;
        else if(FT1<=14.4957)tmp1=-7.8;
        else if(FT1<=14.5256)tmp1=-7.7;
        else if(FT1<=14.5556)tmp1=-7.6;
        else if(FT1<=14.5857)tmp1=-7.5;
        else if(FT1<=14.6158)tmp1=-7.4;
        else if(FT1<=14.6459)tmp1=-7.3;
        else if(FT1<=14.676)tmp1=-7.2;
        else if(FT1<=14.7063)tmp1=-7.1;
        else if(FT1<=14.7365)tmp1=-7;
    }
    else if(FT1<=15.0105){
        if(FT1==14.7365)tmp1=-7;
        else if(FT1<=14.7668)tmp1=-6.9;
        else if(FT1<=14.7971)tmp1=-6.8;
        else if(FT1<=14.8275)tmp1=-6.7;
        else if(FT1<=14.8579)tmp1=-6.6;
        else if(FT1<=14.8883)tmp1=-6.5;
        else if(FT1<=14.9188)tmp1=-6.4;
        else if(FT1<=14.9493)tmp1=-6.3;
        else if(FT1<=14.9799)tmp1=-6.2;
        else if(FT1<=15.0105)tmp1=-6.1;
        else if(FT1<=15.0412)tmp1=-6;
    }
    else if(FT1<=15.3188){
        if(FT1==15.0412)tmp1=-6;
        else if(FT1<=15.0719)tmp1=-5.9;
        else if(FT1<=15.1026)tmp1=-5.8;
        else if(FT1<=15.1334)tmp1=-5.7;
        else if(FT1<=15.1642)tmp1=-5.6;
        else if(FT1<=15.195)tmp1=-5.5;
        else if(FT1<=15.2259)tmp1=-5.4;
        else if(FT1<=15.2568)tmp1=-5.3;
        else if(FT1<=15.2878)tmp1=-5.2;
        else if(FT1<=15.3188)tmp1=-5.1;
        else if(FT1<=15.3499)tmp1=-5;
    }
    else if(FT1<=15.6312){
        if(FT1==15.3499)tmp1=-5;
        else if(FT1<=15.381)tmp1=-4.9;
        else if(FT1<=15.4121)tmp1=-4.8;
        else if(FT1<=15.4433)tmp1=-4.7;
        else if(FT1<=15.4745)tmp1=-4.6;
        else if(FT1<=15.5058)tmp1=-4.5;
        else if(FT1<=15.5371)tmp1=-4.4;
        else if(FT1<=15.5684)tmp1=-4.3;
        else if(FT1<=15.5998)tmp1=-4.2;
        else if(FT1<=15.6312)tmp1=-4.1;
        else if(FT1<=15.6627)tmp1=-4;
    }
    else if(FT1<=15.9477){
        if(FT1==15.6627)tmp1=-4;
        else if(FT1<=15.6942)tmp1=-3.9;
        else if(FT1<=15.7258)tmp1=-3.8;
        else if(FT1<=15.7573)tmp1=-3.7;
        else if(FT1<=15.789)tmp1=-3.6;
        else if(FT1<=15.8206)tmp1=-3.5;
        else if(FT1<=15.8524)tmp1=-3.4;
        else if(FT1<=15.8841)tmp1=-3.3;
        else if(FT1<=15.9159)tmp1=-3.2;
        else if(FT1<=15.9477)tmp1=-3.1;
    }
    else if(FT1<=16.2683){
        if(FT1==15.9796)tmp1=-3;
        else if(FT1<=16.0115)tmp1=-2.9;
        else if(FT1<=16.0435)tmp1=-2.8;
        else if(FT1<=16.0755)tmp1=-2.7;
        else if(FT1<=16.1075)tmp1=-2.6;
        else if(FT1<=16.1396)tmp1=-2.5;
        else if(FT1<=16.1717)tmp1=-2.4;
        else if(FT1<=16.2039)tmp1=-2.3;
        else if(FT1<=16.2361)tmp1=-2.2;
        else if(FT1<=16.2683)tmp1=-2.1;
    }
    else if(FT1<=16.5931){
        if(FT1==16.3006)tmp1=-2;
        else if(FT1<=16.3329)tmp1=-1.9;
        else if(FT1<=16.3653)tmp1=-1.8;
        else if(FT1<=16.3977)tmp1=-1.7;
        else if(FT1<=16.4302)tmp1=-1.6;
        else if(FT1<=16.4627)tmp1=-1.5;
        else if(FT1<=16.4952)tmp1=-1.4;
        else if(FT1<=16.5278)tmp1=-1.3;
        else if(FT1<=16.5604)tmp1=-1.2;
        else if(FT1<=16.5931)tmp1=-1.1;
    }
    else if(FT1<=16.9219){
        if(FT1==16.6258)tmp1=-1;
        else if(FT1<=16.6585)tmp1=-0.9;
        else if(FT1<=16.6913)tmp1=-0.8;
        else if(FT1<=16.7241)tmp1=-0.7;
        else if(FT1<=16.757)tmp1=-0.6;
        else if(FT1<=16.7899)tmp1=-0.5;
        else if(FT1<=16.8228)tmp1=-0.4;
        else if(FT1<=16.8558)tmp1=-0.3;
        else if(FT1<=16.8888)tmp1=-0.2;
        else if(FT1<=16.9219)tmp1=-0.1;
        else if(FT1<=16.955)tmp1=-1.38778e-16;
    }
    else if(FT1<=17.2549){
        if(FT1==16.955)tmp1=0;
        else if(FT1<=16.9882)tmp1=0.1;
        else if(FT1<=17.0214)tmp1=0.2;
        else if(FT1<=17.0546)tmp1=0.3;
        else if(FT1<=17.0879)tmp1=0.4;
        else if(FT1<=17.1212)tmp1=0.5;
        else if(FT1<=17.1546)tmp1=0.6;
        else if(FT1<=17.188)tmp1=0.7;
        else if(FT1<=17.2215)tmp1=0.8;
        else if(FT1<=17.2549)tmp1=0.9;
        else if(FT1<=17.2885)tmp1=1;
    }
    else if(FT1<=17.5921){
        if(FT1==17.2885)tmp1=1;
        else if(FT1<=17.322)tmp1=1.1;
        else if(FT1<=17.3557)tmp1=1.2;
        else if(FT1<=17.3893)tmp1=1.3;
        else if(FT1<=17.423)tmp1=1.4;
        else if(FT1<=17.4568)tmp1=1.5;
        else if(FT1<=17.4905)tmp1=1.6;
        else if(FT1<=17.5244)tmp1=1.7;
        else if(FT1<=17.5582)tmp1=1.8;
        else if(FT1<=17.5921)tmp1=1.9;
    }
    else if(FT1<=17.9335){
        if(FT1==17.6261)tmp1=2;
        else if(FT1<=17.6601)tmp1=2.1;
        else if(FT1<=17.6941)tmp1=2.2;
        else if(FT1<=17.7282)tmp1=2.3;
        else if(FT1<=17.7623)tmp1=2.4;
        else if(FT1<=17.7965)tmp1=2.5;
        else if(FT1<=17.8307)tmp1=2.6;
        else if(FT1<=17.8649)tmp1=2.7;
        else if(FT1<=17.8992)tmp1=2.8;
        else if(FT1<=17.9335)tmp1=2.9;
    }
    else if(FT1<=18.2791){
        if(FT1==17.9679)tmp1=3;
        else if(FT1<=18.0023)tmp1=3.1;
        else if(FT1<=18.0368)tmp1=3.2;
        else if(FT1<=18.0712)tmp1=3.3;
        else if(FT1<=18.1058)tmp1=3.4;
        else if(FT1<=18.1404)tmp1=3.5;
        else if(FT1<=18.175)tmp1=3.6;
        else if(FT1<=18.2096)tmp1=3.7;
        else if(FT1<=18.2444)tmp1=3.8;
        else if(FT1<=18.2791)tmp1=3.9;
    }
    else if(FT1<=18.6289){
        if(FT1==18.3139)tmp1=4;
        else if(FT1<=18.3487)tmp1=4.1;
        else if(FT1<=18.3836)tmp1=4.2;
        else if(FT1<=18.4185)tmp1=4.3;
        else if(FT1<=18.4535)tmp1=4.4;
        else if(FT1<=18.4885)tmp1=4.5;
        else if(FT1<=18.5235)tmp1=4.6;
        else if(FT1<=18.5586)tmp1=4.7;
        else if(FT1<=18.5937)tmp1=4.8;
        else if(FT1<=18.6289)tmp1=4.9;
        else if(FT1<=18.6641)tmp1=5;
    }
    else if(FT1<=18.9829){
        if(FT1==18.6641)tmp1=5;
        else if(FT1<=18.6994)tmp1=5.1;
        else if(FT1<=18.7347)tmp1=5.2;
        else if(FT1<=18.77)tmp1=5.3;
        else if(FT1<=18.8054)tmp1=5.4;
        else if(FT1<=18.8408)tmp1=5.5;
        else if(FT1<=18.8763)tmp1=5.6;
        else if(FT1<=18.9118)tmp1=5.7;
        else if(FT1<=18.9473)tmp1=5.8;
        else if(FT1<=18.9829)tmp1=5.9;
        else if(FT1<=19.0186)tmp1=6;
    }
    else if(FT1<=19.3412){
        if(FT1==19.0186)tmp1=6;
        else if(FT1<=19.0543)tmp1=6.1;
        else if(FT1<=19.09)tmp1=6.2;
        else if(FT1<=19.1257)tmp1=6.3;
        else if(FT1<=19.1615)tmp1=6.4;
        else if(FT1<=19.1974)tmp1=6.5;
        else if(FT1<=19.2333)tmp1=6.6;
        else if(FT1<=19.2692)tmp1=6.7;
        else if(FT1<=19.3052)tmp1=6.8;
        else if(FT1<=19.3412)tmp1=6.9;
        else if(FT1<=19.3773)tmp1=7;
    }
    else if(FT1<=19.7038){
        if(FT1==19.3773)tmp1=7;
        else if(FT1<=19.4134)tmp1=7.1;
        else if(FT1<=19.4495)tmp1=7.2;
        else if(FT1<=19.4857)tmp1=7.3;
        else if(FT1<=19.522)tmp1=7.4;
        else if(FT1<=19.5582)tmp1=7.5;
        else if(FT1<=19.5945)tmp1=7.6;
        else if(FT1<=19.6309)tmp1=7.7;
        else if(FT1<=19.6673)tmp1=7.8;
        else if(FT1<=19.7038)tmp1=7.9;
        else if(FT1<=19.7402)tmp1=8;
    }
    else if(FT1<=20.0706){
        if(FT1==19.7402)tmp1=8;
        else if(FT1<=19.7768)tmp1=8.1;
        else if(FT1<=19.8133)tmp1=8.2;
        else if(FT1<=19.85)tmp1=8.3;
        else if(FT1<=19.8866)tmp1=8.4;
        else if(FT1<=19.9233)tmp1=8.5;
        else if(FT1<=19.9601)tmp1=8.6;
        else if(FT1<=19.9969)tmp1=8.7;
        else if(FT1<=20.0337)tmp1=8.8;
        else if(FT1<=20.0706)tmp1=8.9;
        else if(FT1<=20.1075)tmp1=9;
    }
    else if(FT1<=20.4417){
        if(FT1==20.1075)tmp1=9;
        else if(FT1<=20.1444)tmp1=9.1;
        else if(FT1<=20.1814)tmp1=9.2;
        else if(FT1<=20.2185)tmp1=9.3;
        else if(FT1<=20.2556)tmp1=9.4;
        else if(FT1<=20.2927)tmp1=9.5;
        else if(FT1<=20.3299)tmp1=9.6;
        else if(FT1<=20.3671)tmp1=9.7;
        else if(FT1<=20.4044)tmp1=9.8;
        else if(FT1<=20.4417)tmp1=9.9;
        else if(FT1<=20.479)tmp1=10;
    }
    else if(FT1<=20.817){
        if(FT1==20.479)tmp1=10;
        else if(FT1<=20.5164)tmp1=10.1;
        else if(FT1<=20.5538)tmp1=10.2;
        else if(FT1<=20.5913)tmp1=10.3;
        else if(FT1<=20.6288)tmp1=10.4;
        else if(FT1<=20.6664)tmp1=10.5;
        else if(FT1<=20.704)tmp1=10.6;
        else if(FT1<=20.7416)tmp1=10.7;
        else if(FT1<=20.7793)tmp1=10.8;
        else if(FT1<=20.817)tmp1=10.9;
        else if(FT1<=20.8548)tmp1=11;
    }
    else if(FT1<=21.1967){
        if(FT1==20.8548)tmp1=11;
        else if(FT1<=20.8926)tmp1=11.1;
        else if(FT1<=20.9305)tmp1=11.2;
        else if(FT1<=20.9684)tmp1=11.3;
        else if(FT1<=21.0064)tmp1=11.4;
        else if(FT1<=21.0444)tmp1=11.5;
        else if(FT1<=21.0824)tmp1=11.6;
        else if(FT1<=21.1205)tmp1=11.7;
        else if(FT1<=21.1586)tmp1=11.8;
        else if(FT1<=21.1967)tmp1=11.9;
        else if(FT1<=21.235)tmp1=12;
    }
    else if(FT1<=21.5808){
        if(FT1==21.235)tmp1=12;
        else if(FT1<=21.2732)tmp1=12.1;
        else if(FT1<=21.3115)tmp1=12.2;
        else if(FT1<=21.3498)tmp1=12.3;
        else if(FT1<=21.3882)tmp1=12.4;
        else if(FT1<=21.4266)tmp1=12.5;
        else if(FT1<=21.4651)tmp1=12.6;
        else if(FT1<=21.5036)tmp1=12.7;
        else if(FT1<=21.5422)tmp1=12.8;
        else if(FT1<=21.5808)tmp1=12.9;
        else if(FT1<=21.6194)tmp1=13;
    }
    else if(FT1<=21.9691){
        if(FT1==21.6194)tmp1=13;
        else if(FT1<=21.6581)tmp1=13.1;
        else if(FT1<=21.6968)tmp1=13.2;
        else if(FT1<=21.7356)tmp1=13.3;
        else if(FT1<=21.7744)tmp1=13.4;
        else if(FT1<=21.8133)tmp1=13.5;
        else if(FT1<=21.8522)tmp1=13.6;
        else if(FT1<=21.8911)tmp1=13.7;
        else if(FT1<=21.9301)tmp1=13.8;
        else if(FT1<=21.9691)tmp1=13.9;
        else if(FT1<=22.0082)tmp1=14;
    }
    else if(FT1<=22.3618){
        if(FT1==22.0082)tmp1=14;
        else if(FT1<=22.0473)tmp1=14.1;
        else if(FT1<=22.0865)tmp1=14.2;
        else if(FT1<=22.1257)tmp1=14.3;
        else if(FT1<=22.1649)tmp1=14.4;
        else if(FT1<=22.2042)tmp1=14.5;
        else if(FT1<=22.2436)tmp1=14.6;
        else if(FT1<=22.283)tmp1=14.7;
        else if(FT1<=22.3224)tmp1=14.8;
        else if(FT1<=22.3618)tmp1=14.9;
        else if(FT1<=22.4014)tmp1=15;
    }
    else if(FT1<=22.7589){
        if(FT1==22.4014)tmp1=15;
        else if(FT1<=22.4409)tmp1=15.1;
        else if(FT1<=22.4805)tmp1=15.2;
        else if(FT1<=22.5201)tmp1=15.3;
        else if(FT1<=22.5598)tmp1=15.4;
        else if(FT1<=22.5996)tmp1=15.5;
        else if(FT1<=22.6393)tmp1=15.6;
        else if(FT1<=22.6791)tmp1=15.7;
        else if(FT1<=22.719)tmp1=15.8;
        else if(FT1<=22.7589)tmp1=15.9;
        else if(FT1<=22.7989)tmp1=16;
    }
    else if(FT1<=23.1603){
        if(FT1==22.7989)tmp1=16;
        else if(FT1<=22.8388)tmp1=16.1;
        else if(FT1<=22.8789)tmp1=16.2;
        else if(FT1<=22.919)tmp1=16.3;
        else if(FT1<=22.9591)tmp1=16.4;
        else if(FT1<=22.9992)tmp1=16.5;
        else if(FT1<=23.0394)tmp1=16.6;
        else if(FT1<=23.0797)tmp1=16.7;
        else if(FT1<=23.12)tmp1=16.8;
        else if(FT1<=23.1603)tmp1=16.9;
    }
    else if(FT1<=23.5662){
        if(FT1==23.2007)tmp1=17;
        else if(FT1<=23.2411)tmp1=17.1;
        else if(FT1<=23.2816)tmp1=17.2;
        else if(FT1<=23.3221)tmp1=17.3;
        else if(FT1<=23.3627)tmp1=17.4;
        else if(FT1<=23.4033)tmp1=17.5;
        else if(FT1<=23.4439)tmp1=17.6;
        else if(FT1<=23.4846)tmp1=17.7;
        else if(FT1<=23.5254)tmp1=17.8;
        else if(FT1<=23.5662)tmp1=17.9;
    }
    else if(FT1<=23.9764){
        if(FT1==23.607)tmp1=18;
        else if(FT1<=23.6478)tmp1=18.1;
        else if(FT1<=23.6888)tmp1=18.2;
        else if(FT1<=23.7297)tmp1=18.3;
        else if(FT1<=23.7707)tmp1=18.4;
        else if(FT1<=23.8117)tmp1=18.5;
        else if(FT1<=23.8528)tmp1=18.6;
        else if(FT1<=23.894)tmp1=18.7;
        else if(FT1<=23.9351)tmp1=18.8;
        else if(FT1<=23.9764)tmp1=18.9;
    }
    else if(FT1<=24.391){
        if(FT1==24.0176)tmp1=19;
        else if(FT1<=24.0589)tmp1=19.1;
        else if(FT1<=24.1003)tmp1=19.2;
        else if(FT1<=24.1417)tmp1=19.3;
        else if(FT1<=24.1831)tmp1=19.4;
        else if(FT1<=24.2246)tmp1=19.5;
        else if(FT1<=24.2661)tmp1=19.6;
        else if(FT1<=24.3077)tmp1=19.7;
        else if(FT1<=24.3493)tmp1=19.8;
        else if(FT1<=24.391)tmp1=19.9;
    }
    else if(FT1<=24.81){
        if(FT1==24.4327)tmp1=20;
        else if(FT1<=24.4744)tmp1=20.1;
        else if(FT1<=24.5162)tmp1=20.2;
        else if(FT1<=24.558)tmp1=20.3;
        else if(FT1<=24.5999)tmp1=20.4;
        else if(FT1<=24.6418)tmp1=20.5;
        else if(FT1<=24.6838)tmp1=20.6;
        else if(FT1<=24.7258)tmp1=20.7;
        else if(FT1<=24.7679)tmp1=20.8;
        else if(FT1<=24.81)tmp1=20.9;
    }
    else if(FT1<=25.2334){
        if(FT1==24.8521)tmp1=21;
        else if(FT1<=24.8943)tmp1=21.1;
        else if(FT1<=24.9365)tmp1=21.2;
        else if(FT1<=24.9788)tmp1=21.3;
        else if(FT1<=25.0211)tmp1=21.4;
        else if(FT1<=25.0635)tmp1=21.5;
        else if(FT1<=25.1059)tmp1=21.6;
        else if(FT1<=25.1484)tmp1=21.7;
        else if(FT1<=25.1909)tmp1=21.8;
        else if(FT1<=25.2334)tmp1=21.9;
    }
    else if(FT1<=25.6613){
        if(FT1==25.276)tmp1=22;
        else if(FT1<=25.3186)tmp1=22.1;
        else if(FT1<=25.3613)tmp1=22.2;
        else if(FT1<=25.404)tmp1=22.3;
        else if(FT1<=25.4468)tmp1=22.4;
        else if(FT1<=25.4896)tmp1=22.5;
        else if(FT1<=25.5324)tmp1=22.6;
        else if(FT1<=25.5753)tmp1=22.7;
        else if(FT1<=25.6183)tmp1=22.8;
        else if(FT1<=25.6613)tmp1=22.9;
    }
    else if(FT1<=26.0936){
        if(FT1==25.7043)tmp1=23;
        else if(FT1<=25.7474)tmp1=23.1;
        else if(FT1<=25.7905)tmp1=23.2;
        else if(FT1<=25.8337)tmp1=23.3;
        else if(FT1<=25.8769)tmp1=23.4;
        else if(FT1<=25.9201)tmp1=23.5;
        else if(FT1<=25.9634)tmp1=23.6;
        else if(FT1<=26.0068)tmp1=23.7;
        else if(FT1<=26.0501)tmp1=23.8;
        else if(FT1<=26.0936)tmp1=23.9;
    }
    else if(FT1<=26.5303){
        if(FT1==26.137)tmp1=24;
        else if(FT1<=26.1806)tmp1=24.1;
        else if(FT1<=26.2241)tmp1=24.2;
        else if(FT1<=26.2677)tmp1=24.3;
        else if(FT1<=26.3114)tmp1=24.4;
        else if(FT1<=26.3551)tmp1=24.5;
        else if(FT1<=26.3988)tmp1=24.6;
        else if(FT1<=26.4426)tmp1=24.7;
        else if(FT1<=26.4865)tmp1=24.8;
        else if(FT1<=26.5303)tmp1=24.9;
    }
    else if(FT1<=26.9715){
        if(FT1==26.5742)tmp1=25;
        else if(FT1<=26.6182)tmp1=25.1;
        else if(FT1<=26.6622)tmp1=25.2;
        else if(FT1<=26.7063)tmp1=25.3;
        else if(FT1<=26.7504)tmp1=25.4;
        else if(FT1<=26.7945)tmp1=25.5;
        else if(FT1<=26.8387)tmp1=25.6;
        else if(FT1<=26.8829)tmp1=25.7;
        else if(FT1<=26.9272)tmp1=25.8;
        else if(FT1<=26.9715)tmp1=25.9;
    }
    else if(FT1<=27.4172){
        if(FT1==27.0159)tmp1=26;
        else if(FT1<=27.0603)tmp1=26.1;
        else if(FT1<=27.1048)tmp1=26.2;
        else if(FT1<=27.1493)tmp1=26.3;
        else if(FT1<=27.1938)tmp1=26.4;
        else if(FT1<=27.2384)tmp1=26.5;
        else if(FT1<=27.283)tmp1=26.6;
        else if(FT1<=27.3277)tmp1=26.7;
        else if(FT1<=27.3724)tmp1=26.8;
        else if(FT1<=27.4172)tmp1=26.9;
    }
    else if(FT1<=27.8674){
        if(FT1==27.462)tmp1=27;
        else if(FT1<=27.5069)tmp1=27.1;
        else if(FT1<=27.5518)tmp1=27.2;
        else if(FT1<=27.5967)tmp1=27.3;
        else if(FT1<=27.6417)tmp1=27.4;
        else if(FT1<=27.6868)tmp1=27.5;
        else if(FT1<=27.7318)tmp1=27.6;
        else if(FT1<=27.777)tmp1=27.7;
        else if(FT1<=27.8221)tmp1=27.8;
        else if(FT1<=27.8674)tmp1=27.9;
    }
    else if(FT1<=28.322){
        if(FT1==27.9126)tmp1=28;
        else if(FT1<=27.9579)tmp1=28.1;
        else if(FT1<=28.0033)tmp1=28.2;
        else if(FT1<=28.0487)tmp1=28.3;
        else if(FT1<=28.0941)tmp1=28.4;
        else if(FT1<=28.1396)tmp1=28.5;
        else if(FT1<=28.1851)tmp1=28.6;
        else if(FT1<=28.2307)tmp1=28.7;
        else if(FT1<=28.2763)tmp1=28.8;
        else if(FT1<=28.322)tmp1=28.9;
    }
    else if(FT1<=28.7811){
        if(FT1==28.3677)tmp1=29;
        else if(FT1<=28.4134)tmp1=29.1;
        else if(FT1<=28.4592)tmp1=29.2;
        else if(FT1<=28.5051)tmp1=29.3;
        else if(FT1<=28.551)tmp1=29.4;
        else if(FT1<=28.5969)tmp1=29.5;
        else if(FT1<=28.6429)tmp1=29.6;
        else if(FT1<=28.6889)tmp1=29.7;
        else if(FT1<=28.735)tmp1=29.8;
        else if(FT1<=28.7811)tmp1=29.9;
    }
    else if(FT1<=29.2447){
        if(FT1==28.8273)tmp1=30;
        else if(FT1<=28.8735)tmp1=30.1;
        else if(FT1<=28.9197)tmp1=30.2;
        else if(FT1<=28.966)tmp1=30.3;
        else if(FT1<=29.0123)tmp1=30.4;
        else if(FT1<=29.0587)tmp1=30.5;
        else if(FT1<=29.1052)tmp1=30.6;
        else if(FT1<=29.1516)tmp1=30.7;
        else if(FT1<=29.1981)tmp1=30.8;
        else if(FT1<=29.2447)tmp1=30.9;
    }
    else if(FT1<=29.7128){
        if(FT1==29.2913)tmp1=31;
        else if(FT1<=29.338)tmp1=31.1;
        else if(FT1<=29.3847)tmp1=31.2;
        else if(FT1<=29.4314)tmp1=31.3;
        else if(FT1<=29.4782)tmp1=31.4;
        else if(FT1<=29.525)tmp1=31.5;
        else if(FT1<=29.5719)tmp1=31.6;
        else if(FT1<=29.6188)tmp1=31.7;
        else if(FT1<=29.6658)tmp1=31.8;
        else if(FT1<=29.7128)tmp1=31.9;
    }
    else if(FT1<=30.1855){
        if(FT1==29.7599)tmp1=32;
        else if(FT1<=29.807)tmp1=32.1;
        else if(FT1<=29.8541)tmp1=32.2;
        else if(FT1<=29.9013)tmp1=32.3;
        else if(FT1<=29.9486)tmp1=32.4;
        else if(FT1<=29.9959)tmp1=32.5;
        else if(FT1<=30.0432)tmp1=32.6;
        else if(FT1<=30.0906)tmp1=32.7;
        else if(FT1<=30.138)tmp1=32.8;
        else if(FT1<=30.1855)tmp1=32.9;
    }
    else if(FT1<=30.6626){
        if(FT1==30.233)tmp1=33;
        else if(FT1<=30.2805)tmp1=33.1;
        else if(FT1<=30.3281)tmp1=33.2;
        else if(FT1<=30.3758)tmp1=33.3;
        else if(FT1<=30.4235)tmp1=33.4;
        else if(FT1<=30.4712)tmp1=33.5;
        else if(FT1<=30.519)tmp1=33.6;
        else if(FT1<=30.5668)tmp1=33.7;
        else if(FT1<=30.6147)tmp1=33.8;
        else if(FT1<=30.6626)tmp1=33.9;
    }
    else if(FT1<=31.1443){
        if(FT1==30.7106)tmp1=34;
        else if(FT1<=30.7586)tmp1=34.1;
        else if(FT1<=30.8066)tmp1=34.2;
        else if(FT1<=30.8547)tmp1=34.3;
        else if(FT1<=30.9029)tmp1=34.4;
        else if(FT1<=30.9511)tmp1=34.5;
        else if(FT1<=30.9993)tmp1=34.6;
        else if(FT1<=31.0476)tmp1=34.7;
        else if(FT1<=31.0959)tmp1=34.8;
        else if(FT1<=31.1443)tmp1=34.9;
    }
    else if(FT1<=31.6305){
        if(FT1==31.1927)tmp1=35;
        else if(FT1<=31.2411)tmp1=35.1;
        else if(FT1<=31.2897)tmp1=35.2;
        else if(FT1<=31.3382)tmp1=35.3;
        else if(FT1<=31.3868)tmp1=35.4;
        else if(FT1<=31.4354)tmp1=35.5;
        else if(FT1<=31.4841)tmp1=35.6;
        else if(FT1<=31.5329)tmp1=35.7;
        else if(FT1<=31.5816)tmp1=35.8;
        else if(FT1<=31.6305)tmp1=35.9;
    }
    else if(FT1<=32.1212){
        if(FT1==31.6793)tmp1=36;
        else if(FT1<=31.7283)tmp1=36.1;
        else if(FT1<=31.7772)tmp1=36.2;
        else if(FT1<=31.8262)tmp1=36.3;
        else if(FT1<=31.8753)tmp1=36.4;
        else if(FT1<=31.9244)tmp1=36.5;
        else if(FT1<=31.9735)tmp1=36.6;
        else if(FT1<=32.0227)tmp1=36.7;
        else if(FT1<=32.0719)tmp1=36.8;
        else if(FT1<=32.1212)tmp1=36.9;
    }
    else if(FT1<=32.6165){
        if(FT1==32.1705)tmp1=37;
        else if(FT1<=32.2199)tmp1=37.1;
        else if(FT1<=32.2693)tmp1=37.2;
        else if(FT1<=32.3188)tmp1=37.3;
        else if(FT1<=32.3683)tmp1=37.4;
        else if(FT1<=32.4178)tmp1=37.5;
        else if(FT1<=32.4674)tmp1=37.6;
        else if(FT1<=32.5171)tmp1=37.7;
        else if(FT1<=32.5667)tmp1=37.8;
        else if(FT1<=32.6165)tmp1=37.9;
    }
    else if(FT1<=33.1163){
        if(FT1==32.6663)tmp1=38;
        else if(FT1<=32.7161)tmp1=38.1;
        else if(FT1<=32.7659)tmp1=38.2;
        else if(FT1<=32.8159)tmp1=38.3;
        else if(FT1<=32.8658)tmp1=38.4;
        else if(FT1<=32.9158)tmp1=38.5;
        else if(FT1<=32.9659)tmp1=38.6;
        else if(FT1<=33.016)tmp1=38.7;
        else if(FT1<=33.0661)tmp1=38.8;
        else if(FT1<=33.1163)tmp1=38.9;
    }
    else if(FT1<=33.6207){
        if(FT1==33.1665)tmp1=39;
        else if(FT1<=33.2168)tmp1=39.1;
        else if(FT1<=33.2671)tmp1=39.2;
        else if(FT1<=33.3175)tmp1=39.3;
        else if(FT1<=33.3679)tmp1=39.4;
        else if(FT1<=33.4184)tmp1=39.5;
        else if(FT1<=33.4689)tmp1=39.6;
        else if(FT1<=33.5194)tmp1=39.7;
        else if(FT1<=33.57)tmp1=39.8;
        else if(FT1<=33.6207)tmp1=39.9;
    }
    else if(FT1<=34.1296){
        if(FT1==33.6714)tmp1=40;
        else if(FT1<=33.7221)tmp1=40.1;
        else if(FT1<=33.7729)tmp1=40.2;
        else if(FT1<=33.8237)tmp1=40.3;
        else if(FT1<=33.8746)tmp1=40.4;
        else if(FT1<=33.9255)tmp1=40.5;
        else if(FT1<=33.9764)tmp1=40.6;
        else if(FT1<=34.0275)tmp1=40.7;
        else if(FT1<=34.0785)tmp1=40.8;
        else if(FT1<=34.1296)tmp1=40.9;
    }
    else if(FT1<=34.6431){
        if(FT1==34.1807)tmp1=41;
        else if(FT1<=34.2319)tmp1=41.1;
        else if(FT1<=34.2832)tmp1=41.2;
        else if(FT1<=34.3345)tmp1=41.3;
        else if(FT1<=34.3858)tmp1=41.4;
        else if(FT1<=34.4372)tmp1=41.5;
        else if(FT1<=34.4886)tmp1=41.6;
        else if(FT1<=34.54)tmp1=41.7;
        else if(FT1<=34.5915)tmp1=41.8;
        else if(FT1<=34.6431)tmp1=41.9;
    }
    else if(FT1<=35.1612){
        if(FT1==34.6947)tmp1=42;
        else if(FT1<=34.7463)tmp1=42.1;
        else if(FT1<=34.798)tmp1=42.2;
        else if(FT1<=34.8498)tmp1=42.3;
        else if(FT1<=34.9016)tmp1=42.4;
        else if(FT1<=34.9534)tmp1=42.5;
        else if(FT1<=35.0053)tmp1=42.6;
        else if(FT1<=35.0572)tmp1=42.7;
        else if(FT1<=35.1091)tmp1=42.8;
        else if(FT1<=35.1612)tmp1=42.9;
    }
    else if(FT1<=35.6838){
        if(FT1==35.2132)tmp1=43;
        else if(FT1<=35.2653)tmp1=43.1;
        else if(FT1<=35.3175)tmp1=43.2;
        else if(FT1<=35.3697)tmp1=43.3;
        else if(FT1<=35.4219)tmp1=43.4;
        else if(FT1<=35.4742)tmp1=43.5;
        else if(FT1<=35.5265)tmp1=43.6;
        else if(FT1<=35.5789)tmp1=43.7;
        else if(FT1<=35.6313)tmp1=43.8;
        else if(FT1<=35.6838)tmp1=43.9;
    }
    else if(FT1<=36.211){
        if(FT1==35.7363)tmp1=44;
        else if(FT1<=35.7889)tmp1=44.1;
        else if(FT1<=35.8415)tmp1=44.2;
        else if(FT1<=35.8941)tmp1=44.3;
        else if(FT1<=35.9468)tmp1=44.4;
        else if(FT1<=35.9996)tmp1=44.5;
        else if(FT1<=36.0523)tmp1=44.6;
        else if(FT1<=36.1052)tmp1=44.7;
        else if(FT1<=36.1581)tmp1=44.8;
        else if(FT1<=36.211)tmp1=44.9;
    }
    else if(FT1<=36.7428){
        if(FT1==36.264)tmp1=45;
        else if(FT1<=36.317)tmp1=45.1;
        else if(FT1<=36.37)tmp1=45.2;
        else if(FT1<=36.4231)tmp1=45.3;
        else if(FT1<=36.4763)tmp1=45.4;
        else if(FT1<=36.5295)tmp1=45.5;
        else if(FT1<=36.5828)tmp1=45.6;
        else if(FT1<=36.6361)tmp1=45.7;
        else if(FT1<=36.6894)tmp1=45.8;
        else if(FT1<=36.7428)tmp1=45.9;
    }
    else if(FT1<=37.2791){
        if(FT1==36.7962)tmp1=46;
        else if(FT1<=36.8497)tmp1=46.1;
        else if(FT1<=36.9032)tmp1=46.2;
        else if(FT1<=36.9568)tmp1=46.3;
        else if(FT1<=37.0104)tmp1=46.4;
        else if(FT1<=37.064)tmp1=46.5;
        else if(FT1<=37.1178)tmp1=46.6;
        else if(FT1<=37.1715)tmp1=46.7;
        else if(FT1<=37.2253)tmp1=46.8;
        else if(FT1<=37.2791)tmp1=46.9;
    }
    else if(FT1<=37.8201){
        if(FT1==37.333)tmp1=47;
        else if(FT1<=37.387)tmp1=47.1;
        else if(FT1<=37.4409)tmp1=47.2;
        else if(FT1<=37.495)tmp1=47.3;
        else if(FT1<=37.549)tmp1=47.4;
        else if(FT1<=37.6032)tmp1=47.5;
        else if(FT1<=37.6573)tmp1=47.6;
        else if(FT1<=37.7115)tmp1=47.7;
        else if(FT1<=37.7658)tmp1=47.8;
        else if(FT1<=37.8201)tmp1=47.9;
    }
    else if(FT1<=38.3656){
        if(FT1==37.8744)tmp1=48;
        else if(FT1<=37.9288)tmp1=48.1;
        else if(FT1<=37.9833)tmp1=48.2;
        else if(FT1<=38.0378)tmp1=48.3;
        else if(FT1<=38.0923)tmp1=48.4;
        else if(FT1<=38.1469)tmp1=48.5;
        else if(FT1<=38.2015)tmp1=48.6;
        else if(FT1<=38.2562)tmp1=48.7;
        else if(FT1<=38.3109)tmp1=48.8;
        else if(FT1<=38.3656)tmp1=48.9;
    }
    else if(FT1<=38.9158){
        if(FT1==38.4204)tmp1=49;
        else if(FT1<=38.4753)tmp1=49.1;
        else if(FT1<=38.5302)tmp1=49.2;
        else if(FT1<=38.5851)tmp1=49.3;
        else if(FT1<=38.6401)tmp1=49.4;
        else if(FT1<=38.6952)tmp1=49.5;
        else if(FT1<=38.7502)tmp1=49.6;
        else if(FT1<=38.8054)tmp1=49.7;
        else if(FT1<=38.8605)tmp1=49.8;
        else if(FT1<=38.9158)tmp1=49.9;
    }
    else if(FT1<=39.4705){
        if(FT1==38.971)tmp1=50;
        else if(FT1<=39.0263)tmp1=50.1;
        else if(FT1<=39.0817)tmp1=50.2;
        else if(FT1<=39.1371)tmp1=50.3;
        else if(FT1<=39.1926)tmp1=50.4;
        else if(FT1<=39.2481)tmp1=50.5;
        else if(FT1<=39.3036)tmp1=50.6;
        else if(FT1<=39.3592)tmp1=50.7;
        else if(FT1<=39.4148)tmp1=50.8;
        else if(FT1<=39.4705)tmp1=50.9;
    }
    else if(FT1<=40.0298){
        if(FT1==39.5262)tmp1=51;
        else if(FT1<=39.582)tmp1=51.1;
        else if(FT1<=39.6378)tmp1=51.2;
        else if(FT1<=39.6937)tmp1=51.3;
        else if(FT1<=39.7496)tmp1=51.4;
        else if(FT1<=39.8055)tmp1=51.5;
        else if(FT1<=39.8615)tmp1=51.6;
        else if(FT1<=39.9176)tmp1=51.7;
        else if(FT1<=39.9737)tmp1=51.8;
        else if(FT1<=40.0298)tmp1=51.9;
    }
    else if(FT1<=40.5937){
        if(FT1==40.086)tmp1=52;
        else if(FT1<=40.1422)tmp1=52.1;
        else if(FT1<=40.1985)tmp1=52.2;
        else if(FT1<=40.2548)tmp1=52.3;
        else if(FT1<=40.3112)tmp1=52.4;
        else if(FT1<=40.3676)tmp1=52.5;
        else if(FT1<=40.4241)tmp1=52.6;
        else if(FT1<=40.4806)tmp1=52.7;
        else if(FT1<=40.5371)tmp1=52.8;
        else if(FT1<=40.5937)tmp1=52.9;
    }
    else if(FT1<=41.1623){
        if(FT1==40.6504)tmp1=53;
        else if(FT1<=40.7071)tmp1=53.1;
        else if(FT1<=40.7638)tmp1=53.2;
        else if(FT1<=40.8206)tmp1=53.3;
        else if(FT1<=40.8774)tmp1=53.4;
        else if(FT1<=40.9343)tmp1=53.5;
        else if(FT1<=40.9912)tmp1=53.6;
        else if(FT1<=41.0482)tmp1=53.7;
        else if(FT1<=41.1052)tmp1=53.8;
        else if(FT1<=41.1623)tmp1=53.9;
    }
    else if(FT1<=41.7354){
        if(FT1==41.2194)tmp1=54;
        else if(FT1<=41.2765)tmp1=54.1;
        else if(FT1<=41.3337)tmp1=54.2;
        else if(FT1<=41.391)tmp1=54.3;
        else if(FT1<=41.4483)tmp1=54.4;
        else if(FT1<=41.5056)tmp1=54.5;
        else if(FT1<=41.563)tmp1=54.6;
        else if(FT1<=41.6204)tmp1=54.7;
        else if(FT1<=41.6779)tmp1=54.8;
        else if(FT1<=41.7354)tmp1=54.9;
    }
    else if(FT1<=42.3131){
        if(FT1==41.793)tmp1=55;
        else if(FT1<=41.8506)tmp1=55.1;
        else if(FT1<=41.9082)tmp1=55.2;
        else if(FT1<=41.9659)tmp1=55.3;
        else if(FT1<=42.0237)tmp1=55.4;
        else if(FT1<=42.0815)tmp1=55.5;
        else if(FT1<=42.1393)tmp1=55.6;
        else if(FT1<=42.1972)tmp1=55.7;
        else if(FT1<=42.2552)tmp1=55.8;
        else if(FT1<=42.3131)tmp1=55.9;
    }
    else if(FT1<=42.8955){
        if(FT1==42.3712)tmp1=56;
        else if(FT1<=42.4292)tmp1=56.1;
        else if(FT1<=42.4874)tmp1=56.2;
        else if(FT1<=42.5455)tmp1=56.3;
        else if(FT1<=42.6037)tmp1=56.4;
        else if(FT1<=42.662)tmp1=56.5;
        else if(FT1<=42.7203)tmp1=56.6;
        else if(FT1<=42.7787)tmp1=56.7;
        else if(FT1<=42.837)tmp1=56.8;
        else if(FT1<=42.8955)tmp1=56.9;
    }
    else if(FT1<=43.4824){
        if(FT1==42.954)tmp1=57;
        else if(FT1<=43.0125)tmp1=57.1;
        else if(FT1<=43.0711)tmp1=57.2;
        else if(FT1<=43.1297)tmp1=57.3;
        else if(FT1<=43.1884)tmp1=57.4;
        else if(FT1<=43.2471)tmp1=57.5;
        else if(FT1<=43.3059)tmp1=57.6;
        else if(FT1<=43.3647)tmp1=57.7;
        else if(FT1<=43.4235)tmp1=57.8;
        else if(FT1<=43.4824)tmp1=57.9;
    }
    else if(FT1<=44.074){
        if(FT1==43.5414)tmp1=58;
        else if(FT1<=43.6004)tmp1=58.1;
        else if(FT1<=43.6594)tmp1=58.2;
        else if(FT1<=43.7185)tmp1=58.3;
        else if(FT1<=43.7777)tmp1=58.4;
        else if(FT1<=43.8368)tmp1=58.5;
        else if(FT1<=43.8961)tmp1=58.6;
        else if(FT1<=43.9553)tmp1=58.7;
        else if(FT1<=44.0146)tmp1=58.8;
        else if(FT1<=44.074)tmp1=58.9;
    }
    else if(FT1<=44.6702){
        if(FT1==44.1334)tmp1=59;
        else if(FT1<=44.1929)tmp1=59.1;
        else if(FT1<=44.2524)tmp1=59.2;
        else if(FT1<=44.3119)tmp1=59.3;
        else if(FT1<=44.3715)tmp1=59.4;
        else if(FT1<=44.4312)tmp1=59.5;
        else if(FT1<=44.4908)tmp1=59.6;
        else if(FT1<=44.5506)tmp1=59.7;
        else if(FT1<=44.6104)tmp1=59.8;
        else if(FT1<=44.6702)tmp1=59.9;
    }
    else if(FT1<=45.271){
        if(FT1==44.7301)tmp1=60;
        else if(FT1<=44.79)tmp1=60.1;
        else if(FT1<=44.8499)tmp1=60.2;
        else if(FT1<=44.9099)tmp1=60.3;
        else if(FT1<=44.97)tmp1=60.4;
        else if(FT1<=45.0301)tmp1=60.5;
        else if(FT1<=45.0903)tmp1=60.6;
        else if(FT1<=45.1504)tmp1=60.7;
        else if(FT1<=45.2107)tmp1=60.8;
        else if(FT1<=45.271)tmp1=60.9;
    }
    else if(FT1<=45.8764){
        if(FT1==45.3313)tmp1=61;
        else if(FT1<=45.3917)tmp1=61.1;
        else if(FT1<=45.4521)tmp1=61.2;
        else if(FT1<=45.5126)tmp1=61.3;
        else if(FT1<=45.5731)tmp1=61.4;
        else if(FT1<=45.6337)tmp1=61.5;
        else if(FT1<=45.6943)tmp1=61.6;
        else if(FT1<=45.7549)tmp1=61.7;
        else if(FT1<=45.8156)tmp1=61.8;
        else if(FT1<=45.8764)tmp1=61.9;
    }
    else if(FT1<=46.4864){
        if(FT1==45.9372)tmp1=62;
        else if(FT1<=45.998)tmp1=62.1;
        else if(FT1<=46.0589)tmp1=62.2;
        else if(FT1<=46.1198)tmp1=62.3;
        else if(FT1<=46.1808)tmp1=62.4;
        else if(FT1<=46.2418)tmp1=62.5;
        else if(FT1<=46.3029)tmp1=62.6;
        else if(FT1<=46.364)tmp1=62.7;
        else if(FT1<=46.4252)tmp1=62.8;
        else if(FT1<=46.4864)tmp1=62.9;
    }
    else if(FT1<=47.101){
        if(FT1==46.5476)tmp1=63;
        else if(FT1<=46.6089)tmp1=63.1;
        else if(FT1<=46.6703)tmp1=63.2;
        else if(FT1<=46.7317)tmp1=63.3;
        else if(FT1<=46.7931)tmp1=63.4;
        else if(FT1<=46.8546)tmp1=63.5;
        else if(FT1<=46.9161)tmp1=63.6;
        else if(FT1<=46.9777)tmp1=63.7;
        else if(FT1<=47.0393)tmp1=63.8;
        else if(FT1<=47.101)tmp1=63.9;
    }
    else if(FT1<=47.7203){
        if(FT1==47.1627)tmp1=64;
        else if(FT1<=47.2245)tmp1=64.1;
        else if(FT1<=47.2863)tmp1=64.2;
        else if(FT1<=47.3482)tmp1=64.3;
        else if(FT1<=47.4101)tmp1=64.4;
        else if(FT1<=47.472)tmp1=64.5;
        else if(FT1<=47.534)tmp1=64.6;
        else if(FT1<=47.596)tmp1=64.7;
        else if(FT1<=47.6581)tmp1=64.8;
        else if(FT1<=47.7203)tmp1=64.9;
        else if(FT1<=47.7824)tmp1=65;
    }
    else if(FT1<=48.3441){
        if(FT1==47.7824)tmp1=65;
        else if(FT1<=47.8447)tmp1=65.1;
        else if(FT1<=47.9069)tmp1=65.2;
        else if(FT1<=47.9692)tmp1=65.3;
        else if(FT1<=48.0316)tmp1=65.4;
        else if(FT1<=48.094)tmp1=65.5;
        else if(FT1<=48.1565)tmp1=65.6;
        else if(FT1<=48.219)tmp1=65.7;
        else if(FT1<=48.2815)tmp1=65.8;
        else if(FT1<=48.3441)tmp1=65.9;
        else if(FT1<=48.4067)tmp1=66;
    }
    else if(FT1<=48.9726){
        if(FT1==48.4067)tmp1=66;
        else if(FT1<=48.4694)tmp1=66.1;
        else if(FT1<=48.5322)tmp1=66.2;
        else if(FT1<=48.5949)tmp1=66.3;
        else if(FT1<=48.6578)tmp1=66.4;
        else if(FT1<=48.7206)tmp1=66.5;
        else if(FT1<=48.7836)tmp1=66.6;
        else if(FT1<=48.8465)tmp1=66.7;
        else if(FT1<=48.9095)tmp1=66.8;
        else if(FT1<=48.9726)tmp1=66.9;
        else if(FT1<=49.0357)tmp1=67;
    }
    else if(FT1<=49.6057){
        if(FT1==49.0357)tmp1=67;
        else if(FT1<=49.0988)tmp1=67.1;
        else if(FT1<=49.162)tmp1=67.2;
        else if(FT1<=49.2253)tmp1=67.3;
        else if(FT1<=49.2885)tmp1=67.4;
        else if(FT1<=49.3519)tmp1=67.5;
        else if(FT1<=49.4152)tmp1=67.6;
        else if(FT1<=49.4787)tmp1=67.7;
        else if(FT1<=49.5421)tmp1=67.8;
        else if(FT1<=49.6057)tmp1=67.9;
        else if(FT1<=49.6692)tmp1=68;
    }
    else if(FT1<=50.2433){
        if(FT1==49.6692)tmp1=68;
        else if(FT1<=49.7328)tmp1=68.1;
        else if(FT1<=49.7965)tmp1=68.2;
        else if(FT1<=49.8602)tmp1=68.3;
        else if(FT1<=49.9239)tmp1=68.4;
        else if(FT1<=49.9877)tmp1=68.5;
        else if(FT1<=50.0516)tmp1=68.6;
        else if(FT1<=50.1154)tmp1=68.7;
        else if(FT1<=50.1794)tmp1=68.8;
        else if(FT1<=50.2433)tmp1=68.9;
        else if(FT1<=50.3074)tmp1=69;
    }
    else if(FT1<=50.8857){
        if(FT1==50.3074)tmp1=69;
        else if(FT1<=50.3714)tmp1=69.1;
        else if(FT1<=50.4356)tmp1=69.2;
        else if(FT1<=50.4997)tmp1=69.3;
        else if(FT1<=50.5639)tmp1=69.4;
        else if(FT1<=50.6282)tmp1=69.5;
        else if(FT1<=50.6925)tmp1=69.6;
        else if(FT1<=50.7568)tmp1=69.7;
        else if(FT1<=50.8212)tmp1=69.8;
        else if(FT1<=50.8857)tmp1=69.9;
        else if(FT1<=50.9501)tmp1=70;
    }
    else if(FT1<=51.5326){
        if(FT1==50.9501)tmp1=70;
        else if(FT1<=51.0147)tmp1=70.1;
        else if(FT1<=51.0792)tmp1=70.2;
        else if(FT1<=51.1439)tmp1=70.3;
        else if(FT1<=51.2085)tmp1=70.4;
        else if(FT1<=51.2732)tmp1=70.5;
        else if(FT1<=51.338)tmp1=70.6;
        else if(FT1<=51.4028)tmp1=70.7;
        else if(FT1<=51.4677)tmp1=70.8;
        else if(FT1<=51.5326)tmp1=70.9;
        else if(FT1<=51.5975)tmp1=71;
    }
    else if(FT1<=52.1841){
        if(FT1==51.5975)tmp1=71;
        else if(FT1<=51.6625)tmp1=71.1;
        else if(FT1<=51.7275)tmp1=71.2;
        else if(FT1<=51.7926)tmp1=71.3;
        else if(FT1<=51.8578)tmp1=71.4;
        else if(FT1<=51.9229)tmp1=71.5;
        else if(FT1<=51.9882)tmp1=71.6;
        else if(FT1<=52.0534)tmp1=71.7;
        else if(FT1<=52.1187)tmp1=71.8;
        else if(FT1<=52.1841)tmp1=71.9;
        else if(FT1<=52.2495)tmp1=72;
    }
    else if(FT1<=52.8402){
        if(FT1==52.2495)tmp1=72;
        else if(FT1<=52.3149)tmp1=72.1;
        else if(FT1<=52.3804)tmp1=72.2;
        else if(FT1<=52.446)tmp1=72.3;
        else if(FT1<=52.5116)tmp1=72.4;
        else if(FT1<=52.5772)tmp1=72.5;
        else if(FT1<=52.6429)tmp1=72.6;
        else if(FT1<=52.7086)tmp1=72.7;
        else if(FT1<=52.7744)tmp1=72.8;
        else if(FT1<=52.8402)tmp1=72.9;
        else if(FT1<=52.9061)tmp1=73;
    }
    else if(FT1<=53.501){
        if(FT1==52.9061)tmp1=73;
        else if(FT1<=52.972)tmp1=73.1;
        else if(FT1<=53.038)tmp1=73.2;
        else if(FT1<=53.104)tmp1=73.3;
        else if(FT1<=53.17)tmp1=73.4;
        else if(FT1<=53.2361)tmp1=73.5;
        else if(FT1<=53.3023)tmp1=73.6;
        else if(FT1<=53.3684)tmp1=73.7;
        else if(FT1<=53.4347)tmp1=73.8;
        else if(FT1<=53.501)tmp1=73.9;
        else if(FT1<=53.5673)tmp1=74;
    }
    else if(FT1<=54.1663){
        if(FT1==53.5673)tmp1=74;
        else if(FT1<=53.6337)tmp1=74.1;
        else if(FT1<=53.7001)tmp1=74.2;
        else if(FT1<=53.7665)tmp1=74.3;
        else if(FT1<=53.8331)tmp1=74.4;
        else if(FT1<=53.8996)tmp1=74.5;
        else if(FT1<=53.9662)tmp1=74.6;
        else if(FT1<=54.0329)tmp1=74.7;
        else if(FT1<=54.0996)tmp1=74.8;
        else if(FT1<=54.1663)tmp1=74.9;
        else if(FT1<=54.2331)tmp1=75;
    }
    else if(FT1<=54.8363){
        if(FT1==54.2331)tmp1=75;
        else if(FT1<=54.2999)tmp1=75.1;
        else if(FT1<=54.3668)tmp1=75.2;
        else if(FT1<=54.4337)tmp1=75.3;
        else if(FT1<=54.5007)tmp1=75.4;
        else if(FT1<=54.5677)tmp1=75.5;
        else if(FT1<=54.6348)tmp1=75.6;
        else if(FT1<=54.7019)tmp1=75.7;
        else if(FT1<=54.7691)tmp1=75.8;
        else if(FT1<=54.8363)tmp1=75.9;
        else if(FT1<=54.9035)tmp1=76;
    }
    else if(FT1<=55.5108){
        if(FT1==54.9035)tmp1=76;
        else if(FT1<=54.9708)tmp1=76.1;
        else if(FT1<=55.0381)tmp1=76.2;
        else if(FT1<=55.1055)tmp1=76.3;
        else if(FT1<=55.173)tmp1=76.4;
        else if(FT1<=55.2404)tmp1=76.5;
        else if(FT1<=55.308)tmp1=76.6;
        else if(FT1<=55.3755)tmp1=76.7;
        else if(FT1<=55.4431)tmp1=76.8;
        else if(FT1<=55.5108)tmp1=76.9;
        else if(FT1<=55.5785)tmp1=77;
    }
    else if(FT1<=56.19){
        if(FT1==55.5785)tmp1=77;
        else if(FT1<=55.6463)tmp1=77.1;
        else if(FT1<=55.7141)tmp1=77.2;
        else if(FT1<=55.7819)tmp1=77.3;
        else if(FT1<=55.8498)tmp1=77.4;
        else if(FT1<=55.9177)tmp1=77.5;
        else if(FT1<=55.9857)tmp1=77.6;
        else if(FT1<=56.0538)tmp1=77.7;
        else if(FT1<=56.1218)tmp1=77.8;
        else if(FT1<=56.19)tmp1=77.9;
        else if(FT1<=56.2581)tmp1=78;
    }
    else if(FT1<=56.8737){
        if(FT1==56.2581)tmp1=78;
        else if(FT1<=56.3263)tmp1=78.1;
        else if(FT1<=56.3946)tmp1=78.2;
        else if(FT1<=56.4629)tmp1=78.3;
        else if(FT1<=56.5313)tmp1=78.4;
        else if(FT1<=56.5996)tmp1=78.5;
        else if(FT1<=56.6681)tmp1=78.6;
        else if(FT1<=56.7366)tmp1=78.7;
        else if(FT1<=56.8051)tmp1=78.8;
        else if(FT1<=56.8737)tmp1=78.9;
        else if(FT1<=56.9423)tmp1=79;
    }
    else if(FT1<=57.562){
        if(FT1==56.9423)tmp1=79;
        else if(FT1<=57.011)tmp1=79.1;
        else if(FT1<=57.0797)tmp1=79.2;
        else if(FT1<=57.1485)tmp1=79.3;
        else if(FT1<=57.2173)tmp1=79.4;
        else if(FT1<=57.2862)tmp1=79.5;
        else if(FT1<=57.3551)tmp1=79.6;
        else if(FT1<=57.424)tmp1=79.7;
        else if(FT1<=57.493)tmp1=79.8;
        else if(FT1<=57.562)tmp1=79.9;
        else if(FT1<=57.6311)tmp1=80;
    }
    else if(FT1<=58.255){
        if(FT1==57.6311)tmp1=80;
        else if(FT1<=57.7003)tmp1=80.1;
        else if(FT1<=57.7694)tmp1=80.2;
        else if(FT1<=57.8387)tmp1=80.3;
        else if(FT1<=57.9079)tmp1=80.4;
        else if(FT1<=57.9772)tmp1=80.5;
        else if(FT1<=58.0466)tmp1=80.6;
        else if(FT1<=58.116)tmp1=80.7;
        else if(FT1<=58.1855)tmp1=80.8;
        else if(FT1<=58.255)tmp1=80.9;
        else if(FT1<=58.3245)tmp1=81;
    }
    else if(FT1<=58.9525){
        if(FT1==58.3245)tmp1=81;
        else if(FT1<=58.3941)tmp1=81.1;
        else if(FT1<=58.4637)tmp1=81.2;
        else if(FT1<=58.5334)tmp1=81.3;
        else if(FT1<=58.6032)tmp1=81.4;
        else if(FT1<=58.6729)tmp1=81.5;
        else if(FT1<=58.7428)tmp1=81.6;
        else if(FT1<=58.8126)tmp1=81.7;
        else if(FT1<=58.8825)tmp1=81.8;
        else if(FT1<=58.9525)tmp1=81.9;
        else if(FT1<=59.0225)tmp1=82;
    }
    else if(FT1<=59.6546){
        if(FT1==59.0225)tmp1=82;
        else if(FT1<=59.0926)tmp1=82.1;
        else if(FT1<=59.1626)tmp1=82.2;
        else if(FT1<=59.2328)tmp1=82.3;
        else if(FT1<=59.303)tmp1=82.4;
        else if(FT1<=59.3732)tmp1=82.5;
        else if(FT1<=59.4435)tmp1=82.6;
        else if(FT1<=59.5138)tmp1=82.7;
        else if(FT1<=59.5842)tmp1=82.8;
        else if(FT1<=59.6546)tmp1=82.9;
        else if(FT1<=59.7251)tmp1=83;
    }
    else if(FT1<=60.3613){
        if(FT1==59.7251)tmp1=83;
        else if(FT1<=59.7956)tmp1=83.1;
        else if(FT1<=59.8661)tmp1=83.2;
        else if(FT1<=59.9367)tmp1=83.3;
        else if(FT1<=60.0074)tmp1=83.4;
        else if(FT1<=60.0781)tmp1=83.5;
        else if(FT1<=60.1488)tmp1=83.6;
        else if(FT1<=60.2196)tmp1=83.7;
        else if(FT1<=60.2904)tmp1=83.8;
        else if(FT1<=60.3613)tmp1=83.9;
        else if(FT1<=60.4322)tmp1=84;
    }
    else if(FT1<=61.0726){
        if(FT1==60.4322)tmp1=84;
        else if(FT1<=60.5032)tmp1=84.1;
        else if(FT1<=60.5742)tmp1=84.2;
        else if(FT1<=60.6453)tmp1=84.3;
        else if(FT1<=60.7164)tmp1=84.4;
        else if(FT1<=60.7875)tmp1=84.5;
        else if(FT1<=60.8587)tmp1=84.6;
        else if(FT1<=60.93)tmp1=84.7;
        else if(FT1<=61.0012)tmp1=84.8;
        else if(FT1<=61.0726)tmp1=84.9;
        else if(FT1<=61.144)tmp1=85;
    }
    else if(FT1<=61.7884){
        if(FT1==61.144)tmp1=85;
        else if(FT1<=61.2154)tmp1=85.1;
        else if(FT1<=61.2869)tmp1=85.2;
        else if(FT1<=61.3584)tmp1=85.3;
        else if(FT1<=61.4299)tmp1=85.4;
        else if(FT1<=61.5015)tmp1=85.5;
        else if(FT1<=61.5732)tmp1=85.6;
        else if(FT1<=61.6449)tmp1=85.7;
        else if(FT1<=61.7166)tmp1=85.8;
        else if(FT1<=61.7884)tmp1=85.9;
        else if(FT1<=61.8603)tmp1=86;
    }
    else if(FT1<=62.5089){
        if(FT1==61.8603)tmp1=86;
        else if(FT1<=61.9322)tmp1=86.1;
        else if(FT1<=62.0041)tmp1=86.2;
        else if(FT1<=62.0761)tmp1=86.3;
        else if(FT1<=62.1481)tmp1=86.4;
        else if(FT1<=62.2201)tmp1=86.5;
        else if(FT1<=62.2923)tmp1=86.6;
        else if(FT1<=62.3644)tmp1=86.7;
        else if(FT1<=62.4366)tmp1=86.8;
        else if(FT1<=62.5089)tmp1=86.9;
        else if(FT1<=62.5812)tmp1=87;
    }
    else if(FT1<=63.2339){
        if(FT1==62.5812)tmp1=87;
        else if(FT1<=62.6535)tmp1=87.1;
        else if(FT1<=62.7259)tmp1=87.2;
        else if(FT1<=62.7983)tmp1=87.3;
        else if(FT1<=62.8708)tmp1=87.4;
        else if(FT1<=62.9433)tmp1=87.5;
        else if(FT1<=63.0159)tmp1=87.6;
        else if(FT1<=63.0885)tmp1=87.7;
        else if(FT1<=63.1612)tmp1=87.8;
        else if(FT1<=63.2339)tmp1=87.9;
        else if(FT1<=63.3066)tmp1=88;
    }
    else if(FT1<=63.9634){
        if(FT1==63.3066)tmp1=88;
        else if(FT1<=63.3794)tmp1=88.1;
        else if(FT1<=63.4523)tmp1=88.2;
        else if(FT1<=63.5251)tmp1=88.3;
        else if(FT1<=63.5981)tmp1=88.4;
        else if(FT1<=63.6711)tmp1=88.5;
        else if(FT1<=63.7441)tmp1=88.6;
        else if(FT1<=63.8172)tmp1=88.7;
        else if(FT1<=63.8903)tmp1=88.8;
        else if(FT1<=63.9634)tmp1=88.9;
        else if(FT1<=64.0366)tmp1=89;
    }
    else if(FT1<=64.6976){
        if(FT1==64.0366)tmp1=89;
        else if(FT1<=64.1099)tmp1=89.1;
        else if(FT1<=64.1832)tmp1=89.2;
        else if(FT1<=64.2565)tmp1=89.3;
        else if(FT1<=64.3299)tmp1=89.4;
        else if(FT1<=64.4034)tmp1=89.5;
        else if(FT1<=64.4768)tmp1=89.6;
        else if(FT1<=64.5504)tmp1=89.7;
        else if(FT1<=64.6239)tmp1=89.8;
        else if(FT1<=64.6976)tmp1=89.9;
        else if(FT1<=64.7712)tmp1=90;
    }
    else if(FT1<=65.4363){
        if(FT1==64.7712)tmp1=90;
        else if(FT1<=64.8449)tmp1=90.1;
        else if(FT1<=64.9187)tmp1=90.2;
        else if(FT1<=64.9925)tmp1=90.3;
        else if(FT1<=65.0663)tmp1=90.4;
        else if(FT1<=65.1402)tmp1=90.5;
        else if(FT1<=65.2142)tmp1=90.6;
        else if(FT1<=65.2882)tmp1=90.7;
        else if(FT1<=65.3622)tmp1=90.8;
        else if(FT1<=65.4363)tmp1=90.9;
        else if(FT1<=65.5104)tmp1=91;
    }
    else if(FT1<=66.1795){
        if(FT1==65.5104)tmp1=91;
        else if(FT1<=65.5845)tmp1=91.1;
        else if(FT1<=65.6588)tmp1=91.2;
        else if(FT1<=65.733)tmp1=91.3;
        else if(FT1<=65.8073)tmp1=91.4;
        else if(FT1<=65.8817)tmp1=91.5;
        else if(FT1<=65.9561)tmp1=91.6;
        else if(FT1<=66.0305)tmp1=91.7;
        else if(FT1<=66.105)tmp1=91.8;
        else if(FT1<=66.1795)tmp1=91.9;
        else if(FT1<=66.2541)tmp1=92;
    }
    else if(FT1<=66.9273){
        if(FT1==66.2541)tmp1=92;
        else if(FT1<=66.3287)tmp1=92.1;
        else if(FT1<=66.4034)tmp1=92.2;
        else if(FT1<=66.4781)tmp1=92.3;
        else if(FT1<=66.5528)tmp1=92.4;
        else if(FT1<=66.6276)tmp1=92.5;
        else if(FT1<=66.7025)tmp1=92.6;
        else if(FT1<=66.7774)tmp1=92.7;
        else if(FT1<=66.8523)tmp1=92.8;
        else if(FT1<=66.9273)tmp1=92.9;
        else if(FT1<=67.0023)tmp1=93;
    }
    else if(FT1<=67.6797){
        if(FT1==67.0023)tmp1=93;
        else if(FT1<=67.0774)tmp1=93.1;
        else if(FT1<=67.1525)tmp1=93.2;
        else if(FT1<=67.2277)tmp1=93.3;
        else if(FT1<=67.3029)tmp1=93.4;
        else if(FT1<=67.3782)tmp1=93.5;
        else if(FT1<=67.4535)tmp1=93.6;
        else if(FT1<=67.5288)tmp1=93.7;
        else if(FT1<=67.6042)tmp1=93.8;
        else if(FT1<=67.6797)tmp1=93.9;
        else if(FT1<=67.7551)tmp1=94;
    }
    else if(FT1<=68.4365){
        if(FT1==67.7551)tmp1=94;
        else if(FT1<=67.8307)tmp1=94.1;
        else if(FT1<=67.9062)tmp1=94.2;
        else if(FT1<=67.9819)tmp1=94.3;
        else if(FT1<=68.0575)tmp1=94.4;
        else if(FT1<=68.1332)tmp1=94.5;
        else if(FT1<=68.209)tmp1=94.6;
        else if(FT1<=68.2848)tmp1=94.7;
        else if(FT1<=68.3607)tmp1=94.8;
        else if(FT1<=68.4365)tmp1=94.9;
        else if(FT1<=68.5125)tmp1=95;
    }
    else if(FT1<=69.198){
        if(FT1==68.5125)tmp1=95;
        else if(FT1<=68.5885)tmp1=95.1;
        else if(FT1<=68.6645)tmp1=95.2;
        else if(FT1<=68.7406)tmp1=95.3;
        else if(FT1<=68.8167)tmp1=95.4;
        else if(FT1<=68.8929)tmp1=95.5;
        else if(FT1<=68.9691)tmp1=95.6;
        else if(FT1<=69.0453)tmp1=95.7;
        else if(FT1<=69.1216)tmp1=95.8;
        else if(FT1<=69.198)tmp1=95.9;
        else if(FT1<=69.2744)tmp1=96;
    }
    else if(FT1<=69.9639){
        if(FT1==69.2744)tmp1=96;
        else if(FT1<=69.3508)tmp1=96.1;
        else if(FT1<=69.4273)tmp1=96.2;
        else if(FT1<=69.5038)tmp1=96.3;
        else if(FT1<=69.5804)tmp1=96.4;
        else if(FT1<=69.657)tmp1=96.5;
        else if(FT1<=69.7337)tmp1=96.6;
        else if(FT1<=69.8104)tmp1=96.7;
        else if(FT1<=69.8871)tmp1=96.8;
        else if(FT1<=69.9639)tmp1=96.9;
        else if(FT1<=70.0408)tmp1=97;
    }
    else if(FT1<=70.7344){
        if(FT1==70.0408)tmp1=97;
        else if(FT1<=70.1177)tmp1=97.1;
        else if(FT1<=70.1946)tmp1=97.2;
        else if(FT1<=70.2716)tmp1=97.3;
        else if(FT1<=70.3486)tmp1=97.4;
        else if(FT1<=70.4257)tmp1=97.5;
        else if(FT1<=70.5028)tmp1=97.6;
        else if(FT1<=70.58)tmp1=97.7;
        else if(FT1<=70.6572)tmp1=97.8;
        else if(FT1<=70.7344)tmp1=97.9;
        else if(FT1<=70.8117)tmp1=98;
    }
    else if(FT1<=71.5095){
        if(FT1==70.8117)tmp1=98;
        else if(FT1<=70.8891)tmp1=98.1;
        else if(FT1<=70.9665)tmp1=98.2;
        else if(FT1<=71.0439)tmp1=98.3;
        else if(FT1<=71.1214)tmp1=98.4;
        else if(FT1<=71.1989)tmp1=98.5;
        else if(FT1<=71.2765)tmp1=98.6;
        else if(FT1<=71.3541)tmp1=98.7;
        else if(FT1<=71.4318)tmp1=98.8;
        else if(FT1<=71.5095)tmp1=98.9;
        else if(FT1<=71.5872)tmp1=99;
    }
    else if(FT1<=72.289){
        if(FT1==71.5872)tmp1=99;
        else if(FT1<=71.665)tmp1=99.1;
        else if(FT1<=71.7429)tmp1=99.2;
        else if(FT1<=71.8207)tmp1=99.3;
        else if(FT1<=71.8987)tmp1=99.4;
        else if(FT1<=71.9766)tmp1=99.5;
        else if(FT1<=72.0547)tmp1=99.6;
        else if(FT1<=72.1327)tmp1=99.7;
        else if(FT1<=72.2108)tmp1=99.8;
        else if(FT1<=72.289)tmp1=99.9;
        else if(FT1<=72.3672)tmp1=100;
    }
    else if(FT1<=73.0731){
        if(FT1==72.3672)tmp1=100;
        else if(FT1<=72.4455)tmp1=100.1;
        else if(FT1<=72.5238)tmp1=100.2;
        else if(FT1<=72.6021)tmp1=100.3;
        else if(FT1<=72.6805)tmp1=100.4;
        else if(FT1<=72.7589)tmp1=100.5;
        else if(FT1<=72.8374)tmp1=100.6;
        else if(FT1<=72.9159)tmp1=100.7;
        else if(FT1<=72.9945)tmp1=100.8;
        else if(FT1<=73.0731)tmp1=100.9;
        else if(FT1<=73.1517)tmp1=101;
    }
    else if(FT1<=73.8616){
        if(FT1==73.1517)tmp1=101;
        else if(FT1<=73.2304)tmp1=101.1;
        else if(FT1<=73.3092)tmp1=101.2;
        else if(FT1<=73.388)tmp1=101.3;
        else if(FT1<=73.4668)tmp1=101.4;
        else if(FT1<=73.5457)tmp1=101.5;
        else if(FT1<=73.6246)tmp1=101.6;
        else if(FT1<=73.7036)tmp1=101.7;
        else if(FT1<=73.7826)tmp1=101.8;
        else if(FT1<=73.8616)tmp1=101.9;
        else if(FT1<=73.9407)tmp1=102;
    }
    else if(FT1<=74.6547){
        if(FT1==73.9407)tmp1=102;
        else if(FT1<=74.0199)tmp1=102.1;
        else if(FT1<=74.0991)tmp1=102.2;
        else if(FT1<=74.1783)tmp1=102.3;
        else if(FT1<=74.2576)tmp1=102.4;
        else if(FT1<=74.3369)tmp1=102.5;
        else if(FT1<=74.4163)tmp1=102.6;
        else if(FT1<=74.4957)tmp1=102.7;
        else if(FT1<=74.5752)tmp1=102.8;
        else if(FT1<=74.6547)tmp1=102.9;
        else if(FT1<=74.7343)tmp1=103;
    }
    else if(FT1<=75.4523){
        if(FT1==74.7343)tmp1=103;
        else if(FT1<=74.8139)tmp1=103.1;
        else if(FT1<=74.8935)tmp1=103.2;
        else if(FT1<=74.9732)tmp1=103.3;
        else if(FT1<=75.0529)tmp1=103.4;
        else if(FT1<=75.1327)tmp1=103.5;
        else if(FT1<=75.2125)tmp1=103.6;
        else if(FT1<=75.2924)tmp1=103.7;
        else if(FT1<=75.3723)tmp1=103.8;
        else if(FT1<=75.4523)tmp1=103.9;
        else if(FT1<=75.5323)tmp1=104;
    }
    else if(FT1<=76.2543){
        if(FT1==75.5323)tmp1=104;
        else if(FT1<=75.6123)tmp1=104.1;
        else if(FT1<=75.6924)tmp1=104.2;
        else if(FT1<=75.7726)tmp1=104.3;
        else if(FT1<=75.8527)tmp1=104.4;
        else if(FT1<=75.933)tmp1=104.5;
        else if(FT1<=76.0132)tmp1=104.6;
        else if(FT1<=76.0936)tmp1=104.7;
        else if(FT1<=76.1739)tmp1=104.8;
        else if(FT1<=76.2543)tmp1=104.9;
        else if(FT1<=76.3348)tmp1=105;
    }
    else if(FT1<=77.0609){
        if(FT1==76.3348)tmp1=105;
        else if(FT1<=76.4153)tmp1=105.1;
        else if(FT1<=76.4958)tmp1=105.2;
        else if(FT1<=76.5764)tmp1=105.3;
        else if(FT1<=76.6571)tmp1=105.4;
        else if(FT1<=76.7377)tmp1=105.5;
        else if(FT1<=76.8185)tmp1=105.6;
        else if(FT1<=76.8992)tmp1=105.7;
        else if(FT1<=76.98)tmp1=105.8;
        else if(FT1<=77.0609)tmp1=105.9;
        else if(FT1<=77.1418)tmp1=106;
    }
    else if(FT1<=77.8719){
        if(FT1==77.1418)tmp1=106;
        else if(FT1<=77.2227)tmp1=106.1;
        else if(FT1<=77.3037)tmp1=106.2;
        else if(FT1<=77.3848)tmp1=106.3;
        else if(FT1<=77.4659)tmp1=106.4;
        else if(FT1<=77.547)tmp1=106.5;
        else if(FT1<=77.6282)tmp1=106.6;
        else if(FT1<=77.7094)tmp1=106.7;
        else if(FT1<=77.7906)tmp1=106.8;
        else if(FT1<=77.8719)tmp1=106.9;
        else if(FT1<=77.9533)tmp1=107;
    }
    else if(FT1<=78.6874){
        if(FT1==77.9533)tmp1=107;
        else if(FT1<=78.0347)tmp1=107.1;
        else if(FT1<=78.1161)tmp1=107.2;
        else if(FT1<=78.1976)tmp1=107.3;
        else if(FT1<=78.2791)tmp1=107.4;
        else if(FT1<=78.3607)tmp1=107.5;
        else if(FT1<=78.4423)tmp1=107.6;
        else if(FT1<=78.524)tmp1=107.7;
        else if(FT1<=78.6057)tmp1=107.8;
        else if(FT1<=78.6874)tmp1=107.9;
        else if(FT1<=78.7692)tmp1=108;
    }
    else if(FT1<=79.5074){
        if(FT1==78.7692)tmp1=108;
        else if(FT1<=78.8511)tmp1=108.1;
        else if(FT1<=78.933)tmp1=108.2;
        else if(FT1<=79.0149)tmp1=108.3;
        else if(FT1<=79.0969)tmp1=108.4;
        else if(FT1<=79.1789)tmp1=108.5;
        else if(FT1<=79.261)tmp1=108.6;
        else if(FT1<=79.3431)tmp1=108.7;
        else if(FT1<=79.4252)tmp1=108.8;
        else if(FT1<=79.5074)tmp1=108.9;
        else if(FT1<=79.5897)tmp1=109;
    }
    else if(FT1<=80.3319){
        if(FT1==79.5897)tmp1=109;
        else if(FT1<=79.672)tmp1=109.1;
        else if(FT1<=79.7543)tmp1=109.2;
        else if(FT1<=79.8367)tmp1=109.3;
        else if(FT1<=79.9191)tmp1=109.4;
        else if(FT1<=80.0016)tmp1=109.5;
        else if(FT1<=80.0841)tmp1=109.6;
        else if(FT1<=80.1666)tmp1=109.7;
        else if(FT1<=80.2492)tmp1=109.8;
        else if(FT1<=80.3319)tmp1=109.9;
        else if(FT1<=80.4146)tmp1=110;
    }
    else if(FT1<=81.1608){
        if(FT1==80.4146)tmp1=110;
        else if(FT1<=80.4973)tmp1=110.1;
        else if(FT1<=80.5801)tmp1=110.2;
        else if(FT1<=80.6629)tmp1=110.3;
        else if(FT1<=80.7458)tmp1=110.4;
        else if(FT1<=80.8287)tmp1=110.5;
        else if(FT1<=80.9117)tmp1=110.6;
        else if(FT1<=80.9947)tmp1=110.7;
        else if(FT1<=81.0777)tmp1=110.8;
        else if(FT1<=81.1608)tmp1=110.9;
        else if(FT1<=81.2439)tmp1=111;
    }
    else if(FT1<=81.9942){
        if(FT1==81.2439)tmp1=111;
        else if(FT1<=81.3271)tmp1=111.1;
        else if(FT1<=81.4103)tmp1=111.2;
        else if(FT1<=81.4936)tmp1=111.3;
        else if(FT1<=81.5769)tmp1=111.4;
        else if(FT1<=81.6603)tmp1=111.5;
        else if(FT1<=81.7437)tmp1=111.6;
        else if(FT1<=81.8271)tmp1=111.7;
        else if(FT1<=81.9106)tmp1=111.8;
        else if(FT1<=81.9942)tmp1=111.9;
        else if(FT1<=82.0777)tmp1=112;
    }
    else if(FT1<=82.832){
        if(FT1==82.0777)tmp1=112;
        else if(FT1<=82.1614)tmp1=112.1;
        else if(FT1<=82.245)tmp1=112.2;
        else if(FT1<=82.3288)tmp1=112.3;
        else if(FT1<=82.4125)tmp1=112.4;
        else if(FT1<=82.4963)tmp1=112.5;
        else if(FT1<=82.5802)tmp1=112.6;
        else if(FT1<=82.6641)tmp1=112.7;
        else if(FT1<=82.748)tmp1=112.8;
        else if(FT1<=82.832)tmp1=112.9;
        else if(FT1<=82.916)tmp1=113;
    }
    else if(FT1<=83.6742){
        if(FT1==82.916)tmp1=113;
        else if(FT1<=83.0001)tmp1=113.1;
        else if(FT1<=83.0842)tmp1=113.2;
        else if(FT1<=83.1683)tmp1=113.3;
        else if(FT1<=83.2525)tmp1=113.4;
        else if(FT1<=83.3368)tmp1=113.5;
        else if(FT1<=83.4211)tmp1=113.6;
        else if(FT1<=83.5054)tmp1=113.7;
        else if(FT1<=83.5898)tmp1=113.8;
        else if(FT1<=83.6742)tmp1=113.9;
        else if(FT1<=83.7587)tmp1=114;
    }
    else if(FT1<=84.5209){
        if(FT1==83.7587)tmp1=114;
        else if(FT1<=83.8432)tmp1=114.1;
        else if(FT1<=83.9278)tmp1=114.2;
        else if(FT1<=84.0124)tmp1=114.3;
        else if(FT1<=84.097)tmp1=114.4;
        else if(FT1<=84.1817)tmp1=114.5;
        else if(FT1<=84.2664)tmp1=114.6;
        else if(FT1<=84.3512)tmp1=114.7;
        else if(FT1<=84.436)tmp1=114.8;
        else if(FT1<=84.5209)tmp1=114.9;
        else if(FT1<=84.6058)tmp1=115;
    }
    else if(FT1<=85.372){
        if(FT1==84.6058)tmp1=115;
        else if(FT1<=84.6908)tmp1=115.1;
        else if(FT1<=84.7758)tmp1=115.2;
        else if(FT1<=84.8608)tmp1=115.3;
        else if(FT1<=84.9459)tmp1=115.4;
        else if(FT1<=85.0311)tmp1=115.5;
        else if(FT1<=85.1162)tmp1=115.6;
        else if(FT1<=85.2015)tmp1=115.7;
        else if(FT1<=85.2867)tmp1=115.8;
        else if(FT1<=85.372)tmp1=115.9;
        else if(FT1<=85.4574)tmp1=116;
    }
    else if(FT1<=86.2276){
        if(FT1==85.4574)tmp1=116;
        else if(FT1<=85.5428)tmp1=116.1;
        else if(FT1<=85.6282)tmp1=116.2;
        else if(FT1<=85.7137)tmp1=116.3;
        else if(FT1<=85.7993)tmp1=116.4;
        else if(FT1<=85.8848)tmp1=116.5;
        else if(FT1<=85.9704)tmp1=116.6;
        else if(FT1<=86.0561)tmp1=116.7;
        else if(FT1<=86.1418)tmp1=116.8;
        else if(FT1<=86.2276)tmp1=116.9;
        else if(FT1<=86.3134)tmp1=117;
    }
    else if(FT1<=87.0875){
        if(FT1==86.3134)tmp1=117;
        else if(FT1<=86.3992)tmp1=117.1;
        else if(FT1<=86.4851)tmp1=117.2;
        else if(FT1<=86.571)tmp1=117.3;
        else if(FT1<=86.657)tmp1=117.4;
        else if(FT1<=86.743)tmp1=117.5;
        else if(FT1<=86.8291)tmp1=117.6;
        else if(FT1<=86.9152)tmp1=117.7;
        else if(FT1<=87.0013)tmp1=117.8;
        else if(FT1<=87.0875)tmp1=117.9;
        else if(FT1<=87.1738)tmp1=118;
    }
    else if(FT1<=87.9519){
        if(FT1==87.1738)tmp1=118;
        else if(FT1<=87.26)tmp1=118.1;
        else if(FT1<=87.3464)tmp1=118.2;
        else if(FT1<=87.4327)tmp1=118.3;
        else if(FT1<=87.5192)tmp1=118.4;
        else if(FT1<=87.6056)tmp1=118.5;
        else if(FT1<=87.6921)tmp1=118.6;
        else if(FT1<=87.7787)tmp1=118.7;
        else if(FT1<=87.8653)tmp1=118.8;
        else if(FT1<=87.9519)tmp1=118.9;
        else if(FT1<=88.0386)tmp1=119;
    }
    else if(FT1<=88.8207){
        if(FT1==88.0386)tmp1=119;
        else if(FT1<=88.1253)tmp1=119.1;
        else if(FT1<=88.2121)tmp1=119.2;
        else if(FT1<=88.2989)tmp1=119.3;
        else if(FT1<=88.3857)tmp1=119.4;
        else if(FT1<=88.4726)tmp1=119.5;
        else if(FT1<=88.5596)tmp1=119.6;
        else if(FT1<=88.6465)tmp1=119.7;
        else if(FT1<=88.7336)tmp1=119.8;
        else if(FT1<=88.8207)tmp1=119.9;
        else if(FT1<=88.9078)tmp1=120;
    }
    else if(FT1<=89.6938){
        if(FT1==88.9078)tmp1=120;
        else if(FT1<=88.9949)tmp1=120.1;
        else if(FT1<=89.0821)tmp1=120.2;
        else if(FT1<=89.1694)tmp1=120.3;
        else if(FT1<=89.2567)tmp1=120.4;
        else if(FT1<=89.344)tmp1=120.5;
        else if(FT1<=89.4314)tmp1=120.6;
        else if(FT1<=89.5188)tmp1=120.7;
        else if(FT1<=89.6063)tmp1=120.8;
        else if(FT1<=89.6938)tmp1=120.9;
        else if(FT1<=89.7814)tmp1=121;
    }
    else if(FT1<=90.5714){
        if(FT1==89.7814)tmp1=121;
        else if(FT1<=89.869)tmp1=121.1;
        else if(FT1<=89.9566)tmp1=121.2;
        else if(FT1<=90.0443)tmp1=121.3;
        else if(FT1<=90.132)tmp1=121.4;
        else if(FT1<=90.2198)tmp1=121.5;
        else if(FT1<=90.3076)tmp1=121.6;
        else if(FT1<=90.3955)tmp1=121.7;
        else if(FT1<=90.4834)tmp1=121.8;
        else if(FT1<=90.5714)tmp1=121.9;
        else if(FT1<=90.6593)tmp1=122;
    }
    else if(FT1<=91.4533){
        if(FT1==90.6593)tmp1=122;
        else if(FT1<=90.7474)tmp1=122.1;
        else if(FT1<=90.8355)tmp1=122.2;
        else if(FT1<=90.9236)tmp1=122.3;
        else if(FT1<=91.0118)tmp1=122.4;
        else if(FT1<=91.1)tmp1=122.5;
        else if(FT1<=91.1882)tmp1=122.6;
        else if(FT1<=91.2765)tmp1=122.7;
        else if(FT1<=91.3649)tmp1=122.8;
        else if(FT1<=91.4533)tmp1=122.9;
        else if(FT1<=91.5417)tmp1=123;
    }
    else if(FT1<=92.3396){
        if(FT1==91.5417)tmp1=123;
        else if(FT1<=91.6302)tmp1=123.1;
        else if(FT1<=91.7187)tmp1=123.2;
        else if(FT1<=91.8073)tmp1=123.3;
        else if(FT1<=91.8959)tmp1=123.4;
        else if(FT1<=91.9845)tmp1=123.5;
        else if(FT1<=92.0732)tmp1=123.6;
        else if(FT1<=92.162)tmp1=123.7;
        else if(FT1<=92.2508)tmp1=123.8;
        else if(FT1<=92.3396)tmp1=123.9;
        else if(FT1<=92.4285)tmp1=124;
    }
    else if(FT1<=93.2302){
        if(FT1==92.4285)tmp1=124;
        else if(FT1<=92.5174)tmp1=124.1;
        else if(FT1<=92.6063)tmp1=124.2;
        else if(FT1<=92.6953)tmp1=124.3;
        else if(FT1<=92.7844)tmp1=124.4;
        else if(FT1<=92.8735)tmp1=124.5;
        else if(FT1<=92.9626)tmp1=124.6;
        else if(FT1<=93.0518)tmp1=124.7;
        else if(FT1<=93.141)tmp1=124.8;
        else if(FT1<=93.2302)tmp1=124.9;
        else if(FT1<=93.3196)tmp1=125;
    }
    else if(FT1<=94.1253){
        if(FT1==93.3196)tmp1=125;
        else if(FT1<=93.4089)tmp1=125.1;
        else if(FT1<=93.4983)tmp1=125.2;
        else if(FT1<=93.5877)tmp1=125.3;
        else if(FT1<=93.6772)tmp1=125.4;
        else if(FT1<=93.7667)tmp1=125.5;
        else if(FT1<=93.8563)tmp1=125.6;
        else if(FT1<=93.9459)tmp1=125.7;
        else if(FT1<=94.0356)tmp1=125.8;
        else if(FT1<=94.1253)tmp1=125.9;
        else if(FT1<=94.215)tmp1=126;
    }
    else if(FT1<=95.0247){
        if(FT1==94.215)tmp1=126;
        else if(FT1<=94.3048)tmp1=126.1;
        else if(FT1<=94.3946)tmp1=126.2;
        else if(FT1<=94.4845)tmp1=126.3;
        else if(FT1<=94.5744)tmp1=126.4;
        else if(FT1<=94.6644)tmp1=126.5;
        else if(FT1<=94.7544)tmp1=126.6;
        else if(FT1<=94.8444)tmp1=126.7;
        else if(FT1<=94.9345)tmp1=126.8;
        else if(FT1<=95.0247)tmp1=126.9;
        else if(FT1<=95.1148)tmp1=127;
    }
    else if(FT1<=95.9284){
        if(FT1==95.1148)tmp1=127;
        else if(FT1<=95.2051)tmp1=127.1;
        else if(FT1<=95.2953)tmp1=127.2;
        else if(FT1<=95.3856)tmp1=127.3;
        else if(FT1<=95.476)tmp1=127.4;
        else if(FT1<=95.5664)tmp1=127.5;
        else if(FT1<=95.6568)tmp1=127.6;
        else if(FT1<=95.7473)tmp1=127.7;
        else if(FT1<=95.8378)tmp1=127.8;
        else if(FT1<=95.9284)tmp1=127.9;
        else if(FT1<=96.019)tmp1=128;
    }
    else if(FT1<=96.8364){
        if(FT1==96.019)tmp1=128;
        else if(FT1<=96.1096)tmp1=128.1;
        else if(FT1<=96.2003)tmp1=128.2;
        else if(FT1<=96.2911)tmp1=128.3;
        else if(FT1<=96.3819)tmp1=128.4;
        else if(FT1<=96.4727)tmp1=128.5;
        else if(FT1<=96.5636)tmp1=128.6;
        else if(FT1<=96.6545)tmp1=128.7;
        else if(FT1<=96.7454)tmp1=128.8;
        else if(FT1<=96.8364)tmp1=128.9;
        else if(FT1<=96.9275)tmp1=129;
    }
    else if(FT1<=97.7488){
        if(FT1==96.9275)tmp1=129;
        else if(FT1<=97.0186)tmp1=129.1;
        else if(FT1<=97.1097)tmp1=129.2;
        else if(FT1<=97.2009)tmp1=129.3;
        else if(FT1<=97.2921)tmp1=129.4;
        else if(FT1<=97.3834)tmp1=129.5;
        else if(FT1<=97.4747)tmp1=129.6;
        else if(FT1<=97.566)tmp1=129.7;
        else if(FT1<=97.6574)tmp1=129.8;
        else if(FT1<=97.7488)tmp1=129.9;
        else if(FT1<=97.8403)tmp1=130;
    }
    else if(FT1<=98.6655){
        if(FT1==97.8403)tmp1=130;
        else if(FT1<=97.9318)tmp1=130.1;
        else if(FT1<=98.0234)tmp1=130.2;
        else if(FT1<=98.115)tmp1=130.3;
        else if(FT1<=98.2066)tmp1=130.4;
        else if(FT1<=98.2983)tmp1=130.5;
        else if(FT1<=98.3901)tmp1=130.6;
        else if(FT1<=98.4819)tmp1=130.7;
        else if(FT1<=98.5737)tmp1=130.8;
        else if(FT1<=98.6655)tmp1=130.9;
        else if(FT1<=98.7575)tmp1=131;
    }
    else if(FT1<=99.5866){
        if(FT1==98.7575)tmp1=131;
        else if(FT1<=98.8494)tmp1=131.1;
        else if(FT1<=98.9414)tmp1=131.2;
        else if(FT1<=99.0334)tmp1=131.3;
        else if(FT1<=99.1255)tmp1=131.4;
        else if(FT1<=99.2176)tmp1=131.5;
        else if(FT1<=99.3098)tmp1=131.6;
        else if(FT1<=99.402)tmp1=131.7;
        else if(FT1<=99.4943)tmp1=131.8;
        else if(FT1<=99.5866)tmp1=131.9;
        else if(FT1<=99.6789)tmp1=132;
    }
    else if(FT1<=100.512){
        if(FT1==99.6789)tmp1=132;
        else if(FT1<=99.7713)tmp1=132.1;
        else if(FT1<=99.8637)tmp1=132.2;
        else if(FT1<=99.9562)tmp1=132.3;
        else if(FT1<=100.049)tmp1=132.4;
        else if(FT1<=100.141)tmp1=132.5;
        else if(FT1<=100.234)tmp1=132.6;
        else if(FT1<=100.326)tmp1=132.7;
        else if(FT1<=100.419)tmp1=132.8;
        else if(FT1<=100.512)tmp1=132.9;
        else if(FT1<=100.605)tmp1=133;
    }
    else if(FT1<=101.442){
        if(FT1==100.605)tmp1=133;
        else if(FT1<=100.697)tmp1=133.1;
        else if(FT1<=100.79)tmp1=133.2;
        else if(FT1<=100.883)tmp1=133.3;
        else if(FT1<=100.976)tmp1=133.4;
        else if(FT1<=101.069)tmp1=133.5;
        else if(FT1<=101.162)tmp1=133.6;
        else if(FT1<=101.255)tmp1=133.7;
        else if(FT1<=101.348)tmp1=133.8;
        else if(FT1<=101.442)tmp1=133.9;
        else if(FT1<=101.535)tmp1=134;
    }
    else if(FT1<=102.375){
        if(FT1==101.535)tmp1=134;
        else if(FT1<=101.628)tmp1=134.1;
        else if(FT1<=101.721)tmp1=134.2;
        else if(FT1<=101.815)tmp1=134.3;
        else if(FT1<=101.908)tmp1=134.4;
        else if(FT1<=102.001)tmp1=134.5;
        else if(FT1<=102.095)tmp1=134.6;
        else if(FT1<=102.188)tmp1=134.7;
        else if(FT1<=102.282)tmp1=134.8;
        else if(FT1<=102.375)tmp1=134.9;
        else if(FT1<=102.469)tmp1=135;
    }
    else if(FT1<=103.314){
        if(FT1==102.469)tmp1=135;
        else if(FT1<=102.563)tmp1=135.1;
        else if(FT1<=102.656)tmp1=135.2;
        else if(FT1<=102.75)tmp1=135.3;
        else if(FT1<=102.844)tmp1=135.4;
        else if(FT1<=102.938)tmp1=135.5;
        else if(FT1<=103.032)tmp1=135.6;
        else if(FT1<=103.126)tmp1=135.7;
        else if(FT1<=103.22)tmp1=135.8;
        else if(FT1<=103.314)tmp1=135.9;
        else if(FT1<=103.408)tmp1=136;
    }
    else if(FT1<=104.256){
        if(FT1==103.408)tmp1=136;
        else if(FT1<=103.502)tmp1=136.1;
        else if(FT1<=103.596)tmp1=136.2;
        else if(FT1<=103.69)tmp1=136.3;
        else if(FT1<=103.784)tmp1=136.4;
        else if(FT1<=103.879)tmp1=136.5;
        else if(FT1<=103.973)tmp1=136.6;
        else if(FT1<=104.067)tmp1=136.7;
        else if(FT1<=104.162)tmp1=136.8;
        else if(FT1<=104.256)tmp1=136.9;
        else if(FT1<=104.351)tmp1=137;
    }
    else if(FT1<=105.203){
        if(FT1==104.351)tmp1=137;
        else if(FT1<=104.445)tmp1=137.1;
        else if(FT1<=104.54)tmp1=137.2;
        else if(FT1<=104.634)tmp1=137.3;
        else if(FT1<=104.729)tmp1=137.4;
        else if(FT1<=104.824)tmp1=137.5;
        else if(FT1<=104.918)tmp1=137.6;
        else if(FT1<=105.013)tmp1=137.7;
        else if(FT1<=105.108)tmp1=137.8;
        else if(FT1<=105.203)tmp1=137.9;
        else if(FT1<=105.298)tmp1=138;
    }
    else if(FT1<=106.154){
        if(FT1==105.298)tmp1=138;
        else if(FT1<=105.393)tmp1=138.1;
        else if(FT1<=105.488)tmp1=138.2;
        else if(FT1<=105.583)tmp1=138.3;
        else if(FT1<=105.678)tmp1=138.4;
        else if(FT1<=105.773)tmp1=138.5;
        else if(FT1<=105.868)tmp1=138.6;
        else if(FT1<=105.963)tmp1=138.7;
        else if(FT1<=106.059)tmp1=138.8;
        else if(FT1<=106.154)tmp1=138.9;
        else if(FT1<=106.249)tmp1=139;
    }
    else if(FT1<=107.109){
        if(FT1==106.249)tmp1=139;
        else if(FT1<=106.345)tmp1=139.1;
        else if(FT1<=106.44)tmp1=139.2;
        else if(FT1<=106.536)tmp1=139.3;
        else if(FT1<=106.631)tmp1=139.4;
        else if(FT1<=106.727)tmp1=139.5;
        else if(FT1<=106.822)tmp1=139.6;
        else if(FT1<=106.918)tmp1=139.7;
        else if(FT1<=107.013)tmp1=139.8;
        else if(FT1<=107.109)tmp1=139.9;
        else if(FT1<=107.205)tmp1=140;
    }
    else if(FT1<=108.069){
        if(FT1==107.205)tmp1=140;
        else if(FT1<=107.301)tmp1=140.1;
        else if(FT1<=107.397)tmp1=140.2;
        else if(FT1<=107.492)tmp1=140.3;
        else if(FT1<=107.588)tmp1=140.4;
        else if(FT1<=107.684)tmp1=140.5;
        else if(FT1<=107.78)tmp1=140.6;
        else if(FT1<=107.876)tmp1=140.7;
        else if(FT1<=107.973)tmp1=140.8;
        else if(FT1<=108.069)tmp1=140.9;
        else if(FT1<=108.165)tmp1=141;
    }
    else if(FT1<=109.032){
        if(FT1==108.165)tmp1=141;
        else if(FT1<=108.261)tmp1=141.1;
        else if(FT1<=108.357)tmp1=141.2;
        else if(FT1<=108.454)tmp1=141.3;
        else if(FT1<=108.55)tmp1=141.4;
        else if(FT1<=108.646)tmp1=141.5;
        else if(FT1<=108.743)tmp1=141.6;
        else if(FT1<=108.839)tmp1=141.7;
        else if(FT1<=108.936)tmp1=141.8;
        else if(FT1<=109.032)tmp1=141.9;
        else if(FT1<=109.129)tmp1=142;
    }
    else if(FT1<=110){
        if(FT1==109.129)tmp1=142;
        else if(FT1<=109.226)tmp1=142.1;
        else if(FT1<=109.322)tmp1=142.2;
        else if(FT1<=109.419)tmp1=142.3;
        else if(FT1<=109.516)tmp1=142.4;
        else if(FT1<=109.613)tmp1=142.5;
        else if(FT1<=109.71)tmp1=142.6;
        else if(FT1<=109.806)tmp1=142.7;
        else if(FT1<=109.903)tmp1=142.8;
        else if(FT1<=110)tmp1=142.9;
        else if(FT1<=110.097)tmp1=143;
    }
    else if(FT1<=110.973){
        if(FT1==110.097)tmp1=143;
        else if(FT1<=110.195)tmp1=143.1;
        else if(FT1<=110.292)tmp1=143.2;
        else if(FT1<=110.389)tmp1=143.3;
        else if(FT1<=110.486)tmp1=143.4;
        else if(FT1<=110.583)tmp1=143.5;
        else if(FT1<=110.68)tmp1=143.6;
        else if(FT1<=110.778)tmp1=143.7;
        else if(FT1<=110.875)tmp1=143.8;
        else if(FT1<=110.973)tmp1=143.9;
        else if(FT1<=111.07)tmp1=144;
    }
    else if(FT1<=111.949){
        if(FT1==111.07)tmp1=144;
        else if(FT1<=111.168)tmp1=144.1;
        else if(FT1<=111.265)tmp1=144.2;
        else if(FT1<=111.363)tmp1=144.3;
        else if(FT1<=111.46)tmp1=144.4;
        else if(FT1<=111.558)tmp1=144.5;
        else if(FT1<=111.656)tmp1=144.6;
        else if(FT1<=111.753)tmp1=144.7;
        else if(FT1<=111.851)tmp1=144.8;
        else if(FT1<=111.949)tmp1=144.9;
        else if(FT1<=112.047)tmp1=145;
    }
    else if(FT1<=112.93){
        if(FT1==112.047)tmp1=145;
        else if(FT1<=112.145)tmp1=145.1;
        else if(FT1<=112.243)tmp1=145.2;
        else if(FT1<=112.341)tmp1=145.3;
        else if(FT1<=112.439)tmp1=145.4;
        else if(FT1<=112.537)tmp1=145.5;
        else if(FT1<=112.635)tmp1=145.6;
        else if(FT1<=112.733)tmp1=145.7;
        else if(FT1<=112.831)tmp1=145.8;
        else if(FT1<=112.93)tmp1=145.9;
        else if(FT1<=113.028)tmp1=146;
    }
    else if(FT1<=113.914){
        if(FT1==113.028)tmp1=146;
        else if(FT1<=113.126)tmp1=146.1;
        else if(FT1<=113.225)tmp1=146.2;
        else if(FT1<=113.323)tmp1=146.3;
        else if(FT1<=113.421)tmp1=146.4;
        else if(FT1<=113.52)tmp1=146.5;
        else if(FT1<=113.619)tmp1=146.6;
        else if(FT1<=113.717)tmp1=146.7;
        else if(FT1<=113.816)tmp1=146.8;
        else if(FT1<=113.914)tmp1=146.9;
        else if(FT1<=114.013)tmp1=147;
    }
    else if(FT1<=114.903){
        if(FT1==114.013)tmp1=147;
        else if(FT1<=114.112)tmp1=147.1;
        else if(FT1<=114.211)tmp1=147.2;
        else if(FT1<=114.309)tmp1=147.3;
        else if(FT1<=114.408)tmp1=147.4;
        else if(FT1<=114.507)tmp1=147.5;
        else if(FT1<=114.606)tmp1=147.6;
        else if(FT1<=114.705)tmp1=147.7;
        else if(FT1<=114.804)tmp1=147.8;
        else if(FT1<=114.903)tmp1=147.9;
        else if(FT1<=115.003)tmp1=148;
    }
    else if(FT1<=115.897){
        if(FT1==115.003)tmp1=148;
        else if(FT1<=115.102)tmp1=148.1;
        else if(FT1<=115.201)tmp1=148.2;
        else if(FT1<=115.3)tmp1=148.3;
        else if(FT1<=115.399)tmp1=148.4;
        else if(FT1<=115.499)tmp1=148.5;
        else if(FT1<=115.598)tmp1=148.6;
        else if(FT1<=115.698)tmp1=148.7;
        else if(FT1<=115.797)tmp1=148.8;
        else if(FT1<=115.897)tmp1=148.9;
        else if(FT1<=115.996)tmp1=149;
    }
    else if(FT1<=116.894){
        if(FT1==115.996)tmp1=149;
        else if(FT1<=116.096)tmp1=149.1;
        else if(FT1<=116.195)tmp1=149.2;
        else if(FT1<=116.295)tmp1=149.3;
        else if(FT1<=116.395)tmp1=149.4;
        else if(FT1<=116.494)tmp1=149.5;
        else if(FT1<=116.594)tmp1=149.6;
        else if(FT1<=116.694)tmp1=149.7;
        else if(FT1<=116.794)tmp1=149.8;
        else if(FT1<=116.894)tmp1=149.9;
        else if(FT1<=116.994)tmp1=150;
    }
    else if(FT1<=117.895){
        if(FT1==116.994)tmp1=150;
        else if(FT1<=117.094)tmp1=150.1;
        else if(FT1<=117.194)tmp1=150.2;
        else if(FT1<=117.294)tmp1=150.3;
        else if(FT1<=117.394)tmp1=150.4;
        else if(FT1<=117.494)tmp1=150.5;
        else if(FT1<=117.594)tmp1=150.6;
        else if(FT1<=117.695)tmp1=150.7;
        else if(FT1<=117.795)tmp1=150.8;
        else if(FT1<=117.895)tmp1=150.9;
        else if(FT1<=117.996)tmp1=151;
    }
    else if(FT1<=118.901){
        if(FT1==117.996)tmp1=151;
        else if(FT1<=118.096)tmp1=151.1;
        else if(FT1<=118.197)tmp1=151.2;
        else if(FT1<=118.297)tmp1=151.3;
        else if(FT1<=118.398)tmp1=151.4;
        else if(FT1<=118.498)tmp1=151.5;
        else if(FT1<=118.599)tmp1=151.6;
        else if(FT1<=118.7)tmp1=151.7;
        else if(FT1<=118.8)tmp1=151.8;
        else if(FT1<=118.901)tmp1=151.9;
        else if(FT1<=119.002)tmp1=152;
    }
    else if(FT1<=119.911){
        if(FT1==119.002)tmp1=152;
        else if(FT1<=119.103)tmp1=152.1;
        else if(FT1<=119.203)tmp1=152.2;
        else if(FT1<=119.304)tmp1=152.3;
        else if(FT1<=119.405)tmp1=152.4;
        else if(FT1<=119.506)tmp1=152.5;
        else if(FT1<=119.607)tmp1=152.6;
        else if(FT1<=119.708)tmp1=152.7;
        else if(FT1<=119.81)tmp1=152.8;
        else if(FT1<=119.911)tmp1=152.9;
        else if(FT1<=120.012)tmp1=153;
    }
    else if(FT1<=120.925){
        if(FT1==120.012)tmp1=153;
        else if(FT1<=120.113)tmp1=153.1;
        else if(FT1<=120.215)tmp1=153.2;
        else if(FT1<=120.316)tmp1=153.3;
        else if(FT1<=120.417)tmp1=153.4;
        else if(FT1<=120.519)tmp1=153.5;
        else if(FT1<=120.62)tmp1=153.6;
        else if(FT1<=120.722)tmp1=153.7;
        else if(FT1<=120.823)tmp1=153.8;
        else if(FT1<=120.925)tmp1=153.9;
        else if(FT1<=121.026)tmp1=154;
    }
    else if(FT1<=121.943){
        if(FT1==121.026)tmp1=154;
        else if(FT1<=121.128)tmp1=154.1;
        else if(FT1<=121.23)tmp1=154.2;
        else if(FT1<=121.331)tmp1=154.3;
        else if(FT1<=121.433)tmp1=154.4;
        else if(FT1<=121.535)tmp1=154.5;
        else if(FT1<=121.637)tmp1=154.6;
        else if(FT1<=121.739)tmp1=154.7;
        else if(FT1<=121.841)tmp1=154.8;
        else if(FT1<=121.943)tmp1=154.9;
        else if(FT1<=122.045)tmp1=155;
    }
    else if(FT1<=122.965){
        if(FT1==122.045)tmp1=155;
        else if(FT1<=122.147)tmp1=155.1;
        else if(FT1<=122.249)tmp1=155.2;
        else if(FT1<=122.351)tmp1=155.3;
        else if(FT1<=122.453)tmp1=155.4;
        else if(FT1<=122.556)tmp1=155.5;
        else if(FT1<=122.658)tmp1=155.6;
        else if(FT1<=122.76)tmp1=155.7;
        else if(FT1<=122.862)tmp1=155.8;
        else if(FT1<=122.965)tmp1=155.9;
        else if(FT1<=123.067)tmp1=156;
    }
    else if(FT1<=123.991){
        if(FT1==123.067)tmp1=156;
        else if(FT1<=123.17)tmp1=156.1;
        else if(FT1<=123.272)tmp1=156.2;
        else if(FT1<=123.375)tmp1=156.3;
        else if(FT1<=123.477)tmp1=156.4;
        else if(FT1<=123.58)tmp1=156.5;
        else if(FT1<=123.683)tmp1=156.6;
        else if(FT1<=123.786)tmp1=156.7;
        else if(FT1<=123.888)tmp1=156.8;
        else if(FT1<=123.991)tmp1=156.9;
        else if(FT1<=124.094)tmp1=157;
    }
    else if(FT1<=125.022){
        if(FT1==124.094)tmp1=157;
        else if(FT1<=124.197)tmp1=157.1;
        else if(FT1<=124.3)tmp1=157.2;
        else if(FT1<=124.403)tmp1=157.3;
        else if(FT1<=124.506)tmp1=157.4;
        else if(FT1<=124.609)tmp1=157.5;
        else if(FT1<=124.712)tmp1=157.6;
        else if(FT1<=124.815)tmp1=157.7;
        else if(FT1<=124.918)tmp1=157.8;
        else if(FT1<=125.022)tmp1=157.9;
        else if(FT1<=125.125)tmp1=158;
    }
    else if(FT1<=126.056){
        if(FT1==125.125)tmp1=158;
        else if(FT1<=125.228)tmp1=158.1;
        else if(FT1<=125.331)tmp1=158.2;
        else if(FT1<=125.435)tmp1=158.3;
        else if(FT1<=125.538)tmp1=158.4;
        else if(FT1<=125.642)tmp1=158.5;
        else if(FT1<=125.745)tmp1=158.6;
        else if(FT1<=125.849)tmp1=158.7;
        else if(FT1<=125.952)tmp1=158.8;
        else if(FT1<=126.056)tmp1=158.9;
        else if(FT1<=126.16)tmp1=159;
    }
    else if(FT1<=127.094){
        if(FT1==126.16)tmp1=159;
        else if(FT1<=126.263)tmp1=159.1;
        else if(FT1<=126.367)tmp1=159.2;
        else if(FT1<=126.471)tmp1=159.3;
        else if(FT1<=126.575)tmp1=159.4;
        else if(FT1<=126.679)tmp1=159.5;
        else if(FT1<=126.782)tmp1=159.6;
        else if(FT1<=126.886)tmp1=159.7;
        else if(FT1<=126.99)tmp1=159.8;
        else if(FT1<=127.094)tmp1=159.9;
        else if(FT1<=127.199)tmp1=160;
    }
    else if(FT1<=128.137){
        if(FT1==127.199)tmp1=160;
        else if(FT1<=127.303)tmp1=160.1;
        else if(FT1<=127.407)tmp1=160.2;
        else if(FT1<=127.511)tmp1=160.3;
        else if(FT1<=127.615)tmp1=160.4;
        else if(FT1<=127.72)tmp1=160.5;
        else if(FT1<=127.824)tmp1=160.6;
        else if(FT1<=127.928)tmp1=160.7;
        else if(FT1<=128.033)tmp1=160.8;
        else if(FT1<=128.137)tmp1=160.9;
        else if(FT1<=128.242)tmp1=161;
    }
    else if(FT1<=129.184){
        if(FT1==128.242)tmp1=161;
        else if(FT1<=128.346)tmp1=161.1;
        else if(FT1<=128.451)tmp1=161.2;
        else if(FT1<=128.555)tmp1=161.3;
        else if(FT1<=128.66)tmp1=161.4;
        else if(FT1<=128.765)tmp1=161.5;
        else if(FT1<=128.869)tmp1=161.6;
        else if(FT1<=128.974)tmp1=161.7;
        else if(FT1<=129.079)tmp1=161.8;
        else if(FT1<=129.184)tmp1=161.9;
        else if(FT1<=129.289)tmp1=162;
    }
    else if(FT1<=130.234){
        if(FT1==129.289)tmp1=162;
        else if(FT1<=129.394)tmp1=162.1;
        else if(FT1<=129.499)tmp1=162.2;
        else if(FT1<=129.604)tmp1=162.3;
        else if(FT1<=129.709)tmp1=162.4;
        else if(FT1<=129.814)tmp1=162.5;
        else if(FT1<=129.919)tmp1=162.6;
        else if(FT1<=130.024)tmp1=162.7;
        else if(FT1<=130.129)tmp1=162.8;
        else if(FT1<=130.234)tmp1=162.9;
        else if(FT1<=130.34)tmp1=163;
    }
    else if(FT1<=131.289){
        if(FT1==130.34)tmp1=163;
        else if(FT1<=130.445)tmp1=163.1;
        else if(FT1<=130.55)tmp1=163.2;
        else if(FT1<=130.656)tmp1=163.3;
        else if(FT1<=130.761)tmp1=163.4;
        else if(FT1<=130.867)tmp1=163.5;
        else if(FT1<=130.972)tmp1=163.6;
        else if(FT1<=131.078)tmp1=163.7;
        else if(FT1<=131.184)tmp1=163.8;
        else if(FT1<=131.289)tmp1=163.9;
        else if(FT1<=131.395)tmp1=164;
    }
    else if(FT1<=132.348){
        if(FT1==131.395)tmp1=164;
        else if(FT1<=131.501)tmp1=164.1;
        else if(FT1<=131.606)tmp1=164.2;
        else if(FT1<=131.712)tmp1=164.3;
        else if(FT1<=131.818)tmp1=164.4;
        else if(FT1<=131.924)tmp1=164.5;
        else if(FT1<=132.03)tmp1=164.6;
        else if(FT1<=132.136)tmp1=164.7;
        else if(FT1<=132.242)tmp1=164.8;
        else if(FT1<=132.348)tmp1=164.9;
        else if(FT1<=132.454)tmp1=165;
    }
    else if(FT1<=133.411){
        if(FT1==132.454)tmp1=165;
        else if(FT1<=132.56)tmp1=165.1;
        else if(FT1<=132.666)tmp1=165.2;
        else if(FT1<=132.773)tmp1=165.3;
        else if(FT1<=132.879)tmp1=165.4;
        else if(FT1<=132.985)tmp1=165.5;
        else if(FT1<=133.092)tmp1=165.6;
        else if(FT1<=133.198)tmp1=165.7;
        else if(FT1<=133.304)tmp1=165.8;
        else if(FT1<=133.411)tmp1=165.9;
        else if(FT1<=133.517)tmp1=166;
    }
    else if(FT1<=134.478){
        if(FT1==133.517)tmp1=166;
        else if(FT1<=133.624)tmp1=166.1;
        else if(FT1<=133.731)tmp1=166.2;
        else if(FT1<=133.837)tmp1=166.3;
        else if(FT1<=133.944)tmp1=166.4;
        else if(FT1<=134.05)tmp1=166.5;
        else if(FT1<=134.157)tmp1=166.6;
        else if(FT1<=134.264)tmp1=166.7;
        else if(FT1<=134.371)tmp1=166.8;
        else if(FT1<=134.478)tmp1=166.9;
        else if(FT1<=134.585)tmp1=167;
    }
    else if(FT1<=135.549){
        if(FT1==134.585)tmp1=167;
        else if(FT1<=134.692)tmp1=167.1;
        else if(FT1<=134.799)tmp1=167.2;
        else if(FT1<=134.906)tmp1=167.3;
        else if(FT1<=135.013)tmp1=167.4;
        else if(FT1<=135.12)tmp1=167.5;
        else if(FT1<=135.227)tmp1=167.6;
        else if(FT1<=135.334)tmp1=167.7;
        else if(FT1<=135.441)tmp1=167.8;
        else if(FT1<=135.549)tmp1=167.9;
        else if(FT1<=135.656)tmp1=168;
    }
    else if(FT1<=136.623){
        if(FT1==135.656)tmp1=168;
        else if(FT1<=135.763)tmp1=168.1;
        else if(FT1<=135.871)tmp1=168.2;
        else if(FT1<=135.978)tmp1=168.3;
        else if(FT1<=136.085)tmp1=168.4;
        else if(FT1<=136.193)tmp1=168.5;
        else if(FT1<=136.301)tmp1=168.6;
        else if(FT1<=136.408)tmp1=168.7;
        else if(FT1<=136.516)tmp1=168.8;
        else if(FT1<=136.623)tmp1=168.9;
        else if(FT1<=136.731)tmp1=169;
    }
    else if(FT1<=137.702){
        if(FT1==136.731)tmp1=169;
        else if(FT1<=136.839)tmp1=169.1;
        else if(FT1<=136.947)tmp1=169.2;
        else if(FT1<=137.054)tmp1=169.3;
        else if(FT1<=137.162)tmp1=169.4;
        else if(FT1<=137.27)tmp1=169.5;
        else if(FT1<=137.378)tmp1=169.6;
        else if(FT1<=137.486)tmp1=169.7;
        else if(FT1<=137.594)tmp1=169.8;
        else if(FT1<=137.702)tmp1=169.9;
        else if(FT1<=137.81)tmp1=170;
    }
    else if(FT1<=138.785){
        if(FT1==137.81)tmp1=170;
        else if(FT1<=137.919)tmp1=170.1;
        else if(FT1<=138.027)tmp1=170.2;
        else if(FT1<=138.135)tmp1=170.3;
        else if(FT1<=138.243)tmp1=170.4;
        else if(FT1<=138.351)tmp1=170.5;
        else if(FT1<=138.46)tmp1=170.6;
        else if(FT1<=138.568)tmp1=170.7;
        else if(FT1<=138.677)tmp1=170.8;
        else if(FT1<=138.785)tmp1=170.9;
        else if(FT1<=138.894)tmp1=171;
    }
    else if(FT1<=139.872){
        if(FT1==138.894)tmp1=171;
        else if(FT1<=139.002)tmp1=171.1;
        else if(FT1<=139.111)tmp1=171.2;
        else if(FT1<=139.219)tmp1=171.3;
        else if(FT1<=139.328)tmp1=171.4;
        else if(FT1<=139.437)tmp1=171.5;
        else if(FT1<=139.545)tmp1=171.6;
        else if(FT1<=139.654)tmp1=171.7;
        else if(FT1<=139.763)tmp1=171.8;
        else if(FT1<=139.872)tmp1=171.9;
        else if(FT1<=139.981)tmp1=172;
    }
    else if(FT1<=140.963){
        if(FT1==139.981)tmp1=172;
        else if(FT1<=140.09)tmp1=172.1;
        else if(FT1<=140.199)tmp1=172.2;
        else if(FT1<=140.308)tmp1=172.3;
        else if(FT1<=140.417)tmp1=172.4;
        else if(FT1<=140.526)tmp1=172.5;
        else if(FT1<=140.635)tmp1=172.6;
        else if(FT1<=140.744)tmp1=172.7;
        else if(FT1<=140.853)tmp1=172.8;
        else if(FT1<=140.963)tmp1=172.9;
        else if(FT1<=141.072)tmp1=173;
    }
    else if(FT1<=142.057){
        if(FT1==141.072)tmp1=173;
        else if(FT1<=141.181)tmp1=173.1;
        else if(FT1<=141.291)tmp1=173.2;
        else if(FT1<=141.4)tmp1=173.3;
        else if(FT1<=141.51)tmp1=173.4;
        else if(FT1<=141.619)tmp1=173.5;
        else if(FT1<=141.729)tmp1=173.6;
        else if(FT1<=141.838)tmp1=173.7;
        else if(FT1<=141.948)tmp1=173.8;
        else if(FT1<=142.057)tmp1=173.9;
        else if(FT1<=142.167)tmp1=174;
    }
    else if(FT1<=143.156){
        if(FT1==142.167)tmp1=174;
        else if(FT1<=142.277)tmp1=174.1;
        else if(FT1<=142.387)tmp1=174.2;
        else if(FT1<=142.496)tmp1=174.3;
        else if(FT1<=142.606)tmp1=174.4;
        else if(FT1<=142.716)tmp1=174.5;
        else if(FT1<=142.826)tmp1=174.6;
        else if(FT1<=142.936)tmp1=174.7;
        else if(FT1<=143.046)tmp1=174.8;
        else if(FT1<=143.156)tmp1=174.9;
        else if(FT1<=143.266)tmp1=175;
    }
    else if(FT1<=144.259){
        if(FT1==143.266)tmp1=175;
        else if(FT1<=143.376)tmp1=175.1;
        else if(FT1<=143.486)tmp1=175.2;
        else if(FT1<=143.597)tmp1=175.3;
        else if(FT1<=143.707)tmp1=175.4;
        else if(FT1<=143.817)tmp1=175.5;
        else if(FT1<=143.927)tmp1=175.6;
        else if(FT1<=144.038)tmp1=175.7;
        else if(FT1<=144.148)tmp1=175.8;
        else if(FT1<=144.259)tmp1=175.9;
        else if(FT1<=144.369)tmp1=176;
    }
    else if(FT1<=145.365){
        if(FT1==144.369)tmp1=176;
        else if(FT1<=144.48)tmp1=176.1;
        else if(FT1<=144.59)tmp1=176.2;
        else if(FT1<=144.701)tmp1=176.3;
        else if(FT1<=144.811)tmp1=176.4;
        else if(FT1<=144.922)tmp1=176.5;
        else if(FT1<=145.033)tmp1=176.6;
        else if(FT1<=145.144)tmp1=176.7;
        else if(FT1<=145.254)tmp1=176.8;
        else if(FT1<=145.365)tmp1=176.9;
        else if(FT1<=145.476)tmp1=177;
    }
    else if(FT1<=146.476){
        if(FT1==145.476)tmp1=177;
        else if(FT1<=145.587)tmp1=177.1;
        else if(FT1<=145.698)tmp1=177.2;
        else if(FT1<=145.809)tmp1=177.3;
        else if(FT1<=145.92)tmp1=177.4;
        else if(FT1<=146.031)tmp1=177.5;
        else if(FT1<=146.142)tmp1=177.6;
        else if(FT1<=146.253)tmp1=177.7;
        else if(FT1<=146.364)tmp1=177.8;
        else if(FT1<=146.476)tmp1=177.9;
        else if(FT1<=146.587)tmp1=178;
    }
    else if(FT1<=147.59){
        if(FT1==146.587)tmp1=178;
        else if(FT1<=146.698)tmp1=178.1;
        else if(FT1<=146.809)tmp1=178.2;
        else if(FT1<=146.921)tmp1=178.3;
        else if(FT1<=147.032)tmp1=178.4;
        else if(FT1<=147.144)tmp1=178.5;
        else if(FT1<=147.255)tmp1=178.6;
        else if(FT1<=147.367)tmp1=178.7;
        else if(FT1<=147.478)tmp1=178.8;
        else if(FT1<=147.59)tmp1=178.9;
        else if(FT1<=147.702)tmp1=179;
    }
    else if(FT1<=148.708){
        if(FT1==147.702)tmp1=179;
        else if(FT1<=147.813)tmp1=179.1;
        else if(FT1<=147.925)tmp1=179.2;
        else if(FT1<=148.037)tmp1=179.3;
        else if(FT1<=148.149)tmp1=179.4;
        else if(FT1<=148.26)tmp1=179.5;
        else if(FT1<=148.372)tmp1=179.6;
        else if(FT1<=148.484)tmp1=179.7;
        else if(FT1<=148.596)tmp1=179.8;
        else if(FT1<=148.708)tmp1=179.9;
        else if(FT1<=148.82)tmp1=180;
    }
    else if(FT1<=149.83){
        if(FT1==148.82)tmp1=180;
        else if(FT1<=148.932)tmp1=180.1;
        else if(FT1<=149.044)tmp1=180.2;
        else if(FT1<=149.157)tmp1=180.3;
        else if(FT1<=149.269)tmp1=180.4;
        else if(FT1<=149.381)tmp1=180.5;
        else if(FT1<=149.493)tmp1=180.6;
        else if(FT1<=149.606)tmp1=180.7;
        else if(FT1<=149.718)tmp1=180.8;
        else if(FT1<=149.83)tmp1=180.9;
        else if(FT1<=149.943)tmp1=181;
    }
    else if(FT1<=150.956){
        if(FT1==149.943)tmp1=181;
        else if(FT1<=150.055)tmp1=181.1;
        else if(FT1<=150.168)tmp1=181.2;
        else if(FT1<=150.28)tmp1=181.3;
        else if(FT1<=150.393)tmp1=181.4;
        else if(FT1<=150.505)tmp1=181.5;
        else if(FT1<=150.618)tmp1=181.6;
        else if(FT1<=150.731)tmp1=181.7;
        else if(FT1<=150.843)tmp1=181.8;
        else if(FT1<=150.956)tmp1=181.9;
        else if(FT1<=151.069)tmp1=182;
    }
    else if(FT1<=152.086){
        if(FT1==151.069)tmp1=182;
        else if(FT1<=151.182)tmp1=182.1;
        else if(FT1<=151.295)tmp1=182.2;
        else if(FT1<=151.408)tmp1=182.3;
        else if(FT1<=151.521)tmp1=182.4;
        else if(FT1<=151.634)tmp1=182.5;
        else if(FT1<=151.747)tmp1=182.6;
        else if(FT1<=151.86)tmp1=182.7;
        else if(FT1<=151.973)tmp1=182.8;
        else if(FT1<=152.086)tmp1=182.9;
        else if(FT1<=152.199)tmp1=183;
    }
    else if(FT1<=153.22){
        if(FT1==152.199)tmp1=183;
        else if(FT1<=152.313)tmp1=183.1;
        else if(FT1<=152.426)tmp1=183.2;
        else if(FT1<=152.539)tmp1=183.3;
        else if(FT1<=152.653)tmp1=183.4;
        else if(FT1<=152.766)tmp1=183.5;
        else if(FT1<=152.879)tmp1=183.6;
        else if(FT1<=152.993)tmp1=183.7;
        else if(FT1<=153.106)tmp1=183.8;
        else if(FT1<=153.22)tmp1=183.9;
        else if(FT1<=153.333)tmp1=184;
    }
    else if(FT1<=154.357){
        if(FT1==153.333)tmp1=184;
        else if(FT1<=153.447)tmp1=184.1;
        else if(FT1<=153.561)tmp1=184.2;
        else if(FT1<=153.674)tmp1=184.3;
        else if(FT1<=153.788)tmp1=184.4;
        else if(FT1<=153.902)tmp1=184.5;
        else if(FT1<=154.016)tmp1=184.6;
        else if(FT1<=154.13)tmp1=184.7;
        else if(FT1<=154.244)tmp1=184.8;
        else if(FT1<=154.357)tmp1=184.9;
        else if(FT1<=154.471)tmp1=185;
    }
    else if(FT1<=155.499){
        if(FT1==154.471)tmp1=185;
        else if(FT1<=154.585)tmp1=185.1;
        else if(FT1<=154.699)tmp1=185.2;
        else if(FT1<=154.814)tmp1=185.3;
        else if(FT1<=154.928)tmp1=185.4;
        else if(FT1<=155.042)tmp1=185.5;
        else if(FT1<=155.156)tmp1=185.6;
        else if(FT1<=155.27)tmp1=185.7;
        else if(FT1<=155.385)tmp1=185.8;
        else if(FT1<=155.499)tmp1=185.9;
        else if(FT1<=155.613)tmp1=186;
    }
    else if(FT1<=156.644){
        if(FT1==155.613)tmp1=186;
        else if(FT1<=155.728)tmp1=186.1;
        else if(FT1<=155.842)tmp1=186.2;
        else if(FT1<=155.957)tmp1=186.3;
        else if(FT1<=156.071)tmp1=186.4;
        else if(FT1<=156.186)tmp1=186.5;
        else if(FT1<=156.3)tmp1=186.6;
        else if(FT1<=156.415)tmp1=186.7;
        else if(FT1<=156.529)tmp1=186.8;
        else if(FT1<=156.644)tmp1=186.9;
        else if(FT1<=156.759)tmp1=187;
    }
    else if(FT1<=157.793){
        if(FT1==156.759)tmp1=187;
        else if(FT1<=156.874)tmp1=187.1;
        else if(FT1<=156.988)tmp1=187.2;
        else if(FT1<=157.103)tmp1=187.3;
        else if(FT1<=157.218)tmp1=187.4;
        else if(FT1<=157.333)tmp1=187.5;
        else if(FT1<=157.448)tmp1=187.6;
        else if(FT1<=157.563)tmp1=187.7;
        else if(FT1<=157.678)tmp1=187.8;
        else if(FT1<=157.793)tmp1=187.9;
        else if(FT1<=157.908)tmp1=188;
    }
    else if(FT1<=158.946){
        if(FT1==157.908)tmp1=188;
        else if(FT1<=158.023)tmp1=188.1;
        else if(FT1<=158.139)tmp1=188.2;
        else if(FT1<=158.254)tmp1=188.3;
        else if(FT1<=158.369)tmp1=188.4;
        else if(FT1<=158.484)tmp1=188.5;
        else if(FT1<=158.6)tmp1=188.6;
        else if(FT1<=158.715)tmp1=188.7;
        else if(FT1<=158.831)tmp1=188.8;
        else if(FT1<=158.946)tmp1=188.9;
        else if(FT1<=159.062)tmp1=189;
    }
    else if(FT1<=160.103){
        if(FT1==159.062)tmp1=189;
        else if(FT1<=159.177)tmp1=189.1;
        else if(FT1<=159.293)tmp1=189.2;
        else if(FT1<=159.408)tmp1=189.3;
        else if(FT1<=159.524)tmp1=189.4;
        else if(FT1<=159.64)tmp1=189.5;
        else if(FT1<=159.755)tmp1=189.6;
        else if(FT1<=159.871)tmp1=189.7;
        else if(FT1<=159.987)tmp1=189.8;
        else if(FT1<=160.103)tmp1=189.9;
        else if(FT1<=160.219)tmp1=190;
    }
    else if(FT1<=161.263){
        if(FT1==160.219)tmp1=190;
        else if(FT1<=160.335)tmp1=190.1;
        else if(FT1<=160.451)tmp1=190.2;
        else if(FT1<=160.567)tmp1=190.3;
        else if(FT1<=160.683)tmp1=190.4;
        else if(FT1<=160.799)tmp1=190.5;
        else if(FT1<=160.915)tmp1=190.6;
        else if(FT1<=161.031)tmp1=190.7;
        else if(FT1<=161.147)tmp1=190.8;
        else if(FT1<=161.263)tmp1=190.9;
        else if(FT1<=161.38)tmp1=191;
    }
    else if(FT1<=162.428){
        if(FT1==161.38)tmp1=191;
        else if(FT1<=161.496)tmp1=191.1;
        else if(FT1<=161.612)tmp1=191.2;
        else if(FT1<=161.729)tmp1=191.3;
        else if(FT1<=161.845)tmp1=191.4;
        else if(FT1<=161.961)tmp1=191.5;
        else if(FT1<=162.078)tmp1=191.6;
        else if(FT1<=162.194)tmp1=191.7;
        else if(FT1<=162.311)tmp1=191.8;
        else if(FT1<=162.428)tmp1=191.9;
        else if(FT1<=162.544)tmp1=192;
    }
    else if(FT1<=163.596){
        if(FT1==162.544)tmp1=192;
        else if(FT1<=162.661)tmp1=192.1;
        else if(FT1<=162.778)tmp1=192.2;
        else if(FT1<=162.894)tmp1=192.3;
        else if(FT1<=163.011)tmp1=192.4;
        else if(FT1<=163.128)tmp1=192.5;
        else if(FT1<=163.245)tmp1=192.6;
        else if(FT1<=163.362)tmp1=192.7;
        else if(FT1<=163.479)tmp1=192.8;
        else if(FT1<=163.596)tmp1=192.9;
        else if(FT1<=163.713)tmp1=193;
    }
    else if(FT1<=164.767){
        if(FT1==163.713)tmp1=193;
        else if(FT1<=163.83)tmp1=193.1;
        else if(FT1<=163.947)tmp1=193.2;
        else if(FT1<=164.064)tmp1=193.3;
        else if(FT1<=164.181)tmp1=193.4;
        else if(FT1<=164.298)tmp1=193.5;
        else if(FT1<=164.415)tmp1=193.6;
        else if(FT1<=164.533)tmp1=193.7;
        else if(FT1<=164.65)tmp1=193.8;
        else if(FT1<=164.767)tmp1=193.9;
        else if(FT1<=164.885)tmp1=194;
    }
    else if(FT1<=165.943){
        if(FT1==164.885)tmp1=194;
        else if(FT1<=165.002)tmp1=194.1;
        else if(FT1<=165.12)tmp1=194.2;
        else if(FT1<=165.237)tmp1=194.3;
        else if(FT1<=165.355)tmp1=194.4;
        else if(FT1<=165.472)tmp1=194.5;
        else if(FT1<=165.59)tmp1=194.6;
        else if(FT1<=165.708)tmp1=194.7;
        else if(FT1<=165.825)tmp1=194.8;
        else if(FT1<=165.943)tmp1=194.9;
        else if(FT1<=166.061)tmp1=195;
    }
    else if(FT1<=167.122){
        if(FT1==166.061)tmp1=195;
        else if(FT1<=166.179)tmp1=195.1;
        else if(FT1<=166.296)tmp1=195.2;
        else if(FT1<=166.414)tmp1=195.3;
        else if(FT1<=166.532)tmp1=195.4;
        else if(FT1<=166.65)tmp1=195.5;
        else if(FT1<=166.768)tmp1=195.6;
        else if(FT1<=166.886)tmp1=195.7;
        else if(FT1<=167.004)tmp1=195.8;
        else if(FT1<=167.122)tmp1=195.9;
        else if(FT1<=167.24)tmp1=196;
    }
    else if(FT1<=168.305){
        if(FT1==167.24)tmp1=196;
        else if(FT1<=167.359)tmp1=196.1;
        else if(FT1<=167.477)tmp1=196.2;
        else if(FT1<=167.595)tmp1=196.3;
        else if(FT1<=167.713)tmp1=196.4;
        else if(FT1<=167.832)tmp1=196.5;
        else if(FT1<=167.95)tmp1=196.6;
        else if(FT1<=168.068)tmp1=196.7;
        else if(FT1<=168.187)tmp1=196.8;
        else if(FT1<=168.305)tmp1=196.9;
        else if(FT1<=168.424)tmp1=197;
    }
    else if(FT1<=169.492){
        if(FT1==168.424)tmp1=197;
        else if(FT1<=168.542)tmp1=197.1;
        else if(FT1<=168.661)tmp1=197.2;
        else if(FT1<=168.78)tmp1=197.3;
        else if(FT1<=168.898)tmp1=197.4;
        else if(FT1<=169.017)tmp1=197.5;
        else if(FT1<=169.136)tmp1=197.6;
        else if(FT1<=169.254)tmp1=197.7;
        else if(FT1<=169.373)tmp1=197.8;
        else if(FT1<=169.492)tmp1=197.9;
        else if(FT1<=169.611)tmp1=198;
    }
    else if(FT1<=170.683){
        if(FT1==169.611)tmp1=198;
        else if(FT1<=169.73)tmp1=198.1;
        else if(FT1<=169.849)tmp1=198.2;
        else if(FT1<=169.968)tmp1=198.3;
        else if(FT1<=170.087)tmp1=198.4;
        else if(FT1<=170.206)tmp1=198.5;
        else if(FT1<=170.325)tmp1=198.6;
        else if(FT1<=170.444)tmp1=198.7;
        else if(FT1<=170.563)tmp1=198.8;
        else if(FT1<=170.683)tmp1=198.9;
        else if(FT1<=170.802)tmp1=199;
    }
    else if(FT1<=171.877){
        if(FT1==170.802)tmp1=199;
        else if(FT1<=170.921)tmp1=199.1;
        else if(FT1<=171.04)tmp1=199.2;
        else if(FT1<=171.16)tmp1=199.3;
        else if(FT1<=171.279)tmp1=199.4;
        else if(FT1<=171.399)tmp1=199.5;
        else if(FT1<=171.518)tmp1=199.6;
        else if(FT1<=171.638)tmp1=199.7;
        else if(FT1<=171.757)tmp1=199.8;
        else if(FT1<=171.877)tmp1=199.9;
        else if(FT1<=171.996)tmp1=200;
    }

    return tmp1;
}


