//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//

#include "mainwindow.h"
#include "showimage.h"
#include "pixcelgraph.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <math.h>
#include <CCfits/FITS.h>
//#include <PHDU.h>
#include <CCfits/PHDUT.h>
//#include <FITSUtil.h>
#include <CCfits/FITSUtilT.h>
#include <controlgraphpanel.h>
#include <calibrationgraph.h>
#include <QDir>
#include <Showimage.h>
#include "dataset.h"
#include <time.h>

#define Max_Height 768
#define Max_Width 1024


using namespace std;
using namespace CCfits;

int effectivecounter;

ShowImage::ShowImage(QWidget *parent) : QOpenGLWidget(parent) {
    MAX_V = 0;
    MIN_V = 100000;
    judge = true;
    renderunitflag = true;
    int Width_i;
    int Height_i;
    //QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    /*
    db.setHostName("localhost");
    db.setUserName("root");
    db.setDatabaseName("HEAT_DB");
    */
    bool ok = db.open();
    /*
    if(ok==true){
        double t5,m7;
        int i;
        int mmm=328*248;
        clock_t start1 = clock();
        for(i=0;i<mmm;i++){
            t5 = rand() % 250;
            //m7 = gettemperature(t5);
            //cout<<"1st - ラジアンス "<<t5<<" - "<<"温度 "<<gettemperature(t5)<<endl;
        }
        clock_t end1 = clock();
        clock_t start2 = clock();
        for(i=0;i<mmm;i++){
            t5 = rand() % 250;
            m7 = rad2temp(t5,1,db);
            //cout<<"2nd - ラジアンス "<<t5<<" - "<<"温度 "<<rad2temp(t5,1,db)<<endl;
        }
        clock_t end2 = clock();

        cout << "1st try - "<<(double)(end1-start1) / (CLOCKS_PER_SEC) << "sec" <<endl;
        cout << "2nd try - "<<(double)(end2-start2) / (CLOCKS_PER_SEC) << "sec" <<endl;
    }
    else qDebug()<<"DB is not open by showimage.cpp"<<db.lastError();
*/
    colorselect = 0;

    h_planck = 6.62606957e-34;
    kB = 1.3806488e-23;
    c_speed = 299792458;
    c1 = 2 * M_PI * h_planck * pow(c_speed, 2);
    c2 = (h_planck * c_speed) / kB;
    SIGMA = 5.670373e-8;
    loadFilter();
}

ShowImage::~ShowImage() {}

void ShowImage::initializeGL() {
    glClearColor(0, 0, 0, 0);
    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
}

void ShowImage::initializedarkimage(){
    for(int i=0; i<Height_i; i++){
        for(int j=0; j<Width_i; j++){
            darkimage[i][j]=-1000;
        }
    }
}

void ShowImage::resizeGL(int width, int height) {
    glViewport(0, 0, width, height);
    glLoadIdentity();

    glOrtho(-10, Width_i + 100, Height_i + 10, -10, -1, 1);
}

void ShowImage::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    render();
}

void ShowImage::render() {

    double T = MAX_V - MIN_V;

    if (judge == true) {

        glBegin(GL_POINTS);

        for (int i = 0; i < xmldata.l1.Height_data; i++) {
            for (int j = 0; j < 15; j++) {
                glColor3dv(colorTable[255 - i]);
                glVertex2d(j + getWidth()+15, i);
            }
        }

        glEnd();

        QPainter num(this);
        num.setPen(Qt::cyan);
        num.setFont(QFont("Arial", 15));

        if (bunit=="adu") {
            num.drawText(getWidth()+50, 53, "DN");
        } else {
            num.drawText(getWidth()+50, 53, "K");
        }

        num.drawText(getWidth()+43, 24, QString::number(MAX_V));
        num.drawText(getWidth()+43, 85, QString::number((int)(MAX_V - T / 4)));
        num.drawText(getWidth()+43, 146, QString::number((int)(MAX_V - T / 2)));
        num.drawText(getWidth()+43, 206, QString::number((int)(MAX_V - T * 3 / 4)));
        num.drawText(getWidth()+43, 265, QString::number(MIN_V));
        num.end();
    }

    glBegin(GL_POINTS);

    pixelDraw(T);
    glEnd();
}

void ShowImage::mousePressEvent(QMouseEvent *event) {

    if (event->buttons() & Qt::LeftButton) {

        if (event->x() >= 10 && event->x() <= 394 && event->y() <= 266 &&
                event->y() >= 10) {
            setValueX(QString::number(event->x() - 10));
            setValueY(QString::number(event->y() - 10));
            setValuePixel(QString::number(image[event->y() - 10][event->x() - 10]));
        }
    }

    if (event->buttons() & Qt::RightButton) {

        if (judge != true)
            judge = true;
        else if (judge == true)
            judge = false;
        this->update();
    }
    if (!rubberBand_) {
        rubberBand_ = new QRubberBand(QRubberBand::Rectangle, this);
    }
    else{
        rubberBand_->hide();
    }

    startPos_ = event->pos();
    rubberBand_->setGeometry(QRect(startPos_, QSize()));
    rubberBand_->show();
}

void ShowImage::mouseMoveEvent(QMouseEvent *event){
    endPos = event->pos();
    rubberBand_->setGeometry(QRect(startPos_, endPos).normalized());
}

void ShowImage::mouseReleaseEvent(QMouseEvent *event){
    Q_UNUSED(event)

    //Hide QRubberBand
    //rubberBand_->hide();

    // Output area
    //  qDebug() << rubberBand_->geometry();

    //cout<<endPos.x()-26<<endl;
    //cout<<endPos.y()-16<<endl;
}

void ShowImage::loadFileName(QString name) {
    MAX_V = 0;
    MIN_V = 100000;
    Width_i = xmldata.l1.Width_data;
    Height_i = xmldata.l1.Height_data;
    filename = name;

    loadBuffer();

    makeColorTable();

    this->update();
}

string ShowImage::ReplaceString(string String1, std::string String2,
                                std::string String3) {
    std::string::size_type Pos(String1.find(String2));

    while (Pos != std::string::npos) {
        String1.replace(Pos, String2.length(), String3);
        Pos = String1.find(String2, Pos + String3.length());
    }

    return String1;
}

void ShowImage::loadBuffer() {
    fstream ifs;
    ifs.open(&filename.toStdString()[0], ios::in | ios::binary);

    QFileInfo fileinfo;
    fileinfo.setFile(filename);
    QString ext = fileinfo.suffix();
    ext = ext.toLower();

    if (ext == "fit" || ext == "fits" || ext == "fts") {
        fitsflag = 1;
        valarray<double> contents;
        auto_ptr<FITS> pInfile(0);

//        if(Width_i>Max_Width || Height_i>Max_Height){
//            QMessageBox msgBox;
//            msgBox.setText("The image data size is too large.");
//            msgBox.exec();
//            return;
//        }


        try {
            pInfile.reset(new FITS(filename.toStdString().c_str(), Read, true));
            PHDU &fitsImage = pInfile->pHDU();
            fitsImage.read(contents);
            fitsImage.readAllKeys();
            Width_i = pInfile->pHDU().axis(0);
            Height_i = pInfile->pHDU().axis(1);
            this->setHeight(Height_i);
            this->setWidth(Width_i);
            this->resize(getWidth()+110,getHeight()+20);
            //qDebug()<<"Width_i = "<<Width_i<<" Height = "<<Height_i;
            if(Width_i>Max_Width || Height_i>Max_Height){
                //qDebug()<<"Width_Max = "<<Max_Width<<" Height_Max = "<<Max_Height;
                QMessageBox msgBox;
                msgBox.setText("The image data size is too large.");
                msgBox.exec();
                return;
            }

            try {
                pInfile->pHDU().readKey<string>("ORIGIN", origin);
                printf("Read ORIGIN\n");
            } catch (...) {
                printf("Can not read ORIGIN\n");
            };
            try {
                pInfile->pHDU().readKey<string>("BUNIT", bunit);
                printf("Read BUNIT\n");
            } catch (...) {
                printf("Can not read BUNIT\n");
            };
            try {
                pInfile->pHDU().readKey<string>("DATE", date);
                printf("Read DATE\n");

            } catch (...) {
                printf("Can not read DATE\n");
            };
            try {
                pInfile->pHDU().readKey<string>("DATE-BEG", date_beg);

                printf("Read DATE-BEG\n");
            } catch (...) {
                printf("Can not read DATE-BEG\n");
            };
            try {
                pInfile->pHDU().readKey<string>("DATE-OBS", date_obs);

                printf("Read DATE-OBS\n");
            } catch (...) {
                printf("Can not read DATE-OBS\n");
            };
            try {
                pInfile->pHDU().readKey<string>("DATE-END", date_end);

                printf("Read DATE-END\n");
            } catch (...) {
                printf("Can not read DATE-END\n");
            };
            try {
                pInfile->pHDU().readKey<string>("TELESCOP", telescop);

                printf("Read TELESCOP\n");
            } catch (...) {
                printf("Can not read TELESCOP\n");
            };
            try {
                pInfile->pHDU().readKey<string>("INSTRUME", instrume);

                printf("Read INSTRUME\n");
            } catch (...) {
                printf("Can not read INSTRUME\n");
            };
            try {
                pInfile->pHDU().readKey<string>("OBJECT", object);

                printf("Read OBJECT\n");
            } catch (...) {
                printf("Can not read OBJECT\n");
            };
            try {
                pInfile->pHDU().readKey<double>("XPOSURE", xposure);

                printf("Read XPOSURE\n");
            } catch (...) {
                printf("Can not read XPOSURE\n");
            };
            try {
                pInfile->pHDU().readKey<double>("IFOV", ifov);

                printf("Read IFOV\n");
            } catch (...) {
                printf("Can not read IFOV\n");
            };
            try {
                pInfile->pHDU().readKey<string>("FILTER", filter);

                printf("Read FILTER\n");
            } catch (...) {
                printf("Can not read FILTER\n");
            };
            try {
                pInfile->pHDU().readKey<string>("OPRGNAME", oprgname);

                printf("Read OPRGNAME\n");
            } catch (...) {
                printf("Can not read OPRGNAME\n");
            };
            try {
                pInfile->pHDU().readKey<string>("OPRGNO", oprgno);

                printf("Read OPRGNO\n");
            } catch (...) {
                printf("Can not read OPRGNO\n");
            };
            try {
                pInfile->pHDU().readKey<double>("ROI_LLX", roi_llx);

                printf("Read ROI_LLX\n");
            } catch (...) {
                printf("Can not read ROI-LLX\n");
            };
            try {
                pInfile->pHDU().readKey<double>("ROI_LLY", roi_lly);

                printf("Read ROI_LLY\n");
            } catch (...) {
                printf("Can not read ROI-LLY\n");
            };
            try {
                pInfile->pHDU().readKey<double>("ROI_URX", roi_urx);

                printf("Read ROI_URX\n");
            } catch (...) {
                printf("Can not read ROI_URX\n");
            };
            try {
                pInfile->pHDU().readKey<double>("ROI_URY", roi_ury);

                printf("Read ROI_URY\n");
            } catch (...) {
                printf("Can not read ROI_URY\n");
            };
            try {
                pInfile->pHDU().readKey<double>("DATAMAX", datamax);

                printf("Read DATAMAX\n");
            } catch (...) {
                printf("Can not read DATAMAX\n");
            };
            try {
                pInfile->pHDU().readKey<double>("DATAMIN", datamin);

                printf("Read DATAMIN\n");
            } catch (...) {
                printf("Can not read DATAMIN\n");
            };
            try {
                pInfile->pHDU().readKey<double>("MEAN", mean);

                printf("Read MEAN\n");
            } catch (...) {
                printf("Can not read MEAN\n");
            };
            try {
                pInfile->pHDU().readKey<double>("STDEV", stdev);

                printf("Read STDEV\n");
            } catch (...) {
                printf("Can not read STDEV\n");
            };
            try {
                pInfile->pHDU().readKey<double>("MISS_VAL", miss_val);

                printf("Read MISS_VAL\n");
            } catch (...) {
                printf("Can not read MISS_VAL\n");
            };
            try {
                pInfile->pHDU().readKey<double>("MISS_NUM", miss_num);

                printf("Read MISS_NUM\n");
            } catch (...) {
                printf("Can not read MISS_NUM\n");
            };
            try {
                pInfile->pHDU().readKey<double>("DEAD_VAL", dead_val);

                printf("Read DEAD_VAL\n");
            } catch (...) {
                printf("Can not read DEAD_VAL\n");
            };
            try {
                pInfile->pHDU().readKey<double>("DEAD_NUM", dead_num);

                printf("Read DEAD_NUM\n");
            } catch (...) {
                printf("Can not read DEAD_NUM\n");
            };
            try {
                pInfile->pHDU().readKey<double>("SATU_VAL", satu_val);

                printf("Read SATU_VAL\n");
            } catch (...) {
                printf("Can not read SATU_VAL\n");
            };
            try {
                pInfile->pHDU().readKey<double>("SATU_NUM", satu_num);

                printf("Read SATU_NUM\n");
            } catch (...) {
                printf("Can not read SATU_NUM\n");
            };
            try {
                pInfile->pHDU().readKey<string>("IMGCMPRV", imgcmprv);

                printf("Read IMGCMPRV\n");
            } catch (...) {
                printf("Can not read IMGCMPRV\n");
            };
            try {
                pInfile->pHDU().readKey<string>("IMGCMPAL", imgcmpal);

                printf("Read IMGCMPAL\n");
            } catch (...) {
                printf("Can not read IMGCMPAL\n");
            };

            try {
                pInfile->pHDU().readKey<string>("IMGCMPPR", imgcmppr);

                printf("Read IMGCMPPR\n");
            } catch (...) {
                printf("Can not read IMGCMPPR\n");
            };
            try {
                pInfile->pHDU().readKey<double>("IMG_ERR", img_err);

                printf("Read IMG_ERR\n");
            } catch (...) {
                printf("Can not read IMG_ERR\n");
            };
            try {
                pInfile->pHDU().readKey<string>("IMGSEQC", imgseqc);

                printf("Read IMGSEQC\n");
            } catch (...) {
                printf("Can not read IMGSEQC\n");
            };
            try {
                pInfile->pHDU().readKey<double>("IMGACCM", imgaccm);

                printf("Read IMGACCM\n");
            } catch (...) {
                printf("Can not read IMGACCM\n");
            };
            try {
                pInfile->pHDU().readKey<double>("BITDEPTH", bitdepth);

                printf("Read BITDEPTH\n");
            } catch (...) {
                printf("Can not read BITDEPTH\n");
            };
            try {
                pInfile->pHDU().readKey<string>("PLT_POW", plt_pow);

                printf("Read PLT_POW\n");
            } catch (...) {
                printf("Can not read PLT_POW\n");
            };
            try {
                pInfile->pHDU().readKey<string>("PLT_STAT", plt_stat);

                printf("Read PLT_STAT\n");
            } catch (...) {
                printf("Can not read PLT_STAT\n");
            };
            try {
                pInfile->pHDU().readKey<string>("BOL_STAT", bol_stat);

                printf("Read BOL_STAT\n");
            } catch (...) {
                printf("Can not read BOL_STAT\n");
            };
            try {
                pInfile->pHDU().readKey<double>("BOL_TRGT", bol_trgt);

                printf("Read BOL_TRGT\n");
            } catch (...) {
                printf("Can not read BOL_TRGT\n");
            };
            try {
                pInfile->pHDU().readKey<double>("BOL_RANG", bol_rang);

                printf("Read BOL_RANG\n");
            } catch (...) {
                printf("Can not read BOL_RANG\n");
            };
            try {
                pInfile->pHDU().readKey<double>("BOL_TEMP", bol_temp);

                printf("Read BOL_TEMP\n");
            } catch (...) {
                printf("Can not read BOL_TEMP\n");
            };
            try {
                pInfile->pHDU().readKey<double>("PKG_TEMP", pkg_temp);

                printf("Read PKG_TEMP\n");
            } catch (...) {
                printf("Can not read PKG_TEMP\n");
            };
            try {
                pInfile->pHDU().readKey<double>("CAS_TEMP", cas_temp);

                printf("Read CAS_TEMP\n");
            } catch (...) {
                printf("Can not read CAS_TEMP\n");
            };
            try {
                pInfile->pHDU().readKey<double>("SHT_TEMP", sht_temp);

                printf("Read SHT_TEMP\n");
            } catch (...) {
                printf("Can not read SHT_TEMP\n");
            };
            try {
                pInfile->pHDU().readKey<double>("LEN_TEMP", len_temp);

                printf("Read LEN_TEMP\n");
            } catch (...) {
                printf("Can not read LEN_TEMP\n");
            };
            try {
                pInfile->pHDU().readKey<double>("BGR_VOL", bgr_vol);

                printf("Read BGR_VOL\n");
            } catch (...) {
                printf("Can not read BGR_VOL\n");
            };
            try {
                pInfile->pHDU().readKey<double>("VB1_VOL", vb1_vol);

                printf("Read VB1_VOL\n");
            } catch (...) {
                printf("Can not read VB1_VOL\n");
            };

            try {
                pInfile->pHDU().readKey<double>("ADOFSVOL", adofsvol);

                printf("Read ADOFSVOL\n");
            } catch (...) {
                printf("Can not read ADOFSVOL\n");
            };
            try {
                pInfile->pHDU().readKey<double>("HCE_TEMP", hce_temp);

                printf("Read HCE_TEMP\n");
            } catch (...) {
                printf("Can not read HCE_TEMP\n");
            };
            try {
                pInfile->pHDU().readKey<double>("PNL_TEMP", pnl_temp);

                printf("Read PNL_TEMP\n");
            } catch (...) {
                printf("Can not read PNL_TEMP\n");
            };
            try {
                pInfile->pHDU().readKey<double>("AE_TEMP", ae_temp);

                printf("Read AE_TEMP\n");
            } catch (...) {
                printf("Can not read AE_TEMP\n");
            };
            try {
                pInfile->pHDU().readKey<double>("S_DISTHT", s_distht);

                printf("Read S_DISTHT\n");
            } catch (...) {
                printf("Can not read S_DISTHT\n");
            };
            try {
                pInfile->pHDU().readKey<double>("S_DISTHE", s_disthe);

                printf("Read S_DISTHE\n");
            } catch (...) {
                printf("Can not read S_DISTHE\n");
            };
            try {
                pInfile->pHDU().readKey<double>("S_DISTHS", s_disths);

                printf("Read S_DISTHS\n");
            } catch (...) {
                printf("Can not read S_DISTHS\n");
            };
            try {
                pInfile->pHDU().readKey<double>("S_DISTTS", s_distts);

                printf("Read S_DISTTS\n");
            } catch (...) {
                printf("Can not read S_DISTTS\n");
            };
            try {
                pInfile->pHDU().readKey<double>("S_TGRADI", s_tgradi);

                printf("Read S_TGRADI\n");
            } catch (...) {
                printf("Can not read S_TGRADI\n");
            };
            try {
                pInfile->pHDU().readKey<double>("S_APPDIA", s_appdia);

                printf("Read S_APPDIA\n");
            } catch (...) {
                printf("Can not read S_APPDIA\n");
            };
            try {
                pInfile->pHDU().readKey<double>("S_SOLLAT", s_sollat);

                printf("Read S_SOLLAT\n");
            } catch (...) {
                printf("Can not read S_SOLLAT\n");
            };
            try {
                pInfile->pHDU().readKey<double>("S_SOLLON", s_sollon);

                printf("Read S_SOLLON\n");
            } catch (...) {
                printf("Can not read S_SOLLON\n");
            };
            try {
                pInfile->pHDU().readKey<double>("S_SSCLAT", s_ssclat);

                printf("Read S_SSCLAT\n");
            } catch (...) {
                printf("Can not read S_SSCLAT\n");
            };
            try {
                pInfile->pHDU().readKey<double>("S_SSCLON", s_ssclon);

                printf("Read S_SSCLON\n");
            } catch (...) {
                printf("Can not read S_SSCLON\n");
            };
            try {
                pInfile->pHDU().readKey<double>("S_SSCLST", s_ssclst);

                printf("Read S_SSCLST\n");
            } catch (...) {
                printf("Can not read S_SSCLST\n");
            };
            try {
                pInfile->pHDU().readKey<double>("S_SSCPX", s_sscpx);

                printf("Read S_SSCPX\n");
            } catch (...) {
                printf("Can not read S_SSCPX\n");
            };
            try {
                pInfile->pHDU().readKey<double>("S_SSCPY", s_sscpy);

                printf("Read S_SSCPY\n");
            } catch (...) {
                printf("Can not read S_SSCPY\n");
            };
            try {
                pInfile->pHDU().readKey<double>("S_SCXSAN", s_scxsan);

                printf("Read S_SCXSAN\n");
            } catch (...) {
                printf("Can not read S_SCXSAN\n");
            };
            try {
                pInfile->pHDU().readKey<double>("S_SCYSAN", s_scysan);

                printf("Read S_SCYSAN\n");
            } catch (...) {
                printf("Can not read S_SCYSAN\n");
            };
            try {
                pInfile->pHDU().readKey<double>("S_SCZSAN", s_sczsan);

                printf("Read S_SCZSAN\n");
            } catch (...) {
                printf("Can not read S_SCZSAN\n");
            };
            try {
                pInfile->pHDU().readKey<string>("NAIFNAME", naifname);

                printf("Read NAIFNAME\n");
            } catch (...) {
                printf("Can not read NAIFNAME\n");
            };
            try {
                pInfile->pHDU().readKey<double>("NAIFID", naifid);

                printf("Read NAIFID\n");
            } catch (...) {
                printf("Can not read NAIFID\n");
            };
            try {
                pInfile->pHDU().readKey<string>("MKNAME", mkname);

                printf("Read MKNAME\n");
            } catch (...) {
                printf("Can not read MKNAME\n");
            };
            try {
                pInfile->pHDU().readKey<double>("VERSION", version);

                printf("Read VERSION\n");
            } catch (...) {
                printf("Can not read VERSION\n");
            };
            try {
                pInfile->pHDU().readKey<double>("PKG_TEMP", pkgt);

                printf("Read PKG_TEMP\n");
            } catch (...) {
                printf("Can not read PKG_TEMP\n");
            };
            try {
                pInfile->pHDU().readKey<double>("CAS_TEMP", cast);

                printf("Read CAS_TEMP\n");
            } catch (...) {
                printf("Can not read CAS_TEMP\n");
            };
            try {
                pInfile->pHDU().readKey<double>("SHT_TEMP", shtt);

                printf("Read SHT_TEMP\n");
            } catch (...) {
                printf("Can not read SHT_TEMP\n");
            };
            try {
                pInfile->pHDU().readKey<double>("LEN_TEMP", lent);

                printf("Read LEN_TEMP\n");
            } catch (...) {
                printf("Can not read LEN_TEMP\n");
            };
            try {
                pInfile->pHDU().readKey<double>("BITDEPTH", bitdepth);

                printf("Read BITDEPTH\n");
            } catch (...) {
                printf("Can not read BITDEPTH\n");
            };

            double DNtmp1=0;
            double fitsave=0;
            int counter=0;
            for(int i=0; i<Height_i; i++){
                for(int j=0; j<Width_i; j++){
                    DNtmp1=contents[counter];
                    fitsave+=DNtmp1;
                    counter++;
                }
            }


            fitsave=fitsave/(Height_i*Width_i);

            counter=0;
            double tmp1=0;


            for(int i=0; i<Height_i; i++){
                for(int j=0; j<Width_i; j++){
                    tmp1=contents[counter];
                    if(fitsave<-700){
                        tmp1=tmp1/8;
                    }

                    if(tmp1>MAX_V)
                        MAX_V=tmp1;
                    if(tmp1<MIN_V)
                        MIN_V=tmp1;
                    //image[Height_i-1-i][j]=tmp1;
                    image[i][j]=tmp1;

                    counter++;
                }
            }
        }

        catch (FITS::CantCreate) {
            cout << "Can't open fits image file" << endl;
            return;
        }
    }

    else if (ext == "txt" || ext == "dat") {
        QFile file1(fileinfo.filePath());
        if (!file1.open(QIODevice::ReadOnly)) {
            printf("txt dat open error\n");
            return;
        }

        QTextStream in1(&file1);
        QString str1;

        for (int i = 0; !in1.atEnd(); i++) {
            for (int j = 0; j < 4; j++) {
                in1 >> str1;

                imagetmp[i][j] = str1.toDouble();
            }
        }

        int k = 0;
        for (int i = 0; i < xmldata.l1.Width_data; i++) {
            for (int j = 0; j < xmldata.l1.Height_data; j++) {
                image[xmldata.l1.Height_data - 1 - j][i] = imagetmp[k][3];
                if (image[xmldata.l1.Height_data - 1 - j][i] > MAX_V)
                    MAX_V = image[xmldata.l1.Height_data - 1 - j][i];
                if (image[xmldata.l1.Height_data - 1 - j][i] < MIN_V)
                    MIN_V = image[xmldata.l1.Height_data - 1 - j][i];

                k++;
            }
        }
    }

    else{
        int Count( 0 );
        int i=0;
        int j=0;
        char Data;
        unsigned short tmp;

        this->setWidth(xmldata.l1.Width_data);
        this->setHeight(xmldata.l1.Height_data);
        this->resize(getWidth()+100,getHeight()+20);

        while( !ifs.fail() )
        {
            if( Count >= xmldata.l1.Width_data*xmldata.l1.Height_data*2)  break;

            ifs.read( &Data,1);
            tmp=0xff&Data;
            ++Count;

            ifs.read( &Data,1);


            tmp+=(0xff&Data)*xmldata.l1.Height_data;
            if(tmp==-9999)tmp=0;
            if(tmp>MAX_V)MAX_V=tmp;
            if(tmp<MIN_V)MIN_V=tmp;
            image[i][j]=(double)tmp;
            j++;

            if( Count % (xmldata.l1.Width_data*2) == xmldata.l1.Height_data*2-1 ){
                i++;
                j=0;
            }
            ++Count;
        }
        bunit="adu";
        ifs.close();
    }
}

