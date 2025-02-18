//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//
#include "calibration_tiri.h"
#include "ui_calibration_tiri.h"
#include <iostream>
#include <fstream>

using namespace std;

Calibration_tiri::Calibration_tiri(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Calibration_tiri)
{
    ui->setupUi(this);
    this->setWindowTitle("Calibration_TIRI");

    QObject::connect(ui->All, SIGNAL( clicked() ), this , SLOT( checkAction() ));
    QObject::connect(ui->BB, SIGNAL( clicked() ), this , SLOT( checkAction() ));
    QObject::connect(ui->OilBB, SIGNAL( clicked() ), this , SLOT( checkAction() ));
    QObject::connect(ui->Winselwan, SIGNAL( clicked() ), this , SLOT( checkAction() ));
    QObject::connect(ui->Sky, SIGNAL( clicked() ), this , SLOT( checkAction() ));
    QObject::connect(ui->Murray, SIGNAL( clicked() ), this , SLOT( checkAction() ));
    QObject::connect(ui->Murchison, SIGNAL( clicked() ), this , SLOT( checkAction() ));
    QObject::connect(ui->MARAr, SIGNAL( clicked() ), this , SLOT( checkAction() ));
    QObject::connect(ui->Colli_BB, SIGNAL( clicked() ), this , SLOT( checkAction() ));
    QObject::connect(ui->BB_modified, SIGNAL( clicked() ), this , SLOT( checkAction() ));
    QObject::connect(ui->Colli_BB_modified, SIGNAL( clicked() ), this , SLOT( checkAction() ));
    QObject::connect(ui->Furnace_BB_modified, SIGNAL( clicked() ), this , SLOT( checkAction() ));
    QObject::connect(ui->Oil_bath_BB_modified, SIGNAL( clicked() ), this , SLOT( checkAction() ));


    XValidator = new QIntValidator(0, 1024, this);
    YValidator = new QIntValidator(0, 768, this);
    ui->x->setValidator(XValidator);
    ui->y->setValidator(YValidator);

    this->connectDBtiri();


    QVector<QString> tmp;
    tmp.append("empty");
    pixelList.append(tmp);
}

Calibration_tiri::~Calibration_tiri()
{
    db.close();
    delete ui;
}

void Calibration_tiri::connectDBtiri()
{

    db = QSqlDatabase::addDatabase(QString("QMYSQL"),"konkontiri");

    db.setHostName("localhost");

    db.setUserName(QString("root"));

    db.setPassword(QString("kontake825"));

    db.setDatabaseName(QString("TIRI_pre"));

    db.open();

    query = QSqlQuery(db);

    query.exec(QString("SELECT * FROM tiriimageinfo"));

    if(query.isActive()){
        query.first();
    }


/*
    db.open();

    query = QSqlQuery(db);
    query.exec(QString("SELECT * FROM tirimageinfo"));

    if(query.isActive()){
        query.first();
    }
*/
}

