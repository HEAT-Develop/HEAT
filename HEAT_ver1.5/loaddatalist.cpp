//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//

#include "loaddatalist.h"
#include "ui_loaddatalist.h"
#include <iostream>
#include <fstream>
#include <CCfits/FITS.h>
#include <CCfits/PHDU.h>
#include <CCfits/HDU.h>
#include <string>
#include <stdio.h>
#include <QFileInfo>
#include <ctype.h>
#include <stdlib.h>
#include <typeinfo>
#include "dataset.h"


#define fitscount 77

using namespace CCfits;
using namespace std;

//ファイルパス記憶
QString fileInfo[50];

LoadDataList::LoadDataList(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadDataList)
{
    ui->setupUi(this);
    this->setWindowTitle("Load Data List");
    this->move(10,300);
    QObject::connect(ui->clearListButton,SIGNAL(clicked()),ui->loadedDataList,SLOT(clear()));
}

LoadDataList::~LoadDataList()
{
    delete ui;
}

void LoadDataList::on_loadedDataList_clicked(const QModelIndex &index)
{
    emit SelectDataSignal(fileInfo[index.row()],ui->loadedDataList->item(index.row())->text().length());
}

void LoadDataList::on_tilingButton_clicked()
{
    emit tilingWindowSignal();
}

void LoadDataList::on_closeAllButton_clicked()
{
    emit closeWindowSignal();
}

void LoadDataList::on_loadFileButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Image"), QDir::homePath(), tr("Image Files (*.png *.jpg *.bmp *.img *.fit *.fits *.fts *.txt *.dat *.inf)"));
    fstream ifs;
    ifs.open(&fileName.toStdString()[0],ios::in | ios::binary);
    QFileInfo fileinfo;
    fileinfo.setFile(fileName);
    QString ext = fileinfo.suffix();
    ext=ext.toLower();


    if (ext == "fit" || ext=="fits" || ext=="fts")//(fitファイルの検索)
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
                        //cout<<keyworddouble[i]<<endl;
                        //cout<<"double"<<endl;
                        fitsinfoarray[1+i]=QString::fromStdString(keywordname[i])+ ":  " + QString::number(keyworddouble[i]);
                    }continue;
                }
                catch(...){};

                try{
                    pInfile->pHDU().readKey<int>(keywordname[i], keywordint[i]);
                    if(typeid(keywordint[i])==typeid(int))
                    {
                        //cout<<keywordint[i]<<endl;
                        //cout<<"int"<<endl;
                        fitsinfoarray[1+i]=QString::fromStdString(keywordname[i])+ ":  " + QString::number(keywordint[i]);
                    }continue;
                }
                catch(...){};

                try{
                    pInfile->pHDU().readKey<String>(keywordname[i], keywordstring[i]);
                    if(typeid(keywordstring[i])==typeid(String))
                    {
                        //cout<<keywordstring[i]<<endl;
                        //cout<<"String"<<endl;
                        fitsinfoarray[1+i]=QString::fromStdString(keywordname[i])+ ":  " + QString::fromStdString(keywordstring[i]);
                    }continue;
                }
                catch(...){};

            }
            emit FITSinfoSignal(fitsinfoarray);
        }
    catch(FITS::CantCreate)
        {
            cout<<"Can't open fits image file"<<endl;
            return ;
        }
    }





    int n;
    QString d = "/";

    // 文字列最後尾から"/"と一致するまでの文字数を記憶
    for(int i=1;i<fileName.length();i++){
        n=i-1;
        if(!QString::compare(fileName.mid(fileName.length()-i,1),d)) break;
    }

    //ファイルリストにファイルパスを記憶
    fileInfo[ui->loadedDataList->count()] = fileName;
    //ファイル名だけをリストに追加
    ui->loadedDataList->addItem(fileName.right(n));

    if(fileName!= NULL){
        emit loadDataSignal(fileName,n);
    }
}

void LoadDataList::on_changeParameterButton_clicked()
{
    emit changeParameterSignal(ui->minT->text().toDouble(),ui->maxT->text().toDouble(),ui->changeColor->currentIndex());
}

void LoadDataList::on_substract_clicked()
{
    emit substractSignal();
}

