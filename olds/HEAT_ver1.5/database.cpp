//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//

#include "database.h"
#include "ui_database.h"
#include <QFileDialog>
#include <fstream>
#include <iostream>
#include <mainwindow.h>
#include <math.h>
#include <stdio.h>

#include "dataset.h"

using namespace std;
//QString rawimagedirectorypath="/Volumes/Lacie/HEAT_DB/HEAT_DB";
//QString rawimagedirectorypath="/Users/s1250009/Desktop/HEAT/HEAT_Path/HEATDB/TIR_DATA_original";
QString rawimagedirectorypath="/Volumes/BufflWinMac/HEAT/HEAT_Path/HEAT_Path/HEATDB/TIR_DATA_original";

bool all = false;

double B_max = 0, B_min = 100, P_max = 0, P_min = 100, C_max = 0, C_min = 100,
S_max = 0, S_min = 100, L_max = 0, L_min = 100, T_max = 0, T_min = 100;

int portNum;

Database::Database(QWidget *parent) : QDialog(parent), ui(new Ui::Database) {
    ui->setupUi(this);
    this->setWindowTitle("Database");

    loadInitFile();
    qDebug()<<"Seikou  --->>  "+xmldata.scname;
    logFileDirectory.cd("../search_log");

    QObject::connect(ui->All, SIGNAL(clicked()), this, SLOT(checkAction()));
    QObject::connect(ui->BB, SIGNAL(clicked()), this, SLOT(checkAction()));
    QObject::connect(ui->OilBB, SIGNAL(clicked()), this, SLOT(checkAction()));
    QObject::connect(ui->Winselwan, SIGNAL(clicked()), this, SLOT(checkAction()));
    QObject::connect(ui->Sky, SIGNAL(clicked()), this, SLOT(checkAction()));

    QObject::connect(ui->Murray, SIGNAL(clicked()), this, SLOT(checkAction()));
    QObject::connect(ui->Murchison, SIGNAL(clicked()), this, SLOT(checkAction()));
    QObject::connect(ui->MARAr, SIGNAL(clicked()), this, SLOT(checkAction()));
    QObject::connect(ui->ColliBB, SIGNAL(clicked()), this, SLOT(checkAction()));

    QObject::connect(ui->thumbnailView, SIGNAL(valueChangedY(QString)),
                     ui->heightValue, SLOT(setText(QString)));
    QObject::connect(ui->thumbnailView, SIGNAL(valueChangedX(QString)),
                     ui->widthValue, SLOT(setText(QString)));
    QObject::connect(ui->thumbnailView, SIGNAL(valueChangedPixel(QString)),
                     ui->pixelValue, SLOT(setText(QString)));

    QObject::connect(this, SIGNAL(loadThumbnailSignal(QString)),
                     ui->thumbnailView, SLOT(loadFileName(QString)));

    connectDB();
}

Database::~Database() {
    db.close();
    delete ui;
}

void Database::connectDB() {

    db = QSqlDatabase::addDatabase(QString("QMYSQL"));

    db.setHostName(xmldata.db_info.host);

    db.setUserName(xmldata.db_info.user);
    if(xmldata.db_info.pass == ""){
        qDebug()<<"pass is null.";
    }
    else{
        db.setPassword(xmldata.db_info.pass);
    }
    db.setPassword(xmldata.db_info.pass);
    // db.setPassword(QString("kontake825"));

    db.setDatabaseName(xmldata.db_info.dbname);

    db.open();

    query = QSqlQuery(db);
    query.exec(QString("SELECT * FROM tirimageinfo"));

    pairQuery = QSqlQuery(db);
    pairQuery.exec(QString("SELECT * FROM tirimageinfo"));

    if (query.isActive()) {

        ui->connectState->setText("Status: Connected to database: port " +
                                  QString::number(portNum));
        query.first();
    } else {
        ui->connectState->setText("Status: Could not connected");
    }
}