void ShowImage::pixelDraw(double T) {

    double d = T / 256;

    for (int i = 0; i < 256; i++) {
        colorValue[i] = d * i + MIN_V;
    }

    colorValue[255] = MAX_V + 1;

    for (int i = 0; i < Height_i; i++) {
        for (int j = 0; j < Width_i; j++) {
            for (int k = 0; k < 256; k++) {
                if (image[i][j] < colorValue[k]) {
                    glColor3dv(colorTable[k]);
                    glVertex2d(j, i);
                    break;
                }
            }
        }
    }
}

void ShowImage::setWidth(int a)
{
    Width_i = a;
}

void ShowImage::setHeight(int a)
{
    Height_i = a;
}

int ShowImage::getWidth()
{
    return Width_i;
}

int ShowImage::getHeight()
{
    return Height_i;
}

void ShowImage::makeColorTable() {
    int colorArea;
    double count = 1;

    if (colorselect == 0 || fitsflag == 1) {
        colorArea = 51;
        for (int i = 0; i < 768; i++) {
            if (count > colorArea)
                count = 1;
            if (i == 767)
                count = 52;

            if (i < colorArea) {
                colorTable[i][0] = 1 - count / (colorArea + 1);
                colorTable[i][1] = 0;
                colorTable[i][2] = 1;
            }
            if (colorArea <= i && i < 2 * colorArea) {
                colorTable[i][0] = 0;
                colorTable[i][1] = count / (colorArea + 1);
                colorTable[i][2] = 1;
            }
            if (2 * colorArea <= i && i < 3 * colorArea) {
                colorTable[i][0] = 0;
                colorTable[i][1] = 1;
                colorTable[i][2] = 1 - count / (colorArea + 1);
            }
            if (3 * colorArea <= i && i < 4 * colorArea) {
                colorTable[i][0] = count / (colorArea + 1);
                colorTable[i][1] = 1;
                colorTable[i][2] = 0;
            }
            if (4 * colorArea <= i && i < 5 * colorArea + 1) {
                colorTable[i][0] = 1;
                colorTable[i][1] = 1 - count / (colorArea + 2);
                colorTable[i][2] = 0;
            }
            count++;
        }
    }

    if (colorselect == 1)
        for (int i = 0; i < 768; i++) {
            colorTable[i][0] = (double)i / 767;
            colorTable[i][1] = (double)i / 767;
            colorTable[i][2] = (double)i / 767;
        }

    if (colorselect == 2) {
        colorArea = 64;
        for (int i = 0; i < 768; i++) {
            if (count > colorArea)
                count = 1;

            if (i < colorArea) {
                colorTable[i][0] = 0;
                colorTable[i][1] = 0;
                colorTable[i][2] = count / (colorArea + 1);
            }
            if (colorArea <= i && i < 2 * colorArea) {
                colorTable[i][0] = count / (colorArea + 1);
                colorTable[i][1] = 0;
                colorTable[i][2] = 1 - count / (colorArea + 1);
            }
            if (2 * colorArea <= i && i < 3 * colorArea) {
                colorTable[i][0] = 1;
                colorTable[i][1] = count / (colorArea + 1);
                colorTable[i][2] = 0;
            }
            if (3 * colorArea <= i && i < 4 * colorArea) {
                colorTable[i][0] = 1;
                colorTable[i][1] = 1;
                colorTable[i][2] = count / (colorArea + 1);
            }

            count++;
        }
    }
}

void ShowImage::setValueX(QString value) {
    int tmp = value.toInt() - 16;
    emit valueChangedX(QString::number(tmp));
}

void ShowImage::setValueY(QString value) {
    int tmp = value.toInt() - 6;
    emit valueChangedY(QString::number(tmp));
}

void ShowImage::setValuePixel(QString value) { emit valueChangedPixel(value); }