void Calibration_tiri::checkAction(){
    T_max=-1000,T_min=1000,B_max=0,B_min=100,P_max=0,P_min=100,C_max=0,C_min=100,S_max=0,S_min=100,L_max=0,L_min=100;

    ui->All->setEnabled(true);
    ui->BB->setEnabled(true);
    ui->OilBB->setEnabled(true);
    ui->Winselwan->setEnabled(true);
    ui->Murchison->setEnabled(true);
    ui->Murray->setEnabled(true);
    ui->MARAr->setEnabled(true);
    ui->Sky->setEnabled(true);
    ui->Colli_BB->setEnabled(true);
    ui->BB_modified->setEnabled(true);
    ui->Colli_BB_modified->setEnabled(true);
    ui->Furnace_BB_modified->setEnabled(true);
    ui->Oil_bath_BB_modified->setEnabled(true);

    if(ui->All->checkState()){
        query.first();
        do  {
            setValue();
        }while(query.next());

        ui->BB->setEnabled(false);
        ui->OilBB->setEnabled(false);
        ui->Winselwan->setEnabled(false);
        ui->Murchison->setEnabled(false);
        ui->Murray->setEnabled(false);
        ui->MARAr->setEnabled(false);
        ui->Sky->setEnabled(false);
        ui->Colli_BB->setEnabled(false);
        ui->BB_modified->setEnabled(false);
        ui->Colli_BB_modified->setEnabled(false);
        ui->Furnace_BB_modified->setEnabled(false);
        ui->Oil_bath_BB_modified->setEnabled(false);
    }

    if(ui->BB->checkState()){
        query.first();
        do {
            if(query.value(18).toString()== "BB"){
                setValue();
            }
        }while(query.next());
        ui->All->setEnabled(false);
    }

    if(ui->OilBB->checkState()){
        query.first();
        do {
            if(query.value(6).toString()== "Oil_bath_BB"){
                setValue();
            }
        }while(query.next());
        ui->All->setEnabled(false);
    }

    if(ui->Winselwan->checkState()){
        query.first();
        do {
            if(query.value(6).toString()== "Winselwan"){
                setValue();
            }
        }while(query.next());
        ui->All->setEnabled(false);
    }

    if(ui->Murchison->checkState()){
        query.first();
        do {
            if(query.value(6).toString()== "Murchison"){
                setValue();
            }
        }while(query.next());
        ui->All->setEnabled(false);
    }

    if(ui->Murray->checkState()){
        query.first();
        do {
            if(query.value(6).toString()== "Murray"){
                setValue();
            }
        }while(query.next());
        ui->All->setEnabled(false);
    }
    if(ui->MARAr->checkState()){
        query.first();
        do {
            if(query.value(6).toString()== "MARA_rock_plate"){
                setValue();
            }
        }while(query.next());
        ui->All->setEnabled(false);
    }

    if(ui->Sky->checkState()){
        query.first();
        do {
            if(query.value(6).toString()== "Air"){
                setValue();
            }
        }while(query.next());
        ui->All->setEnabled(false);
    }

    if(ui->Colli_BB->checkState()){
        query.first();
        do {
            if(query.value(6).toString()== "Colli_BB"){
                setValue();
            }
        }while(query.next());
        ui->All->setEnabled(false);
    }
    if(ui->BB_modified->checkState()){
        query.first();
        do {
            if(query.value(6).toString()== "BB"){
                setValue();
            }
        }while(query.next());
        ui->All->setEnabled(false);
    }

    if(ui->Colli_BB_modified->checkState()){
        query.first();
        do {
            if(query.value(18).toString()== "COLL"){
                setValue();
            }
        }while(query.next());
        ui->All->setEnabled(false);
    }

    if(ui->Furnace_BB_modified->checkState()){
        query.first();
        do {
            if(query.value(6).toString()== "Furnace_BB"){
                setValue();
            }
        }while(query.next());
        ui->All->setEnabled(false);
    }

    if(ui->Oil_bath_BB_modified->checkState()){
        query.first();
        do {
            if(query.value(6).toString()== "Oil_bath_BB"){
                setValue();
            }
        }while(query.next());
        ui->All->setEnabled(false);
    }





    fillForm();
}

void Calibration_tiri::fillForm(){

    ui->targetTl->setText(QString::number(T_min));
    ui->targetTh->setText(QString::number(T_max));

    ui->fpa1Tl->setText(QString::number(FPA1_min));
    ui->fpa1Th->setText(QString::number(FPA1_max));

    ui->fpa2Tl->setText(QString::number(FPA2_min));
    ui->fpa2Th->setText(QString::number(FPA2_max));

    ui->caseTl->setText(QString::number(CAS_min));
    ui->caseTh->setText(QString::number(CAS_max));

    ui->lens1Tl->setText(QString::number(L1_min));
    ui->lens1Th->setText(QString::number(L1_max));

    ui->lens2Tl->setText(QString::number(L2_min));
    ui->lens2Th->setText(QString::number(L2_max));

    ui->fw1Tl->setText(QString::number(FW1_min));
    ui->fw1Th->setText(QString::number(FW1_max));

    ui->fw2Tl->setText(QString::number(FW2_min));
    ui->fw2Th->setText(QString::number(FW2_max));

    ui->dcTl->setText(QString::number(DC_min));
    ui->dcTh->setText(QString::number(DC_max));

    ui->hodTl->setText(QString::number(HOD_min));
    ui->hodTh->setText(QString::number(HOD_max));

    ui->rd1Tl->setText(QString::number(RD1_min));
    ui->rd1Th->setText(QString::number(RD1_max));

    ui->rd2Tl->setText(QString::number(RD2_min));
    ui->rd2Th->setText(QString::number(RD2_max));
}

