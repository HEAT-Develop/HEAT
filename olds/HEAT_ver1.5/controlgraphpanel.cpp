//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//

#include "controlgraphpanel.h"
#include "ui_controlgraphpanel.h"
#include <QDebug>
#include <QFileDialog>
#include <calibrationgraph.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <Keyword.h>
#include "dataset.h"

#define fitscount 77
//TIR_DATA_original
//QString rawimagedirectorypath1="/Applications/TIR_DATA_original";
QString rawimagedirectorypath1="/Volumes/BufflWinMac/HEAT/HEAT_Path/HEAT_Path/HEATDB/TIR_DATA_original/";
using namespace CCfits;
using namespace std;

ControlGraphPanel::ControlGraphPanel(QWidget *parent)
    : QDialog(parent), ui(new Ui::ControlGraphPanel) {
    ui->setupUi(this);
    this->setWindowTitle("Conversion panel");
    this->move(0, 0);

    QObject::connect(ui->anotherView, SIGNAL(valueChangedY(QString)), this,
                     SLOT(setY(QString)));
    QObject::connect(ui->anotherView, SIGNAL(valueChangedX(QString)), this,
                     SLOT(setX(QString)));

    QDir tmp, initialFileDirectory;

    QString appPath;

    appPath = QCoreApplication::applicationDirPath();
    initialFileDirectory.cd(appPath);

    QFile iniFile(initialFileDirectory.absolutePath() + "/initial.txt");

    iniFile.open(QIODevice::ReadOnly);

    QTextStream load(&iniFile);

    load >> databPath;

    iniFile.close();

    connectDB();
}

ControlGraphPanel::~ControlGraphPanel() { delete ui; }

void ControlGraphPanel::readLUTfile(QString name)
{    auto_ptr<FITS> pInfile(0);
     pInfile.reset(new FITS(name.toStdString().c_str(), Read, true));
      int hduindex=0;
       String hduName;
        int hduVersion;
         int i,j,counter=0;

          CCfits::ExtHDU::readHduName(pInfile->fitsPointer(),hduindex, hduName, hduVersion);
           std::auto_ptr<FITS> pInfile1(new FITS(name.toStdString(),Read,hduName,false));
            cout<<pInfile1->currentExtensionName()<<endl;
             ExtHDU& EH1 = pInfile1->extension(pInfile1->currentExtensionName(),1);
              EH1.read(slope);
              std::valarray<double> lll;
               w = EH1.axis(0);
                h = EH1.axis(1);


                 counter = 0;
                  hduindex = 1;
                   CCfits::ExtHDU::readHduName(pInfile->fitsPointer(),hduindex, hduName, hduVersion);
                    std::auto_ptr<FITS> pInfile2(new FITS(name.toStdString(),Read,hduName,false));
                     cout<<pInfile2->currentExtensionName()<<endl;
                      ExtHDU& EH2 = pInfile2->extension(pInfile2->currentExtensionName(),1);
                       EH2.read(intercept);
                        w = EH2.axis(0);
                         h = EH2.axis(1);

                        // fitsファイルのキー判別
                            // ヘッダーにMADE_BYというキーがあるかどうかを判別し、存在すれば読み込む　by Ryuji
                        fitskeys="non";
                        CCfits::PHDU& image=pInfile->pHDU();
                        try{
                        image.readKey("MADE_BY",fitskeys);
                        }
                        catch(CCfits::HDU::NoSuchKeyword){
                            return;
                        }


                //fits作成時に確認で作りました。消して大丈夫です。　by Ryuji
                /*
                std::string filename="/Applications/konryuHEAT.fit";
                long naxis=2;
                int xsize=328;
                int ysize=248;
                long naxes[2] = {xsize,ysize};
                int nelements=xsize*ysize;
                vector<long> extAx;
                extAx.push_back(xsize) ;
                extAx.push_back(ysize) ;
                string names = "doskoidoskoi";
                std::auto_ptr<FITS> pFits;
                    try{
                        pFits.reset(new FITS(filename,DOUBLE_IMG,naxis,naxes));
                    }
                    catch (FITS::CantCreate){
                    return;
                    }
                static std::valarray<double> konkong(nelements);
                static std::valarray<double> konkonh(nelements);
                for(int i=0;i<ysize;i++){
                    for(int j=0;j<xsize;j++){
                        konkong[i*xsize+j]=slope[i*xsize+j];
                        konkonh[i*xsize+j]=intercept[i*xsize+j];
                    }
                }
                pFits->pHDU().addKey("MADE_FROM","HEAT","june28");
                pFits->pHDU().write(1,nelements,konkong);
                ExtHDU* addh = pFits->addImage(names,DOUBLE_IMG,extAx);
                addh->write(1,nelements,konkonh);
                */




}

void ControlGraphPanel::popControlGraphPanel() { this->show(); }