void ShowImage::outputCurrentImage(){
    long naxis    =   2;
    long naxes[2] = { getWidth(), getHeight() };
    std::auto_ptr<FITS> pFits(0);
    std::auto_ptr<FITS> pFits1(0);
    try
    {
        QString fitdirectory= QFileDialog::getExistingDirectory(this, tr("Select the directory to save the image"),"/Applications/HEAT_DATA");
        if(fitdirectory == ""){
            return;
        }

        const std::string fileNametemp(fitdirectory.toStdString()+"/"+"FITSoutput.fit");
        pFits.reset( new FITS(fileNametemp,DOUBLE_IMG , naxis , naxes ) );

    }
    catch (FITS::CantCreate)
    {
        return;
    }

    int nelements(1);
    nelements = std::accumulate(&naxes[0],&naxes[naxis],1,std::multiplies<double>());
    std::valarray<double> arraytemp(nelements);

    int k=0;
    if(bunit == "K"){
        for (int i = 0; i < getHeight(); i++){
            for(int j=0;j<getWidth();j++){
                arraytemp[k]=fitstemperature2[i][j];
                k++;
            }
        }
    }
    else{
        for (int i = 0; i < getHeight(); i++){
            for(int j=0;j<getWidth();j++){
                arraytemp[k]=image[i][j];
                k++;
            }
        }
    }
    int  fpixel(1);
    pFits->pHDU().addKey("ORIGIN",origin,"organization responsible for the data");
    pFits->pHDU().addKey("DATE",date,"date of generation of HDU in UTC");
    pFits->pHDU().addKey("DATE-BEG",date_beg,"start date of observation program (UTC)");
    pFits->pHDU().addKey("DATE-OBS",date_obs,"start date of observation (UTC)");
    pFits->pHDU().addKey("DATE-END",date_end,"end date of observation (UTC)");
    pFits->pHDU().addKey("TELESCOP",telescop,"telescope used to acquire data");
    pFits->pHDU().addKey("INSTRUME",instrume,"name of instrument");
    pFits->pHDU().addKey("OBJECT",object,"name of observed object");
    pFits->pHDU().addKey("BUNIT",bunit,"physical unit of array values");
    pFits->pHDU().writeComment("");
    pFits->pHDU().writeComment("***** TIR Common Information *****");
    pFits->pHDU().writeComment("");
    pFits->pHDU().addKey("XPOSURE",QString::number(xposure).toStdString(),"exposure time [sec]");
    pFits->pHDU().addKey("IFOV",QString::number(ifov).toStdString(),"instantaneous field of view [rad]");
    pFits->pHDU().addKey("FILTER",filter,"bandpath range of filter (um)");
    pFits->pHDU().writeComment("");
    pFits->pHDU().writeComment("***** Observation Program *****");
    pFits->pHDU().writeComment("");
    pFits->pHDU().addKey("OPRGNAME",oprgname,"observation program name");
    pFits->pHDU().addKey("OPRGNO",oprgno,"observation program number");
    pFits->pHDU().writeComment("");
    pFits->pHDU().writeComment("***** TIR Image Information *****");
    pFits->pHDU().writeComment("");
    pFits->pHDU().addKey("ROI_LLX",QString::number(roi_llx).toStdString(),"x lower-left corner pixel of image ");
    pFits->pHDU().addKey("ROI_LLY",QString::number(roi_lly).toStdString(),"y lower-left corner pixel of image");
    pFits->pHDU().addKey("ROI_URX",QString::number(roi_urx).toStdString(),"x upper-right corner pixel of image");
    pFits->pHDU().addKey("ROI_URY",QString::number(roi_ury).toStdString(),"y upper-right corner pixel of image");
    pFits->pHDU().addKey("DATAMAX",QString::number(datamax).toStdString(),"maximum data value");
    pFits->pHDU().addKey("DATAMIN",QString::number(datamin).toStdString(),"minimum data value");
    pFits->pHDU().addKey("MEAN",QString::number(mean).toStdString(),"mean value of the data");
    pFits->pHDU().addKey("STDEV",QString::number(stdev).toStdString(),"standard deviation of the data");
    pFits->pHDU().addKey("MISS_VAL",QString::number(miss_val).toStdString(),"flag value of missing pixel");
    pFits->pHDU().addKey("MISS_NUM",QString::number(miss_num).toStdString(),"number of missing pixel");
    pFits->pHDU().addKey("DEAD_VAL",QString::number(dead_val).toStdString(),"flag value of dead pixel");
    pFits->pHDU().addKey("DEAD_NUM",QString::number(dead_num).toStdString(),"number of dead pixel");
    pFits->pHDU().addKey("SATU_VAL",QString::number(satu_val).toStdString(),"flag value of saturated pixel");
    pFits->pHDU().addKey("SATU_NUM",QString::number(satu_num).toStdString(),"number of saturated pixels");
    pFits->pHDU().addKey("IMGCMPRV",imgcmprv,"compression rev.: RAW_DAT/LOSSLESS/LOSSY");
    pFits->pHDU().addKey("IMGCMPAL",imgcmpal,"compression alg.: JPEG2000/STAR_PIXEL");
    pFits->pHDU().addKey("IMGCMPPR",imgcmppr,"compression parameter");
    pFits->pHDU().addKey("IMG_ERR",QString::number(img_err).toStdString(),"onboard image proc. return status");
    pFits->pHDU().writeComment("");
    pFits->pHDU().writeComment("***** TIR Telemetry *****");
    pFits->pHDU().writeComment("");
    pFits->pHDU().addKey("IMGSEQC",imgseqc,"image sequence counter");
    pFits->pHDU().addKey("IMGACCM",QString::number(imgaccm).toStdString(),"number of accumulated images");
    pFits->pHDU().addKey("BITDEPTH",QString::number(bitdepth).toStdString(),"image bit depth");
    pFits->pHDU().addKey("PLT_POW",plt_pow,"peltier ON/OFF");
    pFits->pHDU().addKey("PLT_STAT",plt_stat,"peltier status");
    pFits->pHDU().addKey("BOL_STAT",bol_stat,"bolometer status");
    pFits->pHDU().addKey("BOL_TRGT",QString::number(bol_trgt).toStdString(),"bolometer calibration target");
    pFits->pHDU().addKey("BOL_RANG",QString::number(bol_rang).toStdString(),"bolometer calibration range");
    pFits->pHDU().addKey("BOL_TEMP",QString::number(bol_temp).toStdString(),"bolometer temperature [degC]");
    pFits->pHDU().addKey("PKG_TEMP",QString::number(pkg_temp).toStdString(),"package temperature [degC]");
    pFits->pHDU().addKey("CAS_TEMP",QString::number(cas_temp).toStdString(),"case temperature [degC]");
    pFits->pHDU().addKey("SHT_TEMP",QString::number(sht_temp).toStdString(),"shutter temperature [degC]");
    pFits->pHDU().addKey("LEN_TEMP",QString::number(len_temp).toStdString(),"lens temperature [degC]");
    pFits->pHDU().addKey("BGR_VOL",QString::number(bgr_vol).toStdString(),"BGR voltage [V]");
    pFits->pHDU().addKey("VB1_VOL",QString::number(vb1_vol).toStdString(),"VB1 voltage [V]");
    pFits->pHDU().addKey("ADOFSVOL",QString::number(adofsvol).toStdString(),"A/D_OFS voltage [V]");
    pFits->pHDU().addKey("HCE_TEMP",QString::number(hce_temp).toStdString(),"HCE TIR sensor temperature [degC]");
    pFits->pHDU().addKey("PNL_TEMP",QString::number(pnl_temp).toStdString(),"HCE TIR sensor panel temperature [degC]");
    pFits->pHDU().addKey("AE_TEMP",QString::number(ae_temp).toStdString(),"HCE TIR analog electronics temperature [degC]");
    pFits->pHDU().writeComment("");
    pFits->pHDU().writeComment("***** Observation Information by SPICE kernel *****");
    pFits->pHDU().writeComment("");
    pFits->pHDU().addKey("S_DISTHT",QString::number(s_distht).toStdString(),"distance between HYB2 and the target [km]");
    pFits->pHDU().addKey("S_DISTHE",QString::number(s_disthe).toStdString(),"distance between HYB2 and Earth [km]");
    pFits->pHDU().addKey("S_DISTHS",QString::number(s_disths).toStdString(),"distance between HYB2 and Sun [km]");
    pFits->pHDU().addKey("S_DISTTS",QString::number(s_distts).toStdString(),"distance between the target and Sun [km]");
    pFits->pHDU().addKey("S_TGRADI",QString::number(s_tgradi).toStdString(),"the target radius at the equator [km]");
    pFits->pHDU().addKey("S_APPDIA",QString::number(s_appdia).toStdString(),"apparent diameter of the target [deg]");
    pFits->pHDU().addKey("S_SOLLAT",QString::number(s_sollat).toStdString(),"sub solar latitude [deg] of the target");
    pFits->pHDU().addKey("S_SOLLON",QString::number(s_sollon).toStdString(),"sub solar longitude [deg] of the target");
    pFits->pHDU().addKey("S_SSCLAT",QString::number(s_ssclat).toStdString(),"sub S/C latitude [deg] of the target");
    pFits->pHDU().addKey("S_SSCLON",QString::number(s_ssclon).toStdString(),"sub S/C longitude [deg] of the target");
    pFits->pHDU().addKey("S_SSCLST",QString::number(s_ssclst).toStdString(),"sub S/C local solar time [h] of the target");
    pFits->pHDU().addKey("S_SSCPX",QString::number(s_sscpx).toStdString(),"sub S/C position on Image Array (axis1)");
    pFits->pHDU().addKey("S_SSCPY",QString::number(s_sscpy).toStdString(),"sub S/C position on Image Array (axis2)");
    pFits->pHDU().addKey("S_SCXSAN",QString::number(s_scxsan).toStdString(),"angle of S/C X axis and Sun direction [deg]");
    pFits->pHDU().addKey("S_SCYSAN",QString::number(s_scysan).toStdString(),"angle of S/C Y axis and Sun direction [deg]");
    pFits->pHDU().addKey("S_SCZSAN",QString::number(s_sczsan).toStdString(),"angle of S/C Z axis and Sun directino [deg]");
    pFits->pHDU().writeComment("");
    pFits->pHDU().writeComment("***** SPICE Kernels *****");
    pFits->pHDU().writeComment("");
    pFits->pHDU().addKey("NAIFNAME",naifname,"SPICE instrument name");
    pFits->pHDU().addKey("NAIFID",QString::number(naifid).toStdString(),"SPICE instrument ID");
    pFits->pHDU().addKey("MKNAME",mkname,"SPICE Meta kernel name");
    pFits->pHDU().writeComment("");
    pFits->pHDU().writeComment("***** Version ***** ");
    pFits->pHDU().writeComment("");
    pFits->pHDU().addKey("VERSION",QString::number(version).toStdString(),"version of the HDU");

    pFits->pHDU().write(fpixel,nelements,arraytemp);

}

void ShowImage::changeParameter(double min, double max, int x) {
    if (min != max) {
        MAX_V = max;
        MIN_V = min;
    }
    colorselect = x;
    makeColorTable();
    this->update();
}

void ShowImage::subtractFITSImage(QString file1){
    MAX_V=-100000;
    MIN_V=100000;


    //cout<<"file1"<<endl;
    //cout<<file1.toStdString()<<endl;

    QFileInfo fileinfo;
    fileinfo.setFile(file1);
    QString ext = fileinfo.suffix();
    ext=ext.toLower();

    if (ext == "fit" || ext=="fits" || ext=="fts")//(fitファイルの検索)
    {
        //fitsflag=1;
        valarray<long> contents;
        auto_ptr<FITS> pInfile(0);
        try{
            pInfile.reset(new FITS(file1.toStdString().c_str(), Read, true));
            PHDU& fitsImage=pInfile->pHDU();
            fitsImage.read(contents);
            fitsImage.readAllKeys();
            //Below code must update
            try{
                pInfile->pHDU().readKey<string>("ORIGIN",origin);
                pInfile->pHDU().readKey<string>("DATE",date);
                pInfile->pHDU().readKey<string>("DATE-BEG",date_beg);
                pInfile->pHDU().readKey<string>("DATE-OBS",date_obs);
                pInfile->pHDU().readKey<string>("DATE-END",date_end);
                pInfile->pHDU().readKey<string>("TELESCOP",telescop);
                pInfile->pHDU().readKey<string>("INSTRUME",instrume);
                pInfile->pHDU().readKey<string>("OBJECT",object);
                pInfile->pHDU().readKey<string>("BUNIT",bunit);
                pInfile->pHDU().readKey<double>("XPOSURE",xposure);
                pInfile->pHDU().readKey<double>("IFOV",ifov);
                pInfile->pHDU().readKey<string>("FILTER",filter);
                pInfile->pHDU().readKey<string>("OPRGNAME",oprgname);
                pInfile->pHDU().readKey<string>("OPRGNO",oprgno);
                pInfile->pHDU().readKey<double>("ROI_LLX",roi_llx);
                pInfile->pHDU().readKey<double>("ROI_LLY",roi_lly);
                pInfile->pHDU().readKey<double>("ROI_URX",roi_urx);
                pInfile->pHDU().readKey<double>("ROI_URY",roi_ury);
                pInfile->pHDU().readKey<double>("DATAMAX",datamax);
                pInfile->pHDU().readKey<double>("DATAMIN",datamin);
                pInfile->pHDU().readKey<double>("MEAN",mean);
                pInfile->pHDU().readKey<double>("STDEV",stdev);
                pInfile->pHDU().readKey<double>("MISS_VAL",miss_val);
                pInfile->pHDU().readKey<double>("MISS_NUM",miss_num);
                pInfile->pHDU().readKey<double>("DEAD_VAL",dead_val);
                pInfile->pHDU().readKey<double>("DEAD_NUM",dead_num);
                pInfile->pHDU().readKey<double>("SATU_VAL",satu_val);
                pInfile->pHDU().readKey<double>("SATU_NUM",satu_num);
                pInfile->pHDU().readKey<string>("IMGCMPRV",imgcmprv);
                pInfile->pHDU().readKey<string>("IMGCMPAL",imgcmpal);
                pInfile->pHDU().readKey<string>("IMGCMPPR",imgcmppr);
                pInfile->pHDU().readKey<double>("IMG_ERR",img_err);
                pInfile->pHDU().readKey<string>("IMGSEQC",imgseqc);
                pInfile->pHDU().readKey<double>("IMGACCM",imgaccm);
                pInfile->pHDU().readKey<double>("BITDEPTH",bitdepth);
                pInfile->pHDU().readKey<string>("PLT_POW",plt_pow);
                pInfile->pHDU().readKey<string>("PLT_STAT",plt_stat);
                pInfile->pHDU().readKey<string>("BOL_STAT",bol_stat);
                pInfile->pHDU().readKey<double>("BOL_TRGT",bol_trgt);
                pInfile->pHDU().readKey<double>("BOL_RANG",bol_rang);
                pInfile->pHDU().readKey<double>("BOL_TEMP",bol_temp);
                pInfile->pHDU().readKey<double>("PKG_TEMP",pkg_temp);
                pInfile->pHDU().readKey<double>("CAS_TEMP",cas_temp);
                pInfile->pHDU().readKey<double>("SHT_TEMP",sht_temp);
                pInfile->pHDU().readKey<double>("LEN_TEMP",len_temp);
                pInfile->pHDU().readKey<double>("BGR_VOL",bgr_vol);
                pInfile->pHDU().readKey<double>("VB1_VOL",vb1_vol);
                pInfile->pHDU().readKey<double>("ADOFSVOL",adofsvol);
                pInfile->pHDU().readKey<double>("HCE_TEMP",hce_temp);
                pInfile->pHDU().readKey<double>("PNL_TEMP",pnl_temp);
                pInfile->pHDU().readKey<double>("AE_TEMP",ae_temp);
                pInfile->pHDU().readKey<double>("S_DISTHT",s_distht);
                pInfile->pHDU().readKey<double>("S_DISTHE",s_disthe);
                pInfile->pHDU().readKey<double>("S_DISTHS",s_disths);
                pInfile->pHDU().readKey<double>("S_DISTTS",s_distts);
                pInfile->pHDU().readKey<double>("S_TGRADI",s_tgradi);
                pInfile->pHDU().readKey<double>("S_APPDIA",s_appdia);
                pInfile->pHDU().readKey<double>("S_SOLLAT",s_sollat);
                pInfile->pHDU().readKey<double>("S_SOLLON",s_sollon);
                pInfile->pHDU().readKey<double>("S_SSCLAT",s_ssclat);
                pInfile->pHDU().readKey<double>("S_SSCLON",s_ssclon);
                pInfile->pHDU().readKey<double>("S_SSCLST",s_ssclst);
                pInfile->pHDU().readKey<double>("S_SSCPX",s_sscpx);
                pInfile->pHDU().readKey<double>("S_SSCPY",s_sscpy);
                pInfile->pHDU().readKey<double>("S_SCXSAN",s_scxsan);
                pInfile->pHDU().readKey<double>("S_SCYSAN",s_scysan);
                pInfile->pHDU().readKey<double>("S_SCZSAN",s_sczsan);
                pInfile->pHDU().readKey<string>("NAIFNAME",naifname);
                pInfile->pHDU().readKey<double>("NAIFID",naifid);
                pInfile->pHDU().readKey<string>("MKNAME",mkname);
                pInfile->pHDU().readKey<double>("VERSION",version);

            }
            catch(...){};

            try{
                pInfile->pHDU().readKey<double>("PKG_TEMP",pkgt);
                pInfile->pHDU().readKey<double>("CAS_TEMP",cast);
                pInfile->pHDU().readKey<double>("SHT_TEMP",shtt);
                pInfile->pHDU().readKey<double>("LEN_TEMP",lent);
                pInfile->pHDU().readKey<double>("BITDEPTH",bitdepth);
            }
            catch(...){};

            int counter=0;
            double tmp1=0;

            for(int i=0; i<Height_i; i++){
                for(int j=0; j<Width_i; j++){

                    tmp1=contents[counter];

                    //  tmp1=tmp1/8;

                    if(image[i][j]-tmp1>MAX_V)MAX_V=image[i][j]-tmp1;
                    if(image[i][j]-tmp1<MIN_V)MIN_V=image[i][j]-tmp1;
                    image[i][j]-=tmp1;

                    /*
                    cout<<"x ";
                    cout<<j-16;
                    cout<<"  y ";
                    cout<<i-6;
                    cout<<"  ";
  */
                    //   cout<<image[i][j]<<endl;
                    counter++;

                }
            }
        }

        catch(FITS::CantCreate)
        {
            cout<<"Can't open fits image file"<<endl;
            return ;
        }
    }
    makeColorTable();
    this->update();

}

