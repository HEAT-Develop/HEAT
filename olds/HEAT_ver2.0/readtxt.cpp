#include "readtxt.h"
#include "ui_readtxt.h"
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QString>
#include <QFile>
#include <QTextCodec>
#include <QtConcurrent>
#include <QThread>
#include <QApplication>
#include <QDateTime>


using namespace std;

extern QSqlDatabase Global_DB;

int SelBtn;

QString retErrMsg = "";
QString sigmaArry[] = {"0.0","0.1","0.2","0.3","0.4","0.5"};
QString gammaArry[] = {"0010","0100","0200","0400","0600","0800"};

INI_DATA IniInfo;
QString Tir_Latitude[SIZE_DATA + 1] = {LATITUDE_ERR};
QList<INPOL_DATA> TirList[SIZE_DATA + 1];
QList<INPOL_DATA> RoughnessList[SIZE_DATA + 1][SIZE_SIGMA_GAMMA][SIZE_SIGMA_GAMMA];
QList<INPOL_DATA> InPolList;
QList<KEY_DATA> KeyList;
KEY_DATA SelectKey;
QList<QString> FitList;
QList<INPOL_DATA>lst;

OUT_FILE_DATA ofd[SIZE_DATA + 1];

QString SelectPath;
QString FirstChar;
bool SelectCk;
QString SelectPolygonID;

QMutex mutex2;
QElapsedTimer timer;
bool Processing = false;


void CleanWoker1(int i ,int j , int k)
{
    RoughnessList[i][j][k].clear();
}

void CleanWoker2(int i)
{
    Tir_Latitude[i] = LATITUDE_ERR;
    TirList[i].clear();
}

void SubWoker(int i)
 {

    double a;
    double xa[3], ya[3];
    int j,k,l,ii,jj;
    int m ,n;
    double temp[SIZE_LINE] = {0};
    double diff[SIZE_LINE] = {0};

     m = TirList[i].count();

    if(Tir_Latitude[i] == LATITUDE_ERR) return;
    if(Tir_Latitude[i] == "") return;


    for(j = 0 ; j < SIZE_SIGMA_GAMMA ; j++) //sigma
    {
        for(k = 0 ; k < SIZE_SIGMA_GAMMA ; k++) //gamma
        {
            lst.clear();
            n = RoughnessList[i][j][k].count();

            for(ii = 0 ; ii < n - 1 ; ii++)
            {
                xa[0] = 0;
                xa[1] = RoughnessList[i][j][k].at(ii).time;
                xa[2] = RoughnessList[i][j][k].at(ii + 1).time;

                ya[0] = 0;
                ya[1] = RoughnessList[i][j][k].at(ii).temp;
                ya[2] = RoughnessList[i][j][k].at(ii + 1).temp;

                for(jj = 0 ; jj < m ; jj++)
                {
                    a = TirList[i].at(jj).time;
                    if (a > xa[1]  && a <= xa[2]) {

                          polint(xa, ya, 2, a, &temp[jj], &diff[jj]);
                          INPOL_DATA d;
                          d.time = a;
                          d.temp = 0;
                          d.diff = 0;
                          d.tir_id = TirList[i].at(jj).tir_id;
                          d.polygon_id = i;
                          d.sigma = sigmaArry[j];
                          d.gamma = gammaArry[k];

                          lst.append(d);

                    }
                }

            }

            QtConcurrent::run(CleanWoker1,int(i),int(j),int(k));

            if(lst.count() == 0) continue;

            for( l = 0 ; l < m ; l++)
            {
                INPOL_DATA d = lst.at(l);
                d.temp = temp[l];
                d.diff = diff[l];

                InPolList.append(d);
            }



        }
    }
}


Worker::Worker(QObject *parent) :
    QObject(parent)
{
    _working =false;
    _abort = false;
}

void Worker::requestWork1()
{
    mutex.lock();
    _working = true;
    _abort = false;

    mutex.unlock();

    emit workRequested1();
}

void Worker::requestWork2()
{
    mutex.lock();
    _working = true;
    _abort = false;

    mutex.unlock();

    emit workRequested2();
}

void Worker::requestWork3()
{
    mutex.lock();
    _working = true;
    _abort = false;

    mutex.unlock();

    emit workRequested3();
}



void Worker::abort()
{
    mutex.lock();
    if (_working) {
        _abort = true;

    }
    mutex.unlock();
}

// ALL Data Extraction Thread
void Worker::doWork1()
{

    if(SelBtn != BTN_CALC){

        emit this->processEnd1();
        return;
    }

    int i,j;
    QString msg = "";
    _cnt = 0;

    emit valueChanged("Polygonid=[-] : time=[-]");
    emit this->lbVisibled(true);
    _timer.start();
    //int max = SIZE_DATA
    int max = IniInfo.LoopEnd;
    int loop = 16;


    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    _cnt = _timer.elapsed() / 1000;
    msg = QString("Polygonid=[%1] : time=[%2s]" ).arg(QString::number(0)).arg(QString::number(_cnt));
    emit valueChanged(msg);
    emit lbErr(retErrMsg);


    for(i = 1 ; i <= max ; i++)
    {

        mutex.lock();
        bool abort = _abort;
        mutex.unlock();

        if (abort) {
            break;
        }


        SelectDB_TIR_data(_db,i);
        SelectDB_Roughnessmodel_data(_db,i);

        QFuture<void> t1 = QtConcurrent::run(SubWoker,int(i));
        t1.waitForFinished();

        if(i % loop == 0)
        {
            insertDB_INPOL(_db);

            j = i - loop + 1;

            for(; j <= i ; j++ )
            {
                insertDB_Best_Fit(_db,j);
                QtConcurrent::run(CleanWoker2,int(j));
            }

            deleteDB_t_INPOL(_db);
        }

        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        _cnt = _timer.elapsed() / 1000;
        msg = QString("Polygonid=[%1] : time=[%2s]" ).arg(QString::number(i)).arg(QString::number(_cnt));
        emit valueChanged(msg);
        emit lbErr(retErrMsg);
        if(retErrMsg != "")
        {
            DebugLog(retErrMsg);
            retErrMsg = "";
        }

    }

    updateDB_Key(_db,1);

    mutex.lock();
    _working = false;
    mutex.unlock();

    emit this->lbVisibled(false);
    emit this->btnEnabled1(true);
    emit this->btnEnabled2(true);

    mutex2.lock();
    Processing = false;
    mutex2.unlock();

    // 処理終了
    emit this->processEnd1();

}

// Register in DataBase Thread
void Worker::doWork2()
{
    if(SelBtn != BTN_REG){

        emit this->processEnd2();
        return;
    }

    int i = 1;
    bool ret2 = true;
    QString msg = "";

    emit this->lbVisibled(true);
    _timer.start();

    QStringList nameFilters;
    QString f1 = FirstChar.trimmed() + "*.txt";
    QString f2 = FirstChar.trimmed() + "*.TXT";
    nameFilters << f1 << f2;
    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags;

    QDirIterator it(SelectPath, nameFilters, QDir::Files, flags);
    QStringList files;
    QString file = "";

    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    _cnt = _timer.elapsed() / 1000;
    msg = QString("file=[%1] : time=[%2s]" ).arg(QString::number(i)).arg(QString::number(_cnt));
    emit valueChanged(msg);
    emit lbErr(retErrMsg);



    // file
    while (it.hasNext())
    {
       file = it.next();

       if(SelectDB_History(_db,file))
       {

           if(SelectCk)
           {
               //tir
               ret2 = insertDB_TIR(_db,file);
               if(ret2)
               {
                   ret2 = insertDB_History(_db,file,FILE_TYEP_TIR);
               }


           }else{
               //rouhnessmodel
               ret2 = insertDB_RoughnessModel(_db,file);
               if(ret2)
               {
                   ret2 = insertDB_History(_db,file,FILE_TYEP_ROUGHNESSMODEL);
               }

           }

           files << file;
       }

       if(!ret2)
       {
          retErrMsg = QString("%1 : Read Error").arg(file);

       }

       QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
       _cnt = _timer.elapsed() / 1000;
        msg = QString("file=[%1] : time=[%2s]" ).arg(QString::number(i)).arg(QString::number(_cnt));
       emit valueChanged(msg);

        if(retErrMsg != "")
        {
            emit lbErr(retErrMsg);
            DebugLog(retErrMsg);
            retErrMsg = "";
        }

       i++;

    }

    if(files.count() == 0)
    {
       retErrMsg = "Match file is zero";
       emit lbErr(retErrMsg);
    }

    mutex.lock();
    _working = false;
    mutex.unlock();

    emit this->lbVisibled(false);
    emit this->btnEnabled1(true);
    emit this->btnEnabled2(true);

    mutex2.lock();
    Processing = false;
    mutex2.unlock();

    // 処理終了
    emit this->processEnd2();

}