void Calibration_tiri::setValue()
{
    if(T_min > query.value(2).toDouble()){
        T_min = query.value(2).toDouble();
    }
    if(T_max < query.value(2).toDouble()){
        T_max = query.value(2).toDouble();
    }

    if(FPA1_min > query.value(9).toDouble()){
        FPA1_min = query.value(9).toDouble();
    }
    if(FPA1_max < query.value(9).toDouble()){
        FPA1_max = query.value(9).toDouble();
    }

    if(FPA2_min > query.value(10).toDouble()){
        FPA2_min = query.value(10).toDouble();
    }
    if(FPA2_max < query.value(10).toDouble()){
        FPA2_max = query.value(10).toDouble();
    }

    if(CAS_min > query.value(8).toDouble()){
        CAS_min = query.value(8).toDouble();
    }
    if(CAS_max < query.value(8).toDouble()){
        CAS_max = query.value(8).toDouble();
    }

    if(L1_min > query.value(4).toDouble()){
        L1_min = query.value(4).toDouble();
    }
    if(L1_max < query.value(4).toDouble()){
        L1_max = query.value(4).toDouble();
    }

    if(L2_min > query.value(5).toDouble()){
        L2_min = query.value(5).toDouble();
    }
    if(L2_max < query.value(5).toDouble()){
        L2_max = query.value(5).toDouble();
    }

    if(FW1_min > query.value(6).toDouble()){
        FW1_min = query.value(6).toDouble();
    }
    if(FW1_max < query.value(6).toDouble()){
        FW1_max = query.value(6).toDouble();
    }

    if(FW2_min > query.value(7).toDouble()){
        FW2_min = query.value(7).toDouble();
    }
    if(FW2_max < query.value(7).toDouble()){
        FW2_max = query.value(7).toDouble();
    }

    if(DC_min > query.value(14).toDouble()){
        DC_min = query.value(14).toDouble();
    }
    if(DC_max < query.value(14).toDouble()){
        DC_max = query.value(14).toDouble();
    }

    if(HOD_min > query.value(11).toDouble()){
        HOD_min = query.value(11).toDouble();
    }
    if(HOD_max < query.value(11).toDouble()){
        HOD_max = query.value(11).toDouble();
    }

    if(RD1_min > query.value(12).toDouble()){
        RD1_min = query.value(12).toDouble();
    }
    if(RD1_max < query.value(12).toDouble()){
        RD1_max = query.value(12).toDouble();
    }

    if(RD2_min > query.value(13).toDouble()){
        RD2_min = query.value(13).toDouble();
    }
    if(RD2_max < query.value(13).toDouble()){
        RD2_max = query.value(13).toDouble();
    }






}

QString Calibration_tiri::nameChange(QString tmp)
{
    QString Change=tmp;

    if(tmp=="Air")Change="Sky";
    if(tmp=="BB")Change="Black_body";
    if(tmp=="Oil_bath_BB")Change="Oil_bath_black_body";
    if(tmp=="Winselwan")Change="Winselwan(meteorite)";
    if(tmp=="Murray")Change="Murray(meteorite)";
    if(tmp=="Murchison")Change="Murchison(meteorite)";

    if(tmp=="Sky")Change="Air";
    if(tmp=="Black_body")Change="BB";
    if(tmp=="Oil_bath_black_body")Change="Oil_bath_BB";
    if(tmp=="Winselwan(meteorite)")Change="Winselwan";
    if(tmp=="Murray(meteorite)")Change="Murray";
    if(tmp=="Murchison(meteorite)")Change="Murchison";
    if(tmp=="Colli_BB")Change="Colli_BB";

    return Change;
}

