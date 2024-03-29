
#ifndef Calibration_tiri_TIRI_H
#define Calibration_tiri_TIRI_H

#include <QDialog>
#include <QtSql>
#include <QProgressDialog>
#include <QString>
#include <algorithm>
#include <QFileDialog>
#include "database.h"
#include <QMessageBox>

namespace Ui {
class Calibration_tiri;
}

class Calibration_tiri : public QDialog
{
    Q_OBJECT

public:
    explicit Calibration_tiri(QWidget *parent = 0);
    ~Calibration_tiri();
    Ui::Calibration_tiri *ui;
    QSqlDatabase db;
    QSqlQuery query;
    QSqlQuery pixelQuery;
    QString info[20];
    QVector< QVector<QString> > pixelList;
    QIntValidator *XValidator, *YValidator;
    int queryNum=0;
    int px=-1, py=-1;
    double T_max=-1000,T_min=1000,B_max=0,B_min=100,P_max=0,P_min=100,C_max=0,C_min=100,S_max=0,S_min=100,L_max=0,L_min=100,
    FPA1_max=0,FPA1_min=100,FPA2_max=0,FPA2_min=100,L1_max=0,L1_min=100,L2_max=0,L2_min=100,FW1_max=0,FW1_min=100,
    FW2_max=0,FW2_min=100,CAS_max=0,CAS_min=100,HOD_max=0,HOD_min=100,RD1_max=0,RD1_min=100,RD2_max=0,RD2_min=100,DC_max=0,DC_min=100;
    bool coordinateChangeFlag=false;

    // QVector<QVector<QString>> getCoffecientAndOffset(QString fileName);
    QString dataPath;

private:
    void connectDBtiri();
    void fillForm();
    void setValue();
    QString nameChange(QString tmp);
    bool judgeItem();
    QString judgeTableName(int x, int y);

signals:
    void showCalibrationGraphSignal_tiri(QVector< QVector<QString> >, QString, QString, QString, int, bool);
    void showControlGraphPanel(QString**);

public slots:
    void checkAction();
    void getDataPath(QString);
    void setX(QString x);
    void setY(QString y);

private slots:
    void on_searchPixcelButton_clicked();
    void on_pixelList_clicked(const QModelIndex &index);
    void on_showGraphButton_clicked();
    void showCalibrationPanel();
    void on_x_textChanged(const QString &arg1);
    void on_y_textChanged(const QString &arg1);
};

#endif
