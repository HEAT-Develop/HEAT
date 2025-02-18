//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//
#include "calibrationgraph.h"
#include "ui_calibrationgraph.h"
#include <iostream>
#include <fstream>
#include <QDateTime>
#include <showimage.h>
#include <calibration.h>
#include <controlgraphpanel.h>
#include <exception>
#include <thread>
#include <exception>
//#include <QtConcurrent>
#include <omp.h>
#include<FITS.h>
#include <CCfits/CCfits>
#include "dataset.h"
#include "project.h"

using namespace CCfits;

int xPosition[10000], yPosition[10000], isActive[10000]; 
int n = 0, sum = 0;
QString Usedimage[4000];
double tirfilter[2000][3];

double bol_temp;
double pkg_temp;
double cas_temp;
double sht_temp;
double len_temp;
bool ismodifiedgl;

QString fitsfilename;
QString ImageFilefilepath;

using namespace std;
int pixelxy_thread;
int Outputplotnumber_thread;
QVector<QVector<QString>> tmp2_thread;
QString xAxis_thread;
QString yAxis_thread;
QString initialFileDirectory_thread;
int xaxis_thread[1024];
int yaxis_thread[1024];
QString num2_thread;
//QString residual[248][328];
//QString arra[248][328], arrb[248][328], arrc[248][328], arrd[248][328], arre[248][328], arrf[248][328], arrg[248][328], arrh[248][328];


void ForThread1::run()
{

}

void ForThread2::run()
{
    
}

void ForThread3::run()
{

}

void ForThread4::run()
{


}

void ForThread5::run()
{

}

void ForThread6::run()
{

    

}

CalibrationGraph::CalibrationGraph(QWidget *parent) : QDialog(parent),
    ui(new Ui::CalibrationGraph)
{
    
    ui->setupUi(this);

    ui->widget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->widget->setInteraction(QCP::iSelectPlottables);

    connect(ui->widget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
    connect(ui->widget, SIGNAL(plottableClick(QCPAbstractPlottable *, QMouseEvent *)), this, SLOT(graphClicked(QCPAbstractPlottable *)));
    connect(ui->widget, SIGNAL(mousePress(QMouseEvent *)), this, SLOT(mousePress()));

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
    arra = new QString* [xmldata.l2.Height_data];
    arrb = new QString* [xmldata.l2.Height_data];
    arrc = new QString* [xmldata.l2.Height_data];
    arrd = new QString* [xmldata.l2.Height_data];
    arre = new QString* [xmldata.l2.Height_data];
    arrf = new QString* [xmldata.l2.Height_data];
    arrg = new QString* [xmldata.l2.Height_data];
    arrh = new QString* [xmldata.l2.Height_data];
    residual = new QString* [xmldata.l2.Height_data];
    for(int i=0;i<xmldata.l2.Height_data;i++){
        arra[i] = new QString [xmldata.l2.Width_data];
        arrb[i] = new QString [xmldata.l2.Width_data];
        arrc[i] = new QString [xmldata.l2.Width_data];
        arrd[i] = new QString [xmldata.l2.Width_data];
        arre[i] = new QString [xmldata.l2.Width_data];
        arrf[i] = new QString [xmldata.l2.Width_data];
        arrg[i] = new QString [xmldata.l2.Width_data];
        arrh[i] = new QString [xmldata.l2.Width_data];
        residual[i] = new QString [xmldata.l2.Width_data];
    }
    loadFilter();
}

CalibrationGraph::~CalibrationGraph()
{
    for(int i=0;i<xmldata.l2.Height_data;i++){
        delete[] arra[i];
        delete[] arrb[i];
        delete[] arrc[i];
        delete[] arrd[i];
        delete[] arre[i];
        delete[] arrf[i];
        delete[] arrg[i];
        delete[] arrh[i];
        delete[] residual[i];
    }
    delete arra;
    delete arrb;
    delete arrc;
    delete arrd;
    delete arre;
    delete arrf;
    delete arrg;
    delete arrh;
    delete residual;
    delete ui;
}

void CalibrationGraph::popCalibrationGraph(QVector<QVector<QString>> p, QString xAxisName, QString yAxisName, QString lineType, int itemNum, bool ismodified)
{
    ismodifiedgl = ismodified;
    cout<<"test boy"<<endl;
    if (sum < 10000)
    {
        cg[n] = new CalibrationGraph;
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

    cg[n]->pixelList = p;
    cg[n]->replot = p;

    cg[n]->xAxis = xAxisName;
    cg[n]->yAxis = yAxisName;

    setMax();
    setMin();

    xPosition[n] = cg[n]->pixelList[0][1].toInt();
    yPosition[n] = cg[n]->pixelList[0][2].toInt();

    cg[n]->setWindowTitle("Position : (" + QString::number(xPosition[n]) + "," + QString::number(yPosition[n]) + ")" + "   X : " + xAxisName + "   Y : " + yAxisName);

    int f = judgeAxis(xAxisName, yAxisName);

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

void CalibrationGraph::setInitializeUI()
{

    
    cg[n]->ui->tgtMaxLineEdit->clear();
    cg[n]->ui->tgtMinLineEdit->clear();
    cg[n]->ui->boloMaxLineEdit->clear();
    cg[n]->ui->boloMinLineEdit->clear();
    cg[n]->ui->pkgMaxLineEdit->clear();
    cg[n]->ui->pkgMinLineEdit->clear();
    cg[n]->ui->caseMaxLineEdit->clear();
    cg[n]->ui->caseMinLineEdit->clear();
    cg[n]->ui->shMaxLineEdit->clear();
    cg[n]->ui->shMinLineEdit->clear();
    cg[n]->ui->lensMaxLineEdit->clear();
    cg[n]->ui->lensMinLineEdit->clear();

    cg[n]->ui->regressionFormulaBrowser->clear();
    QDoubleValidator *v = new QDoubleValidator(this);

    cg[n]->ui->tgtMaxSlider->setRange(tgtMin, tgtMax);
    cg[n]->ui->tgtMinSlider->setRange(tgtMin, tgtMax);

    cg[n]->ui->boloMaxSlider->setRange(boloMin, boloMax);
    cg[n]->ui->boloMinSlider->setRange(boloMin, boloMax);

    cg[n]->ui->pkgMaxSlider->setRange(pkgMin, pkgMax);
    cg[n]->ui->pkgMinSlider->setRange(pkgMin, pkgMax);

    cg[n]->ui->caseMaxSlider->setRange(caseMin, caseMax);
    cg[n]->ui->caseMinSlider->setRange(caseMin, caseMax);

    cg[n]->ui->shMaxSlider->setRange(shMin, shMax);
    cg[n]->ui->shMinSlider->setRange(shMin, shMax);

    cg[n]->ui->lensMaxSlider->setRange(lensMin, lensMax);
    cg[n]->ui->lensMinSlider->setRange(lensMin, lensMax);


    cg[n]->ui->tgtMaxSlider->setTickInterval((tgtMax - tgtMin) / 10);
    cg[n]->ui->tgtMinSlider->setTickInterval((tgtMax - tgtMin) / 10);

    cg[n]->ui->boloMaxSlider->setTickInterval((boloMax - boloMin) / 10);
    cg[n]->ui->boloMinSlider->setTickInterval((boloMax - boloMin) / 10);

    cg[n]->ui->pkgMaxSlider->setTickInterval((pkgMax - pkgMin) / 10);
    cg[n]->ui->pkgMinSlider->setTickInterval((pkgMax - pkgMin) / 10);

    cg[n]->ui->caseMaxSlider->setTickInterval((caseMax - caseMin) / 10);
    cg[n]->ui->caseMinSlider->setTickInterval((caseMax - caseMin) / 10);

    cg[n]->ui->shMaxSlider->setTickInterval((shMax - shMin) / 10);
    cg[n]->ui->shMinSlider->setTickInterval((shMax - shMin) / 10);

    cg[n]->ui->lensMaxSlider->setTickInterval((lensMax - lensMin) / 10);
    cg[n]->ui->lensMinSlider->setTickInterval((lensMax - lensMin) / 10);

    cg[n]->ui->tgtMaxSlider->setValue(tgtMax);
    cg[n]->ui->tgtMinSlider->setValue(tgtMin);

    cg[n]->ui->boloMaxSlider->setValue(boloMax);
    cg[n]->ui->boloMinSlider->setValue(boloMin);

    cg[n]->ui->pkgMaxSlider->setValue(0);
    cg[n]->ui->pkgMinSlider->setValue(0);

    cg[n]->ui->caseMaxSlider->setValue(0);
    cg[n]->ui->caseMinSlider->setValue(0);

    cg[n]->ui->shMaxSlider->setValue(0);
    cg[n]->ui->shMinSlider->setValue(0);

    cg[n]->ui->lensMaxSlider->setValue(0);
    cg[n]->ui->lensMinSlider->setValue(0);

    
}

int CalibrationGraph::judgeAxis(QString x, QString y)
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

QVector<double> CalibrationGraph::getAxisValue(QString axis, QVector<QVector<QString>> info, int num)
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
            v[i] = info[i][9].toDouble();
        }
        return v;
    }
    return v;
}