void Database::checkAction() {

    B_max = 0, B_min = 100, P_max = 0, P_min = 100, C_max = 0, C_min = 100,
            S_max = 0, S_min = 100, L_max = 0, L_min = 100, T_max = -100, T_min = 100;

    ui->dbSearchList->clear();
    ui->All->setEnabled(true);
    ui->BB->setEnabled(true);
    ui->OilBB->setEnabled(true);
    ui->Winselwan->setEnabled(true);
    ui->Murchison->setEnabled(true);
    ui->Murray->setEnabled(true);
    ui->MARAr->setEnabled(true);
    ui->Sky->setEnabled(true);
    ui->ColliBB->setEnabled(true);

    if (ui->All->checkState()) {
        query.first();
        do {
            ui->dbSearchList->addItem(query.value(0).toString());
            setValue();
        } while (query.next());

        ui->BB->setEnabled(false);
        ui->OilBB->setEnabled(false);
        ui->Winselwan->setEnabled(false);
        ui->Murchison->setEnabled(false);
        ui->Murray->setEnabled(false);
        ui->MARAr->setEnabled(false);
        ui->Sky->setEnabled(false);
        ui->ColliBB->setEnabled(false);
    }

    if (ui->BB->checkState()) {
        query.first();
        do {
            if (query.value(6).toString() == "BB") {
                ui->dbSearchList->addItem(query.value(0).toString());
                setValue();
            }
        } while (query.next());
        ui->All->setEnabled(false);
    }

    if (ui->OilBB->checkState()) {
        query.first();
        do {
            if (query.value(6).toString() == "Oil_bath_BB") {
                ui->dbSearchList->addItem(query.value(0).toString());
                setValue();
            }
        } while (query.next());
        ui->All->setEnabled(false);
    }

    if (ui->Winselwan->checkState()) {
        query.first();
        do {
            if (query.value(6).toString() == "Winselwan") {
                ui->dbSearchList->addItem(query.value(0).toString());
                setValue();
            }
        } while (query.next());
        ui->All->setEnabled(false);
    }

    if (ui->Murchison->checkState()) {
        query.first();
        do {
            if (query.value(6).toString() == "Murchison") {
                ui->dbSearchList->addItem(query.value(0).toString());
                setValue();
            }
        } while (query.next());
        ui->All->setEnabled(false);
    }

    if (ui->Murray->checkState()) {
        query.first();
        do {
            if (query.value(6).toString() == "Murray") {
                ui->dbSearchList->addItem(query.value(0).toString());
                setValue();
            }
        } while (query.next());
        ui->All->setEnabled(false);
    }
    if (ui->MARAr->checkState()) {
        query.first();
        do {
            if (query.value(6).toString() == "MARA_rock_plate") {
                ui->dbSearchList->addItem(query.value(0).toString());
                setValue();
            }
        } while (query.next());
        ui->All->setEnabled(false);
    }

    if (ui->Sky->checkState()) {
        query.first();
        do {
            if (query.value(6).toString() == "Air") {
                ui->dbSearchList->addItem(query.value(0).toString());
                setValue();
            }
        } while (query.next());
        ui->All->setEnabled(false);
    }

    if (ui->ColliBB->checkState()) {
        query.first();
        do {
            if (query.value(6).toString() == "Colli_BB") {
                ui->dbSearchList->addItem(query.value(0).toString());
                setValue();
            }
        } while (query.next());
        ui->All->setEnabled(false);
    }

    fillForm();
}