void ControlGraphPanel::on_loadFileButton_clicked() {

    fileName = QFileDialog::getOpenFileName(
                this, tr("Open Image"), "/Applications/HEAT_DATA",
                tr("Image Files (*.png *.jpg *.bmp *.img *.fit *.fits *.fts *.txt "
                   "*.dat)"));

    if (fileName != NULL) {
        ui->fileNameBrowser->clear();
        ui->fileNameBrowser->setText(fileName.section('/', -1, -1));
        ui->anotherView->loadFileName(fileName);

        fstream ifs;
        ifs.open(&fileName.toStdString()[0], ios::in | ios::binary);
        QFileInfo fileinfo;
        fileinfo.setFile(fileName);
        QString ext = fileinfo.suffix();
        ext = ext.toLower();

        if (ext == "fit" || ext == "fits" || ext == "fts") {
            auto_ptr<FITS> pInfile(0);
            try {
                pInfile.reset(new FITS(fileName.toStdString().c_str(), Read, true));
                PHDU &fitsImage = pInfile->pHDU();
                fitsImage.readAllKeys();
                String keywordstring[fitscount];
                double keyworddouble[fitscount];
                int keywordint[fitscount];
                String keywordname[fitscount];
                bool keywordbool[fitscount];
                int keywordcount;
                int i = 0;
                keywordcount = 0;
                keywordname[i] = "SIMPLE";
                i++;
                keywordname[i] = "BITPIX";
                i++;
                keywordname[i] = "NAXIS";
                i++;
                keywordname[i] = "NAXIS1";
                i++;
                keywordname[i] = "NAXIS2";
                i++;
                keywordname[i] = "EXTEND";
                i++;
                keywordname[i] = "ORIGIN";
                i++;
                keywordname[i] = "DATE";
                i++;
                keywordname[i] = "DATE-BEG";
                i++;
                keywordname[i] = "DATE-OBS";
                i++;
                keywordname[i] = "DATE-END";
                i++;
                keywordname[i] = "TELESCOP";
                i++;
                keywordname[i] = "INSTRUME";
                i++;
                keywordname[i] = "OBJECT";
                i++;
                keywordname[i] = "BUNIT";
                i++;
                keywordname[i] = "XPOSURE";
                i++;
                keywordname[i] = "IFOV";
                i++;
                keywordname[i] = "FILTER";
                i++;
                keywordname[i] = "OPRGNAME";
                i++;
                keywordname[i] = "OPRGNO";
                i++;
                keywordname[i] = "ROI_LLX";
                i++;
                keywordname[i] = "ROI_LLY";
                i++;
                keywordname[i] = "ROI_URX";
                i++;
                keywordname[i] = "ROI_URY";
                i++;
                keywordname[i] = "DATAMAX";
                i++;
                keywordname[i] = "DATAMIN";
                i++;
                keywordname[i] = "MEAN";
                i++;
                keywordname[i] = "STDEV";
                i++;
                keywordname[i] = "MISS_VAL";
                i++;
                keywordname[i] = "MISS_NUM";
                i++;
                keywordname[i] = "DEAD_VAL";
                i++;
                keywordname[i] = "DEAD_NUM";
                i++;
                keywordname[i] = "SATU_VAL";
                i++;
                keywordname[i] = "SATU_NUM";
                i++;
                keywordname[i] = "IMGCMPRV";
                i++;
                keywordname[i] = "IMGCMPAL";
                i++;
                keywordname[i] = "IMGCMPPR";
                i++;
                keywordname[i] = "IMG_ERR";
                i++;
                keywordname[i] = "IMGSEQC";
                i++;
                keywordname[i] = "IMGACCM";
                i++;
                keywordname[i] = "BITDEPTH";
                i++;
                keywordname[i] = "PLT_POW";
                i++;
                keywordname[i] = "PLT_STAT";
                i++;
                keywordname[i] = "BOL_STAT";
                i++;
                keywordname[i] = "BOL_TRGT";
                i++;
                keywordname[i] = "BOL_RANG";
                i++;
                keywordname[i] = "BOL_TEMP";
                i++;
                keywordname[i] = "PKG_TEMP";
                i++;
                keywordname[i] = "CAS_TEMP";
                i++;
                keywordname[i] = "SHT_TEMP";
                i++;
                keywordname[i] = "LEN_TEMP";
                i++;
                keywordname[i] = "BGR_VOL";
                i++;
                keywordname[i] = "VB1_VOL";
                i++;
                keywordname[i] = "ADOFSVOL";
                i++;
                keywordname[i] = "HCE_TEMP";
                i++;
                keywordname[i] = "PNL_TEMP";
                i++;
                keywordname[i] = "AE_TEMP";
                i++;
                keywordname[i] = "S_DISTHT";
                i++;
                keywordname[i] = "S_DISTHE";
                i++;
                keywordname[i] = "S_DISTHS";
                i++;
                keywordname[i] = "S_DISTTS";
                i++;
                keywordname[i] = "S_TGRADI";
                i++;
                keywordname[i] = "S_APPDIA";
                i++;
                keywordname[i] = "S_SOLLAT";
                i++;
                keywordname[i] = "S_SOLLON";
                i++;
                keywordname[i] = "S_SSCLAT";
                i++;
                keywordname[i] = "S_SSCLON";
                i++;
                keywordname[i] = "S_SSCLST";
                i++;
                keywordname[i] = "S_SSCPX";
                i++;
                keywordname[i] = "S_SSCPY";
                i++;
                keywordname[i] = "S_SCXSAN";
                i++;
                keywordname[i] = "S_SCYSAN";
                i++;
                keywordname[i] = "S_SCZSAN";
                i++;
                keywordname[i] = "NAIFNAME";
                i++;
                keywordname[i] = "NAIFID";
                i++;
                keywordname[i] = "VESION";
                i++;
                keywordcount = i;

                QString fitsinfoarray[fitscount];
                fitsinfoarray[0] = "File Name: " + QFileInfo(fileName).fileName();

                i = 0;
                for (i = 0; i <= keywordcount; i++) {
                    try {
                        pInfile->pHDU().readKey<bool>(keywordname[i], keywordbool[i]);
                        if (typeid(keywordbool[i]) == typeid(bool)) {
                            if (keywordbool[i] == 1) {
                                fitsinfoarray[1 + i] =
                                        QString::fromStdString(keywordname[i]) + ":  T";
                            } else
                                fitsinfoarray[1 + i] =
                                        QString::fromStdString(keywordname[i]) + ":  F";
                        }
                        continue;
                    } catch (...) {
                    };

                    try {
                        pInfile->pHDU().readKey<double>(keywordname[i], keyworddouble[i]);
                        if (typeid(keyworddouble[i]) == typeid(double)) {
                            fitsinfoarray[1 + i] = QString::fromStdString(keywordname[i]) +
                                    ": " + QString::number(keyworddouble[i]);
                        }
                        continue;
                    } catch (...) {
                    };

                    try {
                        pInfile->pHDU().readKey<int>(keywordname[i], keywordint[i]);
                        if (typeid(keywordint[i]) == typeid(int)) {
                            fitsinfoarray[1 + i] = QString::fromStdString(keywordname[i]) +
                                    ": " + QString::number(keywordint[i]);
                        }
                        continue;
                    } catch (...) {
                    };

                    try {
                        pInfile->pHDU().readKey<String>(keywordname[i], keywordstring[i]);
                        if (typeid(keywordstring[i]) == typeid(String)) {
                            fitsinfoarray[1 + i] = QString::fromStdString(keywordname[i]) +
                                    ": " +
                                    QString::fromStdString(keywordstring[i]);
                        }
                        continue;
                    } catch (...) {
                    };
                }
                this->ui->anotherView->resize(this->ui->anotherView->getWidth()+110,this->ui->anotherView->getHeight()+21);

                emit FITSinfoSignal1(fitsinfoarray);
            } catch (FITS::CantCreate) {
                cout << "Can't open fits image file" << endl;
                return;
            }
        }
    }
}

void ControlGraphPanel::on_substructButton_clicked() {

    subFileName = QFileDialog::getOpenFileName(
                this, tr("Open Substruct Image"), "TIR_DATA_original",
                tr("Image Files (*.png *.jpg *.bmp *.img *.fit *.fits *.fts)"));

    if (fileName != NULL) {
        ui->fileNameBrowser->append(subFileName.section('/', -1, -1));
        if (fileName.section('.', -2, -2) == "open") {
            ui->anotherView->subtractImage(fileName, subFileName);
        } else {
            ui->anotherView->subtractImage(subFileName, fileName);
        }
    }
}

void ControlGraphPanel::setX(QString x) {
    ui->xPositionWidget->setText(x);
    emit changeX(x);
}

void ControlGraphPanel::setY(QString y) {
    ui->yPositionWidget->setText(y);
    emit changeY(y);
}