void ShowImage::subtractImage(QString file1, QString file2){
    MAX_V=-100000;
    MIN_V=100000;
    if (file1.contains("close", Qt::CaseInsensitive)){
        QString tmpo;
        tmpo=file1;
        file1=file2;
        file2=tmpo;
        setWidth(384);
        setHeight(256);
    }

    //cout<<"file1"<<endl;
    //cout<<file1.toStdString()<<endl;
    //cout<<"file2"<<endl;
    //cout<<file2.toStdString()<<endl;


    fstream ifs1;
    fstream ifs2;

    ifs1.open(&file1.toStdString()[0],ios::in | ios::binary);
    ifs2.open(&file2.toStdString()[0],ios::in | ios::binary);

    int Count( 0 );
    int i=0;
    int j=0;
    int jjj=0;
    char Data;
    double tmp;


    while(1){
        while( !ifs1.fail() )
        {
            if( Count >= getHeight()*getWidth()*2)  break;

            ifs1.read( &Data,1);
            tmp=0xff&Data;
            ++Count;

            ifs1.read( &Data,1);
            tmp+=(0xff&Data)*256;
            image[i][j]=tmp;
            j++;

            if( Count % (getWidth()*2) == getWidth()*2-1 ){
                i++;
                j=0;
            }

            ++Count;
        }

        ifs1.close();

        Count = 0;
        i=0;
        j=0;

        while( !ifs2.fail() )
        {
            if( Count >= getHeight()*getWidth()*2)  break;

            ifs2.read( &Data,1);
            tmp=0xff&Data;
            ++Count;

            ifs2.read( &Data,1);
            tmp+=(0xff&Data)*256;
            image[i][j] -= tmp;
            //********************************************************************//
            //マスクピクセル判定用
            /*
            //閾値設定
            if(image[i][j]> xxxx){
                int tcount = 0;
                int max1 = 0;
                int max2 = 0;
                int u1 = 0;
                int u2 = 0;
                int h =0;
                int p =0;
                int i=0;
                QString string1;
                QString string2;
                QString string3;

                for (int x = 0; x < 256; x += 32)
                {
                    max1 = x;
                    max1 = max1 + 32;
                    u1 = max1 - 1;

                    for (int y = 0; y < 384; y += 32)
                    {
                        tcount++;
                        max2 = y;
                        max2 = max2 + 32;
                        u2 = max2 - 1;
                        if (tcount < 10)
                        {
                            string1 = "UPDATE pix0" + QString::number(tcount);
                            //  printf("insert into pix0%d (img_file,x,y,pixel) values", tcount);
                        }
                        else
                        {
                            //printf("insert into pix%d (img_file,x,y,pixel) values", tcount);
                            string1 = "UPDATE pix" + QString::number(tcount);
                        }
                        for (h = x; h < max1; h++)
                        {
                            for (p = y; p < max2; p++)
                            {
                          /*      if (i == u1 && p == u2 && image[h][p]<閾値設定 )
                                {
                                    //  cout << "('" << img_file << "'," << p << "," << h << "," << pix[p][h] << ");" << endl;
                                    string2 = " SET mask=0 WHERE target_name=\""+filename+"\" AND x="+h+" AND y="+p+";";
                                }
                                else
                                {
                                    // cout << "('" << img_file << "'," << p << "," << h << "," << pix[p][h] << ");" << endl;
                                    string2 = " SET mask=0 WHERE target_name=\""+filename+"\" AND x="+i+" AND y="+j+";";
                                }

                            }
                        }
                    }
                }

                string3= string1+string2;
                cout<<string3.toStdString()<<endl;
            }
*/


            //********************************************************************//

            if(image[i][j]>MAX_V)MAX_V=image[i][j];
            if(image[i][j]<MIN_V)MIN_V=image[i][j];
            //cout<<"Max = "<<MAX_V<<endl;
            //cout<<"Min = "<<MIN_V<<endl;
            //cout<<endl;
            j++;

            if( Count % (getWidth()*2) == getWidth()*2-1 ){
                i++;
                jjj=j;
                j=0;
            }

            ++Count;
        }
        ifs2.close();
        double AveOB=0,AveCenter=0;
        int countB=0,countC=0;
        for(int x=-2;x<=2;x++){
            for(int y=-2;y<=2;y++){
                if(image[8+y][8+x]<image[8][8]+ 10
                        && image[8+y][8+x]>image[8][8]- 10){
                    AveOB += image[8+y][8+x];
                    countB++;
                }
            }
        }
        AveOB/=(double)countB;

        for(int x=-10;x<=10;x++){
            for(int y=-10;y<=10;y++){
                if(image[128+y][163+x]<image[128][163]+10
                        && image[128+y][163+x]>image[128][163]-10){
                    AveCenter += image[128+y][163+x];
                    countC++;
                }
            }
        }
        AveCenter/=(double)countC;

        if(AveCenter < AveOB){
            ifs1.open(&file2.toStdString()[0],ios::in | ios::binary);
            ifs2.open(&file1.toStdString()[0],ios::in | ios::binary);
            i=0;
            j=0;
            Count=0;
        }
        break;
    }



    if(MAX_V>1000){
        MAX_V=MAX_V/8;
        MIN_V=MIN_V/8;

        for(int xx=0;xx<i;xx++){
            for(int yy=0;yy<jjj;yy++){
                image[xx][yy]=image[xx][yy]/8;
            }
        }
    }
    makeColorTable();
    this->update();

}

void ShowImage::OutputFITSDBsubtractImage(QString file1, QString file2){

    //open-close
    //S=open SS=close

    double MAX_V=-100000;
    double MIN_V=100000;


    //cout<<"file1"<<endl;
    //cout<<file1.toStdString()<<endl;
    //cout<<"file2"<<endl;
    //cout<<file2.toStdString()<<endl;
    if (file1.contains("close", Qt::CaseInsensitive)){
        QString tmpo;
        tmpo=file1;
        file1=file2;
        file2=tmpo;
    }


    fstream ifs1;//open
    fstream ifs2;//close

    ifs1.open(&file1.toStdString()[0],ios::in | ios::binary);
    ifs2.open(&file2.toStdString()[0],ios::in | ios::binary);

    int Count( 0 );
    int i=0;
    int j=0;
    char Data;
    double tmp;
    double image[Max_Height][Max_Width];

    while(1){
        while( !ifs1.fail() )
        {
            double Height_i,Width_i;
            Height_i = getHeight();
            Width_i = getWidth();
            if( Count >= Height_i*Width_i*2)  break;

            ifs1.read( &Data,1);

            //buf[0] buf[1] → buf[1]*256+buf[0]
            //cout <<(int)(0xff&Data)<<endl;

            tmp=0xff&Data;

            //2バイト目
            ++Count;

            ifs1.read( &Data,1);
            tmp+=(0xff&Data)*xmldata.l1.Height_data;
            // tmp=tmp/8;

            image[i][j]=tmp;


            j++;

            if( Count % (int)(Width_i*2) == Width_i*2-1 ){
                i++;
                j=0;
            }

            ++Count;
        }

        ifs1.close();

        Count = 0;
        i=0;
        j=0;

        while( !ifs2.fail() )
        {
            if( Count >= Height_i*Width_i*2)  break;

            ifs2.read( &Data,1);

            //buf[0] buf[1] → buf[1]*256+buf[0]と2biteをリトルエンディアンで変換
            //1バイト目
            //データ表示 cout <<(int)(0xff&Data)<<endl;

            tmp=0xff&Data;

            ++Count;

            ifs2.read( &Data,1);
            tmp+=(0xff&Data)*xmldata.l1.Height_data;
            //  tmp=tmp/8;
            image[i][j] -= tmp;
            if(image[i][j]>MAX_V)MAX_V=image[i][j];
            if(image[i][j]<MIN_V)MIN_V=image[i][j];

            j++;

            if( Count % (Width_i*2) == Width_i*2-1 ){
                i++;
                j=0;
            }

            ++Count;
        }

        ifs2.close();

        long naxis    =   2;
        long naxes[2] = { xmldata.l1.Width_data, xmldata.l1.Height_data };
        std::auto_ptr<FITS> pFits(0);
        std::auto_ptr<FITS> pFits1(0);
        try
        {

            QString initialFileDirectory = QFileDialog::getExistingDirectory(this,tr("Select save Folder"),"/Applications/HEATcalibration");
            if(initialFileDirectory == ""){
                return;

            }

            QString filename1=QFileInfo(file1).fileName();
            QString filename2=QFileInfo(file2).fileName();

            const std::string fileNametemp(initialFileDirectory.toStdString()+"/"+filename1.toStdString()+"_"+filename2.toStdString()+"_openclose.fit");

            pFits.reset( new FITS(fileNametemp,DOUBLE_IMG , naxis , naxes ) );

        }
        catch (FITS::CantCreate)
        {
            return;
        }

        int nelements(1);
        nelements = std::accumulate(&naxes[0],&naxes[naxis],1,std::multiplies<double>());
        std::valarray<double> arraytemp(nelements);

        int k=0;
        for (int i = 0; i < xmldata.l1.Height_data; i++){
            for(int j=0;j<xmldata.l1.Width_data;j++){

                arraytemp[k]=image[i][j];
                k++;
            }
        }
        int  fpixel(1);

        pFits->pHDU().write(fpixel,nelements,arraytemp);






        /*


        //中央の値がオプティカルブラック領域よりも高いことを確認
        //低い場合は逆にして計算
        double AveOB=0,AveCenter=0;
        int countB=0,countC=0;
        for(int x=-2;x<=2;x++){
            for(int y=-2;y<=2;y++){
                //異常値判定
                if(image[8+y][8+x]<image[8][8]+ 10
                        && image[8+y][8+x]>image[8][8]- 10){
                    AveOB += image[8+y][8+x];
                    //異常値を除いた積算回数
                    countB++;
                }
            }
        }
        AveOB/=(double)countB;

        for(int x=-10;x<=10;x++){
            for(int y=-10;y<=10;y++){
                //異常値判定
                if(image[128+y][163+x]<image[128][163]+10
                        && image[128+y][163+x]>image[128][163]-10){
                    AveCenter += image[128+y][163+x];
                    //異常値を除いた積算回数
                    countC++;
                }
            }
        }
        AveCenter/=(double)countC;

        if(AveCenter < AveOB){
            ifs1.open(&file2.toStdString()[0],ios::in | ios::binary);
            ifs2.open(&file1.toStdString()[0],ios::in | ios::binary);
            i=0;
            j=0;
            Count=0;
        }



        //else break;
*/
        break;
    }

    makeColorTable();

    this->update();

}
void ShowImage::calibrateImageforBlackbodyAllPixel(QString s, int x, int y){

    renderunitflag=false;

    double h = s.section(',',-2,-2).toDouble();
    double g = s.section(',',-3,-3).toDouble();
    // double h =0;
    //double g =0;
    double DN = image[y+6][x+16];
    double DN2 =0;
    double tmp1=0;
    double FT1=0;

    int diameter=0;

    //DN2=DN-darkimage[y+6][x+16];
    //cout<<DN2<<endl;


    if(x==0 && y==0){
        effectivecounter=0;
        for(int s=0;s<328;s++){
            for(int e=0;e<248;e++){
                if(s==0 && e==0 && image[e+6][s+1+16]-darkimage[e+6][s+1+16]>50 && image[e+1+6][s+16]-darkimage[e+1+6][s+16]>50 &&image[e+6][s+16]-darkimage[e+6][s+16]>50){
                    effectivecounter++;
                }
                else if(s==327 && e==0 && image[e+6][s-1+16]-darkimage[e+6][s-1+16]>50 && image[e+1+6][s+16]-darkimage[e+1+6][s+16]>50 &&image[e+6][s+16]-darkimage[e+6][s+16]>50){
                    effectivecounter++;
                }
                else if(s==0 && e==247 && image[e-1+6][s+16]-darkimage[e-1+6][s+16]>50 && image[e+6][s+1+16]-darkimage[e+6][s+1+16]>50&&image[e+6][s+16]-darkimage[e+6][s+16]>50){
                    effectivecounter++;
                }
                else if(s==327 && e==247 && image[e-1+6][s+16]-darkimage[e-1+6][s+16]>50 && image[e+6][s-1+16]-darkimage[e+6][s-1+16]>50&&image[e+6][s+16]-darkimage[e+6][s+16]>50){
                    effectivecounter++;
                }
                else if(s==0 && image[e-1+6][s+16]-darkimage[e-1+6][s+16]>50 && image[e+6][s+1+16]-darkimage[e+6][s+1+16]>50 && image[e+6+1][s+16]-darkimage[e+1+6][s+16]>50 &&image[e+6][s+16]-darkimage[e+6][s+16]>50){
                    effectivecounter++;
                }
                else if(s==327 && image[e-1+6][s+16]-darkimage[e-1+6][s+16]>50 && image[e+6][s-1+16]-darkimage[e+6][s-1+16]>50 && image[e+1+6][s+16]-darkimage[e+1+6][s+16]>50 && image[e+6][s+16]-darkimage[e+6][s+16]>50){
                    effectivecounter++;
                }
                else if(e==0 && image[e+6][s-1+16]-darkimage[e+6][s-1+16]>50 && image[e+1+6][s+16]-darkimage[e+1+6][s+16]>50 && image[e+6][s+1+16]-darkimage[e+6][s+1+16]>50 && image[e+6][s+16]-darkimage[e+6][s+16]>50){
                    effectivecounter++;
                }
                else if(e==247 && image[e+6][s-1+16]-darkimage[e+6][s-1+16]>50 && image[e-1+6][s+16]-darkimage[e-1+6][s+16]>50 && image[e+6][s+1+16]-darkimage[e+6][s+1+16]>50 && image[e+6][s+16]-darkimage[e+6][s+16]>50){
                    effectivecounter++;
                }
                else if(image[e+6][s-1+16]-darkimage[e+6][s-1+16]>50 && image[e-1+6][s+16]-darkimage[e-1+6][s+16]>50 && image[e+6][s+1+16]-darkimage[e+6][s+1+16]>50 && image[e+1+6][s+16]-darkimage[e+1+6][s+16]>50 && image[e+6][s+16]-darkimage[e+6][s+16]>50){
                    effectivecounter++;
                }

            }
        }
        qDebug()<<"effectivecounter";
        qDebug()<<effectivecounter;
    }
    else if(x==0 && y==0 && darkimage[124][124]==-1000){
        effectivecounter=81344;
        qDebug()<<"effectivecounter";
        qDebug()<<effectivecounter;
    }


    /*
    //横軸=直径R
    double size[3];
    size[0] = 55;  //コリメータの直径 不変
    size[1] = 121; //黒体炉の直径　不変
    size[2] = 328; //全面黒体の直径　不変

    //横軸=root(pi/s)
    double area[3];
    area[0] = sqrt(PI/2350);  //コリメータの面積 不変
    area[1] = sqrt(PI/11880);//黒体炉の面積　不変
    area[2] = sqrt(PI/81344); //全面黒体の面積　不変

    //横軸=s/(pi*r*r) rはRの半分
    double spir[3];
    spir[0] =2350/(PI*(size[0]/2)*(size[0]/2));  //コリメータの面積 不変
    spir[1] =11880/(PI*(size[1]/2)*(size[1]/2));;//黒体炉の面積　不変
    spir[2] =81344/(PI*(size[2]/2)*(size[2]/2));; //全面黒体の面積　不変
*/

    //horizontal axis=diameter R
    double size[4];
    size[0] = 55.7;  //diameter of collimator invariant
    size[1] = 125.8; //diameter of blackbody furnace invariant
    size[2] = 328;  //diameter of full blackbody invariant
    size[4] = 300; //diameter 300 invariant

    //horizontal axis = root(pi/s)
    double area[2];
    area[0] = sqrt(PI/2350);  //area of collimator invariant
    area[1] = sqrt(PI/11880);//area of blackbody reactor invariant

    // horizontal axis = s/(pi*r*r) r is half of R
    double spir[2];
    spir[0] =2350/(PI*(size[0]/2)*(size[0]/2));   //area of collimator invariant
    spir[1] =11880/(PI*(size[1]/2)*(size[1]/2));  //area of blackbody reactor invariant



    QString BB;
    QString Furnace;
    QString Colli;

    QString BB_FileDirectory = QCoreApplication::applicationDirPath()+"/2019-12-23_00:35:30_443grounddata_BB";
    QString Furnace_FileDirectory = QCoreApplication::applicationDirPath() + "/interpolate_f";
    QString Colli_FileDirectory = QCoreApplication::applicationDirPath() + "/interpolate_c";

    QFile file_BB(BB_FileDirectory+"/" + QString::number(x) + "_" + QString::number(y) + ".txt");

    if(file_BB.exists()){
        file_BB.open(QIODevice::ReadOnly);
        QTextStream load(&file_BB);

        load >> BB;

        file_BB.close();

    }else{
        BB = "0,0,0,0,0,0,0";
    }


    QFile file_Furnace(Furnace_FileDirectory+"/" + QString::number(x) + "_" + QString::number(y) + ".txt");
    if(file_Furnace.exists()){
        file_Furnace.open(QIODevice::ReadOnly);
        QTextStream load(&file_Furnace);

        load >> Furnace;

        file_Furnace.close();

    }else{
        Furnace = "0,0,0,0,0,0,0";
    }


    QFile file_Colli(Colli_FileDirectory+"/" + QString::number(x) + "_" + QString::number(y) + ".txt");
    if(file_Colli.exists()){
        file_Colli.open(QIODevice::ReadOnly);
        QTextStream load(&file_Colli);

        load >> Colli;

        file_Colli.close();

    }else{
        Colli = "0,0,0,0,0,0,0";
    }
    /*
    double slope[3];
    slope[0] = Colli.section(',',-3,-3).toDouble(); //コリメータの傾き
    slope[1] = Furnace.section(',',-3,-3).toDouble();//黒体炉の傾き
    slope[2] = BB.section(',',-3,-3).toDouble(); //全面黒体の傾き

    double intercept[3];
    intercept[0] = Colli.section(',',-2,-2).toDouble(); //コリメータの切片
    intercept[1] = Furnace.section(',',-2,-2).toDouble(); //黒体炉の切片
    intercept[2] = BB.section(',',-2,-2).toDouble(); //全面黒体の切片
  */
    double slope[2];
    slope[0] = Colli.section(',',-3,-3).toDouble(); //slope of coli
    slope[1] = Furnace.section(',',-3,-3).toDouble();//slope of BB Furnace


    double intercept[2];
    intercept[0] = Colli.section(',',-2,-2).toDouble(); //intercept of coli
    intercept[1] = Furnace.section(',',-2,-2).toDouble(); //intercept of Furnace

    double lsma0 = 0, lsma1 = 0;
    double lsma2 = 0, lsma3 = 0;
    double lsma4 = 0, lsma5 = 0;
    double lsma6 = 0, lsma7 = 0;


    diameter=2*sqrt(effectivecounter/PI);
    //diameter=130;

    //<<diameter<<endl;

    //double dist = sqrt((x-164)**2.0 + (y-124)**2.0);

    if(diameter<300){
        lsm_slope(size, slope, &lsma0, &lsma1);
        g=lsma1*diameter + lsma0;
        lsm_intercept(size, intercept, &lsma2, &lsma3);

        h=lsma3*diameter + lsma2;
        FT1=round1((DN-h)/g);
        qDebug()<<DN;
        qDebug()<<FT1;
        //tmp1 =gettemperature_epsillon1(FT1);
        tmp1 = rad2temp(FT1,1,db);
        if(tmp1+273.15>150){
            calibrationImage[y][x] = tmp1 + 273.15;
            fitstemperature2[y][x] = tmp1 + 273.15;
        }
        else{
            /*
            calibrationImage[y+6][x+16] = tmp1 + 273.15;
            fitstemperature2[y][x] = tmp1 + 273.15;
            */

            calibrationImage[y][x] = 150;
            fitstemperature2[y][x] = 150;

        }

    }
    else{
        lsm_slope(size, slope, &lsma0, &lsma1);

        double size_threeh[2];
        size_threeh[0] = 300; //300
        size_threeh[1] = 328;

        double slope_threeh[2];
        slope_threeh[0] = lsma1*300 + lsma0; //Slope by 300
        slope_threeh[1] = lsma1*328 + lsma0; //Slope of all BB
        lsm_slope_threeh(size_threeh,slope_threeh,&lsma4, &lsma5);

        //  printf("a4=%f\na5=%f\n", lsma5, lsma4);

        double g_threeh=lsma5*diameter + lsma4;
        lsm_intercept(size, intercept, &lsma2, &lsma3);

        double intercept_threeh[2];
        intercept_threeh[0] = lsma3*300 + lsma2; //intercept by 300
        intercept_threeh[1] = lsma3*328 + lsma2; //intercepy of all BB

        lsm_intercept_threeh(size_threeh,intercept_threeh,&lsma6, &lsma7);

        //  printf("a6=%f\na7=%f\n", lsma7, lsma6);

        double h_threeh=lsma7*diameter + lsma6;

        FT1=round1((DN-h_threeh)/g_threeh);


        //tmp1 =gettemperature_epsillon1(FT1);
        tmp1 = rad2temp(FT1,1,db);

        if(tmp1+273.15>150){

            calibrationImage[y+6][x+16] = tmp1 + 273.15;
            fitstemperature2[y][x] = tmp1 + 273.15;
        }
        else{
            calibrationImage[y+6][x+16] = 150;
            fitstemperature2[y][x] = 150;
        }

    }
    bunit = 'K';
    setWidth(328);
    setHeight(248);
    this->resize(getWidth()+100,getHeight()+20);

    /*
     *
     * if(DN2>50){

        FT1=round1((DN-h)/g);
        //tmp1 = gettemperature(FT1);
        tmp1 = temp2rad(FT1,0,db);
        //HEAT上で出力
        calibrationImage[y+6][x+16] = tmp1 + 273.15;
        //FITSファイル出力
        fitstemperature2[y][x] = tmp1 + 273.15;
    }
    else{
        //HEAT上で出力
        calibrationImage[y+6][x+16] =150;
        //FITSファイル出力
        fitstemperature2[y][x] = 150;
    }
*/


    // FT1=round1((DN-h)/g);
    //全面BBのとき
    // tmp1 = gettemperature(FT1);
    // tmp1 = temp2rad(FT1,0,db);

    //コリ、黒体炉のとき
    //tmp1 =gettemperature_colli_furnace(FT1);
    //tmp1 = temp2rad(FT1,2,db);


    //HEAT上で出力
    // calibrationImage[y+6][x+16] = tmp1 + 273.15;
    //FITSファイル出力

    //fitstemperature2[y][x] = h;
    //    fitstemperature2[y][x] = tmp1 + 273.15;
    //    fitstemperature2[y][x] = g*planck(50+273.15)+h;


    /*
    int count=-400;
    double yy=0;
    double xx=-400;

    for(xx=-400;xx<600;xx++){
        cout<<"else if(FT1<=";  cout<< round1(planck((xx+0.9+273.15))); cout<<"){"; cout<<endl;
        cout<<"if(FT1==";  cout<< round1(planck((count+273.15))); cout<<")tmp1="; cout<<xx;   cout<<";"; cout<<endl;
        for(yy=count+0.1;yy<count+1.0;yy=yy+0.1){
            if(yy==xx+1)break;
            else
            {
                cout<<"else if(FT1<=";  cout<< round1(planck((yy+273.15))); cout<<")tmp1="; cout<<yy;   cout<<";";
                cout<<endl;
            }
        }
        count=count+1;
        cout<<"}";
        cout<<endl;
    }
*/



    /*
    QFile file("/Users/sukokentarou/Desktop/HEATdata.txt");
    file.open(QIODevice::Append);
    QTextStream out(&file);
    out<<"x: ";    out<<x<<endl;
    out<<"y: ";    out<<y<<endl;
    out<<"DN: ";    out<<DN<<endl;
    out<<"ケルビン: ";    out<<calibrationImage[y+6][x+16]<<endl;
    file.close();
*/

}