// ONE Data Extraction Thread
void Worker::doWork3()
{

    if(SelBtn != BTN_ONE_CALC){

        emit this->processEnd3();
        return;
    }

    int i;  //,j;
    QString msg = "";
    _cnt = 0;

    emit valueChanged("Polygonid=[-] : time=[-]");
    emit this->lbVisibled(true);
    _timer.start();
    //int max = 256;
    //int loop = 16;


    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    _cnt = _timer.elapsed() / 1000;
    msg = QString("Polygonid=[%1] : time=[%2s]" ).arg(QString::number(0)).arg(QString::number(_cnt));
    emit valueChanged(msg);
    emit lbErr(retErrMsg);


    //mutex.lock();
    //bool abort = _abort;
    //mutex.unlock();

    //if (abort) {
        //break;
    //}

    //for(i = 1 ; i <= max ; i++)
    //{
    i = SelectPolygonID.toInt();


        SelectDB_TIR_data(_db,i);
        SelectDB_Roughnessmodel_data(_db,i);

        QFuture<void> t3 = QtConcurrent::run(SubWoker,int(i));
        t3.waitForFinished();

        //if(i % loop == 0)
        //{
            insertDB_INPOL(_db);

           // j = i - loop + 1;

           // for(; j <= loop ; j++ )
           // {
                //insertDB_Best_Fit(_db,j);
                insertDB_Best_Fit(_db,i);
                //QtConcurrent::run(CleanWoker2,int(i));

           // }
        //}


        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        _cnt = _timer.elapsed() / 1000;
        msg = QString("Polygonid=[%1] : time=[%2s]" ).arg(QString::number(i)).arg(QString::number(_cnt));
        emit valueChanged(msg);
        emit lbErr(retErrMsg);
        if(retErrMsg != "")
        {
            DebugLog(retErrMsg);
            retErrMsg = "";
        }

    //}

    //updateDB_Key(_db,1);

    mutex.lock();
    _working = false;
    mutex.unlock();

    emit this->lbVisibled(false);
    emit this->btnEnabled1(true);
    emit this->btnEnabled2(true);

    QString edt = "";
    SelectDB_Best_Fit(_db);
    QtConcurrent::run(CleanWoker2,int(i));

    foreach(QString line,FitList)
    {
        edt += line;
        edt += "\n";

    }

    emit this->editvalueChanged(edt);
    emit this->editFocus();

    deleteDB_tmp_INPOL(_db);
    deleteDB_tmp_Best_Fit(_db);

    mutex2.lock();
    Processing = false;
    mutex2.unlock();

    // 処理終了
    emit this->processEnd3();

}



readtxt::readtxt(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::readtxt)
{
    ui->setupUi(this);
     this->setWindowTitle("T.Inertia");

    QObject::connect(ui->FolderDlgOpenpushButton,SIGNAL(clicked()),this,SLOT(GetFolderPath()));
    QObject::connect(ui->inDataBaseButton,SIGNAL(clicked()),this,SLOT(ProcessReg()));
    QObject::connect(ui->extractDataButton,SIGNAL(clicked()),this,SLOT(ProcessCalc()));
    QObject::connect(ui->showDataButton,SIGNAL(clicked()),this,SLOT(ProcessShow()));
    QObject::connect(ui->oneExtractDataButton,SIGNAL(clicked()),this,SLOT(ProcessOneCalc()));
    QObject::connect(ui->outPutButton,SIGNAL(clicked()),this,SLOT(ProcessOutput()));
    QObject::connect(ui->readIniButton,SIGNAL(clicked()),this,SLOT(ProcessReadIni()));


    Init();

}

readtxt::~readtxt()
{
    myWork->abort();
    myTh->wait();
    delete myWork;
    delete myTh;

    _db.close();
    delete ui;



}


void readtxt::Init()
{

    SelBtn = 0;
    ui->proclb->setVisible(false);
    ui->GroupcomboBox->clear();
    ui->ShowtextEdit->clear();

    SelectKey.id = -1;
    SelectKey.sid = "-1";
    SelectKey.name = "";
    SelectKey.flg = 0;

    SelectPolygonID = "0";
    FitList.clear();

    mutex2.lock();
    Processing = false;
    mutex2.unlock();

    GetIniPass();

    myTh = new QThread(this);
    myWork = new Worker();
    QObject::connect(myTh , SIGNAL(started()) , myWork , SLOT(doWork1()));
    QObject::connect(myWork , SIGNAL(processEnd1()) , myTh , SLOT(quit()),Qt::DirectConnection);
    QObject::connect(myWork, SIGNAL(workRequested1()), myTh, SLOT(start()));

    QObject::connect(myTh , SIGNAL(started()) , myWork , SLOT(doWork2()));
    QObject::connect(myWork , SIGNAL(processEnd2()) , myTh , SLOT(quit()),Qt::DirectConnection);
    QObject::connect(myWork, SIGNAL(workRequested2()), myTh, SLOT(start()));

    QObject::connect(myTh , SIGNAL(started()) , myWork , SLOT(doWork3()));
    QObject::connect(myWork , SIGNAL(processEnd3()) , myTh , SLOT(quit()),Qt::DirectConnection);
    QObject::connect(myWork, SIGNAL(workRequested3()), myTh, SLOT(start()));

    QObject::connect(myWork, SIGNAL(valueChanged(QString)), ui->tmcntlb, SLOT(setText(QString)));
    QObject::connect(myWork, SIGNAL(lbErr(QString)), ui->errlb, SLOT(setText(QString)));
    QObject::connect(myWork, SIGNAL(lbVisibled(bool)), ui->proclb, SLOT(setVisible(bool)));
    QObject::connect(myWork, SIGNAL(btnEnabled1(bool)), ui->extractDataButton, SLOT(setEnabled(bool)));
    QObject::connect(myWork, SIGNAL(btnEnabled2(bool)), ui->inDataBaseButton, SLOT(setEnabled(bool)));

    /*
    ui->ShowtextEdit->setFocus();
    QTextCursor tmpCursor = ui->ShowtextEdit->textCursor();
    tmpCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
    ui->ShowtextEdit->setTextCursor(tmpCursor);
    */

    QObject::connect(myWork, SIGNAL(editFocus()), ui->ShowtextEdit, SLOT(setFocus()));
    QObject::connect(myWork, SIGNAL(editvalueChanged(QString)), ui->ShowtextEdit, SLOT(setText(QString)));


    connectDB(_db);
    MakeComb(-1);

    ui->tmcntlb->setText("");
}


void readtxt::MakeComb(int idx)
{
    ui->GroupcomboBox->clear();
    SelectDB_Key_data(_db);

    foreach (KEY_DATA d, KeyList)
    {
        ui->GroupcomboBox->addItem(d.name,QVariant(d.id));
    }

    ui->GroupcomboBox->setCurrentIndex(idx);
    if(idx != -1)
    {
        ui->GroupcomboBox->setCurrentIndex( ui->GroupcomboBox->findText(SelectKey.name));
    }

    ui->GroupcomboBox->setFocus();

}

void readtxt::GetFolderPath()
{
    QString sDir = ui->FolderPathTextEdit->toPlainText();
    if(sDir == "")
    {
        sDir = QDir::homePath();
    }
    QDir strPath = QFileDialog::getExistingDirectory(this,"Open Directory",sDir);
    if(!strPath.isEmpty() && (!(strPath.path().compare(".") == 0)))
    {
        ui->FolderPathTextEdit->setText(strPath.path());
    }


}