void ControlGraphPanel::on_calibrationtoRadianceButton_clicked(){
    int xmin,xmax,ymin,ymax;
    int dragxstart,dragxend,dragystart,dragyend;
    int dragxend2,dragyend2;

    xmin=ui->MinxlineEdit->text().toInt();
    xmax=ui->MaxxlineEdit->text().toInt();
    ymin=ui->MinylineEdit->text().toInt();
    ymax=ui->MaxylineEdit->text().toInt();

    dragxstart=ui->anotherView->startPos_.x()-26;
    dragxend=ui->anotherView->endPos.x()-26;
    dragystart=ui->anotherView->startPos_.y()-16;
    dragyend=ui->anotherView->endPos.y()-16;
    dragxend2=ui->anotherView->endPos.x()-26;
    dragyend2=ui->anotherView->endPos.y()-16;

    if(dragxstart>dragxend){
        int tmp=0;
        tmp=dragxend;
        dragxend=dragxstart;
        dragxstart=tmp;
    }

    if(dragystart>dragyend){
        int tmp=0;
        tmp=dragyend;
        dragyend=dragystart;
        dragystart=tmp;
    }

    if(xmin==0 && dragxstart!=-26){
        xmin=dragxstart;
    }
    if(xmax==327 && dragxend!=-26){
        xmax=dragxend;
    }
    if(ymin==0 && dragystart!=-16){
        ymin=dragystart;
    }
    if(ymax==247 && dragyend!=-16){
        ymax=dragyend;
    }

    if(dragxend2==-26 && dragyend2==-16){
        xmin=0;
        xmax=327;
        ymin=0;
        ymax=247;
    }


    if(xmin<0 ||xmax>327 ||ymin<0 || ymax>247 || ymin>ymax || xmin>xmax){
        return;
    }

    else{
        DarkImage();

        QString s;
        QString initialFileDirectory = QFileDialog::getExistingDirectory(this,tr("Select Register Formula Folder"),"/Applications/HEATcalibration");
        if(initialFileDirectory == ""){
            return;
        }

        QString fitdirectory= QFileDialog::getExistingDirectory(this, tr("Select the directory to save the image"),"/Applications/HEAT_DATA");
        QProgressDialog p;
        p.setLabelText("Calibration Process");
        p.setRange(0,0);

        ui->anotherView->initializeCalibrateImage();
        int count=0;
        for(int j=xmin; j<xmax+1; j++){
            for(int m=ymin; m<ymax+1; m++){


                s.clear();
                QFile file(initialFileDirectory+"/" + QString::number(j) + "_" + QString::number(m) + ".txt");
                if(file.exists()){
                    file.open(QIODevice::ReadOnly);
                    QTextStream load(&file);

                    load >> s;

                    file.close();

                }else{
                    s = "0,0,0,0,0,0,0";
                }

                ui->anotherView->calibrateImagetoRadianceforBlackbodyAllPixel(s,j,m);
                p.setLabelText("Calibration Process \n" + QString::number(count) + " / " + QString::number(((xmax+1)-xmin)*((ymax+1)-ymin)) +"\n" + "x: "+QString::number(j)+"  y: "+QString::number(m));
                p.show();
                count++;
                QCoreApplication::processEvents();
                if(p.wasCanceled()){
                    ui->anotherView->updateImage(1,initialFileDirectory,fitdirectory);
                    return;
                }
            }
        }


        ui->anotherView->updateImage(1,initialFileDirectory,fitdirectory);

    }
}