bool Calibration_tiri::judgeItem(){

    if(ui->BB->checkState()){
        if(pixelQuery.value(21).toString()== "BB"){
            return true;
        }
    }

    if(ui->OilBB->checkState()){
        if(pixelQuery.value(21).toString()== "Oil_bath_BB"){
            return true;
        }
    }

    if(ui->Winselwan->checkState()){
        if(pixelQuery.value(21).toString()== "Winselwan"){
            return true;
        }

    }

    if(ui->Murchison->checkState()){
        if(pixelQuery.value(21).toString()== "Murchison"){
            return true;
        }
    }

    if(ui->Murray->checkState()){
        if(pixelQuery.value(21).toString()== "Murray"){
            return true;
        }
    }

    if(ui->MARAr->checkState()){
        if(pixelQuery.value(21).toString()== "MARA_rock_plate"){
            return true;
        }
    }

    if(ui->Sky->checkState()){
        if(pixelQuery.value(21).toString()== "Air"){
            return true;
        }
    }

    if(ui->Colli_BB->checkState()){
        if(pixelQuery.value(21).toString()== "Colli_BB"){
            return true;
        }
    }


    if(ui->BB_modified->checkState()){
        if(pixelQuery.value(21).toString()== "BB"){
            return true;
        }
    }

    if(ui->Colli_BB_modified->checkState()){
        if(pixelQuery.value(21).toString()== "COLL"){
            return true;
        }
    }

    if(ui->Furnace_BB_modified->checkState()){
        if(pixelQuery.value(12).toString()== "Furnace_BB"){
            return true;
        }
    }

    if(ui->Oil_bath_BB_modified->checkState()){
        if(pixelQuery.value(12).toString()== "Oil_bath_BB"){
            return true;
        }
    }



    if(ui->All->checkState()){
        return true;
    }

    return false;
}