void Database::fillForm() {

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

bool Database::judgeItem() {

    if (ui->BB->checkState()) {
        if (query.value(6).toString() == "BB") {
            return true;
        }
    }

    if (ui->OilBB->checkState()) {
        if (query.value(6).toString() == "Oil_bath_BB") {
            return true;
        }
    }

    if (ui->Winselwan->checkState()) {
        if (query.value(6).toString() == "Winselwan") {
            return true;
        }
    }

    if (ui->Murchison->checkState()) {
        if (query.value(6).toString() == "Murchison") {
            return true;
        }
    }

    if (ui->Murray->checkState()) {
        if (query.value(6).toString() == "Murray") {
            return true;
        }
    }

    if (ui->MARAr->checkState()) {
        if (query.value(6).toString() == "MARA_rock_plate") {
            return true;
        }
    }

    if (ui->Sky->checkState()) {
        if (query.value(6).toString() == "Air") {
            return true;
        }
    }

    if (ui->ColliBB->checkState()) {
        if (query.value(6).toString() == "Colli_BB") {
            return true;
        }
    }

    if (ui->All->checkState()) {
        return true;
    }

    return false;
}

QString Database::nameChange(QString tmp) {
    QString Change = tmp;

    if (tmp == "Air")
        Change = "Sky";
    if (tmp == "BB")
        Change = "Black_body";
    if (tmp == "Oil_bath_BB")
        Change = "Oil_bath_black_body";
    if (tmp == "Winselwan")
        Change = "Winselwan(meteorite)";
    if (tmp == "Murray")
        Change = "Murray(meteorite)";
    if (tmp == "Murchison")
        Change = "Murchison(meteorite)";

    if (tmp == "Sky")
        Change = "Air";
    if (tmp == "Black_body")
        Change = "BB";
    if (tmp == "Oil_bath_black_body")
        Change = "Oil_bath_BB";
    if (tmp == "Winselwan(meteorite)")
        Change = "Winselwan";
    if (tmp == "Murray(meteorite)")
        Change = "Murray";
    if (tmp == "Murchison(meteorite)")
        Change = "Murchison";

    return Change;
}

void Database::showdbinfo() {

    QString fileName = rawimagedirectorypath +
            query.value(2).toString() + "/" +
            query.value(0).toString();
    //cout<<"1."<<fileName.toStdString()<<endl;
    if (query.value(0).toString().length() != 23 ||
            query.value(0).toString().length() != 22) {

        QString fullname = query.value(0).toString();
        QString fname = fullname.mid(5, 8);

        bool bl = fullname.contains("open", Qt::CaseInsensitive);
        bool bl1 = fullname.contains("close", Qt::CaseInsensitive);
        bool bl2 = fileName.contains("LIR", Qt::CaseInsensitive);
        bool bl3 = fileName.contains("ColliBB", Qt::CaseInsensitive);

        if (bl == true && bl2 == true && bl3 == true) {
            fileName = rawimagedirectorypath +
                    query.value(2).toString() + "/LIR_" + fname + "_open.img";
        }
        if (bl1 == true && bl2 == true && bl3 == true) {
            fileName = rawimagedirectorypath +
                    query.value(2).toString() + "/LIR_" + fname + "_close.img";
        }
    }
    //cout<<"2."<<fileName.toStdString()<<endl;
    emit loadThumbnailSignal(fileName);

    //cout << fileName.toStdString() << endl;

    info[0] = "Image ID : " + query.value(13).toString();
    info[1] = "Target Name : " + query.value(2).toString() + "/" +
            nameChange(query.value(6).toString());
    info[2] = "Date Time : " + query.value(3).toString();
    info[3] = "m : " + query.value(4).toString();
    info[4] = "place : " + query.value(5).toString();
    if (query.value(7).toInt() != 0)
        info[5] = "Φ : " + query.value(7).toString() + " mm";
    else
        info[5] = "Φ : N/A";
    if (query.value(8).toInt() != 0)
        info[6] = "Hood Temperature: " + query.value(8).toString() + " mm";
    else
        info[6] = "Hood Temperature: N/A";
    if (query.value(9).toInt() != -1)
        info[7] = "Set Terget Temperature : " + query.value(9).toString() + " degC";
    else
        info[7] = "Set Terget Temperature : N/A";
    if (query.value(10).toInt() != 0)
        info[8] = "Set Lens Temperature : " + query.value(10).toString() + " degC";
    else
        info[8] = "Set Lens Temperature : N/A";

    if (query.value(11).toInt() != 0)
        info[9] = "Set Plate Temperature : " + query.value(11).toString() + " degC";
    else
        info[9] = "Set Plate Temperature : N/A";
    if (query.value(12).toInt() != 0)
        info[10] =
                "Set Bolometer Temperature : " + query.value(12).toString() + " degC";
    else
        info[10] = "Set Bolometer Temperature : N/A";
    if (query.value(14).toInt() != 0)
        info[11] =
                "Set Peltier Temperature : " + query.value(14).toString() + " degC";
    else
        info[11] = "Set Peltier Temperature : N/A";

    info[12] = "Bolometer Temperature : " + query.value(15).toString() + " degC";
    info[13] = "Package Temperature : " + query.value(16).toString() + " degC";
    info[14] = "Case Temperature : " + query.value(17).toString() + " degC";
    info[15] = "Shutter Temperature : " + query.value(18).toString() + " degC";
    info[16] = "Lens Temperature : " + query.value(19).toString() + " degC";
    info[17] = "Filename(.img) : " + query.value(0).toString();
}

void Database::setValue() {

    if (T_min > query.value(9).toDouble()) {
        T_min = query.value(9).toDouble();
    }

    if (T_max < query.value(9).toDouble()) {
        T_max = query.value(9).toDouble();
    }

    if (B_min > query.value(15).toDouble()) {
        B_min = query.value(15).toDouble();
    }
    if (B_max < query.value(15).toDouble()) {
        B_max = query.value(15).toDouble();
    }

    if (P_min > query.value(16).toDouble()) {
        P_min = query.value(16).toDouble();
    }
    if (P_max < query.value(16).toDouble()) {
        P_max = query.value(16).toDouble();
    }

    if (C_min > query.value(17).toDouble()) {
        C_min = query.value(17).toDouble();
    }
    if (C_max < query.value(17).toDouble()) {
        C_max = query.value(17).toDouble();
    }

    if (S_min > query.value(18).toDouble()) {
        S_min = query.value(18).toDouble();
    }
    if (S_max < query.value(18).toDouble()) {
        S_max = query.value(18).toDouble();
    }

    if (L_min > query.value(19).toDouble()) {
        L_min = query.value(19).toDouble();
    }
    if (L_max < query.value(19).toDouble()) {
        L_max = query.value(19).toDouble();
    }
}

void Database::showThis() {
    this->show();
    this->activateWindow();
}

void Database::on_dbSearchList_clicked(const QModelIndex &index) {

    index.row();

    int c = 0;
    query.first();
    QListWidgetItem *item = ui->dbSearchList->currentItem();

    while (item->text() != query.value(0).toString()) {
        query.next();
        c++;
    }

    showdbinfo();
}

void Database::on_dbSearchButton_clicked() {
    double rangeL;
    double rangeH;

    query.first();
    ui->dbSearchList->clear();

    do {

        if (judgeItem()) {

            rangeL = ui->targetTl->text().toDouble();
            rangeH = ui->targetTh->text().toDouble();
            if (rangeL == 0 && rangeH == 0) {
                rangeL = MAXL;
                rangeH = MAXH;
            }

            if (rangeL <= query.value(9).toDouble() &&
                    rangeH >= query.value(9).toDouble()) {

                rangeL = ui->bolTl->text().toDouble();
                rangeH = ui->bolTh->text().toDouble();
                if (rangeL == 0 && rangeH == 0) {
                    rangeL = MAXL;
                    rangeH = MAXH;
                }

                if (rangeL <= query.value(15).toDouble() &&
                        rangeH >= query.value(15).toDouble()) {
                    rangeL = ui->pkgTl->text().toDouble();
                    rangeH = ui->pkgTh->text().toDouble();
                    if (rangeL == 0 && rangeH == 0) {
                        rangeL = MAXL;
                        rangeH = MAXH;
                    }

                    if (rangeL <= query.value(16).toDouble() &&
                            rangeH >= query.value(16).toDouble()) {
                        rangeL = ui->caseTl->text().toDouble();
                        rangeH = ui->caseTh->text().toDouble();
                        if (rangeL == 0 && rangeH == 0) {
                            rangeL = MAXL;
                            rangeH = MAXH;
                        }

                        if (rangeL <= query.value(17).toDouble() &&
                                rangeH >= query.value(17).toDouble()) {
                            rangeL = ui->shutTl->text().toDouble();
                            rangeH = ui->shutTh->text().toDouble();
                            if (rangeL == 0 && rangeH == 0) {
                                rangeL = MAXL;
                                rangeH = MAXH;
                            }

                            if (rangeL <= query.value(18).toDouble() &&
                                    rangeH >= query.value(18).toDouble()) {
                                rangeL = ui->lensTl->text().toDouble();
                                rangeH = ui->lensTh->text().toDouble();
                                if (rangeL == 0 && rangeH == 0) {
                                    rangeL = MAXL;
                                    rangeH = MAXH;
                                }

                                if (rangeL <= query.value(19).toDouble() &&
                                        rangeH >= query.value(19).toDouble()) {
                                    ui->dbSearchList->addItem(query.value(0).toString());
                                }
                            }
                        }
                    }
                }
            }
        }

    } while (query.next());

    createLogFile();
}

void Database::on_showInfoButton_clicked() { emit infoSignal(info); }

void Database::createLogFile() {

    QString date, time, str;
    QDir tmp;

    date = QDateTime::currentDateTime().date().toString(Qt::ISODate);
    time = QDateTime::currentDateTime().time().toString(Qt::ISODate);

    str = date + "_" + time + ".log";

    QFile log(str);

    log.open(QIODevice::WriteOnly);

    QTextStream out(&log);

    if (ui->All->isChecked())
        out << 1 << endl;
    else
        out << 0 << endl;

    if (ui->BB->isChecked())
        out << 1 << endl;
    else
        out << 0 << endl;

    if (ui->OilBB->isChecked())
        out << 1 << endl;
    else
        out << 0 << endl;

    if (ui->Winselwan->isChecked())
        out << 1 << endl;
    else
        out << 0 << endl;

    if (ui->MARAr->isChecked())
        out << 1 << endl;
    else
        out << 0 << endl;

    if (ui->Murray->isChecked())
        out << 1 << endl;
    else
        out << 0 << endl;

    if (ui->Murchison->isChecked())
        out << 1 << endl;
    else
        out << 0 << endl;

    if (ui->Sky->isChecked())
        out << 1 << endl;
    else
        out << 0 << endl;

    if (ui->ColliBB->isChecked())
        out << 1 << endl;
    else
        out << 0 << endl;

    if (1)
        out << 1 << endl;
    else
        out << 0 << endl;

    out << ui->bolTl->text() << endl;
    out << ui->bolTh->text() << endl;
    out << ui->pkgTl->text() << endl;
    out << ui->pkgTh->text() << endl;
    out << ui->caseTl->text() << endl;
    out << ui->caseTh->text() << endl;
    out << ui->shutTl->text() << endl;
    out << ui->shutTh->text() << endl;
    out << ui->lensTl->text() << endl;
    out << ui->lensTh->text() << endl;

    log.close();

    tmp.rename(log.fileName(), logFileDirectory.path() + "/" + str);
}

void Database::on_loadLogButton_clicked() {

    QString file = QFileDialog::getOpenFileName(this, tr("Open Log Image"),
                                                logFileDirectory.path(),
                                                tr("Image Files (*.log)"));

    QFile log(file);

    log.open(QIODevice::ReadOnly);

    QTextStream load(&log);
    QString str;

    for (int i = 0; !load.atEnd(); i++) {
        load >> str;

        switch (i) {
        case 0:
            ui->All->setChecked(str.toInt());
            break;
        case 1:
            ui->BB->setChecked(str.toInt());
            break;
        case 2:
            ui->OilBB->setChecked(str.toInt());
            break;
        case 3:
            ui->Winselwan->setChecked(str.toInt());
            break;
        case 4:
            ui->MARAr->setChecked(str.toInt());
            break;
        case 5:
            ui->Murray->setChecked(str.toInt());
            break;
        case 6:
            ui->Murchison->setChecked(str.toInt());
            break;
        case 7:
            ui->Sky->setChecked(str.toInt());
            break;
        case 8:
            ui->ColliBB->setChecked(str.toInt());
            break;

        case 9:
            ui->bolTl->setText(str);
            break;
        case 10:
            ui->bolTh->setText(str);
            break;
        case 11:
            ui->pkgTl->setText(str);
            break;
        case 12:
            ui->pkgTh->setText(str);
            break;
        case 13:
            ui->caseTl->setText(str);
            break;
        case 14:
            ui->caseTh->setText(str);
            break;
        case 15:
            ui->shutTl->setText(str);
            break;
        case 16:
            ui->shutTh->setText(str);
            break;
        case 17:
            ui->lensTl->setText(str);
            break;
        case 18:
            ui->lensTh->setText(str);
            break;

        default:
            break;
        }
    }

    log.close();
}

void Database::on_crearLogButton_clicked() {
    QStringList filelist;

    if (logFileDirectory.exists()) {
        filelist = logFileDirectory.entryList();
    }

    for (int i = 0; i < filelist.length(); i++) {
        logFileDirectory.remove(filelist.at(i));
    }
}

void Database::on_subPair_clicked() {
    QString ID;
    int searchID, pairID;
    QString file1, file2;

    on_dbSearchList_clicked(ui->dbSearchList->currentIndex());

    ID = query.value(13).toString();

    searchID = ID.toInt(0, 16);

    //std::cout << searchID << std::endl;

    if (searchID % 2)
        pairID = searchID + 1;

    else
        pairID = searchID - 1;

    pairQuery.first();

    while (1) {

        if (pairQuery.value(13).toString().toInt(0, 16) == pairID &&
                pairQuery.value(6).toString() == query.value(6).toString() &&
                pairQuery.value(2).toString() == query.value(2).toString())
            break;

        pairQuery.next();
    }

    qDebug() << searchID % 2;
    qDebug() << query.value(2).toString();
    qDebug() << query.value(0).toString();

    qDebug() << pairQuery.value(2).toString();
    qDebug() << pairQuery.value(0).toString();

    file1 = rawimagedirectorypath +
            query.value(2).toString() + "/" + query.value(0).toString();
    file2 = rawimagedirectorypath +
            pairQuery.value(2).toString() + "/" + pairQuery.value(0).toString();

    if (query.value(0).toString().length() != 23 ||
            query.value(0).toString().length() != 22 ||
            pairQuery.value(0).toString().length() != 23 ||
            pairQuery.value(0).toString().length() != 22) {

        QString fullname = query.value(0).toString() + query.value(2).toString();
        QString fname = fullname.mid(5, 8);

        QString fullname1 =
                pairQuery.value(0).toString() + pairQuery.value(2).toString();
        QString fname1 = fullname1.mid(5, 8);

        bool bl = fullname.contains("open", Qt::CaseInsensitive);
        bool bl1 = fullname.contains("close", Qt::CaseInsensitive);

        bool bl2 = fullname1.contains("open", Qt::CaseInsensitive);
        bool bl3 = fullname1.contains("close", Qt::CaseInsensitive);

        bool bl4 = fullname.contains("LIR", Qt::CaseInsensitive);
        bool bl5 = fullname.contains("ColliBB", Qt::CaseInsensitive);

        bool bl6 = fullname1.contains("LIR", Qt::CaseInsensitive);
        bool bl7 = fullname1.contains("ColliBB", Qt::CaseInsensitive);

        if (bl == true && bl4 == true && bl5 == true) {
            file1 = rawimagedirectorypath +
                    query.value(2).toString() + "/LIR_" + fname + "_open.img";
        }
        if (bl1 == true && bl4 == true && bl5 == true) {
            file1 = rawimagedirectorypath +
                    query.value(2).toString() + "/LIR_" + fname + "_close.img";
        }
        if (bl2 == true && bl6 == true && bl7 == true) {
            file2 = rawimagedirectorypath +
                    pairQuery.value(2).toString() + "/LIR_" + fname1 + "_open.img";
        }
        if (bl3 == true && bl6 == true && bl7 == true) {
            file2 = rawimagedirectorypath +
                    pairQuery.value(2).toString() + "/LIR_" + fname1 + "_close.img";
        }
    }

    if (searchID % 2)
        ui->thumbnailView->subtractImage(file2, file1);
    else
        ui->thumbnailView->subtractImage(file1, file2);
}

void Database::on_showGraph_clicked() {
    ui->thumbnailView->drawPixcelLineGraph(ui->heightValue->text());
}

void Database::loadInitFile() {

    QDir tmp, initialFileDirectory;

    QString appPath;

    appPath = QCoreApplication::applicationDirPath();
    initialFileDirectory.cd(appPath);

    qDebug() << initialFileDirectory.absolutePath();

    QFile iniFile(initialFileDirectory.absolutePath() + "/initial.txt");

    if (iniFile.exists()) {
        iniFile.open(QIODevice::ReadOnly);

        QTextStream load(&iniFile);

        load >> dataPath;

        iniFile.close();

        qDebug() << "ini file was loaded";
    }

    else {

        dataPath = QFileDialog::getExistingDirectory(this, "select data directory",
                                                     QDir::homePath());

        QFile ini(initialFileDirectory.absolutePath() + "/initial.txt");

        ini.open(QIODevice::WriteOnly);

        QTextStream out(&ini);

        out << dataPath << endl;

        ini.close();
    }
    emit getFilePathSignal(dataPath);
}

void Database::on_setTIRDirectory_clicked() {

    QDir tmp, initialFileDirectory;

    QString appPath;

    appPath = QCoreApplication::applicationDirPath();
    initialFileDirectory.cd(appPath);

    qDebug() << initialFileDirectory.absolutePath();

    QFile iniFile(initialFileDirectory.absolutePath() + "/initial.txt");

    dataPath = QFileDialog::getExistingDirectory(this, "select data directory",
                                                 QDir::homePath());

    QFile ini(initialFileDirectory.absolutePath() + "/initial.txt");

    ini.open(QIODevice::WriteOnly);

    QTextStream out(&ini);

    out << dataPath << endl;

    ini.close();

    emit getFilePathSignal(dataPath);
}