QVector<double> CalibrationGraph::getAxisValue2(QString axis, QVector<QVector<QString>> info, int itemNum)
{

    infoNum = 0;
    pixelList2.clear();

    if (axis == "open DN")
    {
        QVector<QString> tmp1;
        infoNum = 0;
        for (int i = 0; i < itemNum; i++)
        {
            if (info[i][0].section('.', 2, 2) == "open")
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
            if (info[i][0].section('.', 2, 2) == "open")
            {
                
                searchID = info[i][10].toInt(0, 16);

                
                if (searchID % 2)
                {
                    pairID = searchID + 1;
                }
                else
                {

                    pairID = searchID - 1;
                }

                
                for (int j = 0; j < itemNum; j++)
                {
                    if (info[j][10].toInt(0, 16) == pairID && info[i][11] == info[j][11] && info[i][12] == info[j][12])
                    {
                        if(((info[i][3].toDouble() - info[j][3].toDouble()<150)&&(info[i][11].toStdString()=="BB")&&info[i][9].toInt()==50)||
                                ((info[i][3].toDouble() - info[j][3].toDouble()<150)&&(info[i][11].toStdString()=="Oil_bath_BB")&&info[i][9].toInt()==50)||
                                ((info[i][3].toDouble() - info[j][3].toDouble()<325)&&(info[i][11].toStdString()=="Oil_bath_BB")&&info[i][9].toInt()==75)||
                                ((info[i][3].toDouble() - info[j][3].toDouble()<550)&&(info[i][11].toStdString()=="Oil_bath_BB")&&info[i][9].toInt()==100)||
                                ((info[i][3].toDouble() - info[j][3].toDouble()<750)&&(info[i][11].toStdString()=="Oil_bath_BB")&&info[i][9].toInt()==125)){
                        }


                        else{
                            tmp1.clear();
                            for (int m = 0; m < 25; m++)
                            {

                                if (m == 3)
                                {
                                    tmp1.append(QString::number(info[i][3].toDouble() - info[j][3].toDouble()));

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
        }

        return getAxisValue(axis, pixelList2, infoNum);
    }
}

void CalibrationGraph::drawGraph(QVector<double> x, QVector<double> y, QString lineType)
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

void CalibrationGraph::on_regressionButton_clicked()
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

    if (num2 == "Black_Body")
        setRegressionCoefficientforBlack(vx, vy);
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
            y[i] = g * planck(min + 273.15) + h;
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
                if (pixelList[j][10].toInt(0, 16) == pairID && replot[i][11] == pixelList[j][11] && replot[i][12] == pixelList[j][12])
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
                if (replot[i][0].section('.', 2, 2) == "open")
                {
                    tmp1.append(replot[i][0]);
                }
            }
            else if (yAxis == "close DN" || xAxis == "close DN")
            {
                if (replot[i][0].section('.', 2, 2) == "close")
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

void CalibrationGraph::setRegressionCoefficient(QVector<double> vx, QVector<double> vy)
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

void CalibrationGraph::setRegressionCoefficientforBlack(QVector<double> vx, QVector<double> vy)
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

void CalibrationGraph::closeEvent(QCloseEvent *e)
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

void CalibrationGraph::setMax()
{

    tgtMax = boloMax = pkgMax = caseMax = shMax = lensMax = 0;

    for (int i = 0; i < cg[n]->previousNum; i++)
    {
        if (boloMax < cg[n]->pixelList[i][4].toDouble() * 1000)
        {
            boloMax = cg[n]->pixelList[i][4].toDouble() * 1000;
        }
        if (pkgMax < cg[n]->pixelList[i][5].toDouble() * 1000)
        {
            pkgMax = cg[n]->pixelList[i][5].toDouble() * 1000;
        }
        if (caseMax < cg[n]->pixelList[i][6].toDouble() * 1000)
        {
            caseMax = cg[n]->pixelList[i][6].toDouble() * 1000;
        }
        if (shMax < cg[n]->pixelList[i][7].toDouble() * 1000)
        {
            shMax = cg[n]->pixelList[i][7].toDouble() * 1000;
        }
        if (lensMax < cg[n]->pixelList[i][8].toDouble() * 1000)
        {
            lensMax = cg[n]->pixelList[i][8].toDouble() * 1000;
        }
        if (tgtMax < cg[n]->pixelList[i][9].toDouble() * 1000)
        {
            tgtMax = cg[n]->pixelList[i][9].toDouble() * 1000;
        }
    }
}

void CalibrationGraph::setMin()
{

    tgtMin = boloMin = pkgMin = caseMin = shMin = lensMin = 10000 * 1000;

    for (int i = 0; i < cg[n]->previousNum; i++)
    {
        if (boloMin > cg[n]->pixelList[i][4].toDouble() * 1000)
        {
            boloMin = cg[n]->pixelList[i][4].toDouble() * 1000;
        }
        if (pkgMin > cg[n]->pixelList[i][5].toDouble() * 1000)
        {
            pkgMin = cg[n]->pixelList[i][5].toDouble() * 1000;
        }
        if (caseMin > cg[n]->pixelList[i][6].toDouble() * 1000)
        {
            caseMin = cg[n]->pixelList[i][6].toDouble() * 1000;
        }
        if (shMin > cg[n]->pixelList[i][7].toDouble() * 1000)
        {
            shMin = cg[n]->pixelList[i][7].toDouble() * 1000;
        }
        if (lensMin > cg[n]->pixelList[i][8].toDouble() * 1000)
        {
            lensMin = cg[n]->pixelList[i][8].toDouble() * 1000;
        }
        if (tgtMin > cg[n]->pixelList[i][9].toDouble() * 1000)
        {
            tgtMin = cg[n]->pixelList[i][9].toDouble() * 1000;
        }
    }
}

void CalibrationGraph::on_replotButton_clicked()
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

    drawGraph(vx, vy, "");
}

void CalibrationGraph::on_outputCSVFileButton_clicked()
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

void CalibrationGraph::OutputSliderValue(QString fileName)
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

void CalibrationGraph::OutputUsedImage(QString fileName)
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

void CalibrationGraph::on_tgtMaxSlider_valueChanged(int value)
{
    ui->tgtMaxLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[0] = (double)value / 1000;
}

void CalibrationGraph::on_tgtMinSlider_valueChanged(int value)
{
    ui->tgtMinLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[1] = (double)value / 1000;
}

void CalibrationGraph::on_boloMaxSlider_valueChanged(int value)
{
    ui->boloMaxLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[2] = (double)value / 1000;
}

void CalibrationGraph::on_boloMinSlider_valueChanged(int value)
{
    ui->boloMinLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[3] = (double)value / 1000;
}

void CalibrationGraph::on_pkgMaxSlider_valueChanged(int value)
{
    ui->pkgMaxLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[4] = (double)value / 1000;
}

void CalibrationGraph::on_pkgMinSlider_valueChanged(int value)
{
    ui->pkgMinLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[5] = (double)value / 1000;
}

void CalibrationGraph::on_caseMaxSlider_valueChanged(int value)
{
    ui->caseMaxLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[6] = (double)value / 1000;
}

void CalibrationGraph::on_caseMinSlider_valueChanged(int value)
{
    ui->caseMinLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[7] = (double)value / 1000;
}

void CalibrationGraph::on_shMaxSlider_valueChanged(int value)
{
    ui->shMaxLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[8] = (double)value / 1000;
}

void CalibrationGraph::on_shMinSlider_valueChanged(int value)
{
    ui->shMinLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[9] = (double)value / 1000;
}

void CalibrationGraph::on_lensMaxSlider_valueChanged(int value)
{
    ui->lensMaxLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[10] = (double)value / 1000;
}

void CalibrationGraph::on_lensMinSlider_valueChanged(int value)
{
    ui->lensMinLineEdit->setText(QString::number((double)value / 1000));
    OutputSliderValuearray[11] = (double)value / 1000;
}

void CalibrationGraph::on_tgtMaxLineEdit_textChanged(const QString &arg1)
{
    ui->tgtMaxSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph::on_tgtMinLineEdit_textChanged(const QString &arg1)
{
    ui->tgtMinSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph::on_boloMaxLineEdit_textChanged(const QString &arg1)
{
    ui->boloMaxSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph::on_boloMinLineEdit_textChanged(const QString &arg1)
{
    ui->boloMinSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph::on_pkgMaxLineEdit_textChanged(const QString &arg1)
{
    ui->pkgMaxSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph::on_pkgMinLineEdit_textChanged(const QString &arg1)
{
    ui->pkgMinSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph::on_caseMaxLineEdit_textChanged(const QString &arg1)
{
    ui->caseMaxSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph::on_caseMinLineEdit_textChanged(const QString &arg1)
{
    ui->caseMinSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph::on_shMaxLineEdit_textChanged(const QString &arg1)
{
    ui->shMaxSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph::on_shMinLineEdit_textChanged(const QString &arg1)
{
    ui->shMinSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph::on_lensMaxLineEdit_textChanged(const QString &arg1)
{
    ui->lensMaxSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph::on_lensMinLineEdit_textChanged(const QString &arg1)
{
    ui->lensMinSlider->setValue(arg1.toDouble() * 1000);
}

void CalibrationGraph::on_outputGraphImageButton_clicked()
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

void CalibrationGraph::contextMenuRequest(QPoint pos)
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

void CalibrationGraph::graphClicked(QCPAbstractPlottable *plottable)
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

void CalibrationGraph::removeSelectedGraph()
{ 
    if (ui->widget->selectedGraphs().size() > 0)
    {
        ui->widget->removeGraph(ui->widget->selectedGraphs().first());
        ui->widget->replot();
        regressionGraphCount--;
    }
}

void CalibrationGraph::removeRegressionAllGraphs()
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

void CalibrationGraph::mousePress()
{ 
    ui->regressionFormulaBrowser->clear();

}

/* void CalibrationGraph::on_plotFormulaButton_clicked()
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
void CalibrationGraph::on_exportFormulaButton_clicked()  
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
            if(tablecounter2==width+1)break;

            tablecounter++;
            tablecounter2++;
        }

        tablecounter=tablecounter3+12;
        tablecounter3=tablecounter3+12;
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
    p.setRange(0, 248 * 328);
    p.show();
    QCoreApplication::processEvents();

    for (int i = 0; i < 248; i++)
    {
        for (int j = 0; j < 328; j++)
        {
            residual[i][j] = "non";
            arra[i][j] = "non";
            arrb[i][j] = "non";
            arrc[i][j] = "non";
            arrd[i][j] = "non";
            arre[i][j] = "non";
            arrf[i][j] = "non";
            arrg[i][j] = "non";
            arrh[i][j] = "non";
        }
    }

    // makefitssample("kontyan_lut.fit");

    QVector<QString> tmpfortxt;
    int searchIDfortxt, pairIDfortxt;
    for (int i = 0; i < replot.size(); i++)
    {
        searchIDfortxt = replot[i][10].toInt(0, 16);

        if (searchIDfortxt % 2)
        {
            pairIDfortxt = searchIDfortxt + 1;
        }
        else
        {
            pairIDfortxt = searchIDfortxt - 1;
        }

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
            if (pixelList[j][10].toInt(0, 16) == pairIDfortxt && replot[i][11] == pixelList[j][11] && replot[i][12] == pixelList[j][12])
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

    connectDB();
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


    // makefitssample("kontyan_sample.fit");
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

    for(i=0;i<table.length();i++)
    {
        tmp2.clear();
        tmp2_thread.clear();
        if (!(table[i] == "pix12" || table[i] == "pix24" || table[i] == "pix36" || table[i] == "pix48" || table[i] == "pix60" || table[i] == "pix72" || table[i] == "pix84" || table[i] == "pix96"))
        {
            querystring = "SELECT " + table[i] + ".img_file, x, y," + searchObject + "img_id, target_name, path, m, mask FROM " + table[i] + ", tirimageinfo " +
                    "WHERE tirimageinfo.img_file=" + table[i] + ".img_file AND (" + searchName + ") AND x>=16 AND x<=343 AND y>=6 AND y<=253";


            query.clear();
            query.exec(querystring);
            int pixelxy;

            if (table[i] == "pix01")
            {
                pixelxy = 416;
            }
            else if (table[i] == "pix11")
            {
                pixelxy = 624;
            }
            else if (table[i] == "pix85")
            {
                pixelxy = 480;
            }
            else if (table[i] == "pix95")
            {
                pixelxy = 720;
            }

            else if (table[i] == "pix02" || table[i] == "pix03" || table[i] == "pix04" || table[i] == "pix05" || table[i] == "pix06" || table[i] == "pix07" || table[i] == "pix08" || table[i] == "pix09" || table[i] == "pix10")
            {
                pixelxy = 832;
            }

            else if (table[i] == "pix86" || table[i] == "pix87" || table[i] == "pix88" || table[i] == "pix89" || table[i] == "pix90" || table[i] == "pix91" || table[i] == "pix92" || table[i] == "pix93" || table[i] == "pix94")
            {
                pixelxy = 960;
            }
            else if (table[i] == "pix13" || table[i] == "pix25" || table[i] == "pix37" || table[i] == "pix49" || table[i] == "pix61" || table[i] == "pix73")
            {
                pixelxy = 512;
            }
            else if (table[i] == "pix23" || table[i] == "pix35" || table[i] == "pix47" || table[i] == "pix59" || table[i] == "pix71" || table[i] == "pix83")
            {
                pixelxy = 768;
            }
            else
            {
                pixelxy = 1024;
            }

            pixelxy_thread = pixelxy;
            xAxis_thread = xAxis;
            yAxis_thread = yAxis;
            num2_thread = ui->degreeComboBox->currentText();
            int counter = 0;
            int xaxis[pixelxy];
            int yaxis[pixelxy];

            query.first();
            do
            {
                t.clear();
                t.append(query.value(0).toString());

                if(query.value(8).toInt() > 1 && (xAxis == "diff DN" || xAxis == "open DN" || xAxis == "close DN"))
                {
                    t.append(QString::number(query.value(3).toDouble() / 8));
                }
                else
                {
                    t.append(query.value(3).toString());
                }

                if (query.value(8).toInt() > 1 && (yAxis == "diff DN" || yAxis == "open DN" || yAxis == "close DN"))
                {
                    t.append(QString::number(query.value(4).toDouble() / 8));
                }
                else
                {
                    t.append(query.value(4).toString());
                }
                t.append(query.value(5).toString());
                t.append(query.value(6).toString());
                t.append(query.value(7).toString());
                t.append(query.value(9).toString());

                tmp2.append(t);
                tmp2_thread.append(t);
                if (counter < pixelxy)
                {
                    xaxis[counter] = query.value(1).toInt();
                    yaxis[counter] = query.value(2).toInt();
                    xaxis_thread[counter] = query.value(1).toInt();
                    yaxis_thread[counter] = query.value(2).toInt();
                }
                counter++;

            } while (query.next());
            t.clear();
            Outputplotnumber_thread = Outputplotnumber_thread = tmpfortxt.length()/2;
            if(tmp2_thread.size()>1){
#pragma omp parallel for
                for (int pix = 0; pix <pixelxy_thread; pix++)
                {
                    int searchID, pairID;
                    QVector<QString> t;
                    QVector<QVector<QString>> tmp3;
                    QDir tmp5;
                    int xycounter = pix;
                    for (int plotn = 0; plotn < Outputplotnumber_thread * 2; plotn++)
                    {
                        int currentpixel = pixelxy_thread * plotn + pix;
                        if (tmp2_thread[currentpixel][0].section('.', 2, 2) == "open" && tmp2_thread[currentpixel][6]=="1")
                        {
                            searchID = tmp2_thread[currentpixel][3].toInt(0, 16);
                            if (searchID % 2)
                            {
                                pairID = searchID + 1;
                            }
                            else
                            {
                                pairID = searchID - 1;
                            }
                            for (int plotn2 = 0; plotn2 < Outputplotnumber_thread * 2; plotn2++)
                            {
                                int currentpixel2 = pixelxy_thread * plotn2 + pix;
                                if (tmp2_thread[currentpixel2][3].toInt(0, 16) == pairID && tmp2_thread[currentpixel][4] == tmp2_thread[currentpixel2][4] && tmp2_thread[currentpixel][5] == tmp2_thread[currentpixel2][5])
                                {
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
                                        t.append(QString::number(tmp2_thread[currentpixel][2].toDouble() - tmp2_thread[currentpixel2][2].toDouble()));
                                    }
                                    tmp3.append(t);
                                    break;
                                }
                            }
                        }
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
                    double x = 0, x2 = 0, x3 = 0, x4 = 0, x5 = 0, x6 = 0, x7 = 0, x8 = 0, x9 = 0, x10 = 0, x11 = 0, x12 = 0, x13 = 0, x14 = 0;
                    double x7y = 0, x6y = 0, x5y = 0, x4y = 0, x3y = 0, x2y = 0, xy = 0, y = 0;
                    double pivot, mul;
                    double aa, bb, cc, dd, ee, ff, gg, hh;
                    QString coefficient, formula;
                    num = 2;
                    aa = bb = cc = dd = ee = ff = gg = hh = 0;

                    for (int i = 0; i < vx.size(); i++)
                    {
                        x += planck4(vx[i] + 273.15);
                        x2 += pow(planck4(vx[i] + 273.15), 2);
                        x3 += pow(planck4(vx[i] + 273.15), 3);
                        x4 += pow(planck4(vx[i] + 273.15), 4);
                        x5 += pow(planck4(vx[i] + 273.15), 5);
                        x6 += pow(planck4(vx[i] + 273.15), 6);
                        x7 += pow(planck4(vx[i] + 273.15), 7);
                        x8 += pow(planck4(vx[i] + 273.15), 8);
                        x9 += pow(planck4(vx[i] + 273.15), 9);
                        x10 += pow(planck4(vx[i] + 273.15), 10);
                        x11 += pow(planck4(vx[i] + 273.15), 11);
                        x12 += pow(planck4(vx[i] + 273.15), 12);
                        x13 += pow(planck4(vx[i] + 273.15), 13);
                        x14 += pow(planck4(vx[i] + 273.15), 14);
                        x7y += (pow(planck4(vx[i] + 273.15), 7) * vy[i]);
                        x6y += (pow(planck4(vx[i] + 273.15), 6) * vy[i]);
                        x5y += (pow(planck4(vx[i] + 273.15), 5) * vy[i]);
                        x4y += (pow(planck4(vx[i] + 273.15), 4) * vy[i]);
                        x3y += (pow(planck4(vx[i] + 273.15), 3) * vy[i]);
                        x2y += (pow(planck4(vx[i] + 273.15), 2) * vy[i]);
                        xy += (planck4(vx[i] + 273.15) * vy[i]);
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
                    residual[yaxis_thread[xycounter] - 6][xaxis_thread[xycounter] - 16] = QString::number(d);
                    
                    arrg[yaxis_thread[xycounter] - 6][xaxis_thread[xycounter] - 16] = QString::number(gg);
                    arrh[yaxis_thread[xycounter] - 6][xaxis_thread[xycounter] - 16] = QString::number(hh);

                    saveFileInRegisterRegression("residual.csv",initialFileDirectory_thread);

                    
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
    

    makefitssample("kontyan_lut.fit");

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



QString CalibrationGraph::getRegressionCoefficientInRegisterRegression(
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
    residual[yc][xc] = QString::number(d);
    arra[yc][xc] = QString::number(aa);
    arrb[yc][xc] = QString::number(bb);
    arrc[yc][xc] = QString::number(cc);
    arrd[yc][xc] = QString::number(dd);
    arre[yc][xc] = QString::number(ee);
    arrf[yc][xc] = QString::number(ff);
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

QString CalibrationGraph::getRegressionCoefficientInRegisterRegressionforBlackbody(QVector<double> vx, QVector<double> vy, int xc, int yc)
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
    residual[yc][xc] = QString::number(d);
    arra[yc][xc] = QString::number(aa);
    arrb[yc][xc] = QString::number(bb);
    arrc[yc][xc] = QString::number(cc);
    arrd[yc][xc] = QString::number(dd);
    arre[yc][xc] = QString::number(ee);
    arrf[yc][xc] = QString::number(ff);
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

void CalibrationGraph::connectDB()
{
    db.open();
    query = QSqlQuery(db);
    if (query.isActive())
    {
        query.first();
    }
}

QString CalibrationGraph::judgeTableName(int x, int y)
{
    if (0 <= y && y <= 31)
    {
        if (0 <= x && x <= 31)
            return "pix01";
        else if (32 <= x && x <= 63)
            return "pix02";
        else if (64 <= x && x <= 95)
            return "pix03";
        else if (96 <= x && x <= 127)
            return "pix04";
        else if (128 <= x && x <= 159)
            return "pix05";
        else if (160 <= x && x <= 191)
            return "pix06";
        else if (192 <= x && x <= 223)
            return "pix07";
        else if (224 <= x && x <= 255)
            return "pix08";
        else if (256 <= x && x <= 287)
            return "pix09";
        else if (288 <= x && x <= 319)
            return "pix10";
        else if (320 <= x && x <= 351)
            return "pix11";
        else if (352 <= x && x <= 383)
            return "pix12";
    }
    else if (32 <= y && y <= 63)
    {
        if (0 <= x && x <= 31)
            return "pix13";
        else if (32 <= x && x <= 63)
            return "pix14";
        else if (64 <= x && x <= 95)
            return "pix15";
        else if (96 <= x && x <= 127)
            return "pix16";
        else if (128 <= x && x <= 159)
            return "pix17";
        else if (160 <= x && x <= 191)
            return "pix18";
        else if (192 <= x && x <= 223)
            return "pix19";
        else if (224 <= x && x <= 255)
            return "pix20";
        else if (256 <= x && x <= 287)
            return "pix21";
        else if (288 <= x && x <= 319)
            return "pix22";
        else if (320 <= x && x <= 351)
            return "pix23";
        else if (352 <= x && x <= 383)
            return "pix24";
    }
    else if (64 <= y && y <= 95)
    {
        if (0 <= x && x <= 31)
            return "pix25";
        else if (32 <= x && x <= 63)
            return "pix26";
        else if (64 <= x && x <= 95)
            return "pix27";
        else if (96 <= x && x <= 127)
            return "pix28";
        else if (128 <= x && x <= 159)
            return "pix29";
        else if (160 <= x && x <= 191)
            return "pix30";
        else if (192 <= x && x <= 223)
            return "pix31";
        else if (224 <= x && x <= 255)
            return "pix32";
        else if (256 <= x && x <= 287)
            return "pix33";
        else if (288 <= x && x <= 319)
            return "pix34";
        else if (320 <= x && x <= 351)
            return "pix35";
        else if (352 <= x && x <= 383)
            return "pix36";
    }
    else if (96 <= y && y <= 127)
    {
        if (0 <= x && x <= 31)
            return "pix37";
        else if (32 <= x && x <= 63)
            return "pix38";
        else if (64 <= x && x <= 95)
            return "pix39";
        else if (96 <= x && x <= 127)
            return "pix40";
        else if (128 <= x && x <= 159)
            return "pix41";
        else if (160 <= x && x <= 191)
            return "pix42";
        else if (192 <= x && x <= 223)
            return "pix43";
        else if (224 <= x && x <= 255)
            return "pix44";
        else if (256 <= x && x <= 287)
            return "pix45";
        else if (288 <= x && x <= 319)
            return "pix46";
        else if (320 <= x && x <= 351)
            return "pix47";
        else if (352 <= x && x <= 383)
            return "pix48";
    }
    else if (128 <= y && y <= 159)
    {
        if (0 <= x && x <= 31)
            return "pix49";
        else if (32 <= x && x <= 63)
            return "pix50";
        else if (64 <= x && x <= 95)
            return "pix51";
        else if (96 <= x && x <= 127)
            return "pix52";
        else if (128 <= x && x <= 159)
            return "pix53";
        else if (160 <= x && x <= 191)
            return "pix54";
        else if (192 <= x && x <= 223)
            return "pix55";
        else if (224 <= x && x <= 255)
            return "pix56";
        else if (256 <= x && x <= 287)
            return "pix57";
        else if (288 <= x && x <= 319)
            return "pix58";
        else if (320 <= x && x <= 351)
            return "pix59";
        else if (352 <= x && x <= 383)
            return "pix60";
    }
    else if (160 <= y && y <= 191)
    {
        if (0 <= x && x <= 31)
            return "pix61";
        else if (32 <= x && x <= 63)
            return "pix62";
        else if (64 <= x && x <= 95)
            return "pix63";
        else if (96 <= x && x <= 127)
            return "pix64";
        else if (128 <= x && x <= 159)
            return "pix65";
        else if (160 <= x && x <= 191)
            return "pix66";
        else if (192 <= x && x <= 223)
            return "pix67";
        else if (224 <= x && x <= 255)
            return "pix68";
        else if (256 <= x && x <= 287)
            return "pix69";
        else if (288 <= x && x <= 319)
            return "pix70";
        else if (320 <= x && x <= 351)
            return "pix71";
        else if (352 <= x && x <= 383)
            return "pix72";
    }
    else if (192 <= y && y <= 223)
    {
        if (0 <= x && x <= 31)
            return "pix73";
        else if (32 <= x && x <= 63)
            return "pix74";
        else if (64 <= x && x <= 95)
            return "pix75";
        else if (96 <= x && x <= 127)
            return "pix76";
        else if (128 <= x && x <= 159)
            return "pix77";
        else if (160 <= x && x <= 191)
            return "pix78";
        else if (192 <= x && x <= 223)
            return "pix79";
        else if (224 <= x && x <= 255)
            return "pix80";
        else if (256 <= x && x <= 287)
            return "pix81";
        else if (288 <= x && x <= 319)
            return "pix82";
        else if (320 <= x && x <= 351)
            return "pix83";
        else if (352 <= x && x <= 383)
            return "pix84";
    }
    else if (224 <= y && y <= 255)
    {
        if (0 <= x && x <= 31)
            return "pix85";
        else if (32 <= x && x <= 63)
            return "pix86";
        else if (64 <= x && x <= 95)
            return "pix87";
        else if (96 <= x && x <= 127)
            return "pix88";
        else if (128 <= x && x <= 159)
            return "pix89";
        else if (160 <= x && x <= 191)
            return "pix90";
        else if (192 <= x && x <= 223)
            return "pix91";
        else if (224 <= x && x <= 255)
            return "pix92";
        else if (256 <= x && x <= 287)
            return "pix93";
        else if (288 <= x && x <= 319)
            return "pix94";
        else if (320 <= x && x <= 351)
            return "pix95";
        else if (352 <= x && x <= 383)
            return "pix96";
    }

    return "";
}

int CalibrationGraph::judgeTableNameint(int x, int y)
{
    if (0 <= y && y <= 31)
    {
        if (0 <= x && x <= 31)
            return 1;
        else if (32 <= x && x <= 63)
            return 2;
        else if (64 <= x && x <= 95)
            return 3;
        else if (96 <= x && x <= 127)
            return 4;
        else if (128 <= x && x <= 159)
            return 5;
        else if (160 <= x && x <= 191)
            return 6;
        else if (192 <= x && x <= 223)
            return 7;
        else if (224 <= x && x <= 255)
            return 8;
        else if (256 <= x && x <= 287)
            return 9;
        else if (288 <= x && x <= 319)
            return 10;
        else if (320 <= x && x <= 351)
            return 11;
        else if (352 <= x && x <= 383)
            return 12;
    }
    else if (32 <= y && y <= 63)
    {
        if (0 <= x && x <= 31)
            return 13;
        else if (32 <= x && x <= 63)
            return 14;
        else if (64 <= x && x <= 95)
            return 15;
        else if (96 <= x && x <= 127)
            return 16;
        else if (128 <= x && x <= 159)
            return 17;
        else if (160 <= x && x <= 191)
            return 18;
        else if (192 <= x && x <= 223)
            return 19;
        else if (224 <= x && x <= 255)
            return 20;
        else if (256 <= x && x <= 287)
            return 21;
        else if (288 <= x && x <= 319)
            return 22;
        else if (320 <= x && x <= 351)
            return 23;
        else if (352 <= x && x <= 383)
            return 24;
    }
    else if (64 <= y && y <= 95)
    {
        if (0 <= x && x <= 31)
            return 25;
        else if (32 <= x && x <= 63)
            return 26;
        else if (64 <= x && x <= 95)
            return 27;
        else if (96 <= x && x <= 127)
            return 28;
        else if (128 <= x && x <= 159)
            return 29;
        else if (160 <= x && x <= 191)
            return 30;
        else if (192 <= x && x <= 223)
            return 31;
        else if (224 <= x && x <= 255)
            return 32;
        else if (256 <= x && x <= 287)
            return 33;
        else if (288 <= x && x <= 319)
            return 34;
        else if (320 <= x && x <= 351)
            return 35;
        else if (352 <= x && x <= 383)
            return 36;
    }
    else if (96 <= y && y <= 127)
    {
        if (0 <= x && x <= 31)
            return 37;
        else if (32 <= x && x <= 63)
            return 38;
        else if (64 <= x && x <= 95)
            return 39;
        else if (96 <= x && x <= 127)
            return 40;
        else if (128 <= x && x <= 159)
            return 41;
        else if (160 <= x && x <= 191)
            return 42;
        else if (192 <= x && x <= 223)
            return 43;
        else if (224 <= x && x <= 255)
            return 44;
        else if (256 <= x && x <= 287)
            return 45;
        else if (288 <= x && x <= 319)
            return 46;
        else if (320 <= x && x <= 351)
            return 47;
        else if (352 <= x && x <= 383)
            return 48;
    }
    else if (128 <= y && y <= 159)
    {
        if (0 <= x && x <= 31)
            return 49;
        else if (32 <= x && x <= 63)
            return 50;
        else if (64 <= x && x <= 95)
            return 51;
        else if (96 <= x && x <= 127)
            return 52;
        else if (128 <= x && x <= 159)
            return 53;
        else if (160 <= x && x <= 191)
            return 54;
        else if (192 <= x && x <= 223)
            return 55;
        else if (224 <= x && x <= 255)
            return 56;
        else if (256 <= x && x <= 287)
            return 57;
        else if (288 <= x && x <= 319)
            return 58;
        else if (320 <= x && x <= 351)
            return 59;
        else if (352 <= x && x <= 383)
            return 60;
    }
    else if (160 <= y && y <= 191)
    {
        if (0 <= x && x <= 31)
            return 61;
        else if (32 <= x && x <= 63)
            return 62;
        else if (64 <= x && x <= 95)
            return 63;
        else if (96 <= x && x <= 127)
            return 64;
        else if (128 <= x && x <= 159)
            return 65;
        else if (160 <= x && x <= 191)
            return 66;
        else if (192 <= x && x <= 223)
            return 67;
        else if (224 <= x && x <= 255)
            return 68;
        else if (256 <= x && x <= 287)
            return 69;
        else if (288 <= x && x <= 319)
            return 70;
        else if (320 <= x && x <= 351)
            return 71;
        else if (352 <= x && x <= 383)
            return 72;
    }
    else if (192 <= y && y <= 223)
    {
        if (0 <= x && x <= 31)
            return 73;
        else if (32 <= x && x <= 63)
            return 74;
        else if (64 <= x && x <= 95)
            return 75;
        else if (96 <= x && x <= 127)
            return 76;
        else if (128 <= x && x <= 159)
            return 77;
        else if (160 <= x && x <= 191)
            return 78;
        else if (192 <= x && x <= 223)
            return 79;
        else if (224 <= x && x <= 255)
            return 80;
        else if (256 <= x && x <= 287)
            return 81;
        else if (288 <= x && x <= 319)
            return 82;
        else if (320 <= x && x <= 351)
            return 83;
        else if (352 <= x && x <= 383)
            return 84;
    }
    else if (224 <= y && y <= 255)
    {
        if (0 <= x && x <= 31)
            return 85;
        else if (32 <= x && x <= 63)
            return 86;
        else if (64 <= x && x <= 95)
            return 87;
        else if (96 <= x && x <= 127)
            return 88;
        else if (128 <= x && x <= 159)
            return 89;
        else if (160 <= x && x <= 191)
            return 90;
        else if (192 <= x && x <= 223)
            return 91;
        else if (224 <= x && x <= 255)
            return 92;
        else if (256 <= x && x <= 287)
            return 93;
        else if (288 <= x && x <= 319)
            return 94;
        else if (320 <= x && x <= 351)
            return 95;
        else if (352 <= x && x <= 383)
            return 96;
    }
}


void CalibrationGraph::getDataPath(QString path)
{
    databPath = path;
}



void CalibrationGraph::makefitssample(std::string filename){
    
    QString date, time;
    date = QDateTime::currentDateTime().date().toString(Qt::ISODate);
    time = QDateTime::currentDateTime().time().toString(Qt::ISODate);


    filename=initialFileDirectory.toStdString()+"/"+filename;
    long naxis=2;
    int xsize=328;
    int ysize=248;
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
    qDebug()<<"afdfdafda";
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


void CalibrationGraph::saveFileInRegisterRegression(QString fileName, QString folder)
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
                if (fileName == "residual.csv")
                {
                    out << residual[k][m] << ",";
                }
                else if (fileName == "a.csv")
                {
                    out << arra[k][m] << ",";
                }
                else if (fileName == "b.csv")
                {
                    out << arrb[k][m] << ",";
                }
                else if (fileName == "c.csv")
                {
                    out << arrc[k][m] << ",";
                }
                else if (fileName == "d.csv")
                {
                    out << arrd[k][m] << ",";
                }
                else if (fileName == "e.csv")
                {
                    out << arre[k][m] << ",";
                }
                else if (fileName == "f.csv")
                {
                    out << arrf[k][m] << ",";
                }
                else if (fileName == "g.csv")
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
                if (fileName == "residual.csv")
                {
                    out << residual[k][m] << ",";
                }
                else if (fileName == "a.csv")
                {
                    out << arra[k][m] << ",";
                }
                else if (fileName == "b.csv")
                {
                    out << arrb[k][m] << ",";
                }
                else if (fileName == "c.csv")
                {
                    out << arrc[k][m] << ",";
                }
                else if (fileName == "d.csv")
                {
                    out << arrd[k][m] << ",";
                }
                else if (fileName == "e.csv")
                {
                    out << arre[k][m] << ",";
                }
                else if (fileName == "f.csv")
                {
                    out << arrf[k][m] << ",";
                }
                else if (fileName == "g.csv")
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

void ForThread1::saveFileInRegisterRegression(QString fileName, QString folder)
{}
void ForThread2::saveFileInRegisterRegression(QString fileName, QString folder)
{}
void ForThread3::saveFileInRegisterRegression(QString fileName, QString folder)
{}
void ForThread4::saveFileInRegisterRegression(QString fileName, QString folder)
{}
void ForThread5::saveFileInRegisterRegression(QString fileName, QString folder)
{}
void ForThread6::saveFileInRegisterRegression(QString fileName, QString folder)
{}



void CalibrationGraph::on_loadFileButton_clicked()
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

            pInfile->pHDU().readKey<double>("BOL_TEMP", bol_temp);
            pInfile->pHDU().readKey<double>("PKG_TEMP", pkg_temp);
            pInfile->pHDU().readKey<double>("CAS_TEMP", cas_temp);
            pInfile->pHDU().readKey<double>("SHT_TEMP", sht_temp);
            pInfile->pHDU().readKey<double>("LEN_TEMP", len_temp);
            fitsfilename = "File Name: " + QFileInfo(fileName).fileName();

            ui->pkgMaxSlider->setValue(pkg_temp * 1000 + 2000);
            ui->pkgMinSlider->setValue(pkg_temp * 1000 - 1000);

            ui->caseMaxSlider->setValue(cas_temp * 1000 + 2000);
            ui->caseMinSlider->setValue(cas_temp * 1000 - 1000);

            ui->shMaxSlider->setValue(sht_temp * 1000 + 2000);
            ui->shMinSlider->setValue(sht_temp * 1000 - 1000);

            ui->lensMaxSlider->setValue(len_temp * 1000 + 2000);
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
        catch (...)
        {
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

    drawGraph(vx, vy, "");
}

double CalibrationGraph::planck(double T)
{ 
    double lambda, Bt, integral = 0, epsilon = 0.925;

    for (int i = 1; i < 2001; i++)
    {
        lambda = (double)i * 1e-8;
        Bt = ((2 * h_planck * c_speed * c_speed) / (pow(lambda, 5)) / (pow(M_E, c2 / (lambda * T)) - 1.0) * tirfilter[i][1] * epsilon);
        integral += (Bt);
    }
    integral *= 1e-8;
    return integral;
}

double planck4(double T)
{ 
    double lambda, Bt, integral = 0, epsilon = 0.925;
    double h_planck = 6.62606957e-34;
    double kB = 1.3806488e-23;
    double c_speed = 299792458;
    double c1 = 2 * M_PI * h_planck * pow(c_speed, 2);
    double c2 = (h_planck * c_speed) / kB;
    double SIGMA = 5.670373e-8;


    for (int i = 1; i < 2001; i++)
    {
        lambda = (double)i * 1e-8;
        Bt = ((2 * h_planck * c_speed * c_speed) / (pow(lambda, 5)) / (pow(M_E, c2 / (lambda * T)) - 1.0) * tirfilter[i][1] * epsilon);
        integral += (Bt);
    }
    integral *= 1e-8;
    return integral;
}

void CalibrationGraph::loadFilter()
{

    QString str;
    QString appPath;
    appPath = QCoreApplication::applicationDirPath();

    //qDebug()<<appPath;
    QFile file(appPath + "/tir_response.txt");

    if (!file.open(QIODevice::ReadOnly))
    {
        printf("tir_response.txt open error\n");
        return;
    }

    QTextStream in(&file);

    for (int i = 0; !in.atEnd(); i++)
    {
        for (int j = 0; j < 3; j++)
        {
            in >> str;
            tirfilter[i][j] = str.toDouble();
        }
    }
}