void ControlGraphPanel::on_BlackbodycalibrationAllPixelButton_repeat_clicked(){

    ui->anotherView->initializeFITSarray();
    int xmin,xmax,ymin,ymax;
    int dragxstart,dragxend,dragystart,dragyend;
    int dragxend2,dragyend2;

    xmin=ui->MinxlineEdit->text().toInt();
    xmax=ui->MaxxlineEdit->text().toInt();
    ymin=ui->MinylineEdit->text().toInt();
    ymax=ui->MaxylineEdit->text().toInt();

    dragxstart=ui->anotherView->startPos_.x()-26;
    dragxend=ui->anotherView->endPos.x()-26;
    dragystart=ui->anotherView->startPos_.y()-16;
    dragyend=ui->anotherView->endPos.y()-16;
    dragxend2=ui->anotherView->endPos.x()-26;
    dragyend2=ui->anotherView->endPos.y()-16;

    if(dragxstart>dragxend){
        int tmp=0;
        tmp=dragxend;
        dragxend=dragxstart;
        dragxstart=tmp;
    }

    if(dragystart>dragyend){
        int tmp=0;
        tmp=dragyend;
        dragyend=dragystart;
        dragystart=tmp;
    }

    if(xmin==0 && dragxstart!=-26){
        xmin=dragxstart;
    }
    if(xmax==327 && dragxend!=-26){
        xmax=dragxend;
    }
    if(ymin==0 && dragystart!=-16){
        ymin=dragystart;
    }
    if(ymax==247 && dragyend!=-16){
        ymax=dragyend;
    }

    if(dragxend2==-26 && dragyend2==-16){
        xmin=0;
        xmax=327;
        ymin=0;
        ymax=247;
    }
    int removecounter=0;

    if(xmin<0 ||xmax>327 ||ymin<0 || ymax>247 || ymin>ymax || xmin>xmax){
        return;
    }

    else{
        DarkImage();

        QString DirName = QFileDialog::getExistingDirectory(this, tr("Open Image"), "/Applications/HEAT_DATA");
        if(DirName == ""){
            return;
        }
        QDir q_dir(DirName);
        QStringList filelist;
        QStringList filelist4;
        QStringList filelist5;

        QString initialFileDirectory = QFileDialog::getExistingDirectory(this,tr("Select Register Formula Folder"),"/Applications/HEATcalibration");
        if(initialFileDirectory == ""){
            return;
        }

        if(q_dir.exists()){
            filelist = q_dir.entryList();
        }
        for ( int z=2; z < filelist.size(); z++ ){
            qDebug()<<"filelist.size()";
            qDebug()<<filelist.size();

            fileName=DirName+"/"+filelist.at(z);
            fstream ifs;
            ifs.open(&fileName.toStdString()[0],ios::in | ios::binary);
            QFileInfo fileinfo;
            fileinfo.setFile(fileName);
            QString ext = fileinfo.suffix();
            ext=ext.toLower();

            if(ext=="img" || ext=="inf"|| ext=="txt"|| ext=="png"|| ext=="plt"|| ext=="map"|| ext=="ope"|| ext=="fits"){ //|| fileName.contains("_l1.fit",Qt::CaseInsensitive)==0){
                //    cout<<"DELETE: ";
                //  cout<<fileName.toStdString()<<endl;
                //QFile::remove(fileName);
                //removecounter++;
            }
            else{

                filelist4.append(fileName);
            }
        }
        qDebug()<<"filelist4.size()";
        qDebug()<<filelist4.size();


        for ( int z=0; z < filelist4.size(); z++ ){
            fileName=filelist4.at(z);

            fstream ifs;
            ifs.open(&fileName.toStdString()[0],ios::in | ios::binary);
            QFileInfo fileinfo;
            fileinfo.setFile(fileName);
            QString ext = fileinfo.suffix();
            ext=ext.toLower();

            if (ext == "fit" )
            {
                valarray<long> contents;
                auto_ptr<FITS> pInfile(0);
                try{
                    pInfile.reset(new FITS(fileName.toStdString().c_str(), Read, true));
                    PHDU& fitsImage=pInfile->pHDU();
                    fitsImage.read(contents);
                    fitsImage.readAllKeys();

                    double DNtmp1=0;

                    int counter=0;
                    for(int i=0; i<ui->anotherView->Height_i; i++){
                        for(int j=0; j<ui->anotherView->Width_i; j++){
                            DNtmp1=contents[counter];
                            fitsave+=DNtmp1;
                            counter++;
                        }
                    }
                    fitsave=fitsave/(ui->anotherView->Height_i*ui->anotherView->Width_i);
                }

                catch(...){};

            }
            if(fitsave<800)
            {
                filelist5.append(fileName);
            }
        }


        cout<<"----------------------------------------------------------------------------------------------------"<<endl;

        qDebug()<<"filelist5.size()";
        qDebug()<<filelist5.size();
        qDebug()<<filelist5;

        for ( int z=0; z < filelist5.size(); z++ ){
            fileName=filelist5.at(z);

            cout<<"Now converting:  ";
            cout<<fileName.toStdString()<<endl;
            cout<<"Least file:  ";
            cout<<filelist5.size()-z<<endl;

            if(fileName!= NULL){
                ui->fileNameBrowser->clear();
                ui->fileNameBrowser->setText(fileName.section('/',-1,-1));
                ui->anotherView->loadFileName(fileName);

                fstream ifs;
                ifs.open(&fileName.toStdString()[0],ios::in | ios::binary);
                QFileInfo fileinfo;
                fileinfo.setFile(fileName);
                QString ext = fileinfo.suffix();
                ext=ext.toLower();


                if (ext == "fit" || ext=="fits" || ext=="fts")
                {
                    auto_ptr<FITS> pInfile(0);
                    try{
                        pInfile.reset(new FITS(fileName.toStdString().c_str(), Read, true));
                        PHDU& fitsImage=pInfile->pHDU();
                        fitsImage.readAllKeys();

                        String keywordstring[fitscount];
                        double keyworddouble[fitscount];
                        int keywordint[fitscount];
                        String keywordname[fitscount];
                        bool keywordbool[fitscount];
                        int keywordcount;
                        int i=0;
                        keywordcount=0;
                        keywordname[i]="SIMPLE";i++;
                        keywordname[i]="BITPIX";i++;
                        keywordname[i]="NAXIS";i++;
                        keywordname[i]="NAXIS1";i++;
                        keywordname[i]="NAXIS2";i++;
                        keywordname[i]="EXTEND";i++;
                        keywordname[i]="ORIGIN";i++;
                        keywordname[i]="DATE";i++;
                        keywordname[i]="DATE-BEG";i++;
                        keywordname[i]="DATE-OBS";i++;
                        keywordname[i]="DATE-END";i++;
                        keywordname[i]="TELESCOP";i++;
                        keywordname[i]="INSTRUME";i++;
                        keywordname[i]="OBJECT";i++;
                        keywordname[i]="BUNIT";i++;
                        keywordname[i]="XPOSURE";i++;
                        keywordname[i]="IFOV";i++;
                        keywordname[i]="FILTER";i++;
                        keywordname[i]="OPRGNAME";i++;
                        keywordname[i]="OPRGNO";i++;
                        keywordname[i]="ROI_LLX";i++;
                        keywordname[i]="ROI_LLY";i++;
                        keywordname[i]="ROI_URX";i++;
                        keywordname[i]="ROI_URY";i++;
                        keywordname[i]="DATAMAX";i++;
                        keywordname[i]="DATAMIN";i++;
                        keywordname[i]="MEAN";i++;
                        keywordname[i]="STDEV";i++;
                        keywordname[i]="MISS_VAL";i++;
                        keywordname[i]="MISS_NUM";i++;
                        keywordname[i]="DEAD_VAL";i++;
                        keywordname[i]="DEAD_NUM";i++;
                        keywordname[i]="SATU_VAL";i++;
                        keywordname[i]="SATU_NUM";i++;
                        keywordname[i]="IMGCMPRV";i++;
                        keywordname[i]="IMGCMPAL";i++;
                        keywordname[i]="IMGCMPPR";i++;
                        keywordname[i]="IMG_ERR";i++;
                        keywordname[i]="IMGSEQC";i++;
                        keywordname[i]="IMGACCM";i++;
                        keywordname[i]="BITDEPTH";i++;
                        keywordname[i]="PLT_POW";i++;
                        keywordname[i]="PLT_STAT";i++;
                        keywordname[i]="BOL_STAT";i++;
                        keywordname[i]="BOL_TRGT";i++;
                        keywordname[i]="BOL_RANG";i++;
                        keywordname[i]="BOL_TEMP";i++;
                        keywordname[i]="PKG_TEMP";i++;
                        keywordname[i]="CAS_TEMP";i++;
                        keywordname[i]="SHT_TEMP";i++;
                        keywordname[i]="LEN_TEMP";i++;
                        keywordname[i]="BGR_VOL";i++;
                        keywordname[i]="VB1_VOL";i++;
                        keywordname[i]="ADOFSVOL";i++;
                        keywordname[i]="HCE_TEMP";i++;
                        keywordname[i]="PNL_TEMP";i++;
                        keywordname[i]="AE_TEMP";i++;
                        keywordname[i]="S_DISTHT";i++;
                        keywordname[i]="S_DISTHE";i++;
                        keywordname[i]="S_DISTHS";i++;
                        keywordname[i]="S_DISTTS";i++;
                        keywordname[i]="S_TGRADI";i++;
                        keywordname[i]="S_APPDIA";i++;
                        keywordname[i]="S_SOLLAT";i++;
                        keywordname[i]="S_SOLLON";i++;
                        keywordname[i]="S_SSCLAT";i++;
                        keywordname[i]="S_SSCLON";i++;
                        keywordname[i]="S_SSCLST";i++;
                        keywordname[i]="S_SSCPX";i++;
                        keywordname[i]="S_SSCPY";i++;
                        keywordname[i]="S_SCXSAN";i++;
                        keywordname[i]="S_SCYSAN";i++;
                        keywordname[i]="S_SCZSAN";i++;
                        keywordname[i]="NAIFNAME";i++;
                        keywordname[i]="NAIFID";i++;
                        keywordname[i]="VESION";i++;
                        keywordcount=i;


                        QString fitsinfoarray[fitscount];
                        fitsinfoarray[0]="File Name: "+   QFileInfo(fileName).fileName();//ファイル名

                        i=0;
                        for(i=0;i<=keywordcount;i++){
                            try{
                                pInfile->pHDU().readKey<bool>(keywordname[i], keywordbool[i]);
                                if(typeid(keywordbool[i])==typeid(bool))
                                {
                                    if(keywordbool[i]==1){
                                        fitsinfoarray[1+i]=QString::fromStdString(keywordname[i])+ ":  T";
                                    }
                                    else fitsinfoarray[1+i]=QString::fromStdString(keywordname[i])+ ":  F";
                                }continue;
                            }
                            catch(...){};

                            try{
                                pInfile->pHDU().readKey<double>(keywordname[i], keyworddouble[i]);
                                if(typeid(keyworddouble[i])==typeid(double))
                                {
                                    fitsinfoarray[1+i]=QString::fromStdString(keywordname[i])+ ": " + QString::number(keyworddouble[i]);
                                }continue;
                            }
                            catch(...){};

                            try{
                                pInfile->pHDU().readKey<int>(keywordname[i], keywordint[i]);
                                if(typeid(keywordint[i])==typeid(int))
                                {
                                    fitsinfoarray[1+i]=QString::fromStdString(keywordname[i])+ ": " + QString::number(keywordint[i]);
                                }continue;
                            }
                            catch(...){};

                            try{
                                pInfile->pHDU().readKey<String>(keywordname[i], keywordstring[i]);
                                if(typeid(keywordstring[i])==typeid(String))
                                {
                                    fitsinfoarray[1+i]=QString::fromStdString(keywordname[i])+ ": " + QString::fromStdString(keywordstring[i]);
                                }continue;
                            }
                            catch(...){};



                        }
                        emit FITSinfoSignal1(fitsinfoarray);
                    }
                    catch(FITS::CantCreate)
                    {
                        cout<<"Can't open fits image file"<<endl;
                        return ;
                    }

                    QString s;
                    QProgressDialog p;
                    p.setLabelText("Conversion Process");
                    p.setRange(0,0);

                    ui->anotherView->initializeCalibrateImage();
                    int count=0;
                    for(int j=xmin; j<xmax+1; j++){
                        for(int m=ymin; m<ymax+1; m++){
                            /////////////////////////////////
                            s.clear();


                            //     if(ui->anotherView->GetAsteroidPixel(m,j)==1){


                            ////////////////////////////////

                            QFile file(initialFileDirectory+"/" + QString::number(j) + "_" + QString::number(m) + ".txt");
                            if(file.exists()){
                                file.open(QIODevice::ReadOnly);
                                QTextStream load(&file);

                                load >> s;

                                file.close();

                            }else{
                                s = "0,0,0,0,0,0,0";
                            }

                            ui->anotherView->calibrateImageforBlackbodyAllPixel(s,j,m);
                            p.setLabelText("Conversion Process \n" + QString::number(count) + " / " + QString::number(((xmax+1)-xmin)*((ymax+1)-ymin)) +"\n" + "x: "+QString::number(j)+"  y: "+QString::number(m));
                            p.show();
                            count++;
                            QCoreApplication::processEvents();

                            if(p.wasCanceled()){
                                ui->anotherView->updateImage(0,initialFileDirectory,DirName);
                                return;
                            }
                        }
                    }


                    ui->anotherView->updateImage(0,initialFileDirectory,DirName);
                }
            }
        }
        //cout<<"Conversion Finished!!!!!!!!!!!!!!!!!!!!!"<<endl;
    }
}