void Calibration_tiri::on_searchPixcelButton_clicked()
{

    if(query.isActive() == false){
        return;
    }

    QProgressDialog p;
    p.setLabelText("Connecting Database");
    p.setCancelButton(0);
    p.show();
    QCoreApplication::processEvents();

    ui->pixelList->clear();

    if(ui->x->text() == "" || ui->y->text() == ""){
        QVector<QString> tmp;
        tmp.append("empty");
        pixelList.append(tmp);
        return ;
    }

    QString x = QString::number(ui->x->text().toInt());
    QString y = QString::number(ui->y->text().toInt());


    if(px != x.toInt() || py != y.toInt()){
        QString tableName = this->judgeTableName(x.toInt(),y.toInt());


        pixelQuery = QSqlQuery(db);

        pixelQuery.exec("SELECT " + tableName + ".img_file, x, y, pixel, tiriimageinfo_mask.m, target_t, img_id, ls1_t, ls2_t, fw1_t, fw2_t, cas_t, fpa1_t, fpa2_t, hod_t, rd1_t, rd2_t, dc_t ,fw_num, pos_num,ffn,target_name,mask FROM "+ tableName + ", tiriimageinfo_mask WHERE x=" + x + " AND y=" + y
                        + " AND tiriimageinfo_mask.img_file = "+ tableName + ".img_file");


            qDebug()<<"SELECT " + tableName + ".img_file, x, y, pixel, tiriimageinfo_mask.m, target_t, img_id, ls1_t, ls2_t, fw1_t, fw2_t, cas_t, fpa1_t, fpa2_t, hod_t, rd1_t, rd2_t, dc_t ,fw_num, pos_num,ffn,target_name FROM "+ tableName + ", tiriimageinfo_mask WHERE x=" + x + " AND y=" + y
                         + " AND tiriimageinfo_mask.img_file = "+ tableName + ".img_file";


        px = x.toInt();
        py = y.toInt();
    }

    double rangeL;
    double rangeH;

    pixelQuery.first();
    queryNum = 0;


    pixelList.clear();


    do {

        if(judgeItem()){
            rangeL = ui->targetTl->text().toDouble();
            rangeH = ui->targetTh->text().toDouble();
            if(rangeL==0&&rangeH==0){rangeL=MAXL;rangeH=MAXH;}
            if( rangeL <= pixelQuery.value(5).toDouble() && rangeH >= pixelQuery.value(5).toDouble()){
                rangeL = ui->fpa1Tl->text().toDouble();
                rangeH = ui->fpa1Th->text().toDouble();
                if(rangeL==0&&rangeH==0){rangeL=MAXL;rangeH=MAXH;}
                if( rangeL <= pixelQuery.value(12).toDouble() && rangeH >= pixelQuery.value(12).toDouble()){
                rangeL = ui->fpa2Tl->text().toDouble();
                rangeH = ui->fpa2Th->text().toDouble();
                if(rangeL==0&&rangeH==0){rangeL=MAXL;rangeH=MAXH;}
                    if( rangeL <= pixelQuery.value(13).toDouble() && rangeH >= pixelQuery.value(13).toDouble()){
                        rangeL = ui->caseTl->text().toDouble();
                        rangeH = ui->caseTh->text().toDouble();
                        if(rangeL==0&&rangeH==0){rangeL=MAXL;rangeH=MAXH;}
                        if( rangeL <= pixelQuery.value(11).toDouble() && rangeH >= pixelQuery.value(11).toDouble()){
                            rangeL = ui->lens1Tl->text().toDouble();
                            rangeH = ui->lens1Th->text().toDouble();
                            if(rangeL==0&&rangeH==0){rangeL=MAXL;rangeH=MAXH;}
                            if( rangeL <= pixelQuery.value(7).toDouble() && rangeH >= pixelQuery.value(7).toDouble()){
                                rangeL = ui->lens2Tl->text().toDouble();
                                rangeH = ui->lens2Th->text().toDouble();
                                if(rangeL==0&&rangeH==0){rangeL=MAXL;rangeH=MAXH;}
                                if( rangeL <= pixelQuery.value(8).toDouble() && rangeH >= pixelQuery.value(8).toDouble()){
                                    rangeL = ui->fw1Tl->text().toDouble();
                                    rangeH = ui->fw1Th->text().toDouble();
                                    if(rangeL==0&&rangeH==0){rangeL=MAXL;rangeH=MAXH;}
                                    if( rangeL <= pixelQuery.value(9).toDouble() && rangeH >= pixelQuery.value(9).toDouble()){
                                        rangeL = ui->fw2Tl->text().toDouble();
                                        rangeH = ui->fw2Th->text().toDouble();
                                        if(rangeL==0&&rangeH==0){rangeL=MAXL;rangeH=MAXH;}
                                        if( rangeL <= pixelQuery.value(10).toDouble() && rangeH >= pixelQuery.value(10).toDouble()){
                                            rangeL = ui->dcTl->text().toDouble();
                                            rangeH = ui->dcTh->text().toDouble();
                                            if(rangeL==0&&rangeH==0){rangeL=MAXL;rangeH=MAXH;}
                                            if( rangeL <= pixelQuery.value(17).toDouble() && rangeH >= pixelQuery.value(17).toDouble()){
                                                rangeL = ui->hodTl->text().toDouble();
                                                rangeH = ui->hodTh->text().toDouble();
                                                if(rangeL==0&&rangeH==0){rangeL=MAXL;rangeH=MAXH;}
                                                if( rangeL <= pixelQuery.value(14).toDouble() && rangeH >= pixelQuery.value(14).toDouble()){
                                                    rangeL = ui->rd1Tl->text().toDouble();
                                                    rangeH = ui->rd1Th->text().toDouble();
                                                    if(rangeL==0&&rangeH==0){rangeL=MAXL;rangeH=MAXH;}
                                                    if( rangeL <= pixelQuery.value(15).toDouble() && rangeH >= pixelQuery.value(15).toDouble()){
                                                        rangeL = ui->rd2Tl->text().toDouble();
                                                        rangeH = ui->rd2Th->text().toDouble();
                                                        if(rangeL==0&&rangeH==0){rangeL=MAXL;rangeH=MAXH;}
                                                        if( rangeL <= pixelQuery.value(16).toDouble() && rangeH >= pixelQuery.value(16).toDouble()){
                                                            QVector <QString> tmp;
                                                            tmp.append(pixelQuery.value(0).toString());
                                                            tmp.append(QString::number(pixelQuery.value(1).toString().toInt()));
                                                            tmp.append(QString::number(pixelQuery.value(2).toString().toInt()));
                                                            if(pixelQuery.value(4).toInt() > 1){
                                                                if(ui->BB_modified->isChecked() ||ui->Furnace_BB_modified->isChecked() ||ui->Oil_bath_BB_modified->isChecked())
                                                                {

                                                                    tmp.append(QString::number(pixelQuery.value(3).toDouble()/8)); // _modifiedが手に入った時に変更　by ryuji

                                                                }
                                                                else
                                                                {
                                                                    tmp.append(QString::number(pixelQuery.value(3).toDouble()/16));

                                                                }

                                                            }else{

                                                                if(ui->BB_modified->isChecked() ||ui->Furnace_BB_modified->isChecked() ||ui->Oil_bath_BB_modified->isChecked())
                                                                {

                                                                    tmp.append(QString::number(pixelQuery.value(3).toDouble()/16)); // /16=積算枚数 _modifiedが手に入った時に変更　by ryuji

                                                                }
                                                                else
                                                                {
                                                                    tmp.append(QString::number(pixelQuery.value(3).toDouble()/16));

                                                                }
                                                            }


                                                            tmp.append(pixelQuery.value(4).toString()); //4
                                                            tmp.append(pixelQuery.value(5).toString());
                                                            tmp.append(pixelQuery.value(6).toString());
                                                            tmp.append(pixelQuery.value(7).toString());
                                                            tmp.append(pixelQuery.value(8).toString());
                                                            tmp.append(pixelQuery.value(9).toString());
                                                            tmp.append(pixelQuery.value(10).toString());
                                                            tmp.append(pixelQuery.value(11).toString()); //11
                                                            tmp.append(pixelQuery.value(12).toString());

                                                            tmp.append(pixelQuery.value(13).toString());
                                                            tmp.append(pixelQuery.value(14).toString());
                                                            tmp.append(pixelQuery.value(15).toString());//15
                                                            tmp.append(pixelQuery.value(16).toString());
                                                            tmp.append(pixelQuery.value(17).toString());
                                                            tmp.append(pixelQuery.value(18).toString());//18
                                                            tmp.append(pixelQuery.value(19).toString());
                                                            tmp.append(pixelQuery.value(20).toString());
                                                            tmp.append(pixelQuery.value(21).toString());
                                                            tmp.append(pixelQuery.value(22).toString());

                                                            pixelList.append(tmp);
                                                            ui->pixelList->addItem(pixelList[queryNum][0]);
                                                            if(pixelQuery.value(5) == 1){
                                                                ui->pixelList->item(queryNum)->setTextColor(Qt::blue);
                                                            }else{
                                                                ui->pixelList->item(queryNum)->setTextColor(Qt::black);
                                                            }


                                                            queryNum++;
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
    }while(pixelQuery.next());



    if(pixelList.size() < 1){
        pixelList.clear();

        QVector<QString> tmp;
        tmp.append("empty");
        pixelList.append(tmp);
        return;
    }

    ui->dbInfoList->clear();
    ui->dbInfoList->addItem("Pixel Search Succeed");

    QString target = "Target Name  :";
    if (ui->All->isChecked()) {
        target += "  All";
    }
    if (ui->BB->isChecked()) {
        target += "  Black_body";
    }
    if (ui->OilBB->isChecked()) {
        target += "  Oil_bath_black_body";
    }
    if (ui->Winselwan->isChecked()) {
        target += "  Winselwan(meteorite)";
    }
    if (ui->MARAr->isChecked()) {
        target += "  MARA_rock_plate";
    }
    if (ui->Murray->isChecked()) {
        target += "  Murray(meteorite)";
    }
    if (ui->Murchison->isChecked()) {
        target += "  Murchison(meteorite)";
    }
    if (ui->Sky->isChecked()) {
        target += "  Sky";
    }
    if (ui->Colli_BB->isChecked()) {
        target += "  Colli_BB";
    }
    if (ui->BB_modified->isChecked()) {
        target += "  BB_modified";
    }
    if(ui->Colli_BB_modified->isChecked()){
        target += "  COLL";
    }
    if(ui->Furnace_BB_modified->isChecked()){
        target += "  Furnace_BB_modified";
    }
    if(ui->Oil_bath_BB_modified->isChecked()){
        target += "  Oil_bath_BB_modified";
    }

    ui->dbInfoList->addItem(target);
    ui->dbInfoList->addItem("X  :  " + ui->x->text() + "   Y  :  " + ui->y->text());
    ui->dbInfoList->addItem("Target tempareture  :  " + ui->targetTl->text() + " - " + ui->targetTh->text());
    ui->dbInfoList->addItem("FPA1 Temperature  :  " + ui->fpa1Tl->text() + " - " + ui->fpa1Th->text());
    ui->dbInfoList->addItem("FPA2 Temperature  :  " + ui->fpa2Tl->text() + " - " + ui->fpa2Th->text());
    ui->dbInfoList->addItem("Case Temperature  :  " + ui->caseTl->text() + " - " + ui->caseTh->text());
    ui->dbInfoList->addItem("Lens1 Temperature  :  " + ui->lens1Tl->text() + " - " + ui->lens1Th->text());
    ui->dbInfoList->addItem("Lens2 Temperature  :  " + ui->lens2Tl->text() + " - " + ui->lens2Th->text());
    ui->dbInfoList->addItem("FW1 tempareture  :  " + ui->fw1Tl->text() + " - " + ui->fw1Th->text());
    ui->dbInfoList->addItem("FW2 Temperature  :  " + ui->fw2Tl->text() + " - " + ui->fw2Th->text());
    ui->dbInfoList->addItem("DC Temperature  :  " + ui->dcTl->text() + " - " + ui->dcTh->text());
    ui->dbInfoList->addItem("HOOD Temperature  :  " + ui->hodTl->text() + " - " + ui->hodTh->text());
    ui->dbInfoList->addItem("Radiator1 Temperature  :  " + ui->rd1Tl->text() + " - " + ui->rd1Th->text());
    ui->dbInfoList->addItem("Radiator2 Temperature  :  " + ui->rd2Tl->text() + " - " + ui->rd2Th->text());

    coordinateChangeFlag = false;
}

void Calibration_tiri::on_pixelList_clicked(const QModelIndex &index)
{
    index.row();

    pixelQuery.first();
    QListWidgetItem* item = ui->pixelList->currentItem();

    while(item->text() != pixelQuery.value(0).toString()){
        if(pixelQuery.next() == false){
            return;
        }
    }

    info[0] = "Image ID : "+ pixelQuery.value(6).toString();
    info[1] = "Target Name : "+nameChange(pixelQuery.value(21).toString());
    info[2] = "m : "+pixelQuery.value(4).toString();
    if(pixelQuery.value(5).toInt()!=0)info[3]= "Set Terget Temperature : "+pixelQuery.value(5).toString()+" degC";
    else info[3]= "Set Terget Temperature : N/A";
    info[4] = "FPA1 Temperature : "+pixelQuery.value(12).toString()+" degC";
    info[5] = "FPA2 Temperature : "+pixelQuery.value(13).toString()+" degC";
    info[6] = "Case Temperature : "+pixelQuery.value(11).toString()+" degC";
    info[7] = "Lens1 Temperature : "+pixelQuery.value(7).toString()+" degC";
    info[8] = "Lens2 Temperature : "+pixelQuery.value(8).toString()+" degC";
    info[9] = "FW1 Temperature : "+pixelQuery.value(9).toString()+" degC";
    info[10] = "FW2 Temperature : "+pixelQuery.value(10).toString()+" degC";
    info[11] = "DC Temperature : "+pixelQuery.value(17).toString()+" degC";
    info[12] = "HOOD Temperature : "+pixelQuery.value(14).toString()+" degC";
    info[13] = "Radiator1 Temperature : "+pixelQuery.value(15).toString()+" degC";
    info[14] = "Radiator2 Temperature : "+pixelQuery.value(16).toString()+" degC";
    info[15] = "Filename(.img) : "+pixelQuery.value(0).toString();
    info[16] = "Pixel Position : x = " +QString::number(pixelQuery.value(1).toInt() ) +
            " y = " + QString::number(pixelQuery.value(2).toInt() );

    if(ui->BB_modified->isChecked() || ui->Colli_BB_modified->isChecked() ||ui->Furnace_BB_modified->isChecked() ||ui->Oil_bath_BB_modified->isChecked())
    {
        if(pixelQuery.value(10).toInt() > 1){
            info[19] = "Pixel Value : " + QString::number(pixelQuery.value(4).toInt()/8);
        }
        else
        {
            info[19] = "Pixel Value : " + pixelQuery.value(4).toString();

        }
    }
    else
    {
        if(pixelQuery.value(4).toInt() > 1){
            info[17] = "Pixel Value : " + QString::number(pixelQuery.value(3).toInt()/8);
        }
        else
        {
            info[17] = "Pixel Value : " + pixelQuery.value(3).toString();
        }

    }
    info[18]="filter_num : filter" + pixelQuery.value(20).toString();
    info[19]="filter_num : filter" + pixelQuery.value(18).toString();

    ui->dbInfoList->clear();
    for(int i=0;i<20;i++)//19->18
        ui->dbInfoList->addItem(info[i]);

}

QString Calibration_tiri::judgeTableName(int x, int y){
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

void Calibration_tiri::on_showGraphButton_clicked(){

    if(pixelList.size() < 1 || pixelList[0][0] == "empty"){
        QMessageBox::information(this, tr("Not searched pixels"), "There are not searched pixels.");
        return;
    }
    if(coordinateChangeFlag == true){
        coordinateChangeFlag = false;
        QMessageBox::information(this, tr("Warning"),
                                 "You have not pressed \"Search Pixel\" button yet although you change \"Detector Coordinates\".");
        return ;
    }

    QString xAxis, yAxis, lineType;

    xAxis = ui->xComboBox->currentText();
    yAxis = ui->yComboBox->currentText();
    lineType = ui->lineComboBox->currentText();
    if(ui->BB_modified->isChecked() || ui->Furnace_BB_modified->isChecked() ||ui->Oil_bath_BB_modified->isChecked()){
        bool ismodified;
        ismodified = true;
        emit showCalibrationGraphSignal_tiri(pixelList, xAxis, yAxis, lineType, queryNum,ismodified);
    }

    else{
        bool ismodified;
        ismodified = false;
        emit showCalibrationGraphSignal_tiri(pixelList, xAxis, yAxis, lineType, queryNum,ismodified);

    }
}

void Calibration_tiri::showCalibrationPanel(){
    this->raise();
}
void Calibration_tiri::on_x_textChanged(const QString &arg1)
{
    if(arg1.toInt() != px){
        coordinateChangeFlag = true;
    }
}

void Calibration_tiri::on_y_textChanged(const QString &arg1)
{
    if(arg1.toInt() != py){
        coordinateChangeFlag = true;
    }
}

void Calibration_tiri::getDataPath(QString path){
    dataPath = path;
}

void Calibration_tiri::setX(QString x){
    ui->x->setText(QString::number(x.toInt()));
}

void Calibration_tiri::setY(QString y){
    ui->y->setText(QString::number(y.toInt()));
}
