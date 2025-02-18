#ifndef READTXT_H
#define READTXT_H

#include <QDialog>
#include <QtSql>
#include <QObject>
#include <QMutex>
#include <QElapsedTimer>
#include <QDebug>

namespace Ui {
class readtxt;
}

#define FILE_TYEP_TIR "0"
#define FILE_TYEP_ROUGHNESSMODEL "1"
#define TIR_ITEM_COUNT 11
#define ROUGHNESSMODEL_ITEM_COUNT 3
#define INI_PASS_FILE "//Users//demo//project//heat2304//INI//Heat.ini"   // 2306dev:code
// #define INI_PASS_FILE "..//..//INI//Heat.ini"                          // Code. Qt5 onos:code
//#define MYSQL_PASS_FILE "C:\\Project\\mysql_pass.txt"
#define SIZE_LINE 1024
#define SIZE_DATA 196608
#define SIZE_SIGMA_GAMMA 6
#define LATITUDE_ERR "999"
#define OUT_FILE_NON_DATA "9999.999999"
#define NR_END 1
#define FREE_ARG char*
#define BTN_CALC 1
#define BTN_REG 2
#define BTN_SHOW 3
#define BTN_ONE_CALC 4
#define BTN_OUT_PUT 5


typedef struct  {
    QString MySqlPass;
    int MySqlPort;
    QString LogPass;
    int LoopStart;
    int LoopEnd;
}INI_DATA;

typedef struct  {
    double time;
    double temp;
    double diff;
    QString tir_id;
    int polygon_id;
    QString sigma;
    QString gamma;
}INPOL_DATA;

typedef struct {
    int id;
    QString sid;
    QString name;
    int flg;
}KEY_DATA;

typedef struct {
    int id;
    double zansa;
    QString sigma;
    QString gamma;

}BEST_FIT_DATA;

typedef struct {
    QString id;
    QString longitude;
    QString latitude;
    QString sigma;
    QString gamma;

}OUT_FILE_DATA;

void DebugLog(QString msg);
void polint(double xa[], double ya[], int n, double x, double *y, double *dy);

class Worker : public QObject
{
    Q_OBJECT

public:
    explicit Worker(QObject *parent = 0);
    void requestWork1();
    void requestWork2();
    void requestWork3();

    void abort();

private:
    bool _abort;
    bool _working;
    QMutex mutex;
    QElapsedTimer _timer;

public slots:
    void doWork1();
    void doWork2();
    void doWork3();


signals:
    void workRequested1();
    void workRequested2();
    void workRequested3();

    void valueChanged(const QString &value);
    void lbErr(const QString &value);
    void lbVisibled(bool v);
    void btnEnabled1(bool v);
    void btnEnabled2(bool v);
    void btnEnabled3(bool v);
    void editFocus();
    void editvalueChanged(const QString &value);

    void processEnd1();
    void processEnd2();
    void processEnd3();

protected:

    bool SelectDB_History(QSqlDatabase db ,const QString path);
    bool SelectDB_TIR_latitude(QSqlDatabase db);
    bool SelectDB_TIR_data(QSqlDatabase db ,int id);
    bool SelectDB_Roughnessmodel_data(QSqlDatabase db, int id);
    //bool SelectDB_Result(QSqlDatabase db , int id);

    bool insertDB_History(QSqlDatabase db,const QString path ,const QString type);
    bool insertDB_TIR(QSqlDatabase db,const QString path);
    bool insertDB_RoughnessModel(QSqlDatabase db,const QString path);
    bool insertDB_INPOL(QSqlDatabase db);
    bool insertDB_Best_Fit(QSqlDatabase db,int id);

    bool updateDB_Key(QSqlDatabase db,int flg);

    bool deleteDB_tmp_Best_Fit(QSqlDatabase db);
    bool deleteDB_tmp_INPOL(QSqlDatabase db);
    bool deleteDB_t_INPOL(QSqlDatabase db);


public:

    bool SelectDB_Best_Fit(QSqlDatabase db);

    QSqlDatabase _db;
    ulong _cnt;
};




class readtxt : public QDialog
{
    Q_OBJECT

public:
    explicit readtxt(QWidget *parent = nullptr);
    ~readtxt();

    QSqlDatabase _db;
    QThread* myTh;
    Worker* myWork;

public slots:
      void GetFolderPath();
      void ProcessReg();
      void ProcessCalc();
      void ProcessShow();
      void ProcessOneCalc();
      void ProcessOutput();
      void ProcessReadIni();

private slots:

signals:
    void dirPathSignal(QString);

private:
    Ui::readtxt *ui;

public:
    //void pCalc();


protected:

    void Init();
    void MakeComb(int idx);

    // bool GetIniPass();
    bool GetIniPass(QString filePath = INI_PASS_FILE);
    //QString GetMysqlPass();
    void connectDB(QSqlDatabase& db);

    bool GetSelectKey();
    //bool GetFile(QString path,QString first);
   //void connectDB(QSqlDatabase& db);

    bool SelectDB_Key_data(QSqlDatabase db);
    bool SelectDB_Key_data_one(QSqlDatabase db,int id);
    int SelectDB_Roughnessmodel_data_one(QSqlDatabase db);
    int SelectDB_TIR_data_one(QSqlDatabase db);
    QString SelectDB_TIR_data_latitude_one(QSqlDatabase db,int id);
    bool SelectDB_TIR_log_lat(QSqlDatabase db);
    //bool SelectDB_Roughnessmodel_data(QSqlDatabase db,,int id ,int s ,int g);
    //bool SelectDB_TIR_data(QSqlDatabase db,,int id);
    //bool SelectDB_TIR_latitude(QSqlDatabase db);
    //bool SelectDB_History(QSqlDatabase db,QString path);
    bool SelectDB_Result(QSqlDatabase db,int id ,QString s ,QString g);
    bool SelectDB_Best_Fit(QSqlDatabase db);
    bool SelectDB_All_ID_Best_Fit(QSqlDatabase db);

    bool insertDB_Key(QSqlDatabase db,QString name);
    //bool insertDB_History(QSqlDatabase db,QString path ,QString type);
    //bool insertDB_TIR(QSqlDatabase db,QString path);
    //bool insertDB_RoughnessModel(QSqlDatabase db,QString path);
    //bool insertDB_INPOL(QSqlDatabase db);
    //bool insertDB_Best_Fit(QSqlDatabase db,int id);
    bool insertDB_idx(QSqlDatabase db ,int i);

    //bool updateDB_Key(QSqlDatabase db,int flg);

    //void polint(double xa[], double ya[], int n, double x, double *y, double *dy);
    //double *dvector(long nl, long nh);
    //void free_dvector(double *v, long nl, long nh);
    double ceil( double dSrc, int iLen );
    double floor( double dSrc, int iLen );
    double round( double dSrc, int iLen );

    //void DebugLog(QString msg);
    //QString GetMysqlPass();


};

#endif // READTXT_H