void readtxt::ProcessCalc()
{

    if(Processing) return;
    mutex2.lock();
    Processing = true;
    mutex2.unlock();

    SelBtn = BTN_CALC;

    if(!GetSelectKey())
    {
        mutex2.lock();
        Processing = false;
        mutex2.unlock();
        return;
    }



    for(int l = 1 ;  l <= SIZE_DATA ; l++)
    {
        Tir_Latitude[l] = LATITUDE_ERR;
        TirList[l].clear();

    }


    QMessageBox msgBox;
    msgBox.setWindowTitle("Data extraction");
    msgBox.setText("Are you sure you want to run it?");
    msgBox.setStandardButtons(QMessageBox::Yes);
    msgBox.addButton(QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    if(msgBox.exec() == QMessageBox::No){
      mutex2.lock();
      Processing = false;
      mutex2.unlock();
      return;
    }

    retErrMsg = "";

    if(SelectDB_TIR_data_one(_db) == 0)
    {
        QMessageBox::warning(this, "warning", "TIR data not found");
        mutex2.lock();
        Processing = false;
        mutex2.unlock();
        return;
    }

    if(SelectDB_Roughnessmodel_data_one(_db) == 0)
    {
        QMessageBox::warning(this, "warning", "RoughnessModel data not found");
        mutex2.lock();
        Processing = false;
        mutex2.unlock();
        return;
    }

    ui->extractDataButton->setEnabled(false);
    ui->inDataBaseButton->setEnabled(false);

    myWork->_db = _db;
    myWork->moveToThread(myTh);

    myWork->requestWork1();

}

void readtxt::ProcessReg()
{

    if(Processing) return;
    mutex2.lock();
    Processing = true;
    mutex2.unlock();

    SelBtn = BTN_REG;

    if(!GetSelectKey())
    {
        mutex2.lock();
        Processing = false;
        mutex2.unlock();
        return;
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle("Register in database");
    msgBox.setText("Are you sure you want to run it?");
    msgBox.setStandardButtons(QMessageBox::Yes);
    msgBox.addButton(QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    if(msgBox.exec() == QMessageBox::No){
        mutex2.lock();
       Processing = false;
       mutex2.unlock();
      return;
    }


    //timer.start();

    retErrMsg = "";

    //ui->proclb->setVisible(true);
    //this->ui->proclb->repaint();

    QString sDir =  ui->FolderPathTextEdit->toPlainText() + "\\";
    QString sFn =  ui->FirstPartTextEdit->toPlainText().trimmed();
    if(sDir.isNull() || sDir.isEmpty())
    {
        QMessageBox::warning(this, "warning", "Please select to directory path");
        mutex2.lock();
        Processing = false;
        mutex2.unlock();

    }else{
        if(sFn.isNull() || sFn.isEmpty())
        {
            QMessageBox::warning(this, "warning", "Please input to first part of the file name");
            mutex2.lock();
            Processing = false;
            mutex2.unlock();
        }else{

            ui->extractDataButton->setEnabled(false);
            ui->inDataBaseButton->setEnabled(false);

            SelectPath = sDir;
            FirstChar = sFn;
            SelectCk = ui->radioButton0->isChecked();

            myWork->_db = _db;
            myWork->moveToThread(myTh);

            myWork->requestWork2();


            /*
            bool ret = GetFile(sDir,sFn);
            if(!ret)
            {
                QMessageBox::warning(this, "warning", retErrMsg);
            }


            QMessageBox::warning(this, "warning", msg);

            */
        }
    }

   // ui->proclb->setVisible(false);

}

void readtxt::ProcessShow()
{

    if(Processing) return;
    mutex2.lock();
    Processing = true;
    mutex2.unlock();

    SelBtn = BTN_SHOW;

    if(!GetSelectKey())
    {
        mutex2.lock();
        Processing = false;
        mutex2.unlock();
        return;
    }

    QString id = ui->polygonidTxt->text().trimmed();
    int len = id.length();
    if(len == 0){
         QMessageBox::warning(this, "warning", "Please input to PolygonID");
         mutex2.lock();
         Processing = false;
         mutex2.unlock();
         return;
    }

    bool ok;
    int dec = id.toInt(&ok, 10);
    if(!ok || dec == 0 || dec > SIZE_DATA )
    {
        QMessageBox::warning(this, "warning", "Please input to PolygonID");
        mutex2.lock();
        Processing = false;
        mutex2.unlock();
        return;
    }

    int pos = id.indexOf(".",0);
    if(pos != -1)
    {
        QMessageBox::warning(this, "warning", "Please input to PolygonID");
        mutex2.lock();
        Processing = false;
        mutex2.unlock();
        return;
    }

    ui->ShowtextEdit->clear();
    FitList.clear();
    SelectPolygonID = ui->polygonidTxt->text();
    int polygonid = SelectPolygonID.toInt();
    Tir_Latitude[polygonid] = SelectDB_TIR_data_latitude_one(_db,polygonid);

    myWork->SelectDB_Best_Fit(_db);

    foreach (QString line, FitList) {
        ui->ShowtextEdit->append(line);
    }
    ui->ShowtextEdit->setFocus();
    QTextCursor tmpCursor = ui->ShowtextEdit->textCursor();
    tmpCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
    ui->ShowtextEdit->setTextCursor(tmpCursor);

    FitList.clear();
    Tir_Latitude[polygonid] = LATITUDE_ERR;

    mutex2.lock();
    Processing = false;
    mutex2.unlock();

}

void readtxt::ProcessOneCalc()
{

    if(Processing) return;
    mutex2.lock();
    Processing = true;
    mutex2.unlock();

    SelBtn = BTN_ONE_CALC;

    if(!GetSelectKey())
    {
        mutex2.lock();
        Processing = false;
        mutex2.unlock();
        return;
    }

    retErrMsg = "";

    if(SelectDB_TIR_data_one(_db) == 0)
    {
        QMessageBox::warning(this, "warning", "TIR data not found");
        mutex2.lock();
        Processing = false;
        mutex2.unlock();
        return;
    }

    if(SelectDB_Roughnessmodel_data_one(_db) == 0)
    {
        QMessageBox::warning(this, "warning", "RoughnessModel data not found");
        mutex2.lock();
        Processing = false;
        mutex2.unlock();
        return;
    }



    QString id = ui->polygonidTxt->text().trimmed();
    int len = id.length();
    if(len == 0){
         QMessageBox::warning(this, "warning", "Please input to PolygonID");
         mutex2.lock();
         Processing = false;
         mutex2.unlock();
         return;
    }

    bool ok;
    int dec = id.toInt(&ok, 10);
    if(!ok || dec == 0 || dec > SIZE_DATA )
    {
        QMessageBox::warning(this, "warning", "Please input to PolygonID");
        mutex2.lock();
        Processing = false;
        mutex2.unlock();
        return;
    }

    int pos = id.indexOf(".",0);
    if(pos != -1)
    {
        QMessageBox::warning(this, "warning", "Please input to PolygonID");
        mutex2.lock();
        Processing = false;
        mutex2.unlock();
        return;
    }

    ui->ShowtextEdit->clear();
    FitList.clear();

    SelectPolygonID = ui->polygonidTxt->text();

    int polygonid = SelectPolygonID.toInt();

    Tir_Latitude[polygonid] = LATITUDE_ERR;
    TirList[polygonid].clear();

    ui->extractDataButton->setEnabled(false);
    ui->inDataBaseButton->setEnabled(false);


    myWork->_db = _db;
    myWork->moveToThread(myTh);

    myWork->requestWork3();



    /*
    SelectDB_TIR_latitude(_db);

    for(int i = 0 ; i <= SIZE_DATA + 1 ; i++)
    {
       if(Tir_Latitude[i] == "")
       {
           qDebug() << "id=" + QString::number(i);
       }
    }
    */

}

void readtxt::ProcessOutput()
{

    if(Processing) return;
    mutex2.lock();
    Processing = true;
    mutex2.unlock();

    SelBtn = BTN_OUT_PUT;

    if(!GetSelectKey())
    {
        mutex2.lock();
        Processing = false;
        mutex2.unlock();
        return;
    }

    QString outPathGamma = "";
    QString outPathSigma = "";

    retErrMsg = "";

    for(int l = 1 ;  l <= SIZE_DATA ; l++)
    {
         // do not modify
        ofd[l].id = QString::number(l);
        // can modify change
        ofd[l].longitude = OUT_FILE_NON_DATA;
        ofd[l].latitude = OUT_FILE_NON_DATA;
        ofd[l].sigma = OUT_FILE_NON_DATA;
        ofd[l].gamma = OUT_FILE_NON_DATA;

    }

    SelectDB_TIR_log_lat(_db);
    if(retErrMsg != "")
    {
        DebugLog(retErrMsg);
        retErrMsg = "";
        mutex2.lock();
        Processing = false;
        mutex2.unlock();
        return;
    }

    if(SelectDB_All_ID_Best_Fit(_db))
    {
        QFileDialog fileDialog(this);
        fileDialog.setFileMode(QFileDialog::Directory);
        fileDialog.setOption(QFileDialog::ShowDirsOnly, true);
        QStringList filePaths;
        if(fileDialog.exec())
        {
            filePaths = fileDialog.selectedFiles();
        }

        if (filePaths.size() > 0)
        {
            QString strDate = SelectKey.name;
            strDate += "_";
            strDate += QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
            strDate += ".txt";
            QString f_sigma = "sigma_" + strDate;
            QString f_gamma = "gamma_" + strDate;
            outPathGamma = f_gamma;
            outPathSigma = f_sigma;

            QString sfn = filePaths.at(0) + QDir::separator() + f_sigma;
            QString gfn = filePaths.at(0) + QDir::separator() + f_gamma;

            QFile sfile(sfn);
            QFile gfile(gfn);
            bool isA = sfile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
            bool isB = gfile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
            QTextStream sts( &sfile );
            QTextStream gts( &gfile );
            sts.setCodec( QTextCodec::codecForName( "UTF-8" ) );     // Code. Qt5
            gts.setCodec( QTextCodec::codecForName( "UTF-8" ) );     // Code. Qt5
            //sts.setEncoding( QStringConverter::Utf8 );        // Fix. Qt6
            //gts.setEncoding( QStringConverter::Utf8 );        // Fix. At6

            QString tmp_s,tmp_g;
            for( int i = 1 ; i <= IniInfo.LoopEnd ; i++)
            {
                tmp_s = QString("%1 %2 %3 %4").arg(ofd[i].id).arg(ofd[i].longitude).arg(ofd[i].latitude).arg(ofd[i].sigma);
                tmp_g = QString("%1 %2 %3 %4").arg(ofd[i].id).arg(ofd[i].longitude).arg(ofd[i].latitude).arg(ofd[i].gamma);

                // sts << tmp_s << endl;        // Code. Qt5
                // gts << tmp_g << endl;        // Code. Qt5
                sts << tmp_s << Qt::endl;       // Fix. Qt6
                gts << tmp_g << Qt::endl;       // Fix. Qt6
            }
            sfile.close();
            gfile.close();

            QMessageBox::information(this, "OutPut", "Fin");
       }
       mutex2.lock();
       Processing = false;
       mutex2.unlock();
       return;
    }
    mutex2.lock();
    Processing = false;
    mutex2.unlock();

    if(retErrMsg != "")
    {
        DebugLog(retErrMsg);
        retErrMsg = "";
        return;
    }
}

bool readtxt::GetSelectKey()
{
    bool ret = false;
    int id = -1,idx = -1;

    QString key =  ui->GroupcomboBox->currentText();
    if(key.trimmed() == "")
    {
       QString msg = "Please enter or select a Group Name";
       QMessageBox::warning(this, "warning", msg);
       return ret;
     }else if(key.length() > 20)
    {
        QString msg = "Please enter within 20 characters";
        QMessageBox::warning(this, "warning", msg);
        return ret;
    }

    ui->GroupcomboBox->setCurrentIndex( ui->GroupcomboBox->findText(key));
    idx = ui->GroupcomboBox->currentIndex();

    if(idx == -1){

        insertDB_Key(_db,key);
        SelectDB_Key_data(_db);

        foreach (KEY_DATA d, KeyList) {
           if(d.name == key)
           {
                id = d.id;
                break;
           }
        }

        SelectDB_Key_data_one(_db,id);

    }
    else{

        id = ui->GroupcomboBox->itemData(idx).toInt();
        SelectDB_Key_data_one(_db,id);

        if((SelBtn == BTN_CALC || SelBtn == BTN_REG) && SelectKey.id != -1 && SelectKey.flg == 1)
        {
            QString msg = "Extraction has already finished";
            QMessageBox::warning(this, "warning", msg);
            return ret;
        }

        if(SelBtn == BTN_OUT_PUT && (SelectKey.id == -1 || SelectKey.flg == 0))
        {
            QString msg = "Please Go ahead with the Extraction process";
            QMessageBox::warning(this, "warning", msg);
            return ret;
        }
    }

    MakeComb(idx);
    ui->GroupcomboBox->setCurrentIndex(ui->GroupcomboBox->findText(SelectKey.name));

    return true;
}


/*
bool readtxt::GetFile(QString path ,QString first)
{

    bool ret = false;
    bool ret2 = true;
    QStringList nameFilters;
    QString f1 = first.trimmed() + "*.txt";
    QString f2 = first.trimmed() + "*.TXT";
    nameFilters << f1 << f2;

    QDirIterator it(path, nameFilters, QDir::Files, false);
    QStringList files;
    QString file = "";

    if(retErrMsg != "")
    {
        return ret;
    }

    // file
    while (it.hasNext())
    {
       file = it.next();

       if(SelectDB_History(_db,file))
       {

           if(ui->radioButton0->isChecked())
           {
               //tir
               ret2 = insertDB_TIR(_db,file);
               if(ret2)
               {
                   ret2 = insertDB_History(_db,file,FILE_TYEP_TIR);
               }


           }else{
               //rouhnessmodel
               ret2 = insertDB_RoughnessModel(_db,file);
               if(ret2)
               {
                   ret2 = insertDB_History(_db,file,FILE_TYEP_ROUGHNESSMODEL);
               }

           }

           files << file;
       }

       if(!ret2)
       {
          return ret;
       }

    }

    if(files.count() == 0)
    {
       retErrMsg = "Match file is zero";
       return ret;
    }

    ret = true;
    return ret;
}
*/

void readtxt::connectDB(QSqlDatabase& db)
{

    db = QSqlDatabase::addDatabase(QString("QMYSQL"));

    db.setHostName("127.0.0.1");

    db.setUserName("root");

    //db.setPassword(GetMysqlPass());
    //db.setPassword(IniInfo.MySqlPass);    // onos:code
    db.setPassword("demodemo");             // 202306:code

    db.setDatabaseName("heat_db");

    db.setPort(IniInfo.MySqlPort);

    bool ret = db.open();
    if(!ret)
    {
        retErrMsg = QString("can not open db: your password = [%1] port = [%2] ")
                .arg(IniInfo.MySqlPass).arg(QString::number(IniInfo.MySqlPort));
        retErrMsg += db.lastError().text();
        DebugLog(retErrMsg);
        retErrMsg = "";
    }

    Global_DB = db;

    /*for(int i = 1 ; i <= 198806 ; i++)
    {
        insertDB_idx(db,i);
    }
    */

}


bool readtxt::SelectDB_Key_data(QSqlDatabase db)
{
    bool ret = false;

    QSqlQuery my_query = QSqlQuery(db);

    QString sql = QString("SELECT * FROM t_key ;");

    ret = my_query.exec(sql);
    if(ret)
    {
        KeyList.clear();

        KEY_DATA d;
        while (my_query.next()) {

            d.id = my_query.value(0).toUInt();
            d.sid = my_query.value(0).toString();
            d.name = my_query.value(1).toString();
            d.flg = my_query.value(2).toInt();

            KeyList.append(d);
        }
    } else {
        retErrMsg = "SelectDB_Key_data :" + my_query.lastError().text();
    }

    my_query.clear();
    return ret;
}

bool readtxt::SelectDB_Key_data_one(QSqlDatabase db,int id)
{
    bool ret = false;

    QSqlQuery my_query = QSqlQuery(db);

    QString sql = QString("SELECT k.* FROM t_key AS k WHERE k.id = %1 ;").arg(QString::number(id));

    ret = my_query.exec(sql);
    if(ret)
    {
        SelectKey.id = -1;
        SelectKey.sid = "-1";
        SelectKey.name = "";
        SelectKey.flg = 0;

        KEY_DATA d;
        while (my_query.next()) {

            d.id = my_query.value(0).toUInt();
            d.sid = my_query.value(0).toString();
            d.name = my_query.value(1).toString();
            d.flg = my_query.value(2).toInt();

            SelectKey = d;
        }
    } else {
        retErrMsg = "SelectDB_Key_data_one :" + my_query.lastError().text();
    }

    my_query.clear();
    return ret;
}

int readtxt::SelectDB_TIR_data_one(QSqlDatabase db)
{
    int ret = 0;

    QSqlQuery my_query = QSqlQuery(db);

    QString sql = QString("SELECT t.id FROM t_tir AS t WHERE t.key_id = %1 limit 1;").arg(SelectKey.sid);

    if (my_query.exec(sql)) {

        while (my_query.next()) {

           ret++;
        }

    } else {
        retErrMsg = "SelectDB_TIR_data_one :" + my_query.lastError().text();
    }

    my_query.clear();
    return ret;
}

QString readtxt::SelectDB_TIR_data_latitude_one(QSqlDatabase db,int id)
{
    QString ret = LATITUDE_ERR;

    QSqlQuery my_query = QSqlQuery(db);

    QString sql = QString("SELECT t.latitude FROM t_tir AS t WHERE t.key_id = %1 AND t.polygon_id = %2 limit 1;").arg(SelectKey.sid).arg(QString::number(id));

    if (my_query.exec(sql)) {

        while (my_query.next()) {

           ret = QString::number(my_query.value(0).toDouble(),'f',6);
        }

    } else {
        retErrMsg = "SelectDB_TIR_data_latitude_one :" + my_query.lastError().text();
    }

    my_query.clear();
    return ret;
}


int readtxt::SelectDB_Roughnessmodel_data_one(QSqlDatabase db)
{
    int ret = 0;

    QSqlQuery my_query = QSqlQuery(db);

    QString sql = QString("SELECT t.id FROM t_roughnessmodel AS t WHERE t.key_id = %1 limit 1;").arg(SelectKey.sid);

    if (my_query.exec(sql)) {

        while (my_query.next()) {

           ret++;
        }

    } else {
        retErrMsg = "SelectDB_Key_data_one :" + my_query.lastError().text();
    }

    my_query.clear();
    return ret;
}

bool Worker::SelectDB_Roughnessmodel_data(QSqlDatabase db ,int id)
//bool readtxt::SelectDB_Roughnessmodel_data(int id ,int s ,int g)
{
    bool ret = false;


    QSqlQuery my_query = QSqlQuery(db);

    QString sql = "SELECT r.local_solar_time,r.temperature ,r.sigma,r.gamma ,r.sigma_id,r.gamma_id FROM t_roughnessmodel AS r WHERE ";
    QString sqlstr = QString( " r.key_id = %1 AND %2 > r.sub_latitude AND %2 <= add_latitude ORDER BY r.sigma, r.gamma,r.local_solar_time; ")
            .arg(SelectKey.sid).arg(Tir_Latitude[id]);
    sql += sqlstr;

    int sigma_id,gamma_id;


    ret = my_query.exec(sql);

    if (ret) {

        INPOL_DATA d;
        while (my_query.next()) {
            d.time = my_query.value(0).toDouble();
            d.temp = my_query.value(1).toDouble();
            d.diff = 0;
            d.tir_id = "";
            d.polygon_id = id;
            d.sigma = my_query.value(2).toString();
            d.gamma = my_query.value(3).toString();
            sigma_id = my_query.value(4).toInt();
            gamma_id = my_query.value(5).toInt();

            RoughnessList[id][sigma_id][gamma_id].append(d);
        }
    } else {
        retErrMsg = "SelectDB_Roughnessmodel_data :" + my_query.lastError().text();
    }


    my_query.clear();
    return ret;

}

 bool Worker::SelectDB_TIR_data(QSqlDatabase db,int id)
//bool readtxt::SelectDB_TIR_data(int id)
{
    bool ret = false;

    QSqlQuery my_query = QSqlQuery(db);
    QString sql = "SELECT t.id, t.latitude,t.local_solar_time_normal,t.brightness_temperature FROM t_tir AS t " ;
    QString sqlstr = QString("WHERE t.key_id = %1 AND t.polygon_id = %2 ORDER BY t.local_solar_time_normal ; " )
            .arg(SelectKey.sid).arg(QString::number(id));
    sql += sqlstr;


    ret = my_query.exec(sql);
    if (ret) {

        INPOL_DATA d;
       // int id;
       // QString tmp;
        while (my_query.next()) {
            d.tir_id = my_query.value(0).toString();

            if(d.tir_id == "")continue;

            //id = my_query.value(1).toInt();
            Tir_Latitude[id] = my_query.value(1).toString();
            d.time = my_query.value(2).toDouble();
            d.temp = my_query.value(3).toDouble();
            d.diff = 0;
            d.polygon_id = id;
            d.sigma = "";
            d.gamma = "";

            TirList[id].append(d);
        }
    } else {
        retErrMsg = "SelectDB_TIR_data :" + my_query.lastError().text();
    }
    my_query.clear();
    return ret;
}

 bool Worker::SelectDB_History(QSqlDatabase db ,const QString path)
//bool readtxt::SelectDB_History(QSqlDatabase db ,QString path)
{
    bool ret = false;
    QFileInfo fi(path);
    QString fn = fi.fileName();
    QDateTime dt = fi.lastModified();
    QString fts = dt.toString("yyyy-MM-dd HH:mm:ss");


    QSqlQuery my_query = QSqlQuery(db);
    QString sql = "SELECT count(h.id) FROM t_file_reg_history as h WHERE  ";
    QString sqlstr = QString( "h.key_id = %1 AND h.f_nm = '%2' AND h.f_ts = '%3' ;").arg(SelectKey.sid).arg(fn).arg(fts);
    sql += sqlstr;

    ret = my_query.exec(sql);
    if(ret)
    {
        my_query.next();
        QString val = my_query.value(0).toString();
        if(val == "0")
        {
            ret = true;
        }

    }else{

         retErrMsg = "SelectDB_History :" + my_query.lastError().text();

    }

    my_query.clear();
    return ret;

}

 bool readtxt::SelectDB_TIR_log_lat(QSqlDatabase db)
 {
     bool ret = false;

     QSqlQuery my_query = QSqlQuery(db);
     QString sql = QString("SELECT DISTINCT t.polygon_id ,t.longitude ,t.latitude fROM t_tir AS t WHERE t.key_id = %1 ORDER BY t.polygon_id ;" ).arg(SelectKey.sid);

     ret = my_query.exec(sql);
     if(ret)
     {

         int t_id;
         while (my_query.next()) {

             t_id = my_query.value(0).toUInt();
             ofd[t_id].longitude = QString::number(my_query.value(1).toDouble(),'f',6);
             ofd[t_id].latitude = QString::number(my_query.value(2).toDouble(),'f',6);

         }
     } else {
         retErrMsg = "SelectDB_TIR_log_lat :" + my_query.lastError().text();
     }
     my_query.clear();
     return ret;
 }


bool Worker::SelectDB_TIR_latitude(QSqlDatabase db)
//bool readtxt::SelectDB_TIR_latitude(QSqlDatabase db)
{
    bool ret = false;

    QSqlQuery my_query = QSqlQuery(db);
    QString sql = QString("SELECT DISTINCT t.polygon_id ,t.latitude fROM t_tir AS t WHERE t.key_id = %1 ORDER BY t.polygon_id ;" ).arg(SelectKey.sid);

    ret = my_query.exec(sql);
    if(ret)
    {

        int t_id;
        while (my_query.next()) {

            t_id = my_query.value(0).toUInt();
            Tir_Latitude[t_id] = my_query.value(1).toString();

        }
    } else {
        retErrMsg = "SelectDB_TIR_latitude :" + my_query.lastError().text();
    }
    my_query.clear();
    return ret;
}

/*
bool Worker::SelectDB_Result(QSqlDatabase db ,int id)
//bool readtxt::SelectDB_Result(int id ,QString s ,QString g)
{
    bool ret = false;

    QSqlQuery my_query = QSqlQuery(db);

    QString sql = "SELECT SQRT(SUM(a.p)) AS result , MAX(a.sigma) AS sigma, MAX(a.gamma) as gamma FROM ";
    sql += "(SELECT POW(t.brightness_temperature - i.brightness_temperature,2) AS p ,i.sigma,i.gamma,i.polygon_id FROM t_tir t  ";
    sql += "LEFT JOIN t_inpol i ON t.id = i.tir_id WHERE t.local_solar_time_normal >= 9.0 AND t.local_solar_time_normal <= 16.0 AND ";
    QString sqlstr = QString( "t.key_id = %1 AND t.polygon_id = %2) AS a GROUP BY a.sigma,a.gamma;")
            .arg(SelectKey.sid).arg(QString::number(id));
    sql += sqlstr;


    ret = my_query.exec(sql);
    if(ret)
    {
       while (my_query.next()) {
            BEST_FIT_DATA d;
            d.zansa = my_query.value(0).toDouble();
            d.id = id;
            d.sigma = my_query.value(1).toString();
            d.gamma = my_query.value(2).toString();

            //FitList.append(d);
       }

        ret = true;

    }else{

         retErrMsg = "SelectDB_Result :" + my_query.lastError().text();

    }

    my_query.clear();
    return ret;

}
*/

bool Worker::SelectDB_Best_Fit(QSqlDatabase db)
//bool readtxt::SelectDB_Best_Fit(QSqlDatabase db)
{
    bool ret = false;


    QSqlQuery my_query = QSqlQuery(db);
    QString sql = "";

    if(SelBtn == BTN_SHOW)
    {
        sql = QString("SELECT b.zansa,b.sigma,b.gamma FROM t_best_fit AS b WHERE b.key_id = %1 AND b.polygon_id = %2 ORDER BY b.zansa;" )
                .arg(SelectKey.sid).arg(SelectPolygonID);

    }
    else{
        sql = QString("SELECT b.zansa,b.sigma,b.gamma FROM tmp_best_fit AS b WHERE b.key_id = %1 AND b.polygon_id = %2 ORDER BY b.zansa;" )
                .arg(SelectKey.sid).arg(SelectPolygonID);

    }

    ret = my_query.exec(sql);
    if(ret)
    {

        QString s,g,line;
        double l;
        int c = 1;
        while (my_query.next()) {

            l = my_query.value(0).toDouble();
            s = my_query.value(1).toString();
            g = my_query.value(2).toString();
            line = QString("%1:%2  : sigma%3_gamma%4.txt").arg(c,2,10,QLatin1Char('0')).arg(l,12,'f',6).arg(s).arg(g);
            FitList.append(line);
            //ui->ShowtextEdit->append(line);
            c++;
        }

        if(c == 1)
        {
            if(Tir_Latitude[SelectPolygonID.toInt()] == LATITUDE_ERR)
            {
                FitList.append("------NO DATA-----Lost-");
            }else{
                FitList.append("------NO DATA-----Ignore-");
            }

            //ui->ShowtextEdit->append("------NO DATA------");
        }


    } else {

        //ui->ShowtextEdit->append("------NO DATA------");
        FitList.append("------NO DATA-----SQL-");
        retErrMsg = "SelectDB_Best_Fit :" + my_query.lastError().text();
    }
    my_query.clear();
    return ret;
}

bool readtxt::SelectDB_All_ID_Best_Fit(QSqlDatabase db)
{
    bool ret = false;
/*
    QString strDate = "_" + SelectKey.name;
    strDate += QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    strDate += ".txt";
    QString f_sigma = "sigma" + strDate;
    QString f_gamma = "gamma" + strDate;
*/
    QSqlQuery my_query = QSqlQuery(db);

    //QString sql = QString("SELECT x.k , x.p , x.z , b2.sigma ,b2.gamma , MAX(t1.longitude) , MAX(t1.latitude) FROM ");
    QString sql = QString("SELECT x.k , x.p , x.z , b2.sigma ,b2.gamma FROM ");
    sql += QString(" (SELECT MIN(b1.key_id) AS k,MIN(b1.polygon_id) AS p ,MIN(b1.zansa) AS z FROM t_best_fit b1  ");
    sql += QString(" GROUP BY  b1.key_id , b1.polygon_id )  AS x  ");
    sql += QString(" LEFT JOIN t_best_fit b2 ON b2.key_id = x.k AND b2.polygon_id = x.p AND b2.zansa = x.z  ");
   // sql += QString(" LEFT JOIN t_tir t1 ON t1.key_id = x.k AND t1.polygon_id = x.p ");
   // sql += QString(" WHERE k = %1 GROUP BY t1.key_id , t1.polygon_id ORDER BY k,p;").arg(SelectKey.sid);
    sql += QString(" WHERE k = %1 ORDER BY k,p;").arg(SelectKey.sid);


    //QString g_out = "";
    //QString s_out = "";
    //QString k,p,z,s,g,lo,la,tmp_g,tmp_s;
    QString k,p,z,s,g;
    int id;

    if (my_query.exec(sql)) {

 /*
        QString sfn = "..\\..\\DATA\\" + f_sigma;
        QString gfn = "..\\..\\DATA\\" + f_gamma;
        QFile sfile(sfn);
        QFile gfile(gfn);
        sfile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
        gfile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
        QTextStream sts( &sfile );
        QTextStream gts( &gfile );
        sts.setCodec( QTextCodec::codecForName( "UTF-8" ) );
        gts.setCodec( QTextCodec::codecForName( "UTF-8" ) );

 */


        while (my_query.next()) {


          k = my_query.value(0).toString();
          p = my_query.value(1).toString();
          id = p.toInt();
          z = my_query.value(2).toString();
          s = my_query.value(3).toString() + "00000";
          g = QString::number(my_query.value(4).toInt()) + ".000000" ;

          ofd[id].sigma = s;
          ofd[id].gamma = g;

  /*
          lo = QString::number(my_query.value(5).toDouble(),'f',6);
          la =QString::number(my_query.value(6).toDouble(),'f',6);

          tmp_s = QString("%1 %2 %3 %4").arg(p).arg(lo).arg(la).arg(s);
          tmp_g = QString("%1 %2 %3 %4").arg(p).arg(lo).arg(la).arg(g);

          sts << tmp_s << endl;
          gts << tmp_g << endl;

*/

        }

       // sfile.close();
       // gfile.close();

        ret = true;
    } else {
        retErrMsg = "SelectDB_All_ID_Best_Fit :" + my_query.lastError().text();
    }

    my_query.clear();
    return ret;
}

bool readtxt::insertDB_idx(QSqlDatabase db ,int i)
{

    bool ret = false;
    QSqlQuery my_query = QSqlQuery(db);
    QString sql = "INSERT into t_idx (idx) ";
    QString sqlstr = QString( "VALUE('%1');").arg(QString::number(i));
    sql += sqlstr;
    ret = my_query.exec(sql);

    if(!ret)
    {
      retErrMsg = db.lastError().text();
      DebugLog(retErrMsg);
      QMessageBox::warning(this, "insertDB_idx :", retErrMsg);
    }

    my_query.clear();
    return ret;

}


bool readtxt::insertDB_Key(QSqlDatabase db ,QString name)
{

    bool ret = false;
    QSqlQuery my_query = QSqlQuery(db);
    QString sql = "INSERT into t_key (name) ";
    QString sqlstr = QString( "VALUE('%1');").arg(name);
    sql += sqlstr;
    ret = my_query.exec(sql);

    if(!ret)
    {
      retErrMsg = db.lastError().text();
      DebugLog(retErrMsg);
      QMessageBox::warning(this, "insertDB_Key_Data :", retErrMsg);
    }

    my_query.clear();
    return ret;

}

bool Worker::insertDB_History(QSqlDatabase db ,const QString path ,const QString type)
//bool readtxt::insertDB_History(QSqlDatabase db ,QString path ,QString type)
{

    bool ret = false;
    QFileInfo fi(path);
    QString fn = fi.fileName();
    QDateTime dt = fi.lastModified();
    QString fts = dt.toString("yyyy-MM-dd HH:mm:ss");

    QSqlQuery my_query = QSqlQuery(db);
    QString sql = "INSERT into t_file_reg_history (key_id,f_ts,f_nm,f_tp) ";
    QString sqlstr = QString( "VALUE(%1,'%2','%3',%4);").arg(SelectKey.sid).arg(fts).arg(fn).arg(type);
    sql += sqlstr;
    ret = my_query.exec(sql);
    if(!ret)
    {
       retErrMsg = "insertDB_History : " + my_query.lastError().text();
        return ret;
    }

    my_query.clear();
    return ret;

}

bool Worker::insertDB_TIR(QSqlDatabase db ,const QString path)
//bool readtxt::insertDB_TIR(QSqlDatabase db ,QString path)
{
    bool ret = false;

    QFileInfo fi(path);
    QString fn = fi.fileName();
    QStringList datepieces = fn.split( "_" );
    QString ymd = datepieces.value(2);
    QString hms = datepieces.value(3);
    QFile file(path);
    QTextCodec* codec = QTextCodec::codecForName("UTF-8");

    //読込のみでオープンできたかチェック
    if(!file.open(QIODevice::ReadOnly))
    {
        retErrMsg = "insertDB_TIR : file read error \n" + fn ;
        return ret;
    }

    QSqlQuery my_query = QSqlQuery(db);
    QTextStream in(&file);
    in.setCodec( codec );        // Code. Qt5
    //in.setEncoding( QStringConverter::Utf8 );       // Fix. Qt6
    QString inStr = in.readAll();
    file.close();

    QList<QString>lines = inStr.split("\n");
    int max = lines.count() - 1;
    int c = 1;

    QString sql = "INSERT into t_tir (key_id,polygon_id,longitude,latitude,incidence_angle,phase_angle, ";
    sql += "local_solar_time,emission_angle,local_solar_time_normal,pixel_x,pixel_y,brightness_temperature, ";
    sql += "ymd,hms) VALUE ";

    QString sqlstr;

    foreach (QString line, lines) {

        QStringList pieces = line.split( " " );

        if(pieces.count() == TIR_ITEM_COUNT)
        {
            sqlstr = QString( "(%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,'%13','%14')").arg(SelectKey.sid)
                    .arg(pieces.value(0)).arg(pieces.value(1)).arg(pieces.value(2)).arg(pieces.value(3)).arg(pieces.value(4))
                    .arg(pieces.value(5)).arg(pieces.value(6)).arg(pieces.value(7)).arg(pieces.value(8)).arg(pieces.value(9))
                    .arg(pieces.value(10)).arg(ymd).arg(hms) ;
             sql += sqlstr;

             if(max != c)
             {
                 sql += ",";
             }else{
                 sql += ";";
             }
        }

        c++;
    }

    ret = my_query.exec(sql);
    if(!ret)
    {
      retErrMsg = "insertDB_TIR : " + my_query.lastError().text();
    }

    my_query.clear();
    return ret;

}

bool Worker::insertDB_RoughnessModel(QSqlDatabase db,const QString path)
//bool readtxt::insertDB_RoughnessModel(QSqlDatabase db ,QString path)
{
    bool ret = false;
    QFileInfo fi(path);
    QString fn = fi.fileName();
    QStringList datepieces = fn.split( "_" );
    QString sigma = datepieces.value(0);
    sigma = sigma.replace("sigma","");   //{"0010","0100","0200","0400","0600","0800"};

    QString temp = sigma;
    int sigma_id = temp.replace("0.","").toInt();

    QString gamma = datepieces.value(1);
    gamma = gamma.replace("gamma","").replace(".txt","").replace(".TXT","");

    int gamma_id = 0;

    if(gamma == "0100" )
    {
        gamma_id = 1;
    }
    else if(gamma == "0200" )
    {
        gamma_id = 2;
    }
    else if(gamma == "0400" )
    {
         gamma_id = 3;
    }
    else if(gamma == "0600" )
    {
         gamma_id = 4;
    }
    else if(gamma == "0800" )
    {
         gamma_id = 5;
    }


    QFile file(path);
    QTextCodec* codec = QTextCodec::codecForName("UTF-8");

    //読込のみでオープンできたかチェック
    if(!file.open(QIODevice::ReadOnly))
    {
      retErrMsg = "insertDB_RoughnessModel : file read error \n" + fn ;
      return ret;
    }

    QSqlQuery my_query = QSqlQuery(db);
    QTextStream in(&file);
    in.setCodec( codec );        // Code. Qt5
//in.setEncoding(QStringConverter::Utf8);     // Fix. Qt6
    QString inStr = in.readAll();
    file.close();

    QList<QString>lines = inStr.split("\n");
    int max = lines.count() - 1;
    int c = 1;

    QString sql = "INSERT into t_roughnessmodel (key_id,local_solar_time,latitude,sub_latitude ,add_latitude ,temperature,sigma,gamma,sigma_id,gamma_id ) VALUE ";

    QString sqlstr;
    QString buf;
    double  add,sub;

    foreach (QString line, lines) {

        QStringList pieces = line.split( " " );

        if(pieces.count() == ROUGHNESSMODEL_ITEM_COUNT)
        {
            buf = pieces.value(1);
            add = buf.toDouble() + 2.0;
            sub = buf.toDouble() - 2.0;

            sqlstr = sqlstr = QString( "(%1,%2,%3,%4,%5,%6,'%7','%8',%9,%10)" ).arg(SelectKey.sid)
                    .arg(pieces.value(0)).arg(pieces.value(1))
                    .arg(QString::number(sub,'f',6)).arg(QString::number(add,'f',6))
                    .arg(pieces.value(2)).arg(sigma).arg(gamma).arg(QString::number(sigma_id)).arg(QString::number(gamma_id)) ;
             sql += sqlstr;

             if(max != c)
             {
                 sql += ",";
             }else{
                 sql += ";";
             }
        }

        c++;
    }


    ret = my_query.exec(sql);
    if(!ret)
    {
      retErrMsg = "insertDB_RoughnessModel : " + my_query.lastError().text();
    }

    my_query.clear();
    return ret;

}

bool Worker::insertDB_INPOL(QSqlDatabase db)
//bool readtxt::insertDB_INPOL(QSqlDatabase db)
{

    bool ret = false;

     QSqlQuery my_query = QSqlQuery(db);

    int c = 1;
    int max = InPolList.count();
    if(max == 0) return ret;

    QString sql = "";

    if(SelBtn == BTN_CALC)
    {
        sql = "INSERT into t_inpol (key_id,local_solar_time_normal,brightness_temperature,diff,tir_id,polygon_id,sigma,gamma) VALUE ";
    }else{
        sql = "INSERT into tmp_inpol (key_id,local_solar_time_normal,brightness_temperature,diff,tir_id,polygon_id,sigma,gamma) VALUE ";
    }

    foreach (INPOL_DATA d, InPolList)
    {
        QString sqlstr = QString( " (%1,%2,%3,%4,%5,%6,'%7','%8') " ).arg(SelectKey.sid)
                .arg(QString::number(d.time,'f',6)).arg(QString::number(d.temp,'f',6)).arg(QString::number(d.diff,'f',6))
                .arg(d.tir_id).arg(QString::number(d.polygon_id)).arg(d.sigma).arg(d.gamma) ;
        sql += sqlstr;

         if(max != c)
         {
             sql += ",";
         }else{
             sql += ";";
         }

         c++;
    }

    InPolList.clear();
    ret = my_query.exec(sql);
    if(!ret)
    {
       retErrMsg = "insertDB_INPOL :" + my_query.lastError().text();

    }

    my_query.clear();
    return ret;

}

bool Worker::insertDB_Best_Fit(QSqlDatabase db ,int id)
//bool readtxt::insertDB_Best_Fit(QSqlDatabase db ,int id)
{

    bool ret = false;


    QSqlQuery my_query = QSqlQuery(db);
    QString sql = "";
    QString sqlstr = "";

     if(SelBtn == BTN_CALC)
     {
         sql = "INSERT into t_best_fit (key_id,polygon_id,sigma,gamma,zansa) ";
         sqlstr = QString("SELECT %1,MAX(a.polygon_id) AS polygon_id,MAX(a.sigma) AS sigma, MAX(a.gamma) as gamma ,SQRT(SUM(a.p)) AS zansa FROM ").arg(SelectKey.sid);
         sql += sqlstr;
         sql += "(SELECT POW(t.brightness_temperature - i.brightness_temperature,2) AS p ,i.sigma,i.gamma,i.polygon_id FROM t_tir t  ";
         sql += "LEFT JOIN t_inpol i ON t.id = i.tir_id WHERE t.local_solar_time_normal >= 9.0 AND t.local_solar_time_normal <= 16.0 AND ";

     }else{
         sql = "INSERT into tmp_best_fit (key_id,polygon_id,sigma,gamma,zansa) ";
         sqlstr = QString("SELECT %1,MAX(a.polygon_id) AS polygon_id,MAX(a.sigma) AS sigma, MAX(a.gamma) as gamma ,SQRT(SUM(a.p)) AS zansa FROM ").arg(SelectKey.sid);
         sql += sqlstr;
         sql += "(SELECT POW(t.brightness_temperature - i.brightness_temperature,2) AS p ,i.sigma,i.gamma,i.polygon_id FROM t_tir t  ";
         sql += "LEFT JOIN tmp_inpol i ON t.id = i.tir_id WHERE t.local_solar_time_normal >= 9.0 AND t.local_solar_time_normal <= 16.0 AND ";
     }


    sqlstr = QString( "t.key_id = %1 AND t.polygon_id = %2) AS a GROUP BY a.polygon_id,a.sigma,a.gamma;")
            .arg(SelectKey.sid).arg(QString::number(id));
    sql += sqlstr;

    ret = my_query.exec(sql);
    if(!ret)
    {
       DebugLog(sql);
      retErrMsg = "insertDB_Best_Fit :" + my_query.lastError().text();
    }

    my_query.clear();
    return ret;

}

bool Worker::updateDB_Key(QSqlDatabase db ,int flg)
//bool readtxt::updateDB_Key(QSqlDatabase db ,int flg)
{

    bool ret = false;
    QSqlQuery my_query = QSqlQuery(db);
    QString sql = QString("UPDATE t_key SET calc_flg = %1 WHERE id = %2 ;").arg(QString::number(flg)).arg(SelectKey.sid);

    ret = my_query.exec(sql);
    if(!ret)
    {
      retErrMsg = "updateDB_Key :" + my_query.lastError().text();
    }

    my_query.clear();
    return ret;

}

bool Worker::deleteDB_tmp_INPOL(QSqlDatabase db)
{

    bool ret = false;
    QSqlQuery my_query = QSqlQuery(db);
    QString sql = QString("DELETE FROM tmp_inpol ;");

    ret = my_query.exec(sql);
    if(!ret)
    {
      retErrMsg = "deleteDB_tmp_INPOL :" + my_query.lastError().text();
    }

    my_query.clear();
    return ret;

}

bool Worker::deleteDB_t_INPOL(QSqlDatabase db)
{

    bool ret = false;
    QSqlQuery my_query = QSqlQuery(db);
    QString sql = QString("DELETE FROM t_inpol ;");

    ret = my_query.exec(sql);
    if(!ret)
    {
      retErrMsg = "deleteDB_t_INPOL :" + my_query.lastError().text();
    }

    my_query.clear();
    return ret;

}


bool Worker::deleteDB_tmp_Best_Fit(QSqlDatabase db)
{

    bool ret = false;
    QSqlQuery my_query = QSqlQuery(db);
    QString sql = QString("DELETE FROM tmp_best_fit ;");

    ret = my_query.exec(sql);
    if(!ret)
    {
      retErrMsg = "deleteDB_tmp_Best_Fit :" + my_query.lastError().text();
    }

    my_query.clear();
    return ret;

}


/*
QString readtxt::GetMysqlPass()
{
    QString retStr = "";
    QFile file(MYSQL_PASS_FILE);
    QTextCodec* codec = QTextCodec::codecForName("UTF-8");

    //読込のみでオープンできたかチェック
    if(!file.open(QIODevice::ReadOnly))
    {
       return retStr;
    }

    QTextStream in(&file);
    in.setCodec( codec );
    retStr = in.readLine();
    file.close();

    return retStr;
}

*/

// bool readtxt::GetIniPass()
bool readtxt::GetIniPass(QString filePath)
{
    bool ret = false;
    // QFile file(INI_PASS_FILE);
    QFile file(filePath);
    QTextCodec* codec = QTextCodec::codecForName("UTF-8");

    //読込のみでオープンできたかチェック
    if(!file.open(QIODevice::ReadOnly))
    {
        IniInfo.MySqlPass = "demodemo";
        IniInfo.MySqlPort = 3306;
        IniInfo.LogPass = "..//..//Log//ErrLog.log";
        IniInfo.LoopStart = 1;
        IniInfo.LoopEnd = SIZE_DATA;
       return ret;
    }




    try {
        QTextStream in(&file);
        in.setCodec( codec );        // Code. Qt5
        //in.setEncoding( QStringConverter::Utf8 );       // Fix. Qt6
        QString retStr = in.readLine();
        QStringList list = retStr.split(",");

        // Code. Qt5 / onos:code
//        IniInfo.MySqlPass = list[0];
//        IniInfo.MySqlPort = QString(list[1]).toInt();
//        IniInfo.LogPass = list[2];
//        IniInfo.LoopStart = QString(list[3]).toInt();
//        IniInfo.LoopEnd = QString(list[4]).toInt();

        // Fix. Qt6
        int at = 0;
        QList<QString>::Iterator ite = list.begin();
        while (ite != list.end())
        {
            switch (at)
            {
            case 0:
                IniInfo.MySqlPass = *ite;
                break;
            case 1:
                IniInfo.MySqlPort = (*ite).toInt();
                break;
            case 2:
                IniInfo.LogPass = *ite;
                break;
            case 3:
                IniInfo.LoopStart = (*ite).toInt();
                break;
            case 4:
                IniInfo.LoopEnd = (*ite).toInt();
                break;
            default:
                break;
            }
            at++;
            ite++;
        }

        if((IniInfo.LoopEnd < 1 ) || (IniInfo.LoopEnd > SIZE_DATA) )
        {
            IniInfo.LoopEnd = SIZE_DATA;
        }

        // Fix. Qt 6
        QString label3 = "PolygonID (" + QString::number(IniInfo.LoopStart) +  "- " + QString::number(IniInfo.LoopEnd) + ")";
        ui->label_3->setText(label3);
    }
    catch(...)
    {
        IniInfo.MySqlPass = "demodemo";
        IniInfo.MySqlPort = 3306;
        IniInfo.LogPass = "..//..//Log//ErrLog.log";
        IniInfo.LoopStart = 1;
        IniInfo.LoopEnd = SIZE_DATA;
    };


    file.close();

    return true;
}

void DebugLog(QString msg)
//void readtxt::DebugLog(QString msg)
{
    qDebug() << "DEBUG START";
    qDebug() << QString::number(msg.length());
    msg += "\n";
    qDebug() << msg;


    QString strDate = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    strDate += " : ";
    strDate += msg;

    QString fileName = IniInfo.LogPass;
    QFile logfile(fileName);
    logfile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    QTextStream ts( &logfile );
    ts.setCodec( QTextCodec::codecForName( "UTF-8" ) );      // Code. Qt5
    //ts.setEncoding( QStringConverter::Utf8 );       // Fix. Qt6
    ts << strDate << endl;       // Code. Qt5
    //ts << strDate << Qt::endl;      // Fix. Qt6
    logfile.close();

    qDebug() << "DEBUG END";

}

void polint(double xa[], double ya[], int n, double x, double *y, double *dy)
//void readtxt::polint(double xa[], double ya[], int n, double x, double *y, double *dy)
{
    int i,m,ns=1;
    double den,dif,dift,ho,hp,w;
    double *c,*d;

    dif=fabs(x-xa[1]);
    //c=dvector(1,n);
    //d=dvector(1,n);
    c = new double[3];
    d = new double[3];
    for (i=1;i<=n;i++) {
        if ( (dift=fabs(x-xa[i])) < dif) {
            ns=i;
            dif=dift;
        }
        c[i]=ya[i];
        d[i]=ya[i];
    }
    *y=ya[ns--];
    for (m=1;m<n;m++) {
        for (i=1;i<=n-m;i++) {
            ho=xa[i]-x;
            hp=xa[i+m]-x;
            w=c[i+1]-d[i];
            ////if ( (den=ho-hp) == 0.0) nrerror("Error in routine polint");
            den=ho-hp;
            den=w/den;
            d[i]=hp*den;
            c[i]=ho*den;
        }
        *y += (*dy=(2*ns < (n-m) ? c[ns+1] : d[ns--]));
    }

    delete[] c;
    delete[] d;
    //free_dvector(d,1,n);
    //free_dvector(c,1,n);
}

/*
double *readtxt::dvector(long nl, long nh)
/* allocate a double vector with subscript range v[nl..nh]
{
    double *v;

    ////v=(double *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(double)));
    v= new double(nh-nl+1+NR_END);
    ////if (!v) nrerror("allocation failure in dvector()");
    return v-nl+NR_END;
}

void readtxt::free_dvector(double *v, long nl, long nh)
/* free a double vector allocated with dvector()
{
    ////free((FREE_ARG) (v+nl-NR_END));
    delete (v);
}
*/

double readtxt::ceil( double dSrc, int iLen )
{
    double	dRet;

    dRet = dSrc * pow(10.0, iLen);
    dRet = (double)(int)(dRet + 0.9);

    return dRet * pow(10.0, -iLen);
}

double readtxt::floor( double dSrc, int iLen )
{
    double dRet;

    dRet = dSrc * pow(10.0, iLen);
    dRet = (double)(int)(dRet);

    return dRet * pow(10.0, -iLen);
}

double readtxt::round( double dSrc, int iLen )
{
    double	dRet;

    dRet = dSrc * pow(10.0, iLen);
    dRet = (double)(int)(dRet + 0.5);

    return dRet * pow(10.0, -iLen);
}

/*
 *
 */
void readtxt::ProcessReadIni()
{
    QStringList fileFilterList;
    fileFilterList += "INI (*.ini)";

    QFileDialog fileDialog(this);
    fileDialog.setNameFilters(fileFilterList);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);

    QStringList filePaths;
    if(fileDialog.exec())
    {
        filePaths = fileDialog.selectedFiles();
        QString iniFile = filePaths.at(0);
        bool ret = GetIniPass(iniFile);
        bool check = ret;
    }
}