void ShowImage::calibrateLUTl2a(double g,double h,int y,int x){

    renderunitflag=false;

    // double h =0;
    //double g =0;
    double DN = image[y+6][x+16];
    double DN2 =0;
    double tmp1=0;
    double FT1=0;

    int diameter=0;

    //DN2=DN-darkimage[y+6][x+16];
    //cout<<DN2<<endl;


    if(x==0 && y==0){
        effectivecounter=0;
        for(int s=0;s<328;s++){
            for(int e=0;e<248;e++){
                if(s==0 && e==0 && image[e+6][s+1+16]-darkimage[e+6][s+1+16]>50 && image[e+1+6][s+16]-darkimage[e+1+6][s+16]>50 &&image[e+6][s+16]-darkimage[e+6][s+16]>50){
                    effectivecounter++;
                }
                else if(s==327 && e==0 && image[e+6][s-1+16]-darkimage[e+6][s-1+16]>50 && image[e+1+6][s+16]-darkimage[e+1+6][s+16]>50 &&image[e+6][s+16]-darkimage[e+6][s+16]>50){
                    effectivecounter++;
                }
                else if(s==0 && e==247 && image[e-1+6][s+16]-darkimage[e-1+6][s+16]>50 && image[e+6][s+1+16]-darkimage[e+6][s+1+16]>50&&image[e+6][s+16]-darkimage[e+6][s+16]>50){
                    effectivecounter++;
                }
                else if(s==327 && e==247 && image[e-1+6][s+16]-darkimage[e-1+6][s+16]>50 && image[e+6][s-1+16]-darkimage[e+6][s-1+16]>50&&image[e+6][s+16]-darkimage[e+6][s+16]>50){
                    effectivecounter++;
                }
                else if(s==0 && image[e-1+6][s+16]-darkimage[e-1+6][s+16]>50 && image[e+6][s+1+16]-darkimage[e+6][s+1+16]>50 && image[e+6+1][s+16]-darkimage[e+1+6][s+16]>50 &&image[e+6][s+16]-darkimage[e+6][s+16]>50){
                    effectivecounter++;
                }
                else if(s==327 && image[e-1+6][s+16]-darkimage[e-1+6][s+16]>50 && image[e+6][s-1+16]-darkimage[e+6][s-1+16]>50 && image[e+1+6][s+16]-darkimage[e+1+6][s+16]>50 && image[e+6][s+16]-darkimage[e+6][s+16]>50){
                    effectivecounter++;
                }
                else if(e==0 && image[e+6][s-1+16]-darkimage[e+6][s-1+16]>50 && image[e+1+6][s+16]-darkimage[e+1+6][s+16]>50 && image[e+6][s+1+16]-darkimage[e+6][s+1+16]>50 && image[e+6][s+16]-darkimage[e+6][s+16]>50){
                    effectivecounter++;
                }
                else if(e==247 && image[e+6][s-1+16]-darkimage[e+6][s-1+16]>50 && image[e-1+6][s+16]-darkimage[e-1+6][s+16]>50 && image[e+6][s+1+16]-darkimage[e+6][s+1+16]>50 && image[e+6][s+16]-darkimage[e+6][s+16]>50){
                    effectivecounter++;
                }
                else if(image[e+6][s-1+16]-darkimage[e+6][s-1+16]>50 && image[e-1+6][s+16]-darkimage[e-1+6][s+16]>50 && image[e+6][s+1+16]-darkimage[e+6][s+1+16]>50 && image[e+1+6][s+16]-darkimage[e+1+6][s+16]>50 && image[e+6][s+16]-darkimage[e+6][s+16]>50){
                    effectivecounter++;
                }

            }
        }
    }
    else if(x==0 && y==0 && darkimage[124][124]==-1000){
        effectivecounter=81344;
    }


    /*
    //横軸=直径R
    double size[3];
    size[0] = 55;  //コリメータの直径 不変
    size[1] = 121; //黒体炉の直径　不変
    size[2] = 328; //全面黒体の直径　不変

    //横軸=root(pi/s)
    double area[3];
    area[0] = sqrt(PI/2350);  //コリメータの面積 不変
    area[1] = sqrt(PI/11880);//黒体炉の面積　不変
    area[2] = sqrt(PI/81344); //全面黒体の面積　不変

    //横軸=s/(pi*r*r) rはRの半分
    double spir[3];
    spir[0] =2350/(PI*(size[0]/2)*(size[0]/2));  //コリメータの面積 不変
    spir[1] =11880/(PI*(size[1]/2)*(size[1]/2));;//黒体炉の面積　不変
    spir[2] =81344/(PI*(size[2]/2)*(size[2]/2));; //全面黒体の面積　不変
*/

    //horizontal axis=diameter R
    double size[4];
    size[0] = 55.7;  //diameter of collimator invariant
    size[1] = 125.8; //diameter of blackbody furnace invariant
    size[2] = 328;  //diameter of full blackbody invariant
    size[4] = 300; //diameter 300 invariant

    //horizontal axis = root(pi/s)
    double area[2];
    area[0] = sqrt(PI/2350);  //area of collimator invariant
    area[1] = sqrt(PI/11880);//area of blackbody reactor invariant

    // horizontal axis = s/(pi*r*r) r is half of R
    double spir[2];
    spir[0] =2350/(PI*(size[0]/2)*(size[0]/2));   //area of collimator invariant
    spir[1] =11880/(PI*(size[1]/2)*(size[1]/2));  //area of blackbody reactor invariant



    QString BB;
    QString Furnace;
    QString Colli;


    QString BB_FileDirectory = "/Applications/BB.fit";
    QString Furnace_FileDirectory = "/Applications/Furance.fit";
    QString Colli_FileDirectory = "/Applications/Colli.fit";

    /*
    QString BB_FileDirectory = "/Applications/2019-12-23_00:35:30_443grounddata_BB";
    QString Furnace_FileDirectory = "/Applications/interpolate_f";
    QString Colli_FileDirectory = "/Applications/interpolate_c";

    QFile file_BB(BB_FileDirectory+"/" + QString::number(x) + "_" + QString::number(y) + ".txt");

    if(file_BB.exists()){
        file_BB.open(QIODevice::ReadOnly);
        QTextStream load(&file_BB);

        load >> BB;

        file_BB.close();

    }else{
        BB = "0,0,0,0,0,0,0";
    }

    QFile file_Furnace(Furnace_FileDirectory+"/" + QString::number(x) + "_" + QString::number(y) + ".txt");
    if(file_Furnace.exists()){
        file_Furnace.open(QIODevice::ReadOnly);
        QTextStream load(&file_Furnace);

        load >> Furnace;

        file_Furnace.close();

    }else{
        Furnace = "0,0,0,0,0,0,0";
    }

    QFile file_Colli(Colli_FileDirectory+"/" + QString::number(x) + "_" + QString::number(y) + ".txt");
    if(file_Colli.exists()){
        file_Colli.open(QIODevice::ReadOnly);
        QTextStream load(&file_Colli);

        load >> Colli;

        file_Colli.close();

    }else{
        Colli = "0,0,0,0,0,0,0";
    }
    */

    double slope[2];
    double intercept[2];
    int index=328*y+x;

    readLUTfile(Colli_FileDirectory,slope,intercept,index,0);
    readLUTfile(Furnace_FileDirectory,slope,intercept,index,1);






    double lsma0 = 0, lsma1 = 0;
    double lsma2 = 0, lsma3 = 0;
    double lsma4 = 0, lsma5 = 0;
    double lsma6 = 0, lsma7 = 0;


    diameter=2*sqrt(effectivecounter/PI);
    //diameter=130;

    //<<diameter<<endl;

    //double dist = sqrt((x-164)**2.0 + (y-124)**2.0);

    if(diameter<300){
        lsm_slope(size, slope, &lsma0, &lsma1);
        //cout<<"lsma1 "<<lsma1<<" diam "<<diameter<<" lsma0 "<<lsma0<<endl;
        g=lsma1*diameter + lsma0;
        lsm_intercept(size, intercept, &lsma2, &lsma3);
        //cout<<"lsma3 "<<lsma3<<" diam "<<diameter<<" lsma2 "<<lsma2<<endl;
        h=lsma3*diameter + lsma2;
        FT1=round1((DN-h)/g);
        //tmp1 =gettemperature_epsillon1(FT1);
        tmp1 = rad2temp(FT1,1,db);
        cout<<"DN "<<DN<<" h "<<h<<" g "<<g<<endl;
        //cout<<FT1<<" saisyo tmp1 = "<<tmp1<<endl;
        if(tmp1+273.15>150){
            calibrationImage[y][x] = tmp1 + 273.15;
            fitstemperature2[y][x] = tmp1 + 273.15;
        }
        else{
            /*
            calibrationImage[y+6][x+16] = tmp1 + 273.15;
            fitstemperature2[y][x] = tmp1 + 273.15;
            */

            calibrationImage[y][x] = 150;
            fitstemperature2[y][x] = 150;

        }

    }
    else{
        lsm_slope(size, slope, &lsma0, &lsma1);

        double size_threeh[2];
        size_threeh[0] = 300; //300
        size_threeh[1] = 328;

        double slope_threeh[2];
        slope_threeh[0] = lsma1*300 + lsma0; //Slope by 300
        slope_threeh[1] = lsma1*328 + lsma0; //Slope of all BB
        lsm_slope_threeh(size_threeh,slope_threeh,&lsma4, &lsma5);

        //  printf("a4=%f\na5=%f\n", lsma5, lsma4);

        double g_threeh=lsma5*diameter + lsma4;
        lsm_intercept(size, intercept, &lsma2, &lsma3);

        double intercept_threeh[2];
        intercept_threeh[0] = lsma3*300 + lsma2; //intercept by 300
        intercept_threeh[1] = lsma3*328 + lsma2; //intercepy of all BB

        lsm_intercept_threeh(size_threeh,intercept_threeh,&lsma6, &lsma7);

        //  printf("a6=%f\na7=%f\n", lsma7, lsma6);

        double h_threeh=lsma7*diameter + lsma6;

        FT1=round1((DN-h_threeh)/g_threeh);


        //tmp1 =gettemperature_epsillon1(FT1);
        tmp1 = rad2temp(FT1,1,db);
        cout<<FT1<<" tmp1 = "<<tmp1<<endl;
        if(tmp1+273.15>150){

            calibrationImage[y+6][x+16] = tmp1 + 273.15;
            fitstemperature2[y][x] = tmp1 + 273.15;
        }
        else{
            calibrationImage[y+6][x+16] = 150;
            fitstemperature2[y][x] = 150;
        }

    }
    bunit = 'K';
    setWidth(328);
    setHeight(248);
    this->resize(getWidth()+100,getHeight()+20);


}