void ControlGraphPanel::on_BlackbodycalibrationAllPixelButton_clicked(){

    ui->anotherView->initializeFITSarray();

    int xmin=0,xmax=0,ymin=0,ymax=0;
    int dragxstart=0,dragxend=0,dragystart=0,dragyend=0;
    int dragxend2=0,dragyend2=0;

    xmin=ui->MinxlineEdit->text().toInt();
    xmax=ui->MaxxlineEdit->text().toInt();
    ymin=ui->MinylineEdit->text().toInt();
    ymax=ui->MaxylineEdit->text().toInt();

    dragxstart=ui->anotherView->startPos_.x()-26;
    dragxend=ui->anotherView->endPos.x()-26;
    dragystart=ui->anotherView->startPos_.y()-16;
    dragyend=ui->anotherView->endPos.y()-16;
    dragxend2=ui->anotherView->endPos.x()-26;
    dragyend2=ui->anotherView->endPos.y()-16;

    if(dragxstart>dragxend){
        int tmp=0;
        tmp=dragxend;
        dragxend=dragxstart;
        dragxstart=tmp;
    }

    if(dragystart>dragyend){
        int tmp=0;
        tmp=dragyend;
        dragyend=dragystart;
        dragystart=tmp;
    }

    if(xmin==0 && dragxstart!=-26){
        xmin=dragxstart;
    }
    if(xmax==327 && dragxend!=-26){
        xmax=dragxend;
    }
    if(ymin==0 && dragystart!=-16){
        ymin=dragystart;
    }
    if(ymax==247 && dragyend!=-16){
        ymax=dragyend;
    }

    if(dragxend2==-26 && dragyend2==-16){
        xmin=0;
        xmax=327;
        ymin=0;
        ymax=247;
    }

    if(xmin<0 ||xmax>327 ||ymin<0 || ymax>247 || ymin>ymax || xmin>xmax){
        return;
    }

    else{
        DarkImage();



        QString s;
        QString initialFileDirectory = QFileDialog::getExistingDirectory(this,tr("Select Register Formula Folder"),QDir::homePath()+"/HEATcalibration/");
        if(initialFileDirectory == ""){
            return;
        }

        QString fitdirectory= QFileDialog::getExistingDirectory(this, tr("Select the directory to save the image"),"/Applications/HEAT_DATA");

        QProgressDialog p;
        p.setLabelText("Calibration Process");
        p.setRange(0,0);

        ui->anotherView->initializeCalibrateImage();
        int count=0;
        for(int j=xmin; j<xmax+1; j++){
            for(int m=ymin; m<ymax+1; m++){

                s.clear();


                QFile file(initialFileDirectory+"/" + QString::number(j) + "_" + QString::number(m) + ".txt");
                if(file.exists()){
                    file.open(QIODevice::ReadOnly);
                    QTextStream load(&file);

                    load >> s;

                    file.close();

                }else{
                    s = "0,0,0,0,0,0,0";
                }

                ui->anotherView->calibrateImageforBlackbodyAllPixel(s,j,m);
                p.setLabelText("Calibration Process \n" + QString::number(count) + " / " + QString::number(((xmax+1)-xmin)*((ymax+1)-ymin)) +"\n" + "x: "+QString::number(j)+"  y: "+QString::number(m));
                p.show();
                count++;
                QCoreApplication::processEvents();

                if(p.wasCanceled()){
                    ui->anotherView->updateImage(0,initialFileDirectory,fitdirectory);
                    return;
                }
            }
        }


        ui->anotherView->updateImage(0,initialFileDirectory,fitdirectory);
        this->ui->anotherView->resize(this->ui->anotherView->getWidth()+110,this->ui->anotherView->getHeight()+21);
    }
}

/* void ControlGraphPanel::on_confirmation_clicked() {
    int xmin, xmax, ymin, ymax;
    int dragxstart,dragxend,dragystart,dragyend;
    int dragxend2,dragyend2;

    xmin = ui->MinxlineEdit->text().toInt();
    xmax = ui->MaxxlineEdit->text().toInt();
    ymin = ui->MinylineEdit->text().toInt();
    ymax = ui->MaxylineEdit->text().toInt();



    dragxstart=ui->anotherView->startPos_.x()-26;
    dragxend=ui->anotherView->endPos.x()-26;
    dragystart=ui->anotherView->startPos_.y()-16;
    dragyend=ui->anotherView->endPos.y()-16;
    dragxend2=ui->anotherView->endPos.x()-26;
    dragyend2=ui->anotherView->endPos.y()-16;

    if(dragxstart>dragxend){
        int tmp=0;
        tmp=dragxend;
        dragxend=dragxstart;
        dragxstart=tmp;
    }

    if(dragystart>dragyend){
        int tmp=0;
        tmp=dragyend;
        dragyend=dragystart;
        dragystart=tmp;
    }

    if(xmin==0 && dragxstart!=-26){
        xmin=dragxstart;
    }
    if(xmax==327 && dragxend!=-26){
        xmax=dragxend;
    }
    if(ymin==0 && dragystart!=-16){
        ymin=dragystart;
    }
    if(ymax==247 && dragyend!=-16){
        ymax=dragyend;
    }

    if(dragxend2==-26 && dragyend2==-16){
        xmin=0;
        xmax=327;
        ymin=0;
        ymax=247;
    }

    if (xmin < 0 || xmax > 327 || ymin < 0 || ymax > 247 || ymin > ymax ||
            xmin > xmax) {
        return;
    }

    else {

        QString s;

        QString initialFileDirectory = QFileDialog::getExistingDirectory(
                    this, tr("Select Register Formula Folder"),
                    "/Applications/HEATcalibration");
        if (initialFileDirectory == "") {
            return;
        }

        QProgressDialog p;
        p.setLabelText("Calibration Process");
        p.setRange(0, 0);

        ui->anotherView->initializeCalibrateImage();

        QString subFileName1 = QFileDialog::getOpenFileName(
                    this, tr("Open Substruct Image"), "TIR_DATA_original",
                    tr("Image Files (*.png *.jpg *.bmp *.img *.fit *.fits *.fts *.inf)"));

        int count = 0;
        for (int j = xmin; j < xmax + 1; j++) {
            for (int m = ymin; m < ymax + 1; m++) {

                s.clear();

                QFile file(initialFileDirectory + "/" + QString::number(j) + "_" +
                           QString::number(m) + ".txt");

                if (file.exists()) {
                    file.open(QIODevice::ReadOnly);
                    QTextStream load(&file);

                    load >> s;

                    file.close();
                } else {
                    s = "0,0,0,0,0,0,0";
                }

                ui->anotherView->confirmation(s, j, m, subFileName1);

                p.setLabelText("Calibration Process \n" + QString::number(count) +
                               " / " + QString::number((xmax - xmin) * (ymax - ymin)) +
                               "\n" + "x: " + QString::number(j) +
                               "  y: " + QString::number(m));
                p.show();
                count++;
                QCoreApplication::processEvents();

                if (p.wasCanceled()) {
                    ui->anotherView->updateImage(0, initialFileDirectory, NULL);
                    return;
                }
            }
        }

        ui->anotherView->updateImage(0, initialFileDirectory, NULL);
    }
}
*/

