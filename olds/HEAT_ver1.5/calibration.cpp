//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//
#include "calibration.h"
#include "ui_calibration.h"
#include <iostream>
#include <fstream>
#include "dataset.h"
using namespace std;

Calibration::Calibration(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Calibration)
{
    ui->setupUi(this);
    this->setWindowTitle("Calibration");


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


    XValidator = new QIntValidator(0, 327, this);
    YValidator = new QIntValidator(0, 247, this);
    ui->x->setValidator(XValidator);
    ui->y->setValidator(YValidator);


    this->connectDB();


    QVector<QString> tmp;
    tmp.append("empty");
    pixelList.append(tmp);
}

Calibration::~Calibration()
{
    db.close();
    delete ui;
}

void Calibration::connectDB()
{
    db.open();

    query = QSqlQuery(db);
    query.exec(QString("SELECT * FROM tirimageinfo"));

    if(query.isActive()){
        query.first();
    }



}

void Calibration::checkAction(){
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
            if(query.value(6).toString()== "BB"){
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
            if(query.value(6).toString()== "Colli_BB"){
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

void Calibration::fillForm(){

    ui->targetTl->setText(QString::number(T_min));
    ui->targetTh->setText(QString::number(T_max));

    ui->bolTl->setText(QString::number(B_min));
    ui->bolTh->setText(QString::number(B_max));

    ui->pkgTl->setText(QString::number(P_min));
    ui->pkgTh->setText(QString::number(P_max));

    ui->caseTl->setText(QString::number(C_min));
    ui->caseTh->setText(QString::number(C_max));

    ui->shutTl->setText(QString::number(S_min));
    ui->shutTh->setText(QString::number(S_max));

    ui->lensTl->setText(QString::number(L_min));
    ui->lensTh->setText(QString::number(L_max));
}

void Calibration::setValue()
{
    if(T_min > query.value(9).toDouble()){
        T_min = query.value(9).toDouble();
    }
    if(T_max < query.value(9).toDouble()){
        T_max = query.value(9).toDouble();
    }

    if(B_min > query.value(15).toDouble()){
        B_min = query.value(15).toDouble();
    }
    if(B_max < query.value(15).toDouble()){
        B_max = query.value(15).toDouble();
    }

    if(P_min > query.value(16).toDouble()){
        P_min = query.value(16).toDouble();
    }
    if(P_max < query.value(16).toDouble()){
        P_max = query.value(16).toDouble();
    }

    if(C_min > query.value(17).toDouble()){
        C_min = query.value(17).toDouble();
    }
    if(C_max < query.value(17).toDouble()){
        C_max = query.value(17).toDouble();
    }

    if(S_min > query.value(18).toDouble()){
        S_min = query.value(18).toDouble();
    }
    if(S_max < query.value(18).toDouble()){
        S_max = query.value(18).toDouble();
    }

    if(L_min > query.value(19).toDouble()){
        L_min = query.value(19).toDouble();
    }
    if(L_max < query.value(19).toDouble()){
        L_max = query.value(19).toDouble();
    }

}

QString Calibration::nameChange(QString tmp)
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

bool Calibration::judgeItem(){

    if(ui->BB->checkState()){
        if(pixelQuery.value(12).toString()== "BB"){
            return true;
        }
    }

    if(ui->OilBB->checkState()){
        if(pixelQuery.value(12).toString()== "Oil_bath_BB"){
            return true;
        }
    }

    if(ui->Winselwan->checkState()){
        if(pixelQuery.value(12).toString()== "Winselwan"){
            return true;
        }

    }

    if(ui->Murchison->checkState()){
        if(pixelQuery.value(12).toString()== "Murchison"){
            return true;
        }
    }

    if(ui->Murray->checkState()){
        if(pixelQuery.value(12).toString()== "Murray"){
            return true;
        }
    }

    if(ui->MARAr->checkState()){
        if(pixelQuery.value(12).toString()== "MARA_rock_plate"){
            return true;
        }
    }

    if(ui->Sky->checkState()){
        if(pixelQuery.value(12).toString()== "Air"){
            return true;
        }
    }

    if(ui->Colli_BB->checkState()){
        if(pixelQuery.value(12).toString()== "Colli_BB"){
            return true;
        }
    }


    if(ui->BB_modified->checkState()){
        if(pixelQuery.value(12).toString()== "BB"){
            return true;
        }
    }

    if(ui->Colli_BB_modified->checkState()){
        if(pixelQuery.value(12).toString()== "Colli_BB"){
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

void Calibration::on_searchPixcelButton_clicked()
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

    QString x = QString::number(ui->x->text().toInt() + 16);
    QString y = QString::number(ui->y->text().toInt() + 6);

    if(px != x.toInt() || py != y.toInt()){
        QString tableName = this->judgeTableName(x.toInt(),y.toInt());

        pixelQuery = QSqlQuery(db);
        pixelQuery.exec("SELECT " + tableName + ".img_file, x, y, pixel,pixel_modified, mask, tirimageinfo.img_file, thumbnail, path, date_time, m,"
                        + " place, target_name, phi, hood_t, target_t, len_t, plt_t, bol_t, img_id, plt_t_set, bol_t_mon, pkg_t_mon, "
                        + "case_t_mon, sh_t_mon, lens_t_mon FROM "+ tableName + ", tirimageinfo WHERE x=" + x + " AND y=" + y
                        + " AND tirimageinfo.img_file=" + tableName + ".img_file");




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
            if( rangeL <= pixelQuery.value(15).toDouble() && rangeH >= pixelQuery.value(15).toDouble()){
                rangeL = ui->bolTl->text().toDouble();
                rangeH = ui->bolTh->text().toDouble();
                if(rangeL==0&&rangeH==0){rangeL=MAXL;rangeH=MAXH;}
                if( rangeL <= pixelQuery.value(21).toDouble() && rangeH >= pixelQuery.value(21).toDouble()){
                    rangeL = ui->pkgTl->text().toDouble();
                    rangeH = ui->pkgTh->text().toDouble();
                    if(rangeL==0&&rangeH==0){rangeL=MAXL;rangeH=MAXH;}
                    if( rangeL <= pixelQuery.value(22).toDouble() && rangeH >= pixelQuery.value(22).toDouble()){
                        rangeL = ui->caseTl->text().toDouble();
                        rangeH = ui->caseTh->text().toDouble();
                        if(rangeL==0&&rangeH==0){rangeL=MAXL;rangeH=MAXH;}

                        if( rangeL <= pixelQuery.value(23).toDouble() && rangeH >= pixelQuery.value(23).toDouble()){
                            rangeL = ui->shutTl->text().toDouble();
                            rangeH = ui->shutTh->text().toDouble();
                            if(rangeL==0&&rangeH==0){rangeL=MAXL;rangeH=MAXH;}

                            if( rangeL <= pixelQuery.value(24).toDouble() && rangeH >= pixelQuery.value(24).toDouble()){
                                rangeL = ui->lensTl->text().toDouble();
                                rangeH = ui->lensTh->text().toDouble();
                                if(rangeL==0&&rangeH==0){rangeL=MAXL;rangeH=MAXH;}

                                if( rangeL <= pixelQuery.value(25).toDouble() && rangeH >= pixelQuery.value(25).toDouble()){
                                    QVector <QString> tmp;
                                    tmp.append(pixelQuery.value(0).toString());
                                    tmp.append(QString::number(pixelQuery.value(1).toString().toInt() - 16));
                                    tmp.append(QString::number(pixelQuery.value(2).toString().toInt() - 6));
                                    if(pixelQuery.value(10).toInt() > 1){
                                        if(ui->BB_modified->isChecked() || ui->Colli_BB_modified->isChecked() ||ui->Furnace_BB_modified->isChecked() ||ui->Oil_bath_BB_modified->isChecked())
                                        {

                                            tmp.append(QString::number(pixelQuery.value(4).toDouble()/8));

                                        }
                                        else
                                        {
                                            tmp.append(QString::number(pixelQuery.value(3).toDouble()/8));

                                        }

                                    }else{

                                        if(ui->BB_modified->isChecked() || ui->Colli_BB_modified->isChecked() ||ui->Furnace_BB_modified->isChecked() ||ui->Oil_bath_BB_modified->isChecked())
                                        {

                                            tmp.append(QString::number(pixelQuery.value(4).toDouble()));

                                        }
                                        else
                                        {
                                            tmp.append(QString::number(pixelQuery.value(3).toDouble()));

                                        }
                                    }


                                    tmp.append(pixelQuery.value(21).toString());
                                    tmp.append(pixelQuery.value(22).toString());
                                    tmp.append(pixelQuery.value(23).toString());
                                    tmp.append(pixelQuery.value(24).toString());
                                    tmp.append(pixelQuery.value(25).toString());
                                    tmp.append(pixelQuery.value(15).toString());
                                    tmp.append(pixelQuery.value(19).toString());
                                    tmp.append(pixelQuery.value(12).toString());
                                    tmp.append(pixelQuery.value(8).toString());

                                    tmp.append(pixelQuery.value(5).toString());
                                    tmp.append(pixelQuery.value(6).toString());
                                    tmp.append(pixelQuery.value(7).toString());
                                    tmp.append(pixelQuery.value(9).toString());
                                    tmp.append(pixelQuery.value(10).toString());
                                    tmp.append(pixelQuery.value(11).toString());
                                    tmp.append(pixelQuery.value(13).toString());
                                    tmp.append(pixelQuery.value(14).toString());
                                    tmp.append(pixelQuery.value(16).toString());
                                    tmp.append(pixelQuery.value(17).toString());
                                    tmp.append(pixelQuery.value(18).toString());
                                    tmp.append(pixelQuery.value(20).toString());

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
        target += "  Colli_BB_modified";
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
    ui->dbInfoList->addItem("Bolometer Temperature  :  " + ui->bolTl->text() + " - " + ui->bolTh->text());
    ui->dbInfoList->addItem("Package Temperature  :  " + ui->pkgTl->text() + " - " + ui->pkgTh->text());
    ui->dbInfoList->addItem("Case Temperature  :  " + ui->caseTl->text() + " - " + ui->caseTh->text());
    ui->dbInfoList->addItem("Shutter Temperature  :  " + ui->shutTl->text() + " - " + ui->shutTh->text());
    ui->dbInfoList->addItem("Lens Temperature  :  " + ui->lensTl->text() + " - " + ui->lensTh->text());

    coordinateChangeFlag = false;
}

void Calibration::on_pixelList_clicked(const QModelIndex &index)
{
    index.row();

    pixelQuery.first();
    QListWidgetItem* item = ui->pixelList->currentItem();

    while(item->text() != pixelQuery.value(0).toString()){
        if(pixelQuery.next() == false){
            return;
        }
    }

    info[0] = "Image ID : "+ pixelQuery.value(19).toString();
    info[1] = "Target Name : "+nameChange(pixelQuery.value(12).toString());
    info[2] = "Date Time : "+pixelQuery.value(9).toString();
    info[3] = "m : "+pixelQuery.value(10).toString();
    info[4] = "place : "+pixelQuery.value(11).toString();
    if(pixelQuery.value(13).toInt()!=0) info[5]= "Φ : "+pixelQuery.value(13).toString()+" mm";
    else info[5] = "Φ : N/A";
    if(pixelQuery.value(14).toInt()!=0)info[6]= "Hood Temperature: "+pixelQuery.value(14).toString()+" mm";
    else info[6]= "Hood Temperature: N/A";
    if(pixelQuery.value(15).toInt()!=0)info[7]= "Set Terget Temperature : "+pixelQuery.value(15).toString()+" degC";
    else info[7]= "Set Terget Temperature : N/A";
    if(pixelQuery.value(16).toInt()!=0)info[8]= "Set Lens Temperature : "+pixelQuery.value(16).toString()+" degC";
    else info[8]= "Set Lens Temperature : N/A";

    if(pixelQuery.value(17).toInt()!=0)info[9]= "Set Plate Temperature : "+pixelQuery.value(17).toString()+" degC";
    else info[9]= "Set Plate Temperature : N/A";
    if(pixelQuery.value(18).toInt()!=0)info[10]= "Set Bolometer Temperature : "+pixelQuery.value(18).toString()+" degC";
    else info[10]= "Set Bolometer Temperature : N/A";
    if(pixelQuery.value(20).toInt()!=0)info[11]= "Set Peltier Temperature : "+pixelQuery.value(20).toString()+" degC";
    else info[11]= "Set Peltier Temperature : N/A";

    info[12] = "Bolometer Temperature : "+pixelQuery.value(21).toString()+" degC";
    info[13] = "Package Temperature : "+pixelQuery.value(22).toString()+" degC";
    info[14] = "Case Temperature : "+pixelQuery.value(23).toString()+" degC";
    info[15] = "Shutter Temperature : "+pixelQuery.value(24).toString()+" degC";
    info[16] = "Lens Temperature : "+pixelQuery.value(25).toString()+" degC";
    info[17] = "Filename(.img) : "+pixelQuery.value(0).toString();
    info[18] = "Pixel Position : x = " +QString::number(pixelQuery.value(1).toInt() - 16) +
            " y = " + QString::number(pixelQuery.value(2).toInt() - 6);

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
        if(pixelQuery.value(10).toInt() > 1){
            info[19] = "Pixel Value : " + QString::number(pixelQuery.value(3).toInt()/8);
        }
        else
        {
            info[19] = "Pixel Value : " + pixelQuery.value(3).toString();
        }

    }

    ui->dbInfoList->clear();
    for(int i=0;i<20;i++)
        ui->dbInfoList->addItem(info[i]);

}

QString Calibration::judgeTableName(int x, int y){
    if(0 <=  y && y <= 31){
        if(0 <= x && x <= 31) return "pix01";
        else if(32 <= x && x <= 63) return "pix02";
        else if(64 <= x && x <= 95) return "pix03";
        else if(96 <= x && x <= 127) return "pix04";
        else if(128 <= x && x <= 159) return "pix05";
        else if(160 <= x && x <= 191) return "pix06";
        else if(192 <= x && x <= 223) return "pix07";
        else if(224 <= x && x <= 255) return "pix08";
        else if(256 <= x && x <= 287) return "pix09";
        else if(288 <= x && x <= 319) return "pix10";
        else if(320 <= x && x <= 351) return "pix11";
        else if(352 <= x && x <= 383) return "pix12";
    }else if(32 <= y && y <= 63){
        if(0 <= x && x <= 31) return "pix13";
        else if(32 <= x && x <= 63) return "pix14";
        else if(64 <= x && x <= 95) return "pix15";
        else if(96 <= x && x <= 127) return "pix16";
        else if(128 <= x && x <= 159) return "pix17";
        else if(160 <= x && x <= 191) return "pix18";
        else if(192 <= x && x <= 223) return "pix19";
        else if(224 <= x && x <= 255) return "pix20";
        else if(256 <= x && x <= 287) return "pix21";
        else if(288 <= x && x <= 319) return "pix22";
        else if(320 <= x && x <= 351) return "pix23";
        else if(352 <= x && x <= 383) return "pix24";
    }else if(64 <= y && y <= 95){
        if(0 <= x && x <= 31) return "pix25";
        else if(32 <= x && x <= 63) return "pix26";
        else if(64 <= x && x <= 95) return "pix27";
        else if(96 <= x && x <= 127) return "pix28";
        else if(128 <= x && x <= 159) return "pix29";
        else if(160 <= x && x <= 191) return "pix30";
        else if(192 <= x && x <= 223) return "pix31";
        else if(224 <= x && x <= 255) return "pix32";
        else if(256 <= x && x <= 287) return "pix33";
        else if(288 <= x && x <= 319) return "pix34";
        else if(320 <= x && x <= 351) return "pix35";
        else if(352 <= x && x <= 383) return "pix36";
    }else if(96 <= y && y <= 127){
        if(0 <= x && x <= 31) return "pix37";
        else if(32 <= x && x <= 63) return "pix38";
        else if(64 <= x && x <= 95) return "pix39";
        else if(96 <= x && x <= 127) return "pix40";
        else if(128 <= x && x <= 159) return "pix41";
        else if(160 <= x && x <= 191) return "pix42";
        else if(192 <= x && x <= 223) return "pix43";
        else if(224 <= x && x <= 255) return "pix44";
        else if(256 <= x && x <= 287) return "pix45";
        else if(288 <= x && x <= 319) return "pix46";
        else if(320 <= x && x <= 351) return "pix47";
        else if(352 <= x && x <= 383) return "pix48";
    }else if(128 <= y && y <= 159){
        if(0 <= x && x <= 31) return "pix49";
        else if(32 <= x && x <= 63) return "pix50";
        else if(64 <= x && x <= 95) return "pix51";
        else if(96 <= x && x <= 127) return "pix52";
        else if(128 <= x && x <= 159) return "pix53";
        else if(160 <= x && x <= 191) return "pix54";
        else if(192 <= x && x <= 223) return "pix55";
        else if(224 <= x && x <= 255) return "pix56";
        else if(256 <= x && x <= 287) return "pix57";
        else if(288 <= x && x <= 319) return "pix58";
        else if(320 <= x && x <= 351) return "pix59";
        else if(352 <= x && x <= 383) return "pix60";
    }else if(160 <= y && y <= 191){
        if(0 <= x && x <= 31) return "pix61";
        else if(32 <= x && x <= 63) return "pix62";
        else if(64 <= x && x <= 95) return "pix63";
        else if(96 <= x && x <= 127) return "pix64";
        else if(128 <= x && x <= 159) return "pix65";
        else if(160 <= x && x <= 191) return "pix66";
        else if(192 <= x && x <= 223) return "pix67";
        else if(224 <= x && x <= 255) return "pix68";
        else if(256 <= x && x <= 287) return "pix69";
        else if(288 <= x && x <= 319) return "pix70";
        else if(320 <= x && x <= 351) return "pix71";
        else if(352 <= x && x <= 383) return "pix72";
    }else if(192 <= y && y <= 223){
        if(0 <= x && x <= 31) return "pix73";
        else if(32 <= x && x <= 63) return "pix74";
        else if(64 <= x && x <= 95) return "pix75";
        else if(96 <= x && x <= 127) return "pix76";
        else if(128 <= x && x <= 159) return "pix77";
        else if(160 <= x && x <= 191) return "pix78";
        else if(192 <= x && x <= 223) return "pix79";
        else if(224 <= x && x <= 255) return "pix80";
        else if(256 <= x && x <= 287) return "pix81";
        else if(288 <= x && x <= 319) return "pix82";
        else if(320 <= x && x <= 351) return "pix83";
        else if(352 <= x && x <= 383) return "pix84";
    }else if(224 <= y && y <= 255){
        if(0 <= x && x <= 31) return "pix85";
        else if(32 <= x && x <= 63) return "pix86";
        else if(64 <= x && x <= 95) return "pix87";
        else if(96 <= x && x <= 127) return "pix88";
        else if(128 <= x && x <= 159) return "pix89";
        else if(160 <= x && x <= 191) return "pix90";
        else if(192 <= x && x <= 223) return "pix91";
        else if(224 <= x && x <= 255) return "pix92";
        else if(256 <= x && x <= 287) return "pix93";
        else if(288 <= x && x <= 319) return "pix94";
        else if(320 <= x && x <= 351) return "pix95";
        else if(352 <= x && x <= 383) return "pix96";
    }

    return "";
}

void Calibration::on_showGraphButton_clicked(){
    cout<<"noooo?";
    if(pixelList.size() < 1 || pixelList[0][0] == "empty"){
        cout<<"moo?";
        QMessageBox::information(this, tr("Not searched pixels"), "There are not searched pixels.");
        return;
    }
    if(coordinateChangeFlag == true){
        cout<<"moo?@@2";
        coordinateChangeFlag = false;
        QMessageBox::information(this, tr("Warning"),
                                 "You have not pressed \"Search Pixel\" button yet although you change \"Detector Coordinates\".");
        return ;
    }

    QString xAxis, yAxis, lineType;

    xAxis = ui->xComboBox->currentText();
    yAxis = ui->yComboBox->currentText();
    lineType = ui->lineComboBox->currentText();
    if(ui->BB_modified->isChecked() || ui->Colli_BB_modified->isChecked() ||ui->Furnace_BB_modified->isChecked() ||ui->Oil_bath_BB_modified->isChecked()){
        bool ismodified;
        ismodified = true;
        emit showCalibrationGraphSignal(pixelList, xAxis, yAxis, lineType, queryNum,ismodified);
    }

    else{
        bool ismodified;
        ismodified = false;
        emit showCalibrationGraphSignal(pixelList, xAxis, yAxis, lineType, queryNum,ismodified);

    }
    cout<<"lllllo"<<endl;
}

void Calibration::showCalibrationPanel(){
    this->raise();
}
void Calibration::on_x_textChanged(const QString &arg1)
{
    if(arg1.toInt() != px){
        coordinateChangeFlag = true;
    }
}

void Calibration::on_y_textChanged(const QString &arg1)
{
    if(arg1.toInt() != py){
        coordinateChangeFlag = true;
    }
}

void Calibration::getDataPath(QString path){
    dataPath = path;
}

void Calibration::setX(QString x){
    ui->x->setText(QString::number(x.toInt()));
}

void Calibration::setY(QString y){
    ui->y->setText(QString::number(y.toInt()));
}