double ShowImage::rad2temp(double FT1, int select, QSqlDatabase db){
    QSqlQuery rad(db);
    QString table_name;
    if(select == 1){
        table_name = "rad2temp";
        if(FT1>171.877) return 200;
        if(FT1<0.03304) return -150;
    }
    else if(select == 2){
        table_name = "rad2temp_ep";
        if(FT1>185.813) return 200;
        if(FT1<0.03572) return -150;
    }
    else if(select == 3){
        table_name = "rad2temp_fu";
        if(FT1>180.238) return 200;
        if(FT1<0.03465) return -150;
    }
    else{
        qDebug("Not table");
        return 0;
    }
    //上は大丈夫
    double temp_value,rad_value;
    QVariant FT1_temp = FT1;
    QString str = "SELECT rad, temp FROM " + table_name +
            " WHERE rad >= " + FT1_temp.toString() +
            " LIMIT 1;";
    if(rad.exec(str)){
        rad.first();
        temp_value = rad.value(0).toDouble();
        //qDebug()<<"-- 成功 ----- "<<temp_value;
        rad.clear();
    }
    else{
        qDebug()<<rad.lastError();
    }
    //db.close();
    return temp_value;
}

void ShowImage::confirmation(QString s, int x, int y, QString subFileName1) {
    fstream ifs;
    ifs.open(&subFileName1.toStdString()[0], ios::in | ios::binary);

    QFileInfo fileinfo;
    fileinfo.setFile(subFileName1);
    QString ext = fileinfo.suffix();
    ext = ext.toLower();

    if (ext == "fit" || ext == "fits" || ext == "fts") {
        valarray<long> contents;
        auto_ptr<FITS> pInfile(0);
        try {
            pInfile.reset(new FITS(subFileName1.toStdString().c_str(), Read, true));
            PHDU &fitsImage = pInfile->pHDU();
            int fitsWidth = fitsImage.axis(0);
            int fitsHeight = fitsImage.axis(1);
            fitsImage.read(contents);

            int counter = 0;
            double tmp1 = 0;

            for (int i = 0; i < fitsHeight; i++) {
                for (int j = 0; j < fitsWidth; j++) {

                    tmp1 = contents[counter];
                    fitsimagesub[fitsHeight - i - 1][j] = tmp1;

                    counter++;
                }
            }
        }

        catch (FITS::CantCreate) {
            cout << "Can't open fits image file" << endl;
            return;
        }
    }

    calibrationImage[y + 2][x + 16] = fitsimagesub[y][x];
}

void ShowImage::calibrateImagetoRadianceforBlackbodyAllPixel(QString s, int x, int y){

    double h = s.section(',',-2,-2).toDouble();
    double g = s.section(',',-3,-3).toDouble();
    double DN = image[y+2][x+16];
    double DN2=0;
    double tmp1=0;
    double FT1;
    double Radiance=0;
    DN2=DN-darkimage[y+6][x+16];
    // cout<<darkimage[y+6][x+16]<<endl;

    if(DN2>50){

        FT1=round1((DN-h)/g);
        //flag1
        //tmp1 = gettemperature(FT1);
        tmp1 = rad2temp(FT1,0,db);
        Radiance = epsilon *((tmp1+273.15)*(tmp1+273.15)*(tmp1+273.15)*(tmp1+273.15))/PI;
    }
    else{
        Radiance=0;
    }

    calibrationImage[y+2][x+16] = Radiance;

    fitstemperature2[y][x] = Radiance;

}

void ShowImage::initializeFITSarray(){
    for(int i=0; i<248; i++){
        for(int j=0; j<328; j++){
            fitstemperature2[i][j]=150;
        }
    }
}
double ShowImage::getPixelValue(int y, int x) { return image[y][x]; }

void ShowImage::drawPixcelLineGraph(QString heightValue) {

    QVector<double> w(Width_i), DN(Width_i);
    bool checkInt = false;
    int y = heightValue.toInt(&checkInt, 10);

    if (0 <= y && y <= Width_i && checkInt == true) {
        for (int i = 0; i < Width_i; i++)
            w[i] = i;

        for (int i = 0; i < Width_i; i++)
            DN[i] = this->getPixelValue(y, i);
    } else {
    }

    pg.drawGraph(w, DN);
    pg.show();
}

double ShowImage::round3(double dIn) {
    double dOut;

    dOut = dIn * pow(10.0, 3);
    if (dIn >= 0) {
        dOut = (double)(int)(dOut + 0.5);
    } else
        dOut = (double)(int)(dOut - 0.5);
    return dOut * pow(10.0, -3);
}

double ShowImage::round2(double dIn) {
    double dOut;

    dOut = dIn * pow(10.0, 2);
    if (dIn >= 0) {
        dOut = (double)(int)(dOut + 0.5);
    } else
        dOut = (double)(int)(dOut - 0.5);
    return dOut * pow(10.0, -2);
}

double ShowImage::round1(double dIn) {
    double dOut;

    dOut = dIn * pow(10.0, 5);
    if (dIn >= 0) {
        dOut = (double)(int)(dOut + 0.5);
    } else
        dOut = (double)(int)(dOut - 0.5);
    return dOut * pow(10.0, -5);
}

vector<string> split(string& input, char delimiter)
{
    istringstream stream(input);
    string field;
    vector<string> result;
    while (getline(stream, field, delimiter)) {
        result.push_back(field);
    }
    return result;
}

void ShowImage::calibrationLutImage(double g, double h, int y, int x)
{
    double DN = image[y + 6][x + 16];
    double c = 6.125;


    double DN1 = DN - c*(cas_temp - pkg_temp);//(3)
    c = 6.158;
    double DN2 = DN1 - c*(28 - sht_temp);//(4)
    double l2_radiance = (DN2 - h) / g;//(6)
    double l2_temp = 0;
    if(x==0&&y==0){
        string csv = QCoreApplication::applicationDirPath().toStdString() + "/temp_radiance_table.csv";
        qDebug() << QCoreApplication::applicationDirPath() + "/temp_radiance_table.csv";
        stringstream ss;
        ifstream ifs_csv_file(csv);
        string str;
        int count_csv =0;
        ifstream ifs(csv);

            string line;
            while (getline(ifs, line)) {

                vector<string> strvec = split(line, ',');

                for (int i=0; i<strvec.size();i++){
                 // printf("%5d\n", stoi(strvec.at(i)));
            //  qDebug()<<stof(strvec.at(i));
                csv_data[count_csv/2][count_csv%2] = stof(strvec.at(i));
                qDebug() <<"csv_data["<<count_csv/2<<"]"<<"["<<count_csv%2<<"]"<<"-"<<csv_data[count_csv/2][count_csv%2];
                count_csv++;
                }

            }
        for(int i=0;i<350;i++){
            for(int j=0;j<2;j++){
            //    qDebug()<<csv_data[i][j];
            }
        }
        //--------------------------------

        for (int row = 0; row < 350; row++)
        {
            for (int col = 0; col < 1; col++)
            {
                getline(ifs_csv_file.seekg(0,ios_base::cur),str,',');
                ss.str(str);
          //    ss>>csv_data[row][col];
             // qDebug()<<csv_data[row][col]<<" ";
                ss.str("");
                ss.clear(stringstream::goodbit);
            }
            std::getline(ifs_csv_file.seekg(0,ios_base::cur),str,'\r');
            ss.str(str);
      //    ss>>csv_data[row][1];
           //qDebug()<<csv_data[row][1]<<endl;
            ss.str("");
            ss.clear(stringstream::goodbit);

        }
        ifs_csv_file.close();
        setHeight(248);
        setWidth(328);
        bunit = "K";
    }
    //-------------------------
    double rad = l2_radiance;
    //ebug() << rad;
    int temp_i = 0;
    if(rad>=0.2589148 && rad<=222.4975){

        for(int i=1;i<350;i++){
            //(7)
         // qDebug()<<csv_data[i][1];
            if(csv_data[i][1]>=rad){

                temp_i = i;
                l2_temp = ((csv_data[i][0] - csv_data[i-1][0]) / (csv_data[i][1] - csv_data[i-1][1])) * (rad - csv_data[i-1][1]) + (csv_data[i-1][0]);

              //qDebug()<<"("<<csv_data[i][0]<<"-"<<csv_data[i-1][0]<<") / "<<"("<<csv_data[i][1]<<"-"<<csv_data[i-1][1]<<") x ("<<rad<<"-"<<csv_data[i-1][1]<<") + "<<csv_data[i-1][0]<<" = "<<l2_temp<<endl;
                l2_temp = round2(l2_temp);
                break;
            }
        }
    }
    else if(rad>222.4975){
        l2_temp = round2((500.0));
    }
    else if(rad<0.2589148){
        l2_temp = round2((150.0));
    }
    image[y][x] = l2_temp;
    calibrationImage[y][x] = l2_temp;
    fitstemperature2[y][x] = l2_temp;

   //Debug()<<y<<"-"<<x<<" DN->"<<DN<<" DN'->"<<DN1<<" DN''->"<<DN2<<" radiance :"<<l2_radiance<<" temp :"<<l2_temp;
    //cout<<csv_data[temp_i][0]<<" "<<csv_data[temp_i][1]<<endl;
}


void ShowImage::SetDarkImage(QString getdarkfilename)
{
    valarray<double> contents;
    auto_ptr<FITS> pInfile(0);
    QString appPath;
    appPath = QCoreApplication::applicationDirPath();

    //  cout<<getdarkfilename.toStdString()<<endl;

    QString darkfilename;
    darkfilename=appPath+"/"+getdarkfilename;
    try{
        pInfile.reset(new FITS(darkfilename.toStdString().c_str(), Read, true));
        PHDU& fitsImage=pInfile->pHDU();
        fitsImage.read(contents);
        fitsImage.readAllKeys();

        int counter=0;
        double tmp1=0;

        for(int i=0; i<Height_i; i++){
            for(int j=0; j<Width_i; j++){
                tmp1=contents[counter];

                //  tmp1=tmp1/8;
                darkimage[i][j]=tmp1;
                counter++;
                //  cout<<darkimage[i][j]<<endl;
            }
        }
    }

    catch(FITS::CantCreate)
    {
        cout<<"Can't open fits image file"<<endl;
    }
}

void ShowImage::lsm_slope(double size[], double slope[], double *lsma0, double *lsma1)
{
    int i;
    double A00 = 0, A01 = 0, A02 = 0, A11 = 0, A12 = 0;

    for (i = 0; i < 2; i++)
    {
        A00 += 1.0;
        A01 += size[i];
        A02 += slope[i];
        A11 += size[i] * size[i];
        A12 += size[i] * slope[i];
    }

    *lsma0 = ((A02 * A11) - (A01 * A12)) / ((A00 * A11) - (A01 * A01));
    *lsma1 = ((A00 * A12 - A01 * A02)) / ((A00 * A11) - (A01 * A01));
}

void ShowImage::lsm_intercept(double size[], double intercept[], double *lsma2, double *lsma3)
{
    int i;
    double A00 = 0, A01 = 0, A02 = 0, A11 = 0, A12 = 0;

    for (i = 0; i < 2; i++)
    {
        A00 += 1.0;
        A01 += size[i];
        A02 += intercept[i];
        A11 += size[i] * size[i];
        A12 += size[i] * intercept[i];
    }

    *lsma2 = ((A02 * A11) - (A01 * A12)) / ((A00 * A11) - (A01 * A01));
    *lsma3 = ((A00 * A12 - A01 * A02)) / ((A00 * A11) - (A01 * A01));
}

void ShowImage::lsm_slope_threeh(double size[], double slope[], double *lsma4, double *lsma5)
{
    int i;
    double A00 = 0, A01 = 0, A02 = 0, A11 = 0, A12 = 0;

    for (i = 0; i < 2; i++)
    {
        A00 += 1.0;
        A01 += size[i];
        A02 += slope[i];
        A11 += size[i] * size[i];
        A12 += size[i] * slope[i];
    }

    *lsma4 = ((A02 * A11) - (A01 * A12)) / ((A00 * A11) - (A01 * A01));
    *lsma5 = ((A00 * A12 - A01 * A02)) / ((A00 * A11) - (A01 * A01));
}

void ShowImage::lsm_intercept_threeh(double size[], double intercept[], double *lsma6, double *lsma7)
{
    int i;
    double A00 = 0, A01 = 0, A02 = 0, A11 = 0, A12 = 0;

    for (i = 0; i < 2; i++)
    {
        A00 += 1.0;
        A01 += size[i];
        A02 += intercept[i];
        A11 += size[i] * size[i];
        A12 += size[i] * intercept[i];
    }

    *lsma6 = ((A02 * A11) - (A01 * A12)) / ((A00 * A11) - (A01 * A01));
    *lsma7 = ((A00 * A12 - A01 * A02)) / ((A00 * A11) - (A01 * A01));
}

void ShowImage::slope_300(double size[], double intercept[], double *lsma6, double *lsma7)
{
    int i;
    double A00 = 0, A01 = 0, A02 = 0, A11 = 0, A12 = 0;

    for (i = 0; i < 2; i++)
    {
        A00 += 1.0;
        A01 += size[i];
        A02 += intercept[i];
        A11 += size[i] * size[i];
        A12 += size[i] * intercept[i];
    }

    *lsma6 = ((A02 * A11) - (A01 * A12)) / ((A00 * A11) - (A01 * A01));
    *lsma7 = ((A00 * A12 - A01 * A02)) / ((A00 * A11) - (A01 * A01));
}


void ShowImage::intercept_300(double size[], double intercept[], double *lsma6, double *lsma7)
{
    int i;
    double A00 = 0, A01 = 0, A02 = 0, A11 = 0, A12 = 0;

    for (i = 0; i < 2; i++)
    {
        A00 += 1.0;
        A01 += size[i];
        A02 += intercept[i];
        A11 += size[i] * size[i];
        A12 += size[i] * intercept[i];
    }

    *lsma6 = ((A02 * A11) - (A01 * A12)) / ((A00 * A11) - (A01 * A01));
    *lsma7 = ((A00 * A12 - A01 * A02)) / ((A00 * A11) - (A01 * A01));
}

void ShowImage::calibrateImage(QString s, int x, int y) {

    double h = s.section(',', -2, -2).toDouble();
    double g = s.section(',', -3, -3).toDouble();
    double f = s.section(',', -4, -4).toDouble();
    double e = s.section(',', -5, -5).toDouble();
    double d = s.section(',', -6, -6).toDouble();
    double c = s.section(',', -7, -7).toDouble();
    double b = s.section(',', -8, -8).toDouble();
    double a = s.section(',', -9, -9).toDouble();

    double DN = image[y + 2][x + 16];
    double tmp = 0;

    tmp = a * pow(DN, 7) + b * pow(DN, 6) + c * pow(DN, 5) + d * pow(DN, 4) +
            e * pow(DN, 3) + f * pow(DN, 2) + g * DN + h;
    calibrationImage[y + 2][x + 16] = tmp;
}