//don't use now.
void ControlGraphPanel::on_calibrationButton_clicked() {

    int xmin, xmax, ymin, ymax;
    int dragxstart,dragxend,dragystart,dragyend;
    int dragxend2,dragyend2;

    xmin = ui->MinxlineEdit->text().toInt();
    xmax = ui->MaxxlineEdit->text().toInt();
    ymin = ui->MinylineEdit->text().toInt();
    ymax = ui->MaxylineEdit->text().toInt();
    dragxstart=ui->anotherView->startPos_.x()-26;
    dragxend=ui->anotherView->endPos.x()-26;
    dragystart=ui->anotherView->startPos_.y()-16;
    dragyend=ui->anotherView->endPos.y()-16;
    dragxend2=ui->anotherView->endPos.x()-26;
    dragyend2=ui->anotherView->endPos.y()-16;

    if(dragxstart>dragxend){
        int tmp=0;
        tmp=dragxend;
        dragxend=dragxstart;
        dragxstart=tmp;
    }

    if(dragystart>dragyend){
        int tmp=0;
        tmp=dragyend;
        dragyend=dragystart;
        dragystart=tmp;
    }

    if(xmin==0 && dragxstart!=-26){
        xmin=dragxstart;
    }
    if(xmax==327 && dragxend!=-26){
        xmax=dragxend;
    }
    if(ymin==0 && dragystart!=-16){
        ymin=dragystart;
    }
    if(ymax==247 && dragyend!=-16){
        ymax=dragyend;
    }

    if(dragxend2==-26 && dragyend2==-16){
        xmin=0;
        xmax=327;
        ymin=0;
        ymax=247;
    }

    if (xmin < 0 || xmax > 327 || ymin < 0 || ymax > 247 || ymin > ymax ||
            xmin > xmax) {
        return;
    }

    else {

        DarkImage();
        QString s;

        QString initialFileDirectory = QFileDialog::getExistingDirectory(
                    this, "Select Register Formula Folder");
        if (initialFileDirectory == "") {
            return;
        }

        QProgressDialog p;
        p.setLabelText("Calibration Process");
        p.setRange(0, 0);

        ui->anotherView->initializeCalibrateImage();

        int count = 0;
        for (int j = xmin; j < xmax + 1; j++) {
            for (int m = ymin; m < ymax + 1; m++) {
                s.clear();

                QFile file(initialFileDirectory + "/" + QString::number(j) + "_" +
                           QString::number(m) + ".txt");

                if (file.exists()) {
                    file.open(QIODevice::ReadOnly);
                    QTextStream load(&file);

                    load >> s;

                    file.close();
                } else {
                    s = "0,0,0,0,0,0,0";
                }

                ui->anotherView->calibrateImage(s, j, m);

                p.setLabelText("Calibration Process \n" + QString::number(count) +
                               " / " + QString::number((xmax - xmin) * (ymax - ymin)) +
                               "\n" + "x: " + QString::number(j) +
                               "  y: " + QString::number(m));
                p.show();
                count++;
                QCoreApplication::processEvents();

                if (p.wasCanceled()) {
                    ui->anotherView->updateImage(0, initialFileDirectory, NULL);
                    return;
                }
            }
        }

        ui->anotherView->updateImage(0, initialFileDirectory, NULL);
    }
}