void ShowImage::updateImage(int judge,QString dirpath,QString fitdirectory){

    MAX_V = -100000;
    MIN_V = 100000;
    Width_i = getWidth();
    Height_i = getHeight();
    if(judge==0){


        for(int i=0; i<Height_i; i++){
            for(int j=0; j<Width_i; j++){

                if(5<i && i<253 && 15 < j && j<343){
                    if(MAX_V < calibrationImage[i][j]){
                        MAX_V = calibrationImage[i][j];
                    }

                    if(MIN_V > calibrationImage[i][j]){
                        MIN_V = calibrationImage[i][j];
                    }

                    image[i][j] = calibrationImage[i][j];
                }
            }
        }

        for(int i=0; i<Height_i; i++){
            for(int j=0; j<Width_i; j++){
                if(!(5<i && i<253 && 15 < j && j<343)){
                    image[i][j] = MIN_V - ((MAX_V - MIN_V)/10);
                }
            }
        }
        int k=0;
        for(int i=0; i<248; i++){
            for(int j=0; j<328; j++){

                fitstemperature[k] =fitstemperature2[i][j];
                //                fitstemperature[k] =fitstemperature2[248-1-i][j];
                k++;
            }
        }
        makeColorTable();
        this->update();
    }


    else if(judge==1)
    {

        double sigma=5.67032*pow(10,-8);

        for(int i=0; i<Height_i; i++){
            for(int j=0; j<Width_i; j++){

                if(5<i && i<253 && 15 < j && j<343){
                    if(MAX_V < (epsilon*sigma*pow(calibrationImage[i][j],4))/PI){
                        MAX_V = (epsilon*sigma*pow(calibrationImage[i][j],4))/PI;
                    }

                    if(MIN_V > (epsilon*sigma*pow(calibrationImage[i][j],4))/PI){
                        MIN_V = (epsilon*sigma*pow(calibrationImage[i][j],4))/PI;
                    }

                    image[i][j] = (epsilon*sigma*pow(calibrationImage[i][j],4))/PI;
                }
            }
        }

        for(int i=0; i<Height_i; i++){
            for(int j=0; j<Width_i; j++){
                if(!(5<i && i<253 && 15 < j && j<343)){
                    image[i][j] = MIN_V - ((MAX_V - MIN_V)/10);
                }
            }
        }
        int k=0;
        for(int i=0; i<248; i++){
            for(int j=0; j<328; j++){
                fitstemperature[k] =fitstemperature2[i][j];
                //fitstemperature[k] =fitstemperature2[248-1-i][j];
                k++;
            }
        }

        makeColorTable();
        this->update();
    }


    long naxis    =   2;
    // long naxes[2] = { 384, 256 };
    long naxes[2] = { 328, 248 };
    std::auto_ptr<FITS> pFits(0);
    std::auto_ptr<FITS> pFits1(0);
    try
    {

        //QString fitdirectory= QFileDialog::getExistingDirectory(this, tr("Select the directory to save the image"),"/Applications/HEAT_DATA");
        if(fitdirectory == ""){
            return;
        }


        QString dateconversion=QString::fromStdString(date);
        QString YYYY,MM,DD,hh,mm,ss;
        QString YMD,hms;



        YYYY=dateconversion.mid(0,4);
        MM=dateconversion.mid(5,2);
        DD=dateconversion.mid(8,2);
        hh=dateconversion.mid(11,2);
        mm=dateconversion.mid(14,2);
        ss=dateconversion.mid(17,2);
        /*
        cout<<YYYY.toStdString()<<endl;
        cout<<MM.toStdString()<<endl;
        cout<<DD.toStdString()<<endl;
        cout<<hh.toStdString()<<endl;
        cout<<mm.toStdString()<<endl;
        cout<<ss.toStdString()<<endl;
        */
        YMD=YYYY+MM+DD;
        hms=hh+mm+ss;


        QString fitfilename=QFileInfo(filename).fileName();
        QString fitfilenamewithoutsuffix = QFileInfo(fitfilename).baseName();


        if(fitfilenamewithoutsuffix.contains("_l1",Qt::CaseInsensitive)==1){
            fitfilenamewithoutsuffix.replace("_l1",qgetenv(""));
        }
        //const std::string fileName("!"+fitdirectory.toStdString()+"/"+fitfilename.toStdString()+"_temperature.fit");
        const std::string fileNametemp("!"+fitdirectory.toStdString()+"/"+fitfilenamewithoutsuffix.toStdString()+"_l2a.fit");
        const std::string fileNameRadiance("!"+fitdirectory.toStdString()+"/"+fitfilenamewithoutsuffix.toStdString()+"_l2b.fit");
        //const std::string fileNametemp("!"+fitdirectory.toStdString()+"/hyb2_tir__"+fitfilenamewithoutsuffix.toStdString()+"_l2a.fit");
        //const std::string fileNameRadiance("!"+fitdirectory.toStdString()+"/hyb2_tir__"+fitfilenamewithoutsuffix.toStdString()+"_l2b.fit");
        //  pFits.reset( new FITS(fileNametemp, SHORT_IMG , naxis , naxes ) );
        //pFits1.reset( new FITS(fileNameRadiance, SHORT_IMG , naxis , naxes ) );
        pFits.reset( new FITS(fileNametemp,DOUBLE_IMG , naxis , naxes ) );
        pFits1.reset( new FITS(fileNameRadiance, DOUBLE_IMG , naxis , naxes ) );


    }
    catch (FITS::CantCreate)
    {
        return;
    }

    int nelements(1);
    nelements = std::accumulate(&naxes[0],&naxes[naxis],1,std::multiplies<double>());
    std::valarray<double> arraytemp(nelements);

    for (int i = 0; i < 328*248; i++)
    {
        arraytemp[i]=fitstemperature[i];
    }
    int  fpixel(1);
    pFits->pHDU().addKey("ORIGIN",origin,"organization responsible for the data");
    pFits->pHDU().addKey("DATE",date,"date of generation of HDU in UTC");
    pFits->pHDU().addKey("DATE-BEG",date_beg,"start date of observation program (UTC)");
    pFits->pHDU().addKey("DATE-OBS",date_obs,"start date of observation (UTC)");
    pFits->pHDU().addKey("DATE-END",date_end,"end date of observation (UTC)");
    pFits->pHDU().addKey("TELESCOP",telescop,"telescope used to acquire data");
    pFits->pHDU().addKey("INSTRUME",instrume,"name of instrument");
    pFits->pHDU().addKey("OBJECT",object,"name of observed object");
    pFits->pHDU().addKey("BUNIT",bunit,"physical unit of array values");
    pFits->pHDU().writeComment("");
    pFits->pHDU().writeComment("***** TIR Common Information *****");
    pFits->pHDU().writeComment("");
    pFits->pHDU().addKey("XPOSURE",QString::number(xposure).toStdString(),"exposure time [sec]");
    pFits->pHDU().addKey("IFOV",QString::number(ifov).toStdString(),"instantaneous field of view [rad]");
    pFits->pHDU().addKey("FILTER",filter,"bandpath range of filter (um)");
    pFits->pHDU().writeComment("");
    pFits->pHDU().writeComment("***** Observation Program *****");
    pFits->pHDU().writeComment("");
    pFits->pHDU().addKey("OPRGNAME",oprgname,"observation program name");
    pFits->pHDU().addKey("OPRGNO",oprgno,"observation program number");
    pFits->pHDU().writeComment("");
    pFits->pHDU().writeComment("***** TIR Image Information *****");
    pFits->pHDU().writeComment("");
    pFits->pHDU().addKey("ROI_LLX",QString::number(roi_llx).toStdString(),"x lower-left corner pixel of image ");
    pFits->pHDU().addKey("ROI_LLY",QString::number(roi_lly).toStdString(),"y lower-left corner pixel of image");
    pFits->pHDU().addKey("ROI_URX",QString::number(roi_urx).toStdString(),"x upper-right corner pixel of image");
    pFits->pHDU().addKey("ROI_URY",QString::number(roi_ury).toStdString(),"y upper-right corner pixel of image");
    pFits->pHDU().addKey("DATAMAX",QString::number(datamax).toStdString(),"maximum data value");
    pFits->pHDU().addKey("DATAMIN",QString::number(datamin).toStdString(),"minimum data value");
    pFits->pHDU().addKey("MEAN",QString::number(mean).toStdString(),"mean value of the data");
    pFits->pHDU().addKey("STDEV",QString::number(stdev).toStdString(),"standard deviation of the data");
    pFits->pHDU().addKey("MISS_VAL",QString::number(miss_val).toStdString(),"flag value of missing pixel");
    pFits->pHDU().addKey("MISS_NUM",QString::number(miss_num).toStdString(),"number of missing pixel");
    pFits->pHDU().addKey("DEAD_VAL",QString::number(dead_val).toStdString(),"flag value of dead pixel");
    pFits->pHDU().addKey("DEAD_NUM",QString::number(dead_num).toStdString(),"number of dead pixel");
    pFits->pHDU().addKey("SATU_VAL",QString::number(satu_val).toStdString(),"flag value of saturated pixel");
    pFits->pHDU().addKey("SATU_NUM",QString::number(satu_num).toStdString(),"number of saturated pixels");
    pFits->pHDU().addKey("IMGCMPRV",imgcmprv,"compression rev.: RAW_DAT/LOSSLESS/LOSSY");
    pFits->pHDU().addKey("IMGCMPAL",imgcmpal,"compression alg.: JPEG2000/STAR_PIXEL");
    pFits->pHDU().addKey("IMGCMPPR",imgcmppr,"compression parameter");
    pFits->pHDU().addKey("IMG_ERR",QString::number(img_err).toStdString(),"onboard image proc. return status");
    pFits->pHDU().writeComment("");
    pFits->pHDU().writeComment("***** TIR Telemetry *****");
    pFits->pHDU().writeComment("");
    pFits->pHDU().addKey("IMGSEQC",imgseqc,"image sequence counter");
    pFits->pHDU().addKey("IMGACCM",QString::number(imgaccm).toStdString(),"number of accumulated images");
    pFits->pHDU().addKey("BITDEPTH",QString::number(bitdepth).toStdString(),"image bit depth");
    pFits->pHDU().addKey("PLT_POW",plt_pow,"peltier ON/OFF");
    pFits->pHDU().addKey("PLT_STAT",plt_stat,"peltier status");
    pFits->pHDU().addKey("BOL_STAT",bol_stat,"bolometer status");
    pFits->pHDU().addKey("BOL_TRGT",QString::number(bol_trgt).toStdString(),"bolometer calibration target");
    pFits->pHDU().addKey("BOL_RANG",QString::number(bol_rang).toStdString(),"bolometer calibration range");
    pFits->pHDU().addKey("BOL_TEMP",QString::number(bol_temp).toStdString(),"bolometer temperature [degC]");
    pFits->pHDU().addKey("PKG_TEMP",QString::number(pkg_temp).toStdString(),"package temperature [degC]");
    pFits->pHDU().addKey("CAS_TEMP",QString::number(cas_temp).toStdString(),"case temperature [degC]");
    pFits->pHDU().addKey("SHT_TEMP",QString::number(sht_temp).toStdString(),"shutter temperature [degC]");
    pFits->pHDU().addKey("LEN_TEMP",QString::number(len_temp).toStdString(),"lens temperature [degC]");
    pFits->pHDU().addKey("BGR_VOL",QString::number(bgr_vol).toStdString(),"BGR voltage [V]");
    pFits->pHDU().addKey("VB1_VOL",QString::number(vb1_vol).toStdString(),"VB1 voltage [V]");
    pFits->pHDU().addKey("ADOFSVOL",QString::number(adofsvol).toStdString(),"A/D_OFS voltage [V]");
    pFits->pHDU().addKey("HCE_TEMP",QString::number(hce_temp).toStdString(),"HCE TIR sensor temperature [degC]");
    pFits->pHDU().addKey("PNL_TEMP",QString::number(pnl_temp).toStdString(),"HCE TIR sensor panel temperature [degC]");
    pFits->pHDU().addKey("AE_TEMP",QString::number(ae_temp).toStdString(),"HCE TIR analog electronics temperature [degC]");
    pFits->pHDU().writeComment("");
    pFits->pHDU().writeComment("***** Observation Information by SPICE kernel *****");
    pFits->pHDU().writeComment("");
    pFits->pHDU().addKey("S_DISTHT",QString::number(s_distht).toStdString(),"distance between HYB2 and the target [km]");
    pFits->pHDU().addKey("S_DISTHE",QString::number(s_disthe).toStdString(),"distance between HYB2 and Earth [km]");
    pFits->pHDU().addKey("S_DISTHS",QString::number(s_disths).toStdString(),"distance between HYB2 and Sun [km]");
    pFits->pHDU().addKey("S_DISTTS",QString::number(s_distts).toStdString(),"distance between the target and Sun [km]");
    pFits->pHDU().addKey("S_TGRADI",QString::number(s_tgradi).toStdString(),"the target radius at the equator [km]");
    pFits->pHDU().addKey("S_APPDIA",QString::number(s_appdia).toStdString(),"apparent diameter of the target [deg]");
    pFits->pHDU().addKey("S_SOLLAT",QString::number(s_sollat).toStdString(),"sub solar latitude [deg] of the target");
    pFits->pHDU().addKey("S_SOLLON",QString::number(s_sollon).toStdString(),"sub solar longitude [deg] of the target");
    pFits->pHDU().addKey("S_SSCLAT",QString::number(s_ssclat).toStdString(),"sub S/C latitude [deg] of the target");
    pFits->pHDU().addKey("S_SSCLON",QString::number(s_ssclon).toStdString(),"sub S/C longitude [deg] of the target");
    pFits->pHDU().addKey("S_SSCLST",QString::number(s_ssclst).toStdString(),"sub S/C local solar time [h] of the target");
    pFits->pHDU().addKey("S_SSCPX",QString::number(s_sscpx).toStdString(),"sub S/C position on Image Array (axis1)");
    pFits->pHDU().addKey("S_SSCPY",QString::number(s_sscpy).toStdString(),"sub S/C position on Image Array (axis2)");
    pFits->pHDU().addKey("S_SCXSAN",QString::number(s_scxsan).toStdString(),"angle of S/C X axis and Sun direction [deg]");
    pFits->pHDU().addKey("S_SCYSAN",QString::number(s_scysan).toStdString(),"angle of S/C Y axis and Sun direction [deg]");
    pFits->pHDU().addKey("S_SCZSAN",QString::number(s_sczsan).toStdString(),"angle of S/C Z axis and Sun directino [deg]");
    pFits->pHDU().writeComment("");
    pFits->pHDU().writeComment("***** SPICE Kernels *****");
    pFits->pHDU().writeComment("");
    pFits->pHDU().addKey("NAIFNAME",naifname,"SPICE instrument name");
    pFits->pHDU().addKey("NAIFID",QString::number(naifid).toStdString(),"SPICE instrument ID");
    pFits->pHDU().addKey("MKNAME",mkname,"SPICE Meta kernel name");
    pFits->pHDU().writeComment("");
    pFits->pHDU().writeComment("***** Version ***** ");
    pFits->pHDU().writeComment("");
    pFits->pHDU().addKey("VERSION",QString::number(version).toStdString(),"version of the HDU");

    /*
    //FITSヘッダに使った地上実験ファイル名を列挙する場合に使用
    QFile filex(dirpath+"/UsedImage.txt");
    if(filex.exists()){
        filex.open(QIODevice::ReadOnly);
        QTextStream load(&filex);

        int i=0;
        QString usedimage[5000];

        while(!load.atEnd()){
            usedimage[i]= load.readLine();
            pFits->pHDU().addKey("IMAGE"+QString::number(i).toStdString(),usedimage[i].toStdString(),"used ground data");
            i++;
        }
        filex.close();
    }

  */


    pFits->pHDU().write(fpixel,nelements,arraytemp);

    int nelements1(1);
    nelements1 = std::accumulate(&naxes[0],&naxes[naxis],1,std::multiplies<double>());
    std::valarray<double> arrayRadiance(nelements1);


    double sigma=5.67032*pow(10,-8);
    for (int i = 0; i < 328*248; i++)
    {
        arrayRadiance[i]=(epsilon*sigma*pow(fitstemperature[i],4))/PI;

    }
    int  fpixel1(1);
    pFits1->pHDU().addKey("ORIGIN",origin,"organization responsible for the data");
    pFits1->pHDU().addKey("DATE",date,"date of generation of HDU in UTC");
    pFits1->pHDU().addKey("DATE-BEG",date_beg,"start date of observation program (UTC)");
    pFits1->pHDU().addKey("DATE-OBS",date_obs,"start date of observation (UTC)");
    pFits1->pHDU().addKey("DATE-END",date_end,"end date of observation (UTC)");
    pFits1->pHDU().addKey("TELESCOP",telescop,"telescope used to acquire data");
    pFits1->pHDU().addKey("INSTRUME",instrume,"name of instrument");
    pFits1->pHDU().addKey("OBJECT",object,"name of observed object");
    pFits1->pHDU().addKey("BUNIT",bunit,"physical unit of array values");
    pFits1->pHDU().writeComment("");
    pFits1->pHDU().writeComment("***** TIR Common Information *****");
    pFits1->pHDU().writeComment("");
    pFits1->pHDU().addKey("XPOSURE",QString::number(xposure).toStdString(),"exposure time [sec]");
    pFits1->pHDU().addKey("IFOV",QString::number(ifov).toStdString(),"instantaneous field of view [rad]");
    pFits1->pHDU().addKey("FILTER",filter,"bandpath range of filter (um)");
    pFits1->pHDU().writeComment("");
    pFits1->pHDU().writeComment("***** Observation Program *****");
    pFits1->pHDU().writeComment("");
    pFits1->pHDU().addKey("OPRGNAME",oprgname,"observation program name");
    pFits1->pHDU().addKey("OPRGNO",oprgno,"observation program number");
    pFits1->pHDU().writeComment("");
    pFits1->pHDU().writeComment("***** TIR Image Information *****");
    pFits1->pHDU().writeComment("");
    pFits1->pHDU().addKey("ROI_LLX",QString::number(roi_llx).toStdString(),"x lower-left corner pixel of image ");
    pFits1->pHDU().addKey("ROI_LLY",QString::number(roi_lly).toStdString(),"y lower-left corner pixel of image");
    pFits1->pHDU().addKey("ROI_URX",QString::number(roi_urx).toStdString(),"x upper-right corner pixel of image");
    pFits1->pHDU().addKey("ROI_URY",QString::number(roi_ury).toStdString(),"y upper-right corner pixel of image");
    pFits1->pHDU().addKey("DATAMAX",QString::number(datamax).toStdString(),"maximum data value");
    pFits1->pHDU().addKey("DATAMIN",QString::number(datamin).toStdString(),"minimum data value");
    pFits1->pHDU().addKey("MEAN",QString::number(mean).toStdString(),"mean value of the data");
    pFits1->pHDU().addKey("STDEV",QString::number(stdev).toStdString(),"standard deviation of the data");
    pFits1->pHDU().addKey("MISS_VAL",QString::number(miss_val).toStdString(),"flag value of missing pixel");
    pFits1->pHDU().addKey("MISS_NUM",QString::number(miss_num).toStdString(),"number of missing pixel");
    pFits1->pHDU().addKey("DEAD_VAL",QString::number(dead_val).toStdString(),"flag value of dead pixel");
    pFits1->pHDU().addKey("DEAD_NUM",QString::number(dead_num).toStdString(),"number of dead pixel");
    pFits1->pHDU().addKey("SATU_VAL",QString::number(satu_val).toStdString(),"flag value of saturated pixel");
    pFits1->pHDU().addKey("SATU_NUM",QString::number(satu_num).toStdString(),"number of saturated pixels");
    pFits1->pHDU().addKey("IMGCMPRV",imgcmprv,"compression rev.: RAW_DAT/LOSSLESS/LOSSY");
    pFits1->pHDU().addKey("IMGCMPAL",imgcmpal,"compression alg.: JPEG2000/STAR_PIXEL");
    pFits1->pHDU().addKey("IMGCMPPR",imgcmppr,"compression parameter");
    pFits1->pHDU().addKey("IMG_ERR",QString::number(img_err).toStdString(),"onboard image proc. return status");
    pFits1->pHDU().writeComment("");
    pFits1->pHDU().writeComment("***** TIR Telemetry *****");
    pFits1->pHDU().writeComment("");
    pFits1->pHDU().addKey("IMGSEQC",imgseqc,"image sequence counter");
    pFits1->pHDU().addKey("IMGACCM",QString::number(imgaccm).toStdString(),"number of accumulated images");
    pFits1->pHDU().addKey("BITDEPTH",QString::number(bitdepth).toStdString(),"image bit depth");
    pFits1->pHDU().addKey("PLT_POW",plt_pow,"peltier ON/OFF");
    pFits1->pHDU().addKey("PLT_STAT",plt_stat,"peltier status");
    pFits1->pHDU().addKey("BOL_STAT",bol_stat,"bolometer status");
    pFits1->pHDU().addKey("BOL_TRGT",QString::number(bol_trgt).toStdString(),"bolometer calibration target");
    pFits1->pHDU().addKey("BOL_RANG",QString::number(bol_rang).toStdString(),"bolometer calibration range");
    pFits1->pHDU().addKey("BOL_TEMP",QString::number(bol_temp).toStdString(),"bolometer temperature [degC]");
    pFits1->pHDU().addKey("PKG_TEMP",QString::number(pkg_temp).toStdString(),"package temperature [degC]");
    pFits1->pHDU().addKey("CAS_TEMP",QString::number(cas_temp).toStdString(),"case temperature [degC]");
    pFits1->pHDU().addKey("SHT_TEMP",QString::number(sht_temp).toStdString(),"shutter temperature [degC]");
    pFits1->pHDU().addKey("LEN_TEMP",QString::number(len_temp).toStdString(),"lens temperature [degC]");
    pFits1->pHDU().addKey("BGR_VOL",QString::number(bgr_vol).toStdString(),"BGR voltage [V]");
    pFits1->pHDU().addKey("VB1_VOL",QString::number(vb1_vol).toStdString(),"VB1 voltage [V]");
    pFits1->pHDU().addKey("ADOFSVOL",QString::number(adofsvol).toStdString(),"A/D_OFS voltage [V]");
    pFits1->pHDU().addKey("HCE_TEMP",QString::number(hce_temp).toStdString(),"HCE TIR sensor temperature [degC]");
    pFits1->pHDU().addKey("PNL_TEMP",QString::number(pnl_temp).toStdString(),"HCE TIR sensor panel temperature [degC]");
    pFits1->pHDU().addKey("AE_TEMP",QString::number(ae_temp).toStdString(),"HCE TIR analog electronics temperature [degC]");
    pFits1->pHDU().writeComment("");
    pFits1->pHDU().writeComment("***** Observation Information by SPICE kernel *****");
    pFits1->pHDU().writeComment("");
    pFits1->pHDU().addKey("S_DISTHT",QString::number(s_distht).toStdString(),"distance between HYB2 and the target [km]");
    pFits1->pHDU().addKey("S_DISTHE",QString::number(s_disthe).toStdString(),"distance between HYB2 and Earth [km]");
    pFits1->pHDU().addKey("S_DISTHS",QString::number(s_disths).toStdString(),"distance between HYB2 and Sun [km]");
    pFits1->pHDU().addKey("S_DISTTS",QString::number(s_distts).toStdString(),"distance between the target and Sun [km]");
    pFits1->pHDU().addKey("S_TGRADI",QString::number(s_tgradi).toStdString(),"the target radius at the equator [km]");
    pFits1->pHDU().addKey("S_APPDIA",QString::number(s_appdia).toStdString(),"apparent diameter of the target [deg]");
    pFits1->pHDU().addKey("S_SOLLAT",QString::number(s_sollat).toStdString(),"sub solar latitude [deg] of the target");
    pFits1->pHDU().addKey("S_SOLLON",QString::number(s_sollon).toStdString(),"sub solar longitude [deg] of the target");
    pFits1->pHDU().addKey("S_SSCLAT",QString::number(s_ssclat).toStdString(),"sub S/C latitude [deg] of the target");
    pFits1->pHDU().addKey("S_SSCLON",QString::number(s_ssclon).toStdString(),"sub S/C longitude [deg] of the target");
    pFits1->pHDU().addKey("S_SSCLST",QString::number(s_ssclst).toStdString(),"sub S/C local solar time [h] of the target");
    pFits1->pHDU().addKey("S_SSCPX",QString::number(s_sscpx).toStdString(),"sub S/C position on Image Array (axis1)");
    pFits1->pHDU().addKey("S_SSCPY",QString::number(s_sscpy).toStdString(),"sub S/C position on Image Array (axis2)");
    pFits1->pHDU().addKey("S_SCXSAN",QString::number(s_scxsan).toStdString(),"angle of S/C X axis and Sun direction [deg]");
    pFits1->pHDU().addKey("S_SCYSAN",QString::number(s_scysan).toStdString(),"angle of S/C Y axis and Sun direction [deg]");
    pFits1->pHDU().addKey("S_SCZSAN",QString::number(s_sczsan).toStdString(),"angle of S/C Z axis and Sun directino [deg]");
    pFits1->pHDU().writeComment("");
    pFits1->pHDU().writeComment("***** SPICE Kernels *****");
    pFits1->pHDU().writeComment("");
    pFits1->pHDU().addKey("NAIFNAME",naifname,"SPICE instrument name");
    pFits1->pHDU().addKey("NAIFID",QString::number(naifid).toStdString(),"SPICE instrument ID");
    pFits1->pHDU().addKey("MKNAME",mkname,"SPICE Meta kernel name");
    pFits1->pHDU().writeComment("");
    pFits1->pHDU().writeComment("***** Version ***** ");
    pFits1->pHDU().writeComment("");
    pFits1->pHDU().addKey("VERSION",QString::number(version).toStdString(),"version of the HDU");


    /*
    //FITSヘッダに使った地上実験ファイル名を列挙する場合に使用
    if(filex.exists()){
        filex.open(QIODevice::ReadOnly);
        QTextStream load(&filex);

        int i=0;
        QString usedimage[5000];

        while(!load.atEnd()){
            usedimage[i]= load.readLine();
            pFits1->pHDU().addKey("IMAGE"+QString::number(i).toStdString(),usedimage[i].toStdString(),"used ground data");
            i++;
        }
        filex.close();
    }
*/

    pFits1->pHDU().write(fpixel1,nelements1,arrayRadiance);
    //    std::cout << pFits1->pHDU() << std::endl;



}