void ControlGraphPanel::on_directConversionButton_clicked() {

    QProgressDialog p;
    p.setLabelText("Converting image");
    p.setCancelButton(0);
    p.show();
    QCoreApplication::processEvents();

    imageData raw;
    imageData tmpI;
    QVector<double> tmp1;
    QVector<double> tmp2;
    int rawMAXDN = 0;

    QVector<imageData> images;

    QVector<imageData> selectNear;

    QVector<double> tempItem;

    double nearest = INT_MAX;
    QVector<double> diffDN(2);
    QVector<int> pointNum(2);
    QVector<double> getT(5);

    QString IDm;
    int searchIDm, pairIDm, i = 0;
    QString file1m, file2m;

    QVector<double> vx, vy;

    double tmp;
    QVector<double> weight(5);
    double weight2 = 0;

    int count = 0, mcount = 0, count2;

    QVector<double> resultImage;

    QVector<double> select_temp;

    QVector<double> dist;
    QVector<double> selectDist;
    QVector<interpolateFunction> interpolateTemp;

    QVector<imageData> interpolatePoints;

    double AVE = 0;
    double AVE2 = 0;
    int num;

    int judge = 0;

    tmp1 = ui->anotherView->getImageD();

    tmp2 = ui->anotherView->getImageD();

    count = 0;

    for (int ii = 0; ii < xmldata.l1.Height_data; ii++) {
        for (int jj = 0; jj < xmldata.l1.Width_data; jj++) {

            if (rawMAXDN < tmp2[count] / 8.0)
                rawMAXDN = tmp2[count] / 8.0;

            tmp = tmp2[count] / 8.0;

            raw.image.append(tmp2[count] / 8.0);

            count++;
        }
    }

    QString filepath, T5;

    count = 0;
    filepath = QFileDialog::getOpenFileName(
                this, tr("Open Image"), QDir::homePath(),
                tr("Image Files (*.png *.jpg *.bmp *.img *.inf)"));

    QFile ini(filepath);

    ini.open(QIODevice::ReadOnly);

    QTextStream in(&ini);

    while (!in.atEnd()) {
        in >> filepath;

        if (!QString::compare(filepath.right(6), "_T_MON")) {
            in >> T5;
            getT[count] = T5.toDouble();

            count++;
        }
    }

    raw.bol_t = getT[0];
    raw.pkg_t = getT[1];
    raw.case_t = getT[2];
    raw.shut_t = getT[3];
    raw.lens_t = getT[4];

    raw.target_t = 0;

    count = 0;

    query.first();

    double alpha = 2.0;
    while (1) {

        judge = 0;
        if((query.value(6).toString() == "BB" || query.value(6).toString() == "Oil_bath_BB")
                && raw.bol_t<query.value(15).toDouble() + alpha
                && raw.bol_t > query.value(15).toDouble() - alpha
                && raw.pkg_t<query.value(16).toDouble()  + alpha
                && raw.pkg_t  > query.value(16).toDouble() - alpha
                && raw.case_t<query.value(17).toDouble() + alpha
                && raw.case_t > query.value(17).toDouble() - alpha
                && raw.shut_t<query.value(18).toDouble() + alpha
                && raw.shut_t > query.value(18).toDouble() - alpha
                && raw.lens_t<query.value(19).toDouble() + alpha
                && raw.lens_t > query.value(19).toDouble() - alpha){

            tmpI.image = tmp1;
            tmpI.bol_t = query.value(15).toDouble();
            tmpI.pkg_t = query.value(16).toDouble();
            tmpI.case_t = query.value(17).toDouble();
            tmpI.shut_t = query.value(18).toDouble();
            tmpI.lens_t = query.value(19).toDouble();
            tmpI.target_t = query.value(9).toDouble();
            tmpI.distance = sqrt(pow((raw.bol_t - tmpI.bol_t), 2.0) +
                                 pow((raw.pkg_t - tmpI.pkg_t), 2.0) +
                                 pow((raw.case_t - tmpI.case_t), 2.0) +
                                 pow((raw.shut_t - tmpI.shut_t), 2.0) +
                                 pow((raw.lens_t - tmpI.lens_t), 2.0));

            if (tmpI.distance > 4.0) {
                if (!query.next())
                    break;
                continue;
            }

            IDm = query.value(13).toString();

            searchIDm = QString("0x" + IDm).toInt(0, 16);

            if (searchIDm % 2)
                pairIDm = searchIDm + 1;
            else
                pairIDm = searchIDm - 1;

            pairQuery.first();
            while (1) {

                if (pairQuery.value(13).toString().toInt(0, 16) == pairIDm &&
                        pairQuery.value(6).toString() == query.value(6).toString() &&
                        pairQuery.value(2).toString() == query.value(2).toString())
                    break;

                pairQuery.next();
            }

            databPath=rawimagedirectorypath1;

            file1m = databPath + query.value(2).toString() + "/" +
                    query.value(0).toString();
            file2m = databPath + pairQuery.value(2).toString() + "/" +
                    pairQuery.value(0).toString();
            if (searchIDm % 2)
                ui->anotherView->subtractImage(file1m, file2m);
            else
                ui->anotherView->subtractImage(file2m, file1m);

            mcount = 0;
            tmp1 = ui->anotherView->getImageD();

            if (ui->anotherView->getMinDN() < -40) {
                if (!query.next())
                    break;
                continue;
            }

            if (query.value(4).toString() != "1") {
                for (int height = 0; height < xmldata.l1.Height_data; height++) {
                    for (int width = 0; width < xmldata.l1.Width_data; width++) {

                        tmp1[mcount] /= 8;
                        mcount++;
                    }
                }
            }

            tmpI.image = tmp1;
            tmpI.bol_t = query.value(15).toDouble();
            tmpI.pkg_t = query.value(16).toDouble();
            tmpI.case_t = query.value(17).toDouble();
            tmpI.shut_t = query.value(18).toDouble();
            tmpI.lens_t = query.value(19).toDouble();
            tmpI.target_t = query.value(9).toDouble();
            tmpI.distance = sqrt(pow((raw.bol_t - tmpI.bol_t), 2.0) +
                                 pow((raw.pkg_t - tmpI.pkg_t), 2.0) +
                                 pow((raw.case_t - tmpI.case_t), 2.0) +
                                 pow((raw.shut_t - tmpI.shut_t), 2.0) +
                                 pow((raw.lens_t - tmpI.lens_t), 2.0));

            if (nearest > tmpI.distance)
                nearest = tmpI.distance;

            images.append(tmpI);

            count++;
            if (count % 100 == 0)
                qDebug() << count;
        }

        if (!query.next())
            break;
    }

    mcount = 0;

    qSort(images.begin(), images.end(), this->compDist);

    tempItem.append(images[0].target_t);

    for (int ii = 0; ii < images.size(); ii++) {
        judge = -1;
        for (int k = 0; k < tempItem.size(); k++) {
            if (tempItem[k] == images[ii].target_t)
                judge = 1;
        }

        if (judge != 1)
            tempItem.append(images[ii].target_t);
    }

    for (int jj = 0; jj < tempItem.size(); jj++) {
        count = 0;
        count2 = 0;
        for (int k = 0; k < images.size(); k++) {
            if (tempItem[jj] == images[k].target_t) {
                if (count <= count2) {
                    interpolatePoints.append(images[k]);

                    count++;
                    count2 = 0;
                }
                count2++;
            }
            if (count == 5)
                break;
        }
    }

    count = 0;
    count2 = 0;

    interpolateFunction tmpT;

    for (int h = 0; h < xmldata.l1.Height_data; h++) {
        for (int w = 0; w < xmldata.l1.Width_data; w++) {

            if (6 < h && h < 248 && 16 < w && w < 328) {

                for (int ii = 0; ii < 5; ii++) {

                    tmpT.DN = 0;
                    tmpT.targetT = 0;
                    tmpT.distAve = 0;
                    diffDN[0] = 100000;
                    diffDN[1] = 100000;
                    pointNum[0] = -1;
                    pointNum[1] = -1;

                    for (int jj = 0; jj < tempItem.size(); jj++) {

                        if (diffDN[0] >
                                sqrt(pow(raw.image[count] -
                                         interpolatePoints[ii + jj * 5].image[count],
                                         2.0))) {
                            diffDN[1] = diffDN[0];
                            pointNum[1] = pointNum[0];
                            diffDN[0] =
                                    sqrt(pow(raw.image[count] -
                                             interpolatePoints[ii + jj * 5].image[count],
                                         2.0));
                            pointNum[0] = ii + jj * 5;
                        } else if (diffDN[1] >
                                   sqrt(pow(raw.image[count] -
                                            interpolatePoints[ii + jj * 5].image[count],
                                            2.0))) {
                            diffDN[1] =
                                    sqrt(pow(raw.image[count] -
                                             interpolatePoints[ii + jj * 5].image[count],
                                         2.0));
                            pointNum[1] = ii + jj * 5;
                        }
                    }

                    tmp = (diffDN[0] + diffDN[1]);

                    tmpT.targetT = (interpolatePoints[pointNum[0]].target_t + 273) *
                            (1 - (diffDN[0] / tmp)) +
                            (interpolatePoints[pointNum[1]].target_t + 273) *
                            (1 - (diffDN[1] / tmp));

                    for (int jj = 0; jj < tempItem.size(); jj++) {
                        tmpT.distAve += interpolatePoints[ii + jj * 5].distance;
                    }
                    tmpT.distAve /= tempItem.size();

                    interpolateTemp.append(tmpT);
                }

                tmp = 0;
                weight2 = 0;

                num = 0;
                for (int ii = count2 * 5; ii < (count2 + 1) * 5; ii++) {
                    weight[num] = interpolateTemp[ii].distAve;
                    weight2 += interpolateTemp[ii].distAve;
                    num++;
                }
                qSort(weight.begin(), weight.end(), qGreater<double>());

                num = 0;
                for (int ii = count2 * 5; ii < (count2 + 1) * 5; ii++) {

                    tmp += (interpolateTemp[ii].targetT) * (weight[num] / weight2);

                    num++;
                }
                qDebug() << tmp;
                count2++;
                resultImage.append(tmp);
            } else
                resultImage.append(240);

            count++;
            qDebug() << count;
        }
    }

    count = 0;

    for (int h = 0; h < xmldata.l1.Height_data; h++) {
        for (int w = 0; w < xmldata.l1.Width_data; w++) {

            if (6 < h && h < xmldata.l2.Height_data && 16 < w && w < xmldata.l2.Width_data) {
                AVE = 0;
                AVE2 = 0;
                count2 = 0;
                for (int x = -2; x <= 2; x++) {
                    for (int y = -2; y <= 2; y++) {
                        AVE += resultImage[count + x + xmldata.l1.Width_data * y];
                        if (resultImage[count + x + xmldata.l1.Width_data * y] > 240.9 ||
                                resultImage[count + x + xmldata.l1.Width_data * y] < 239.1)
                            count2++;
                    }
                }
                AVE /= 25.0;

                AVE2 /= 5.0;

                if (AVE < 240.8 && AVE > 239.2 && count2 < 2) {
                    resultImage[count] = 240;
                } else if (AVE2 < 240.8 && AVE2 > 239.2)
                    resultImage[count] = 240;
            } else
                resultImage[count] = 240;

            count++;
        }
    }

    count = 0;
    for (int h = 0; h < xmldata.l1.Height_data; h++) {
        for (int w = 0; w < xmldata.l1.Width_data; w++) {
            if (resultImage[count] == 240)
                resultImage[count] = 150;
            count++;
        }
    }

    qDebug() << "fin";

    for (int ii = 0; ii < 5; ii++) {
        qDebug() << interpolateTemp[ii].targetT;
    }

    ui->anotherView->loadImageD(resultImage);
}

void ControlGraphPanel::getDataPath(QString path) {}

bool ControlGraphPanel::compDist(const imageData e1, const imageData e2) {
    return e1.distance < e2.distance;
}

void ControlGraphPanel::connectDB() {

    query = QSqlQuery(db);
    query.exec(QString("SELECT * FROM tirimageinfo"));

    pairQuery = QSqlQuery(db);
    pairQuery.exec(QString("SELECT * FROM tirimageinfo"));
}


void ControlGraphPanel::on_pushButton_clicked()
{   
    QString lutFileName = QFileDialog::getOpenFileName(this, tr("Load LUT file"), "/");

    //QFile lutFile(lutFileName);
    cout<<lutFileName.toStdString()<<endl;
    readLUTfile(lutFileName);

    calibrationLutFile();
}
void ControlGraphPanel::calibrationLutFile(){
    int xmin, xmax, ymin, ymax;
    int dragxstart=0,dragxend=0,dragystart=0,dragyend=0;
    int dragxend2=0,dragyend2=0;

    xmin = ui->MinxlineEdit->text().toInt();
    xmax = ui->MaxxlineEdit->text().toInt();
    ymin = ui->MinylineEdit->text().toInt();
    ymax = ui->MaxylineEdit->text().toInt();

    dragxstart=ui->anotherView->startPos_.x()-26;
    dragxend=ui->anotherView->endPos.x()-26;
    dragystart=ui->anotherView->startPos_.y()-16;
    dragyend=ui->anotherView->endPos.y()-16;
    dragxend2=ui->anotherView->endPos.x()-26;
    dragyend2=ui->anotherView->endPos.y()-16;

    if(dragxstart>dragxend){
        int tmp=0;
        tmp=dragxend;
        dragxend=dragxstart;
        dragxstart=tmp;
    }

    if(dragystart>dragyend){
        int tmp=0;
        tmp=dragyend;
        dragyend=dragystart;
        dragystart=tmp;
    }

    if(xmin==0 && dragxstart!=-26){
        xmin=dragxstart;
    }
    if(xmax==327 && dragxend!=-26){
        xmax=dragxend;
    }
    if(ymin==0 && dragystart!=-16){
        ymin=dragystart;
    }
    if(ymax==247 && dragyend!=-16){
        ymax=dragyend;
    }

    if(dragxend2==-26 && dragyend2==-16){
        xmin=0;
        xmax=327;
        ymin=0;
        ymax=247;
    }

    if (xmin < 0 || xmax > 327 || ymin < 0 || ymax > 247 || ymin > ymax ||
            xmin > xmax) {
        return;
    }
    else{
        DarkImage();
        int i=0,j=0,counter=0,count=0;
        double sl[248][328],in[248][328];
        for(i=0;i<248;i++){
            for(j=0;j<328;j++){
                sl[i][j] = slope[counter];
                in[i][j] = intercept[counter];
                counter++;
            }
        }
        QProgressDialog p;
        p.setLabelText("Conversion Process");
        p.setRange(0, 0);
        i=0,j=0;
        for(i=ymin;i<ymax+1;i++){
            for(j=xmin;j<xmax+1;j++){
                //cout<<i<<","<<j<<" sl[][] "<<sl[i][j]<<" in[][] "<<in[i][j]<<endl;
                if(fitskeys=="HEAT") ui->anotherView->calibrateLUTl2a(sl[i][j],in[i][j],i,j);
                else ui->anotherView->calibrationLutImage(sl[i][j],in[i][j],i,j);
                p.setLabelText("Calibration Process \n" + QString::number(count) +
                               " / " + QString::number(((xmax - xmin)+1) * ((ymax - ymin)+1)) +
                               "\n" + "x: " + QString::number(j) +
                               "  y: " + QString::number(i));
                p.show();
                count++;
                QCoreApplication::processEvents();
                if (p.wasCanceled()) {
                    ui->anotherView->updateImage(0, QDir::homePath(), NULL);
                    return;
                }
            }
        }
        ui->anotherView->updateImage(0, QDir::homePath(), NULL);
        ui->anotherView->resize(ui->anotherView->getWidth()+110,ui->anotherView->getHeight()+20);
        i=0;
    }
}

void ControlGraphPanel::on_outputCurrentImageButton_clicked()
{
    ui->anotherView->outputCurrentImage();
}


void ControlGraphPanel::on_substructFITSButton_clicked()
{
    subFileName = QFileDialog::getOpenFileName(this, tr("Open Substruct Image"), "TIR_DATA_original", tr("Image Files (*.img *.fit *.fits *.fts)"));
    if(fileName!= NULL){
        ui->fileNameBrowser->append(subFileName.section('/',-1,-1));
        ui->anotherView->subtractFITSImage(subFileName);
    }
}


void ControlGraphPanel::on_StartConversionButton_clicked()
{
    QString conversionmethod = ui->ConversionmethodcomboBox->currentText();
    if(conversionmethod=="Direct Conversion"){
        on_directConversionButton_clicked();
    }
    //Don't use "Regression Conversion".
    else if(conversionmethod=="Regression Conversion"){
        on_calibrationButton_clicked();
    }
    else if(conversionmethod=="l2b Conversion"){
        on_calibrationtoRadianceButton_clicked();
    }
    else if(conversionmethod=="l2a Conversion"){
        on_BlackbodycalibrationAllPixelButton_clicked();
    }
    else if(conversionmethod=="l2a Conversion (Directory)"){
        on_BlackbodycalibrationAllPixelButton_repeat_clicked();
    }
    else if(conversionmethod=="LUT Fits file Conversion"){
        on_pushButton_clicked();
    }
}

void ControlGraphPanel::DarkImage(){
    QString dark = ui->DarkcomboBox->currentText();
    if(dark=="Set Dark: 2018-06-29"){
        ui->anotherView->SetDarkImage("hyb2_tir_20180629_083003_dark.fit");
    }
    else if(dark=="Set Dark: 2018-11-09"){
        ui->anotherView->SetDarkImage("hyb2_tir_20181109_060903_dark.fit");
    }
    else if(dark=="Set Dark: No"){
        ui->anotherView->initializedarkimage();
    }
}