void ShowImage::initializeCalibrateImage() {
    for (int i = 0; i < Height_i; i++) {
        for (int j = 0; j < Width_i; j++) {

            calibrationImage[i][j] = 150;
        }
    }
}

void ShowImage::loadImageD(QVector<double> img) {
    int count = 0;
    MAX_V = -10000;
    MIN_V = 10000;
    for (int i = 0; i < Height_i; i++) {
        for (int j = 0; j < Width_i; j++) {
            image[i][j] = img[count];
            if (image[i][j] > MAX_V)
                MAX_V = image[i][j];
            if (image[i][j] < MIN_V)
                MIN_V = image[i][j];
            count++;
        }
    }

    for (int i = 1;; i++) {
        if (100 * (i - 1) < MAX_V && MAX_V < 100 * i) {
            MAX_V = 100 * i;
            break;
        }
    }

    makeColorTable();

    this->update();
}

void ShowImage::loadTxtImageD(QString name) {

    MAX_V = 0;
    MIN_V = 100000;

    openTxtImageD(name);
    makeColorTable();

    this->update();
}

int ShowImage::getMaxDN() { return MAX_V; }

int ShowImage::getMinDN() { return MIN_V; }

QVector<double> ShowImage::getImageD() {

    QVector<double> tmp;

    for (int i = 0; i < xmldata.l1.Height_data; i++) {
        for (int j = 0; j < xmldata.l1.Width_data; j++) {
            tmp.append(image[i][j]);
        }
    }

    return tmp;
}

void ShowImage::openTxtImageD(QString name) {

    QFile read(name);
    QString tmpS;

    if (!read.open(QIODevice::ReadOnly))
        return;

    int i = 0;
    int j = 0;
    double tmp;

    QTextStream in(&read);

    while (!in.atEnd()) {
        in >> tmpS;
        tmp = tmpS.toDouble();

        if (251 > i && i > 8 && 342 > j && j > 6) {
            if (tmp > MAX_V)
                MAX_V = tmp;
            if (tmp < MIN_V)
                MIN_V = tmp;
        }
        image[i][j] = tmp;

        j++;

        if (j / (Width_i) == 1) {
            i++;
            j = 0;
        }
    }

    read.close();
}

double ShowImage::planck(double T){
    //double x0=8e-6, x1=12e-6; //8μm~12μm area
    //double x0=1e-8, x1=20e-6; //
    double lambda,Bt,integral=0;

    // 1000=10μm　100000=100μm
    for (int i=1;i<2001;i++) {
        lambda = (double)i*1e-8; // 0.00000001m=0.01μm step
        Bt = ((2*h_planck*c_speed*c_speed)/(pow(lambda,5))/(pow(M_E,c2/(lambda*T))-1.0)*tirfilterforshowimage[i-1][1]*epsilon);
        integral+=(Bt);
    }
    integral *= 1e-8;//0.00000001
    return integral;
}

void ShowImage::loadFilter() {

    QString str;

    QString appPath;

    appPath = QCoreApplication::applicationDirPath();

    QFile file(appPath + "/tir_response.txt");

    if (!file.open(QIODevice::ReadOnly)) {
        printf("tir_response.txt open error\n");
        return;
    }

    QTextStream in(&file);

    for (int i = 0; !in.atEnd(); i++) {
        for (int j = 0; j < 3; j++) {
            in >> str;

            tirfilterforshowimage[i][j] = str.toDouble();
        }
    }
}

void ShowImage::connectDB() {
    db.open();

    query = QSqlQuery(db);
    if (query.isActive()) {
        query.first();
    }
}


void ShowImage::readLUTfile(QString name,double *slope_lut,double *intercept_lut,int index,int num)
{
    auto_ptr<FITS> pInfile(0);
    pInfile.reset(new FITS(name.toStdString().c_str(), Read, true));
    int hduindex=0;
    String hduName;
    int hduVersion;
    int h;
    int w;
    int i,j,counter=0;

    CCfits::ExtHDU::readHduName(pInfile->fitsPointer(),hduindex, hduName, hduVersion);
    std::auto_ptr<FITS> pInfile1(new FITS(name.toStdString(),Read,hduName,false));
    cout<<pInfile1->currentExtensionName()<<endl;
    ExtHDU& EH1 = pInfile1->extension(pInfile1->currentExtensionName(),1);
    EH1.read(slope);
    w = EH1.axis(0);
    h = EH1.axis(1);

    slope_lut[num]=slope[index];

    counter = 0;
    hduindex = 1;
    CCfits::ExtHDU::readHduName(pInfile->fitsPointer(),hduindex, hduName, hduVersion);
    std::auto_ptr<FITS> pInfile2(new FITS(name.toStdString(),Read,hduName,false));
    cout<<pInfile2->currentExtensionName()<<endl;
    ExtHDU& EH2 = pInfile2->extension(pInfile2->currentExtensionName(),1);
    EH2.read(intercept);
    w = EH2.axis(0);
    h = EH2.axis(1);

    intercept_lut[num]=intercept[index];
}


void ShowImage::readLUTfile(QString name)
{
    name = "";
    auto_ptr<FITS> pInfile(0);
    pInfile.reset(new FITS(name.toStdString().c_str(), Read, true));
    int hduindex=0;
    String hduName;
    int hduVersion;
    int h;
    int w;
    int i,j,counter=0;

    CCfits::ExtHDU::readHduName(pInfile->fitsPointer(),hduindex, hduName, hduVersion);
    std::auto_ptr<FITS> pInfile1(new FITS(name.toStdString(),Read,hduName,false));
    //cout<<pInfile1->currentExtensionName()<<endl;
    ExtHDU& EH1 = pInfile1->extension(pInfile1->currentExtensionName(),1);
    EH1.read(slope);
    w = EH1.axis(0);
    h = EH1.axis(1);

    counter = 0;
    hduindex = 1;
    CCfits::ExtHDU::readHduName(pInfile->fitsPointer(),hduindex, hduName, hduVersion);
    std::auto_ptr<FITS> pInfile2(new FITS(name.toStdString(),Read,hduName,false));
    //cout<<pInfile2->currentExtensionName()<<endl;
    ExtHDU& EH2 = pInfile2->extension(pInfile2->currentExtensionName(),1);
    EH2.read(intercept);
    w = EH2.axis(0);
    h = EH2.axis(1);
}

