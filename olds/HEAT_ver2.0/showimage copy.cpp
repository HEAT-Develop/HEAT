//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//

#include "showimage.h"
#include "mainwindow.h"
#include "pixcelgraph.h"
#include <FITS.h>
#include <calibrationgraph.h>
#include <controlgraphpanel.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <fitsio.h>
#include <QDir>

#define Max_Width 1024
#define Max_Height 768

using namespace std;
using namespace CCfits;

int effectivecounter;

ShowImage::ShowImage(QWidget *parent) : QOpenGLWidget(parent) {
    MAX_V = 0;
    MIN_V = 100000;
    judge = true;
    renderunitflag = true;
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
    for(int i=0; i<Height; i++){
        for(int j=0; j<Width; j++){
            darkimage[i][j]=-1000;
        }
    }
}

void ShowImage::resizeGL(int width, int height) {
    glViewport(0, 0, width, height);
    glLoadIdentity();

    glOrtho(-10, Width + 100, Height + 10, -10, -1, 1);
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
        for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 15; j++) {
                glColor3dv(colorTable[255 - i]);
                glVertex2d(j + 395, i);
            }
        }

        glEnd();

        QPainter num(this);
        num.setPen(Qt::cyan);
        num.setFont(QFont("Arial", 15));

        if (bunit=="adu"||bunit=="DN") {
            num.drawText(getWidth()+86, 53, "DN");
        } else {
            num.drawText(getWidth()+86, 53, "K");
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

void ShowImage::render_TIRI() {

    double T = MAX_V - MIN_V;

    if (judge == true) {

        glBegin(GL_POINTS);
        for (int i = 0; i < 768; i++) {
            for (int j = 0; j < 15; j++) {
                glColor3dv(colorTable_TIRI[767 - i]);
                glVertex2d(j + 395, i);
            }
        }

        glEnd();

        QPainter num(this);
        num.setPen(Qt::cyan);
        num.setFont(QFont("Arial", 15));

        if (bunit=="adu"||bunit=="DN") {
            num.drawText(getWidth()+86, 53, "DN");
        } else {
            num.drawText(getWidth()+86, 53, "K");
        }

        num.drawText(getWidth()+43, 24, QString::number(MAX_V));
        num.drawText(getWidth()+43, 85, QString::number((int)(MAX_V - T / 4)));
        num.drawText(getWidth()+43, 146, QString::number((int)(MAX_V - T / 2)));
        num.drawText(getWidth()+43, 206, QString::number((int)(MAX_V - T * 3 / 4)));
        num.drawText(getWidth()+43, 265, QString::number(MIN_V));
        num.end();
    }

    glBegin(GL_POINTS);

    pixelDraw_TIRI(T);
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
    Width = 384;
    Height = 256;
    filename = name;

    loadBuffer();

    makeColorTable();

    this->update();
}


void ShowImage::loadFileName_TIRI(QString name) {
    MAX_V = 0;
    MIN_V = 100000;
    Width = 1024;
    Height = 768;
    filename = name;

    loadBuffer_TIRI();

    //makeColorTable_TIRI();
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
        if(Width>Max_Width || Height>Max_Height){
            QMessageBox msgBox;
            msgBox.setText("The image data size is too large.");
            msgBox.exec();
            return;
        }


        try {
            pInfile.reset(new FITS(filename.toStdString().c_str(), Read, true));
            PHDU &fitsImage = pInfile->pHDU();
            fitsImage.read(contents);
            fitsImage.readAllKeys();
            Width = pInfile->pHDU().axis(0);
            Height = pInfile->pHDU().axis(1);
            this->setHeight(Height);
            this->setWidth(Width);
            this->resize(getWidth()+110,getHeight()+20);
            if(Width>Max_Width || Height>Max_Height){
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
            for(int i=0; i<Height; i++){
                for(int j=0; j<Width; j++){
                    DNtmp1=contents[counter];
                    fitsave+=DNtmp1;
                    //qDebug()<<DNtmp1;
                    counter++;
                }
            }


            fitsave=fitsave/(Height*Width);
            qDebug()<<"fitsave="<<fitsave;

            counter=0;
            double tmp1=0;

            for(int i=0; i<Height; i++){
                for(int j=0; j<Width; j++){
                    tmp1=contents[counter];

                    if(fitsave<-700){
                        tmp1=tmp1/8;
                    }

                    if(tmp1>MAX_V)
                        MAX_V=tmp1;
                    if(tmp1<MIN_V)
                        MIN_V=tmp1;
                    //image[Height-1-i][j]=tmp1;
                    image[i][j]=tmp1;

                    counter++;
                }
            }
            qDebug()<<image[130][170];
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
        for (int i = 0; i < 384; i++) {
            for (int j = 0; j < 256; j++) {
                image[255 - j][i] = imagetmp[k][3];
                if (image[255 - j][i] > MAX_V)
                    MAX_V = image[255 - j][i];
                if (image[255 - j][i] < MIN_V)
                    MIN_V = image[255 - j][i];

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

        this->setWidth(384);
        this->setHeight(256);
        this->resize(getWidth()+100,getHeight()+20);
        while( !ifs.fail() )
        {
            if( Count >= getHeight()*getWidth()*2)  break;

            ifs.read( &Data,1);
            tmp=0xff&Data;
            ++Count;

            ifs.read( &Data,1);


            tmp+=(0xff&Data)*256;
            if(tmp==-9999)tmp=0;
            if(tmp>MAX_V)MAX_V=tmp;
            if(tmp<MIN_V)MIN_V=tmp;

            image[i][j]=(double)tmp;
            j++;
            if( Count % (getWidth()*2) == getWidth()*2-1 ){
                i++;
                j=0;
            }

            ++Count;
        }
        ifs.close();
    }
}





void ShowImage::loadBuffer_TIRI(){
    fstream ifs;
    ifs.open(&filename.toStdString()[0], ios::in | ios::binary);

    QFileInfo fileinfo;
    fileinfo.setFile(filename);
    QString ext = fileinfo.suffix();
    ext = ext.toLower();
    TIRIflag=1;

    if (ext == "fit" || ext == "fits" || ext == "fts") {
        fitsflag = 1;
        valarray<double> contents;
        auto_ptr<FITS> pInfile(0);
        if(Width>Max_Width || Height>Max_Height){
            QMessageBox msgBox;
            msgBox.setText("The image data size is too large.");
            msgBox.exec();
            return;
        }


        try {
            pInfile.reset(new FITS(filename.toStdString().c_str(), Read, true));
            PHDU &fitsImage = pInfile->pHDU();
            fitsImage.read(contents);
            fitsImage.readAllKeys();
            Width = pInfile->pHDU().axis(0);
            Height = pInfile->pHDU().axis(1);
            this->setHeight(Height);
            this->setWidth(Width);
            this->resize(getWidth()+110,getHeight()+20);
            if(Width>Max_Width || Height>Max_Height){
                QMessageBox msgBox;
                msgBox.setText("The image data size is too large.");
                msgBox.exec();
                return;
            }


            try {
                pInfile->pHDU().readKey<string>("BUNIT", bunit);
                printf("Read BUNIT\n");
            } catch (...) {
                printf("Can not read BUNIT\n");
            };
            

            

            double DNtmp1=0;
            double fitsave=0;
            int counter=0;
            for(int i=0; i<Height; i++){
                for(int j=0; j<Width; j++){
                    DNtmp1=contents[counter];
                    fitsave+=DNtmp1;
                    counter++;
                }
            }


            fitsave=fitsave/(Height*Width);

            counter=0;
            double tmp1=0;

            for(int i=0; i<Height; i++){
                for(int j=0; j<Width; j++){
                    tmp1=contents[counter];

                    if(fitsave<-700){
                        tmp1=tmp1/8;
                    }

                    if(tmp1>MAX_V)
                        MAX_V=tmp1;
                    if(tmp1<MIN_V)
                        MIN_V=tmp1;
                    //image[Height-1-i][j]=tmp1;
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
        for (int i = 0; i < 1024; i++) {
            for (int j = 0; j < 768; j++) {
                image[767 - j][i] = imagetmp[k][3];
                if (image[767 - j][i] > MAX_V)
                    MAX_V = image[767 - j][i];
                if (image[767 - j][i] < MIN_V)
                    MIN_V = image[767 - j][i];

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

        this->setWidth(1024);
        this->setHeight(768);
        this->resize(getWidth()+100,getHeight()+20);
        while( !ifs.fail() )
        {
            if( Count >= getHeight()*getWidth()*2)  break;

            ifs.read( &Data,1);
            tmp=0xff&Data;
            ++Count;

            ifs.read( &Data,1);


            tmp+=(0xff&Data)*768;
            if(tmp==-9999)tmp=0;
            if(tmp>MAX_V)MAX_V=tmp;
            if(tmp<MIN_V)MIN_V=tmp;

            image[i][j]=(double)tmp;
            j++;
            if( Count % (getWidth()*2) == getWidth()*2-1 ){
                i++;
                j=0;
            }

            ++Count;
        }
        ifs.close();
    }
}






void ShowImage::pixelDraw(double T) {

    double d = T / 256;

    for (int i = 0; i < 256; i++) {
        colorValue[i] = d * i + MIN_V;
    }

    colorValue[255] = MAX_V + 1;

    for (int i = 0; i < Height; i++) {
        for (int j = 0; j < Width; j++) {
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

void ShowImage::pixelDraw_TIRI(double T) {

    double d = T / 256;

    for (int i = 0; i < 256; i++) {
        colorValue[i] = d * i + MIN_V;
    }

    colorValue[255] = MAX_V + 1;

    for (int i = 0; i < Height; i++) {
        for (int j = 0; j < Width; j++) {
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
    Width = a;
}

void ShowImage::setHeight(int a)
{
    Height = a;
}

int ShowImage::getWidth()
{
    return Width;
}

int ShowImage::getHeight()
{
    return Height;
}
void ShowImage::makeColorTable() {
    int colorArea;
    double count = 1;

    if (colorselect == 0 || fitsflag == 1) {
        colorArea = 51;
        for (int i = 0; i < 256; i++) {
            if (count > colorArea)
                count = 1;
            if (i == 255)
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
        for (int i = 0; i < 256; i++) {
            colorTable[i][0] = (double)i / 255;
            colorTable[i][1] = (double)i / 255;
            colorTable[i][2] = (double)i / 255;
        }

    if (colorselect == 2) {
        colorArea = 64;
        for (int i = 0; i < 256; i++) {
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
void ShowImage::makeColorTable_TIRI() {
    int colorArea;
    double count = 1;

    if (colorselect == 0 || fitsflag == 1) {
        colorArea = 154;
        for (int i = 0; i < 768; i++) {
            if (count > colorArea)
                count = 1;
            if (i == 767)
                count = 155;

            if (i < colorArea) {
                colorTable_TIRI[i][0] = 1 - count / (colorArea + 1);
                colorTable_TIRI[i][1] = 0;
                colorTable_TIRI[i][2] = 1;
            }
            if (colorArea <= i && i < 2 * colorArea) {
                colorTable_TIRI[i][0] = 0;
                colorTable_TIRI[i][1] = count / (colorArea + 1);
                colorTable_TIRI[i][2] = 1;
            }
            if (2 * colorArea <= i && i < 3 * colorArea) {
                colorTable_TIRI[i][0] = 0;
                colorTable_TIRI[i][1] = 1;
                colorTable_TIRI[i][2] = 1 - count / (colorArea + 1);
            }
            if (3 * colorArea <= i && i < 4 * colorArea) {
                colorTable_TIRI[i][0] = count / (colorArea + 1);
                colorTable_TIRI[i][1] = 1;
                colorTable_TIRI[i][2] = 0;
            }
            if (4 * colorArea <= i && i < 5 * colorArea + 1) {
                colorTable_TIRI[i][0] = 1;
                colorTable_TIRI[i][1] = 1 - count / (colorArea + 2);
                colorTable_TIRI[i][2] = 0;
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


    cout<<"file1"<<endl;
    cout<<file1.toStdString()<<endl;

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

            for(int i=0; i<Height; i++){
                for(int j=0; j<Width; j++){

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

    cout<<"file1"<<endl;
    cout<<file1.toStdString()<<endl;
    cout<<"file2"<<endl;
    cout<<file2.toStdString()<<endl;


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
            cout<<"Max = "<<MAX_V<<endl;
            cout<<"Min = "<<MIN_V<<endl;
            cout<<endl;
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
    cout << "i = " <<i<<", jjj = "<<jjj;


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

    MAX_V=-100000;
    MIN_V=100000;


    cout<<"file1"<<endl;
    cout<<file1.toStdString()<<endl;
    cout<<"file2"<<endl;
    cout<<file2.toStdString()<<endl;
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


    while(1){
        while( !ifs1.fail() )
        {
            if( Count >= Height*Width*2)  break;

            ifs1.read( &Data,1);

            //buf[0] buf[1] → buf[1]*256+buf[0]
            //cout <<(int)(0xff&Data)<<endl;

            tmp=0xff&Data;

            //2バイト目
            ++Count;

            ifs1.read( &Data,1);
            tmp+=(0xff&Data)*256;
            // tmp=tmp/8;
            image[i][j]=tmp;


            j++;

            if( Count % (Width*2) == Width*2-1 ){
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
            if( Count >= Height*Width*2)  break;

            ifs2.read( &Data,1);

            //buf[0] buf[1] → buf[1]*256+buf[0]と2biteをリトルエンディアンで変換
            //1バイト目
            //データ表示 cout <<(int)(0xff&Data)<<endl;

            tmp=0xff&Data;

            ++Count;

            ifs2.read( &Data,1);
            tmp+=(0xff&Data)*256;
            //  tmp=tmp/8;
            image[i][j] -= tmp;
            if(image[i][j]>MAX_V)MAX_V=image[i][j];
            if(image[i][j]<MIN_V)MIN_V=image[i][j];

            j++;

            if( Count % (Width*2) == Width*2-1 ){
                i++;
                j=0;
            }

            ++Count;
        }

        ifs2.close();

        long naxis    =   2;
        long naxes[2] = { 384, 256 };
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
        for (int i = 0; i < 256; i++){
            for(int j=0;j<384;j++){

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

        for(int i=0; i<Height; i++){
            for(int j=0; j<Width; j++){
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


void ShowImage::calibrateImageforBlackbodyAllPixel(QString s, int x, int y){

    renderunitflag=false;

    double h = s.section(',',-2,-2).toDouble();
    double g = s.section(',',-3,-3).toDouble();
    
    //double h =500;
    //double g =500;
    qDebug()<<"h = "<<h;
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
    double slope[2];
    slope[0] = Colli.section(',',-3,-3).toDouble(); //slope of coli
    slope[1] = Furnace.section(',',-3,-3).toDouble();//slope of BB Furnace
    //slope[0] = s.section(',',-3,-3).toDouble(); //slope of coli
    //slope[1] = s.section(',',-3,-3).toDouble();//slope of BB Furnace


    double intercept[2];
    intercept[0] = Colli.section(',',-2,-2).toDouble(); //intercept of coli
    intercept[1] = Furnace.section(',',-2,-2).toDouble(); //intercept of Furnace

    //intercept[0] = s.section(',',-2,-2).toDouble(); //intercept of coli
    //intercept[1] = s.section(',',-2,-2).toDouble(); //intercept of Furnace
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
        tmp1 =gettemperature_epsillon1(FT1);

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


        tmp1 =gettemperature_epsillon1(FT1);
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


void ShowImage::calibrateImageforBlackbodyAllPixellut(double g, double h,int x, int y){

//     renderunitflag=false;

    
//     double DN = image[y+6][x+16];
//     double tmp1=0;
//     double FT=0;
//     double FT1=0;
//     double FT2=0;
//     double FT3=0;
//     FT=(DN-h)/g;
//     FT1=round1((DN-h)/g);
//     tmp1 = gettemperature(FT1);

//     /*    int count=-400;
//     double yy=0;
//     double xx=-400;

//     for(xx=-400;xx<600;xx++){
//         cout<<"else if(FT1<=";  cout<< round1(planck((xx+0.9+273.15))); cout<<"){"; cout<<endl;
//         cout<<"if(FT1==";  cout<< round1(planck((count+273.15))); cout<<")tmp1="; cout<<xx;   cout<<";"; cout<<endl;
//         for(yy=count+0.1;yy<count+1.0;yy=yy+0.1){
//             if(yy==xx+1)break;
//             else
//             {
//                 cout<<"else if(FT1<=";  cout<< round1(planck((yy+273.15))); cout<<")tmp1="; cout<<yy;   cout<<";";
//                 cout<<endl;
//             }
//         }
//         count=count+1;
//         cout<<"}";
//         cout<<endl;
//     }
// */
// /*
//     if(x==156 && y==116){
//         for(int i=-600;i<1001;i++){

//             cout<<"DN  ";
//             cout<<i;
//             cout<<"   F  ";
//             cout<<((i-h)/g)<<endl;
//         }
//     }
// */

//     calibrationImage[y][x] = tmp1 + 273.15;
//     fitstemperature2[y][x] = tmp1 + 273.15;




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
        g=lsma1*diameter + lsma0;
        lsm_intercept(size, intercept, &lsma2, &lsma3);

        h=lsma3*diameter + lsma2;
        FT1=round1((DN-h)/g);
        tmp1 =gettemperature_epsillon1(FT1);

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


        tmp1 =gettemperature_epsillon1(FT1);
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
        tmp1 = gettemperature(FT1);
        Radiance = epsilon *((tmp1+273.15)*(tmp1+273.15)*(tmp1+273.15)*(tmp1+273.15))/PI;
    }
    else{
        Radiance=0;
    }

    calibrationImage[y+2][x+16] = Radiance;

    fitstemperature2[y][x] = Radiance;

}

double ShowImage::getPixelValue(int y, int x) { return image[y][x]; }

void ShowImage::drawPixcelLineGraph(QString heightValue) {

    QVector<double> w(Width), DN(Width);
    bool checkInt = false;
    int y = heightValue.toInt(&checkInt, 10);

    if (0 <= y && y <= Width && checkInt == true) {
        for (int i = 0; i < Width; i++)
            w[i] = i;

        for (int i = 0; i < Width; i++)
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

void ShowImage::initializeFITSarray(){
    for(int i=0; i<248; i++){
        for(int j=0; j<328; j++){
            fitstemperature2[i][j]=150;
        }
    }
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
    Width = getWidth();
    Height = getHeight();
    if(judge==0){


        for(int i=0; i<Height; i++){
            for(int j=0; j<Width; j++){

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

        for(int i=0; i<Height; i++){
            for(int j=0; j<Width; j++){
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
        qDebug()<<"//makeColorTable();";
        this->update();
    }


    else if(judge==1)
    {

        double sigma=5.67032*pow(10,-8);

        for(int i=0; i<Height; i++){
            for(int j=0; j<Width; j++){

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

        for(int i=0; i<Height; i++){
            for(int j=0; j<Width; j++){
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
    /*

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
   */
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

    /*
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

    */
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

    //pFits1->pHDU().write(fpixel1,nelements1,arrayRadiance);
    //    std::cout << pFits1->pHDU() << std::endl;



}


void ShowImage::initializeCalibrateImage() {
    for (int i = 0; i < Height; i++) {
        for (int j = 0; j < Width; j++) {

            calibrationImage[i][j] = 150;
        }
    }
}

void ShowImage::loadImageD(QVector<double> img) {
    int count = 0;
    MAX_V = -10000;
    MIN_V = 10000;
    for (int i = 0; i < Height; i++) {
        for (int j = 0; j < Width; j++) {
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

    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 384; j++) {
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

    //qDebug() << "2";

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

        if (j / (Width) == 1) {
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

void ShowImage::readLUTfile(QString name,double *sloperyuji,double *interceptryuji,int index,int num)
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

    sloperyuji[num]=slope[index];

    counter = 0;
    hduindex = 1;
    CCfits::ExtHDU::readHduName(pInfile->fitsPointer(),hduindex, hduName, hduVersion);
    std::auto_ptr<FITS> pInfile2(new FITS(name.toStdString(),Read,hduName,false));
    cout<<pInfile2->currentExtensionName()<<endl;
    ExtHDU& EH2 = pInfile2->extension(pInfile2->currentExtensionName(),1);
    EH2.read(intercept);
    w = EH2.axis(0);
    h = EH2.axis(1);

    interceptryuji[num]=intercept[index];
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
    cout<<pInfile1->currentExtensionName()<<endl;
    ExtHDU& EH1 = pInfile1->extension(pInfile1->currentExtensionName(),1);
    EH1.read(slope);
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
}


void ShowImage::calibrationLutImage(double g, double h, int y, int x)
{
    double DN = image[y + 2][x + 16];
    //double DN = image[y + 2][x + 16];
    double c = 6.125;
    //qDebug()<<image[0][0];


    double DN1 = DN - c*(cas_temp - pkg_temp);//(3)
    c = 6.158;
    double DN2 = DN1 - c*(28 - sht_temp);//(4)
    double l2_radiance = (DN2 - h) / g;//(6)
    double l2_temp = 0;
    if(x==0&&y==0){
        string csv = "/Applications/temp_radiance_table.csv";
        stringstream ss;
        ifstream ifs_csv_file(csv);
        string str;
        for (int row = 0; row < 350; row++)
        {
            for (int col = 0; col < 1; col++)
            {
                getline(ifs_csv_file.seekg(0,ios_base::cur),str,',');
                ss.str(str);
                ss>>csv_data[row][col];
                cout<<csv_data[row][col]<<" ";
                ss.str("");
                ss.clear(stringstream::goodbit);
            }
            std::getline(ifs_csv_file.seekg(0,ios_base::cur),str,'\r');
            ss.str(str);
            ss>>csv_data[row][1];
            cout<<csv_data[row][1]<<endl;
            ss.str("");
            ss.clear(stringstream::goodbit);
            cout<<endl;
        }
        ifs_csv_file.close();
        setHeight(248);
        setWidth(328);
        bunit = "K";
    }
    double rad = l2_radiance;
    int temp_i = 0;
    if(rad>=2.589148e-01 && rad<=222.497500){
        for(int i=0;i<350;i++){
            //(7)
            if(csv_data[i][1]>rad){
                temp_i = i;
                l2_temp = ((csv_data[i][0] - csv_data[i-1][0]) / (csv_data[i][1] - csv_data[i-1][1])) * (rad - csv_data[i-1][1]) + (csv_data[i-1][0]);
                //cout<<"("<<csv_data[i][0]<<"-"<<csv_data[i-1][0]<<") / "<<"("<<csv_data[i][1]<<"-"<<csv_data[i-1][1]<<") x ("<<rad<<"-"<<csv_data[i-1][1]<<") + "<<csv_data[i-1][0]<<" = "<<l2_temp<<endl;
                l2_temp = round2(l2_temp);
                break;
            }
        }
    }
    else if(rad>222.497500){
        l2_temp = round2((500.0));
    }
    else if(rad<2.589148e-01){
        l2_temp = round2((150.0));
    }
    //calibrationImage[y][x] = l2_temp;
    calibrationImage[y+2][x+16] = l2_temp;
    fitstemperature2[y][x] = l2_temp;

    //cout<<y<<"-"<<x<<" DN->"<<DN<<" DN'->"<<DN1<<" DN''->"<<DN2<<" radiance :"<<l2_radiance<<" temp :"<<l2_temp<<endl;
    //cout<<csv_data[temp_i][0]<<" "<<csv_data[temp_i][1]<<endl;
}








void ShowImage::moveimage()
{

}
double ShowImage::gettemperature(double FT1){

    double tmp1=0;

    if(FT1<=0)tmp1=-123.15;
    else if(FT1<=0.03578){
        if(FT1==0.03304)tmp1=-150;
        else if(FT1<=0.03333)tmp1=-149.9;
        else if(FT1<=0.03363)tmp1=-149.8;
        else if(FT1<=0.03393)tmp1=-149.7;
        else if(FT1<=0.03423)tmp1=-149.6;
        else if(FT1<=0.03454)tmp1=-149.5;
        else if(FT1<=0.03485)tmp1=-149.4;
        else if(FT1<=0.03515)tmp1=-149.3;
        else if(FT1<=0.03547)tmp1=-149.2;
        else if(FT1<=0.03578)tmp1=-149.1;
        else if(FT1<=0.03609)tmp1=-149;
    }
    else if(FT1<=0.03904){
        if(FT1==0.03609)tmp1=-149;
        else if(FT1<=0.03641)tmp1=-148.9;
        else if(FT1<=0.03673)tmp1=-148.8;
        else if(FT1<=0.03706)tmp1=-148.7;
        else if(FT1<=0.03738)tmp1=-148.6;
        else if(FT1<=0.03771)tmp1=-148.5;
        else if(FT1<=0.03804)tmp1=-148.4;
        else if(FT1<=0.03837)tmp1=-148.3;
        else if(FT1<=0.0387)tmp1=-148.2;
        else if(FT1<=0.03904)tmp1=-148.1;
        else if(FT1<=0.03938)tmp1=-148;
    }
    else if(FT1<=0.04255){
        if(FT1==0.03938)tmp1=-148;
        else if(FT1<=0.03972)tmp1=-147.9;
        else if(FT1<=0.04007)tmp1=-147.8;
        else if(FT1<=0.04041)tmp1=-147.7;
        else if(FT1<=0.04076)tmp1=-147.6;
        else if(FT1<=0.04111)tmp1=-147.5;
        else if(FT1<=0.04147)tmp1=-147.4;
        else if(FT1<=0.04182)tmp1=-147.3;
        else if(FT1<=0.04218)tmp1=-147.2;
        else if(FT1<=0.04255)tmp1=-147.1;
        else if(FT1<=0.04291)tmp1=-147;
    }
    else if(FT1<=0.0463){
        if(FT1==0.04291)tmp1=-147;
        else if(FT1<=0.04328)tmp1=-146.9;
        else if(FT1<=0.04365)tmp1=-146.8;
        else if(FT1<=0.04402)tmp1=-146.7;
        else if(FT1<=0.04439)tmp1=-146.6;
        else if(FT1<=0.04477)tmp1=-146.5;
        else if(FT1<=0.04515)tmp1=-146.4;
        else if(FT1<=0.04553)tmp1=-146.3;
        else if(FT1<=0.04592)tmp1=-146.2;
        else if(FT1<=0.0463)tmp1=-146.1;
        else if(FT1<=0.0467)tmp1=-146;
    }
    else if(FT1<=0.05033){
        if(FT1==0.0467)tmp1=-146;
        else if(FT1<=0.04709)tmp1=-145.9;
        else if(FT1<=0.04748)tmp1=-145.8;
        else if(FT1<=0.04788)tmp1=-145.7;
        else if(FT1<=0.04828)tmp1=-145.6;
        else if(FT1<=0.04869)tmp1=-145.5;
        else if(FT1<=0.0491)tmp1=-145.4;
        else if(FT1<=0.04951)tmp1=-145.3;
        else if(FT1<=0.04992)tmp1=-145.2;
        else if(FT1<=0.05033)tmp1=-145.1;
        else if(FT1<=0.05075)tmp1=-145;
    }
    else if(FT1<=0.05464){
        if(FT1==0.05075)tmp1=-145;
        else if(FT1<=0.05117)tmp1=-144.9;
        else if(FT1<=0.0516)tmp1=-144.8;
        else if(FT1<=0.05202)tmp1=-144.7;
        else if(FT1<=0.05245)tmp1=-144.6;
        else if(FT1<=0.05289)tmp1=-144.5;
        else if(FT1<=0.05332)tmp1=-144.4;
        else if(FT1<=0.05376)tmp1=-144.3;
        else if(FT1<=0.0542)tmp1=-144.2;
        else if(FT1<=0.05464)tmp1=-144.1;
        else if(FT1<=0.05509)tmp1=-144;
    }
    else if(FT1<=0.05925){
        if(FT1==0.05509)tmp1=-144;
        else if(FT1<=0.05554)tmp1=-143.9;
        else if(FT1<=0.056)tmp1=-143.8;
        else if(FT1<=0.05645)tmp1=-143.7;
        else if(FT1<=0.05691)tmp1=-143.6;
        else if(FT1<=0.05737)tmp1=-143.5;
        else if(FT1<=0.05784)tmp1=-143.4;
        else if(FT1<=0.05831)tmp1=-143.3;
        else if(FT1<=0.05878)tmp1=-143.2;
        else if(FT1<=0.05925)tmp1=-143.1;
        else if(FT1<=0.05973)tmp1=-143;
    }
    else if(FT1<=0.06418){
        if(FT1==0.05973)tmp1=-143;
        else if(FT1<=0.06021)tmp1=-142.9;
        else if(FT1<=0.0607)tmp1=-142.8;
        else if(FT1<=0.06119)tmp1=-142.7;
        else if(FT1<=0.06168)tmp1=-142.6;
        else if(FT1<=0.06217)tmp1=-142.5;
        else if(FT1<=0.06267)tmp1=-142.4;
        else if(FT1<=0.06317)tmp1=-142.3;
        else if(FT1<=0.06367)tmp1=-142.2;
        else if(FT1<=0.06418)tmp1=-142.1;
        else if(FT1<=0.06469)tmp1=-142;
    }
    else if(FT1<=0.06943){
        if(FT1==0.06469)tmp1=-142;
        else if(FT1<=0.0652)tmp1=-141.9;
        else if(FT1<=0.06572)tmp1=-141.8;
        else if(FT1<=0.06624)tmp1=-141.7;
        else if(FT1<=0.06676)tmp1=-141.6;
        else if(FT1<=0.06729)tmp1=-141.5;
        else if(FT1<=0.06782)tmp1=-141.4;
        else if(FT1<=0.06835)tmp1=-141.3;
        else if(FT1<=0.06889)tmp1=-141.2;
        else if(FT1<=0.06943)tmp1=-141.1;
        else if(FT1<=0.06998)tmp1=-141;
    }
    else if(FT1<=0.07503){
        if(FT1==0.06998)tmp1=-141;
        else if(FT1<=0.07052)tmp1=-140.9;
        else if(FT1<=0.07107)tmp1=-140.8;
        else if(FT1<=0.07163)tmp1=-140.7;
        else if(FT1<=0.07219)tmp1=-140.6;
        else if(FT1<=0.07275)tmp1=-140.5;
        else if(FT1<=0.07331)tmp1=-140.4;
        else if(FT1<=0.07388)tmp1=-140.3;
        else if(FT1<=0.07445)tmp1=-140.2;
        else if(FT1<=0.07503)tmp1=-140.1;
        else if(FT1<=0.07561)tmp1=-140;
    }
    else if(FT1<=0.08099){
        if(FT1==0.07561)tmp1=-140;
        else if(FT1<=0.07619)tmp1=-139.9;
        else if(FT1<=0.07678)tmp1=-139.8;
        else if(FT1<=0.07737)tmp1=-139.7;
        else if(FT1<=0.07797)tmp1=-139.6;
        else if(FT1<=0.07856)tmp1=-139.5;
        else if(FT1<=0.07916)tmp1=-139.4;
        else if(FT1<=0.07977)tmp1=-139.3;
        else if(FT1<=0.08038)tmp1=-139.2;
        else if(FT1<=0.08099)tmp1=-139.1;
        else if(FT1<=0.08161)tmp1=-139;
    }
    else if(FT1<=0.08733){
        if(FT1==0.08161)tmp1=-139;
        else if(FT1<=0.08223)tmp1=-138.9;
        else if(FT1<=0.08285)tmp1=-138.8;
        else if(FT1<=0.08348)tmp1=-138.7;
        else if(FT1<=0.08411)tmp1=-138.6;
        else if(FT1<=0.08475)tmp1=-138.5;
        else if(FT1<=0.08539)tmp1=-138.4;
        else if(FT1<=0.08603)tmp1=-138.3;
        else if(FT1<=0.08668)tmp1=-138.2;
        else if(FT1<=0.08733)tmp1=-138.1;
        else if(FT1<=0.08799)tmp1=-138;
    }
    else if(FT1<=0.09407){
        if(FT1==0.08799)tmp1=-138;
        else if(FT1<=0.08865)tmp1=-137.9;
        else if(FT1<=0.08931)tmp1=-137.8;
        else if(FT1<=0.08998)tmp1=-137.7;
        else if(FT1<=0.09065)tmp1=-137.6;
        else if(FT1<=0.09133)tmp1=-137.5;
        else if(FT1<=0.09201)tmp1=-137.4;
        else if(FT1<=0.09269)tmp1=-137.3;
        else if(FT1<=0.09338)tmp1=-137.2;
        else if(FT1<=0.09407)tmp1=-137.1;
        else if(FT1<=0.09477)tmp1=-137;
    }
    else if(FT1<=0.10123){
        if(FT1==0.09477)tmp1=-137;
        else if(FT1<=0.09547)tmp1=-136.9;
        else if(FT1<=0.09618)tmp1=-136.8;
        else if(FT1<=0.09688)tmp1=-136.7;
        else if(FT1<=0.0976)tmp1=-136.6;
        else if(FT1<=0.09832)tmp1=-136.5;
        else if(FT1<=0.09904)tmp1=-136.4;
        else if(FT1<=0.09976)tmp1=-136.3;
        else if(FT1<=0.10049)tmp1=-136.2;
        else if(FT1<=0.10123)tmp1=-136.1;
        else if(FT1<=0.10197)tmp1=-136;
    }
    else if(FT1<=0.10882){
        if(FT1==0.10197)tmp1=-136;
        else if(FT1<=0.10271)tmp1=-135.9;
        else if(FT1<=0.10346)tmp1=-135.8;
        else if(FT1<=0.10421)tmp1=-135.7;
        else if(FT1<=0.10497)tmp1=-135.6;
        else if(FT1<=0.10573)tmp1=-135.5;
        else if(FT1<=0.10649)tmp1=-135.4;
        else if(FT1<=0.10726)tmp1=-135.3;
        else if(FT1<=0.10804)tmp1=-135.2;
        else if(FT1<=0.10882)tmp1=-135.1;
        else if(FT1<=0.1096)tmp1=-135;
    }
    else if(FT1<=0.11686){
        if(FT1==0.1096)tmp1=-135;
        else if(FT1<=0.11039)tmp1=-134.9;
        else if(FT1<=0.11118)tmp1=-134.8;
        else if(FT1<=0.11198)tmp1=-134.7;
        else if(FT1<=0.11278)tmp1=-134.6;
        else if(FT1<=0.11359)tmp1=-134.5;
        else if(FT1<=0.1144)tmp1=-134.4;
        else if(FT1<=0.11521)tmp1=-134.3;
        else if(FT1<=0.11604)tmp1=-134.2;
        else if(FT1<=0.11686)tmp1=-134.1;
        else if(FT1<=0.11769)tmp1=-134;
    }
    else if(FT1<=0.12538){
        if(FT1==0.11769)tmp1=-134;
        else if(FT1<=0.11853)tmp1=-133.9;
        else if(FT1<=0.11937)tmp1=-133.8;
        else if(FT1<=0.12021)tmp1=-133.7;
        else if(FT1<=0.12106)tmp1=-133.6;
        else if(FT1<=0.12191)tmp1=-133.5;
        else if(FT1<=0.12277)tmp1=-133.4;
        else if(FT1<=0.12364)tmp1=-133.3;
        else if(FT1<=0.1245)tmp1=-133.2;
        else if(FT1<=0.12538)tmp1=-133.1;
        else if(FT1<=0.12626)tmp1=-133;
    }
    else if(FT1<=0.13439){
        if(FT1==0.12626)tmp1=-133;
        else if(FT1<=0.12714)tmp1=-132.9;
        else if(FT1<=0.12803)tmp1=-132.8;
        else if(FT1<=0.12892)tmp1=-132.7;
        else if(FT1<=0.12982)tmp1=-132.6;
        else if(FT1<=0.13072)tmp1=-132.5;
        else if(FT1<=0.13163)tmp1=-132.4;
        else if(FT1<=0.13254)tmp1=-132.3;
        else if(FT1<=0.13346)tmp1=-132.2;
        else if(FT1<=0.13439)tmp1=-132.1;
        else if(FT1<=0.13532)tmp1=-132;
    }
    else if(FT1<=0.14391){
        if(FT1==0.13532)tmp1=-132;
        else if(FT1<=0.13625)tmp1=-131.9;
        else if(FT1<=0.13719)tmp1=-131.8;
        else if(FT1<=0.13813)tmp1=-131.7;
        else if(FT1<=0.13908)tmp1=-131.6;
        else if(FT1<=0.14004)tmp1=-131.5;
        else if(FT1<=0.141)tmp1=-131.4;
        else if(FT1<=0.14196)tmp1=-131.3;
        else if(FT1<=0.14294)tmp1=-131.2;
        else if(FT1<=0.14391)tmp1=-131.1;
        else if(FT1<=0.14489)tmp1=-131;
    }
    else if(FT1<=0.15397){
        if(FT1==0.14489)tmp1=-131;
        else if(FT1<=0.14588)tmp1=-130.9;
        else if(FT1<=0.14687)tmp1=-130.8;
        else if(FT1<=0.14787)tmp1=-130.7;
        else if(FT1<=0.14887)tmp1=-130.6;
        else if(FT1<=0.14988)tmp1=-130.5;
        else if(FT1<=0.1509)tmp1=-130.4;
        else if(FT1<=0.15191)tmp1=-130.3;
        else if(FT1<=0.15294)tmp1=-130.2;
        else if(FT1<=0.15397)tmp1=-130.1;
        else if(FT1<=0.15501)tmp1=-130;
    }
    else if(FT1<=0.16459){
        if(FT1==0.15501)tmp1=-130;
        else if(FT1<=0.15605)tmp1=-129.9;
        else if(FT1<=0.1571)tmp1=-129.8;
        else if(FT1<=0.15815)tmp1=-129.7;
        else if(FT1<=0.15921)tmp1=-129.6;
        else if(FT1<=0.16027)tmp1=-129.5;
        else if(FT1<=0.16134)tmp1=-129.4;
        else if(FT1<=0.16242)tmp1=-129.3;
        else if(FT1<=0.1635)tmp1=-129.2;
        else if(FT1<=0.16459)tmp1=-129.1;
        else if(FT1<=0.16568)tmp1=-129;
    }
    else if(FT1<=0.17578){
        if(FT1==0.16568)tmp1=-129;
        else if(FT1<=0.16678)tmp1=-128.9;
        else if(FT1<=0.16788)tmp1=-128.8;
        else if(FT1<=0.16899)tmp1=-128.7;
        else if(FT1<=0.17011)tmp1=-128.6;
        else if(FT1<=0.17123)tmp1=-128.5;
        else if(FT1<=0.17236)tmp1=-128.4;
        else if(FT1<=0.17349)tmp1=-128.3;
        else if(FT1<=0.17463)tmp1=-128.2;
        else if(FT1<=0.17578)tmp1=-128.1;
        else if(FT1<=0.17693)tmp1=-128;
    }
    else if(FT1<=0.18757){
        if(FT1==0.17693)tmp1=-128;
        else if(FT1<=0.17809)tmp1=-127.9;
        else if(FT1<=0.17925)tmp1=-127.8;
        else if(FT1<=0.18042)tmp1=-127.7;
        else if(FT1<=0.1816)tmp1=-127.6;
        else if(FT1<=0.18278)tmp1=-127.5;
        else if(FT1<=0.18397)tmp1=-127.4;
        else if(FT1<=0.18517)tmp1=-127.3;
        else if(FT1<=0.18637)tmp1=-127.2;
        else if(FT1<=0.18757)tmp1=-127.1;
        else if(FT1<=0.18879)tmp1=-127;
    }
    else if(FT1<=0.19999){
        if(FT1==0.18879)tmp1=-127;
        else if(FT1<=0.19001)tmp1=-126.9;
        else if(FT1<=0.19123)tmp1=-126.8;
        else if(FT1<=0.19247)tmp1=-126.7;
        else if(FT1<=0.1937)tmp1=-126.6;
        else if(FT1<=0.19495)tmp1=-126.5;
        else if(FT1<=0.1962)tmp1=-126.4;
        else if(FT1<=0.19746)tmp1=-126.3;
        else if(FT1<=0.19872)tmp1=-126.2;
        else if(FT1<=0.19999)tmp1=-126.1;
        else if(FT1<=0.20127)tmp1=-126;
    }
    else if(FT1<=0.21306){
        if(FT1==0.20127)tmp1=-126;
        else if(FT1<=0.20255)tmp1=-125.9;
        else if(FT1<=0.20384)tmp1=-125.8;
        else if(FT1<=0.20514)tmp1=-125.7;
        else if(FT1<=0.20644)tmp1=-125.6;
        else if(FT1<=0.20775)tmp1=-125.5;
        else if(FT1<=0.20907)tmp1=-125.4;
        else if(FT1<=0.21039)tmp1=-125.3;
        else if(FT1<=0.21172)tmp1=-125.2;
        else if(FT1<=0.21306)tmp1=-125.1;
        else if(FT1<=0.2144)tmp1=-125;
    }
    else if(FT1<=0.22679){
        if(FT1==0.2144)tmp1=-125;
        else if(FT1<=0.21575)tmp1=-124.9;
        else if(FT1<=0.21711)tmp1=-124.8;
        else if(FT1<=0.21847)tmp1=-124.7;
        else if(FT1<=0.21984)tmp1=-124.6;
        else if(FT1<=0.22122)tmp1=-124.5;
        else if(FT1<=0.2226)tmp1=-124.4;
        else if(FT1<=0.22399)tmp1=-124.3;
        else if(FT1<=0.22539)tmp1=-124.2;
        else if(FT1<=0.22679)tmp1=-124.1;
        else if(FT1<=0.22821)tmp1=-124;
    }
    else if(FT1<=0.24123){
        if(FT1==0.22821)tmp1=-124;
        else if(FT1<=0.22962)tmp1=-123.9;
        else if(FT1<=0.23105)tmp1=-123.8;
        else if(FT1<=0.23248)tmp1=-123.7;
        else if(FT1<=0.23392)tmp1=-123.6;
        else if(FT1<=0.23537)tmp1=-123.5;
        else if(FT1<=0.23682)tmp1=-123.4;
        else if(FT1<=0.23828)tmp1=-123.3;
        else if(FT1<=0.23975)tmp1=-123.2;
        else if(FT1<=0.24123)tmp1=-123.1;
        else if(FT1<=0.24271)tmp1=-123;
    }
    else if(FT1<=0.25638){
        if(FT1==0.24271)tmp1=-123;
        else if(FT1<=0.2442)tmp1=-122.9;
        else if(FT1<=0.24569)tmp1=-122.8;
        else if(FT1<=0.2472)tmp1=-122.7;
        else if(FT1<=0.24871)tmp1=-122.6;
        else if(FT1<=0.25023)tmp1=-122.5;
        else if(FT1<=0.25175)tmp1=-122.4;
        else if(FT1<=0.25329)tmp1=-122.3;
        else if(FT1<=0.25483)tmp1=-122.2;
        else if(FT1<=0.25638)tmp1=-122.1;
        else if(FT1<=0.25793)tmp1=-122;
    }
    else if(FT1<=0.27227){
        if(FT1==0.25793)tmp1=-122;
        else if(FT1<=0.25949)tmp1=-121.9;
        else if(FT1<=0.26106)tmp1=-121.8;
        else if(FT1<=0.26264)tmp1=-121.7;
        else if(FT1<=0.26423)tmp1=-121.6;
        else if(FT1<=0.26582)tmp1=-121.5;
        else if(FT1<=0.26742)tmp1=-121.4;
        else if(FT1<=0.26903)tmp1=-121.3;
        else if(FT1<=0.27065)tmp1=-121.2;
        else if(FT1<=0.27227)tmp1=-121.1;
        else if(FT1<=0.2739)tmp1=-121;
    }
    else if(FT1<=0.28893){
        if(FT1==0.2739)tmp1=-121;
        else if(FT1<=0.27554)tmp1=-120.9;
        else if(FT1<=0.27719)tmp1=-120.8;
        else if(FT1<=0.27884)tmp1=-120.7;
        else if(FT1<=0.2805)tmp1=-120.6;
        else if(FT1<=0.28217)tmp1=-120.5;
        else if(FT1<=0.28385)tmp1=-120.4;
        else if(FT1<=0.28554)tmp1=-120.3;
        else if(FT1<=0.28723)tmp1=-120.2;
        else if(FT1<=0.28893)tmp1=-120.1;
        else if(FT1<=0.29064)tmp1=-120;
    }
    else if(FT1<=0.30639){
        if(FT1==0.29064)tmp1=-120;
        else if(FT1<=0.29236)tmp1=-119.9;
        else if(FT1<=0.29409)tmp1=-119.8;
        else if(FT1<=0.29582)tmp1=-119.7;
        else if(FT1<=0.29756)tmp1=-119.6;
        else if(FT1<=0.29931)tmp1=-119.5;
        else if(FT1<=0.30107)tmp1=-119.4;
        else if(FT1<=0.30284)tmp1=-119.3;
        else if(FT1<=0.30461)tmp1=-119.2;
        else if(FT1<=0.30639)tmp1=-119.1;
        else if(FT1<=0.30818)tmp1=-119;
    }
    else if(FT1<=0.32467){
        if(FT1==0.30818)tmp1=-119;
        else if(FT1<=0.30998)tmp1=-118.9;
        else if(FT1<=0.31179)tmp1=-118.8;
        else if(FT1<=0.31361)tmp1=-118.7;
        else if(FT1<=0.31543)tmp1=-118.6;
        else if(FT1<=0.31726)tmp1=-118.5;
        else if(FT1<=0.3191)tmp1=-118.4;
        else if(FT1<=0.32095)tmp1=-118.3;
        else if(FT1<=0.32281)tmp1=-118.2;
        else if(FT1<=0.32467)tmp1=-118.1;
        else if(FT1<=0.32655)tmp1=-118;
    }
    else if(FT1<=0.3438){
        if(FT1==0.32655)tmp1=-118;
        else if(FT1<=0.32843)tmp1=-117.9;
        else if(FT1<=0.33032)tmp1=-117.8;
        else if(FT1<=0.33222)tmp1=-117.7;
        else if(FT1<=0.33413)tmp1=-117.6;
        else if(FT1<=0.33605)tmp1=-117.5;
        else if(FT1<=0.33797)tmp1=-117.4;
        else if(FT1<=0.33991)tmp1=-117.3;
        else if(FT1<=0.34185)tmp1=-117.2;
        else if(FT1<=0.3438)tmp1=-117.1;
        else if(FT1<=0.34576)tmp1=-117;
    }
    else if(FT1<=0.36381){
        if(FT1==0.34576)tmp1=-117;
        else if(FT1<=0.34773)tmp1=-116.9;
        else if(FT1<=0.34971)tmp1=-116.8;
        else if(FT1<=0.3517)tmp1=-116.7;
        else if(FT1<=0.35369)tmp1=-116.6;
        else if(FT1<=0.3557)tmp1=-116.5;
        else if(FT1<=0.35771)tmp1=-116.4;
        else if(FT1<=0.35973)tmp1=-116.3;
        else if(FT1<=0.36176)tmp1=-116.2;
        else if(FT1<=0.36381)tmp1=-116.1;
        else if(FT1<=0.36585)tmp1=-116;
    }
    else if(FT1<=0.38471){
        if(FT1==0.36585)tmp1=-116;
        else if(FT1<=0.36791)tmp1=-115.9;
        else if(FT1<=0.36998)tmp1=-115.8;
        else if(FT1<=0.37206)tmp1=-115.7;
        else if(FT1<=0.37414)tmp1=-115.6;
        else if(FT1<=0.37624)tmp1=-115.5;
        else if(FT1<=0.37834)tmp1=-115.4;
        else if(FT1<=0.38046)tmp1=-115.3;
        else if(FT1<=0.38258)tmp1=-115.2;
        else if(FT1<=0.38471)tmp1=-115.1;
        else if(FT1<=0.38685)tmp1=-115;
    }
    else if(FT1<=0.40655){
        if(FT1==0.38685)tmp1=-115;
        else if(FT1<=0.389)tmp1=-114.9;
        else if(FT1<=0.39116)tmp1=-114.8;
        else if(FT1<=0.39333)tmp1=-114.7;
        else if(FT1<=0.39551)tmp1=-114.6;
        else if(FT1<=0.3977)tmp1=-114.5;
        else if(FT1<=0.3999)tmp1=-114.4;
        else if(FT1<=0.4021)tmp1=-114.3;
        else if(FT1<=0.40432)tmp1=-114.2;
        else if(FT1<=0.40655)tmp1=-114.1;
        else if(FT1<=0.40878)tmp1=-114;
    }
    else if(FT1<=0.42934){
        if(FT1==0.40878)tmp1=-114;
        else if(FT1<=0.41103)tmp1=-113.9;
        else if(FT1<=0.41328)tmp1=-113.8;
        else if(FT1<=0.41555)tmp1=-113.7;
        else if(FT1<=0.41782)tmp1=-113.6;
        else if(FT1<=0.4201)tmp1=-113.5;
        else if(FT1<=0.4224)tmp1=-113.4;
        else if(FT1<=0.4247)tmp1=-113.3;
        else if(FT1<=0.42702)tmp1=-113.2;
        else if(FT1<=0.42934)tmp1=-113.1;
        else if(FT1<=0.43167)tmp1=-113;
    }
    else if(FT1<=0.45312){
        if(FT1==0.43167)tmp1=-113;
        else if(FT1<=0.43401)tmp1=-112.9;
        else if(FT1<=0.43637)tmp1=-112.8;
        else if(FT1<=0.43873)tmp1=-112.7;
        else if(FT1<=0.4411)tmp1=-112.6;
        else if(FT1<=0.44349)tmp1=-112.5;
        else if(FT1<=0.44588)tmp1=-112.4;
        else if(FT1<=0.44828)tmp1=-112.3;
        else if(FT1<=0.45069)tmp1=-112.2;
        else if(FT1<=0.45312)tmp1=-112.1;
        else if(FT1<=0.45555)tmp1=-112;
    }
    else if(FT1<=0.47791){
        if(FT1==0.45555)tmp1=-112;
        else if(FT1<=0.45799)tmp1=-111.9;
        else if(FT1<=0.46045)tmp1=-111.8;
        else if(FT1<=0.46291)tmp1=-111.7;
        else if(FT1<=0.46538)tmp1=-111.6;
        else if(FT1<=0.46787)tmp1=-111.5;
        else if(FT1<=0.47036)tmp1=-111.4;
        else if(FT1<=0.47287)tmp1=-111.3;
        else if(FT1<=0.47538)tmp1=-111.2;
        else if(FT1<=0.47791)tmp1=-111.1;
        else if(FT1<=0.48045)tmp1=-111;
    }
    else if(FT1<=0.50375){
        if(FT1==0.48045)tmp1=-111;
        else if(FT1<=0.48299)tmp1=-110.9;
        else if(FT1<=0.48555)tmp1=-110.8;
        else if(FT1<=0.48812)tmp1=-110.7;
        else if(FT1<=0.4907)tmp1=-110.6;
        else if(FT1<=0.49328)tmp1=-110.5;
        else if(FT1<=0.49588)tmp1=-110.4;
        else if(FT1<=0.49849)tmp1=-110.3;
        else if(FT1<=0.50111)tmp1=-110.2;
        else if(FT1<=0.50375)tmp1=-110.1;
        else if(FT1<=0.50639)tmp1=-110;
    }
    else if(FT1<=0.53065){
        if(FT1==0.50639)tmp1=-110;
        else if(FT1<=0.50904)tmp1=-109.9;
        else if(FT1<=0.5117)tmp1=-109.8;
        else if(FT1<=0.51438)tmp1=-109.7;
        else if(FT1<=0.51706)tmp1=-109.6;
        else if(FT1<=0.51976)tmp1=-109.5;
        else if(FT1<=0.52247)tmp1=-109.4;
        else if(FT1<=0.52519)tmp1=-109.3;
        else if(FT1<=0.52791)tmp1=-109.2;
        else if(FT1<=0.53065)tmp1=-109.1;
        else if(FT1<=0.53341)tmp1=-109;
    }
    else if(FT1<=0.55867){
        if(FT1==0.53341)tmp1=-109;
        else if(FT1<=0.53617)tmp1=-108.9;
        else if(FT1<=0.53894)tmp1=-108.8;
        else if(FT1<=0.54173)tmp1=-108.7;
        else if(FT1<=0.54452)tmp1=-108.6;
        else if(FT1<=0.54733)tmp1=-108.5;
        else if(FT1<=0.55015)tmp1=-108.4;
        else if(FT1<=0.55297)tmp1=-108.3;
        else if(FT1<=0.55581)tmp1=-108.2;
        else if(FT1<=0.55867)tmp1=-108.1;
        else if(FT1<=0.56153)tmp1=-108;
    }
    else if(FT1<=0.58781){
        if(FT1==0.56153)tmp1=-108;
        else if(FT1<=0.5644)tmp1=-107.9;
        else if(FT1<=0.56729)tmp1=-107.8;
        else if(FT1<=0.57019)tmp1=-107.7;
        else if(FT1<=0.57309)tmp1=-107.6;
        else if(FT1<=0.57601)tmp1=-107.5;
        else if(FT1<=0.57895)tmp1=-107.4;
        else if(FT1<=0.58189)tmp1=-107.3;
        else if(FT1<=0.58484)tmp1=-107.2;
        else if(FT1<=0.58781)tmp1=-107.1;
        else if(FT1<=0.59079)tmp1=-107;
    }
    else if(FT1<=0.61811){
        if(FT1==0.59079)tmp1=-107;
        else if(FT1<=0.59378)tmp1=-106.9;
        else if(FT1<=0.59678)tmp1=-106.8;
        else if(FT1<=0.59979)tmp1=-106.7;
        else if(FT1<=0.60281)tmp1=-106.6;
        else if(FT1<=0.60585)tmp1=-106.5;
        else if(FT1<=0.6089)tmp1=-106.4;
        else if(FT1<=0.61196)tmp1=-106.3;
        else if(FT1<=0.61503)tmp1=-106.2;
        else if(FT1<=0.61811)tmp1=-106.1;
        else if(FT1<=0.62121)tmp1=-106;
    }
    else if(FT1<=0.64961){
        if(FT1==0.62121)tmp1=-106;
        else if(FT1<=0.62432)tmp1=-105.9;
        else if(FT1<=0.62744)tmp1=-105.8;
        else if(FT1<=0.63057)tmp1=-105.7;
        else if(FT1<=0.63371)tmp1=-105.6;
        else if(FT1<=0.63687)tmp1=-105.5;
        else if(FT1<=0.64004)tmp1=-105.4;
        else if(FT1<=0.64322)tmp1=-105.3;
        else if(FT1<=0.64641)tmp1=-105.2;
        else if(FT1<=0.64961)tmp1=-105.1;
        else if(FT1<=0.65283)tmp1=-105;
    }
    else if(FT1<=0.68233){
        if(FT1==0.65283)tmp1=-105;
        else if(FT1<=0.65606)tmp1=-104.9;
        else if(FT1<=0.6593)tmp1=-104.8;
        else if(FT1<=0.66255)tmp1=-104.7;
        else if(FT1<=0.66582)tmp1=-104.6;
        else if(FT1<=0.6691)tmp1=-104.5;
        else if(FT1<=0.67239)tmp1=-104.4;
        else if(FT1<=0.67569)tmp1=-104.3;
        else if(FT1<=0.679)tmp1=-104.2;
        else if(FT1<=0.68233)tmp1=-104.1;
        else if(FT1<=0.68567)tmp1=-104;
    }
    else if(FT1<=0.71631){
        if(FT1==0.68567)tmp1=-104;
        else if(FT1<=0.68903)tmp1=-103.9;
        else if(FT1<=0.69239)tmp1=-103.8;
        else if(FT1<=0.69577)tmp1=-103.7;
        else if(FT1<=0.69916)tmp1=-103.6;
        else if(FT1<=0.70256)tmp1=-103.5;
        else if(FT1<=0.70598)tmp1=-103.4;
        else if(FT1<=0.70941)tmp1=-103.3;
        else if(FT1<=0.71285)tmp1=-103.2;
        else if(FT1<=0.71631)tmp1=-103.1;
        else if(FT1<=0.71977)tmp1=-103;
    }
    else if(FT1<=0.75157){
        if(FT1==0.71977)tmp1=-103;
        else if(FT1<=0.72326)tmp1=-102.9;
        else if(FT1<=0.72675)tmp1=-102.8;
        else if(FT1<=0.73026)tmp1=-102.7;
        else if(FT1<=0.73377)tmp1=-102.6;
        else if(FT1<=0.73731)tmp1=-102.5;
        else if(FT1<=0.74085)tmp1=-102.4;
        else if(FT1<=0.74441)tmp1=-102.3;
        else if(FT1<=0.74798)tmp1=-102.2;
        else if(FT1<=0.75157)tmp1=-102.1;
        else if(FT1<=0.75516)tmp1=-102;
    }
    else if(FT1<=0.78814){
        if(FT1==0.75516)tmp1=-102;
        else if(FT1<=0.75878)tmp1=-101.9;
        else if(FT1<=0.7624)tmp1=-101.8;
        else if(FT1<=0.76604)tmp1=-101.7;
        else if(FT1<=0.76969)tmp1=-101.6;
        else if(FT1<=0.77335)tmp1=-101.5;
        else if(FT1<=0.77703)tmp1=-101.4;
        else if(FT1<=0.78072)tmp1=-101.3;
        else if(FT1<=0.78442)tmp1=-101.2;
        else if(FT1<=0.78814)tmp1=-101.1;
        else if(FT1<=0.79187)tmp1=-101;
    }
    else if(FT1<=0.82607){
        if(FT1==0.79187)tmp1=-101;
        else if(FT1<=0.79562)tmp1=-100.9;
        else if(FT1<=0.79938)tmp1=-100.8;
        else if(FT1<=0.80315)tmp1=-100.7;
        else if(FT1<=0.80693)tmp1=-100.6;
        else if(FT1<=0.81073)tmp1=-100.5;
        else if(FT1<=0.81455)tmp1=-100.4;
        else if(FT1<=0.81837)tmp1=-100.3;
        else if(FT1<=0.82221)tmp1=-100.2;
        else if(FT1<=0.82607)tmp1=-100.1;
        else if(FT1<=0.82993)tmp1=-100;
    }
    else if(FT1<=0.86537){
        if(FT1==0.82993)tmp1=-100;
        else if(FT1<=0.83382)tmp1=-99.9;
        else if(FT1<=0.83771)tmp1=-99.8;
        else if(FT1<=0.84162)tmp1=-99.7;
        else if(FT1<=0.84554)tmp1=-99.6;
        else if(FT1<=0.84948)tmp1=-99.5;
        else if(FT1<=0.85343)tmp1=-99.4;
        else if(FT1<=0.8574)tmp1=-99.3;
        else if(FT1<=0.86138)tmp1=-99.2;
        else if(FT1<=0.86537)tmp1=-99.1;
        else if(FT1<=0.86938)tmp1=-99;
    }
    else if(FT1<=0.90608){
        if(FT1==0.86938)tmp1=-99;
        else if(FT1<=0.8734)tmp1=-98.9;
        else if(FT1<=0.87743)tmp1=-98.8;
        else if(FT1<=0.88148)tmp1=-98.7;
        else if(FT1<=0.88555)tmp1=-98.6;
        else if(FT1<=0.88963)tmp1=-98.5;
        else if(FT1<=0.89372)tmp1=-98.4;
        else if(FT1<=0.89783)tmp1=-98.3;
        else if(FT1<=0.90195)tmp1=-98.2;
        else if(FT1<=0.90608)tmp1=-98.1;
        else if(FT1<=0.91023)tmp1=-98;
    }
    else if(FT1<=0.94824){
        if(FT1==0.91023)tmp1=-98;
        else if(FT1<=0.9144)tmp1=-97.9;
        else if(FT1<=0.91858)tmp1=-97.8;
        else if(FT1<=0.92277)tmp1=-97.7;
        else if(FT1<=0.92698)tmp1=-97.6;
        else if(FT1<=0.9312)tmp1=-97.5;
        else if(FT1<=0.93544)tmp1=-97.4;
        else if(FT1<=0.93969)tmp1=-97.3;
        else if(FT1<=0.94396)tmp1=-97.2;
        else if(FT1<=0.94824)tmp1=-97.1;
        else if(FT1<=0.95254)tmp1=-97;
    }
    else if(FT1<=0.99188){
        if(FT1==0.95254)tmp1=-97;
        else if(FT1<=0.95685)tmp1=-96.9;
        else if(FT1<=0.96118)tmp1=-96.8;
        else if(FT1<=0.96552)tmp1=-96.7;
        else if(FT1<=0.96987)tmp1=-96.6;
        else if(FT1<=0.97424)tmp1=-96.5;
        else if(FT1<=0.97863)tmp1=-96.4;
        else if(FT1<=0.98303)tmp1=-96.3;
        else if(FT1<=0.98745)tmp1=-96.2;
        else if(FT1<=0.99188)tmp1=-96.1;
        else if(FT1<=0.99632)tmp1=-96;
    }
    else if(FT1<=1.03702){
        if(FT1==0.99632)tmp1=-96;
        else if(FT1<=1.00078)tmp1=-95.9;
        else if(FT1<=1.00526)tmp1=-95.8;
        else if(FT1<=1.00975)tmp1=-95.7;
        else if(FT1<=1.01426)tmp1=-95.6;
        else if(FT1<=1.01878)tmp1=-95.5;
        else if(FT1<=1.02332)tmp1=-95.4;
        else if(FT1<=1.02787)tmp1=-95.3;
        else if(FT1<=1.03244)tmp1=-95.2;
        else if(FT1<=1.03702)tmp1=-95.1;
        else if(FT1<=1.04162)tmp1=-95;
    }
    else if(FT1<=1.0837){
        if(FT1==1.04162)tmp1=-95;
        else if(FT1<=1.04623)tmp1=-94.9;
        else if(FT1<=1.05086)tmp1=-94.8;
        else if(FT1<=1.05551)tmp1=-94.7;
        else if(FT1<=1.06017)tmp1=-94.6;
        else if(FT1<=1.06484)tmp1=-94.5;
        else if(FT1<=1.06954)tmp1=-94.4;
        else if(FT1<=1.07424)tmp1=-94.3;
        else if(FT1<=1.07897)tmp1=-94.2;
        else if(FT1<=1.0837)tmp1=-94.1;
        else if(FT1<=1.08846)tmp1=-94;
    }
    else if(FT1<=1.13196){
        if(FT1==1.08846)tmp1=-94;
        else if(FT1<=1.09323)tmp1=-93.9;
        else if(FT1<=1.09802)tmp1=-93.8;
        else if(FT1<=1.10282)tmp1=-93.7;
        else if(FT1<=1.10763)tmp1=-93.6;
        else if(FT1<=1.11247)tmp1=-93.5;
        else if(FT1<=1.11732)tmp1=-93.4;
        else if(FT1<=1.12218)tmp1=-93.3;
        else if(FT1<=1.12706)tmp1=-93.2;
        else if(FT1<=1.13196)tmp1=-93.1;
        else if(FT1<=1.13688)tmp1=-93;
    }
    else if(FT1<=1.18183){
        if(FT1==1.13688)tmp1=-93;
        else if(FT1<=1.14181)tmp1=-92.9;
        else if(FT1<=1.14675)tmp1=-92.8;
        else if(FT1<=1.15171)tmp1=-92.7;
        else if(FT1<=1.15669)tmp1=-92.6;
        else if(FT1<=1.16169)tmp1=-92.5;
        else if(FT1<=1.1667)tmp1=-92.4;
        else if(FT1<=1.17172)tmp1=-92.3;
        else if(FT1<=1.17677)tmp1=-92.2;
        else if(FT1<=1.18183)tmp1=-92.1;
        else if(FT1<=1.1869)tmp1=-92;
    }
    else if(FT1<=1.23333){
        if(FT1==1.1869)tmp1=-92;
        else if(FT1<=1.192)tmp1=-91.9;
        else if(FT1<=1.1971)tmp1=-91.8;
        else if(FT1<=1.20223)tmp1=-91.7;
        else if(FT1<=1.20737)tmp1=-91.6;
        else if(FT1<=1.21253)tmp1=-91.5;
        else if(FT1<=1.21771)tmp1=-91.4;
        else if(FT1<=1.2229)tmp1=-91.3;
        else if(FT1<=1.22811)tmp1=-91.2;
        else if(FT1<=1.23333)tmp1=-91.1;
        else if(FT1<=1.23857)tmp1=-91;
    }
    else if(FT1<=1.28651){
        if(FT1==1.23857)tmp1=-91;
        else if(FT1<=1.24383)tmp1=-90.9;
        else if(FT1<=1.24911)tmp1=-90.8;
        else if(FT1<=1.2544)tmp1=-90.7;
        else if(FT1<=1.25971)tmp1=-90.6;
        else if(FT1<=1.26503)tmp1=-90.5;
        else if(FT1<=1.27038)tmp1=-90.4;
        else if(FT1<=1.27574)tmp1=-90.3;
        else if(FT1<=1.28111)tmp1=-90.2;
        else if(FT1<=1.28651)tmp1=-90.1;
        else if(FT1<=1.29192)tmp1=-90;
    }
    else if(FT1<=1.34139){
        if(FT1==1.29192)tmp1=-90;
        else if(FT1<=1.29735)tmp1=-89.9;
        else if(FT1<=1.30279)tmp1=-89.8;
        else if(FT1<=1.30825)tmp1=-89.7;
        else if(FT1<=1.31373)tmp1=-89.6;
        else if(FT1<=1.31923)tmp1=-89.5;
        else if(FT1<=1.32475)tmp1=-89.4;
        else if(FT1<=1.33028)tmp1=-89.3;
        else if(FT1<=1.33583)tmp1=-89.2;
        else if(FT1<=1.34139)tmp1=-89.1;
        else if(FT1<=1.34697)tmp1=-89;
    }
    else if(FT1<=1.39801){
        if(FT1==1.34697)tmp1=-89;
        else if(FT1<=1.35258)tmp1=-88.9;
        else if(FT1<=1.35819)tmp1=-88.8;
        else if(FT1<=1.36383)tmp1=-88.7;
        else if(FT1<=1.36948)tmp1=-88.6;
        else if(FT1<=1.37515)tmp1=-88.5;
        else if(FT1<=1.38084)tmp1=-88.4;
        else if(FT1<=1.38655)tmp1=-88.3;
        else if(FT1<=1.39227)tmp1=-88.2;
        else if(FT1<=1.39801)tmp1=-88.1;
        else if(FT1<=1.40377)tmp1=-88;
    }
    else if(FT1<=1.45641){
        if(FT1==1.40377)tmp1=-88;
        else if(FT1<=1.40955)tmp1=-87.9;
        else if(FT1<=1.41534)tmp1=-87.8;
        else if(FT1<=1.42116)tmp1=-87.7;
        else if(FT1<=1.42699)tmp1=-87.6;
        else if(FT1<=1.43284)tmp1=-87.5;
        else if(FT1<=1.4387)tmp1=-87.4;
        else if(FT1<=1.44459)tmp1=-87.3;
        else if(FT1<=1.45049)tmp1=-87.2;
        else if(FT1<=1.45641)tmp1=-87.1;
        else if(FT1<=1.46235)tmp1=-87;
    }
    else if(FT1<=1.51661){
        if(FT1==1.46235)tmp1=-87;
        else if(FT1<=1.4683)tmp1=-86.9;
        else if(FT1<=1.47428)tmp1=-86.8;
        else if(FT1<=1.48027)tmp1=-86.7;
        else if(FT1<=1.48628)tmp1=-86.6;
        else if(FT1<=1.49231)tmp1=-86.5;
        else if(FT1<=1.49836)tmp1=-86.4;
        else if(FT1<=1.50442)tmp1=-86.3;
        else if(FT1<=1.51051)tmp1=-86.2;
        else if(FT1<=1.51661)tmp1=-86.1;
        else if(FT1<=1.52273)tmp1=-86;
    }
    else if(FT1<=1.57865){
        if(FT1==1.52273)tmp1=-86;
        else if(FT1<=1.52887)tmp1=-85.9;
        else if(FT1<=1.53503)tmp1=-85.8;
        else if(FT1<=1.5412)tmp1=-85.7;
        else if(FT1<=1.5474)tmp1=-85.6;
        else if(FT1<=1.55361)tmp1=-85.5;
        else if(FT1<=1.55984)tmp1=-85.4;
        else if(FT1<=1.56609)tmp1=-85.3;
        else if(FT1<=1.57236)tmp1=-85.2;
        else if(FT1<=1.57865)tmp1=-85.1;
        else if(FT1<=1.58496)tmp1=-85;
    }
    else if(FT1<=1.64256){
        if(FT1==1.58496)tmp1=-85;
        else if(FT1<=1.59128)tmp1=-84.9;
        else if(FT1<=1.59763)tmp1=-84.8;
        else if(FT1<=1.60399)tmp1=-84.7;
        else if(FT1<=1.61037)tmp1=-84.6;
        else if(FT1<=1.61677)tmp1=-84.5;
        else if(FT1<=1.62319)tmp1=-84.4;
        else if(FT1<=1.62963)tmp1=-84.3;
        else if(FT1<=1.63609)tmp1=-84.2;
        else if(FT1<=1.64256)tmp1=-84.1;
        else if(FT1<=1.64906)tmp1=-84;
    }
    else if(FT1<=1.70838){
        if(FT1==1.64906)tmp1=-84;
        else if(FT1<=1.65557)tmp1=-83.9;
        else if(FT1<=1.66211)tmp1=-83.8;
        else if(FT1<=1.66866)tmp1=-83.7;
        else if(FT1<=1.67523)tmp1=-83.6;
        else if(FT1<=1.68183)tmp1=-83.5;
        else if(FT1<=1.68844)tmp1=-83.4;
        else if(FT1<=1.69507)tmp1=-83.3;
        else if(FT1<=1.70172)tmp1=-83.2;
        else if(FT1<=1.70838)tmp1=-83.1;
        else if(FT1<=1.71507)tmp1=-83;
    }
    else if(FT1<=1.77615){
        if(FT1==1.71507)tmp1=-83;
        else if(FT1<=1.72178)tmp1=-82.9;
        else if(FT1<=1.72851)tmp1=-82.8;
        else if(FT1<=1.73525)tmp1=-82.7;
        else if(FT1<=1.74202)tmp1=-82.6;
        else if(FT1<=1.74881)tmp1=-82.5;
        else if(FT1<=1.75561)tmp1=-82.4;
        else if(FT1<=1.76244)tmp1=-82.3;
        else if(FT1<=1.76928)tmp1=-82.2;
        else if(FT1<=1.77615)tmp1=-82.1;
        else if(FT1<=1.78303)tmp1=-82;
    }
    else if(FT1<=1.84588){
        if(FT1==1.78303)tmp1=-82;
        else if(FT1<=1.78993)tmp1=-81.9;
        else if(FT1<=1.79686)tmp1=-81.8;
        else if(FT1<=1.8038)tmp1=-81.7;
        else if(FT1<=1.81076)tmp1=-81.6;
        else if(FT1<=1.81775)tmp1=-81.5;
        else if(FT1<=1.82475)tmp1=-81.4;
        else if(FT1<=1.83177)tmp1=-81.3;
        else if(FT1<=1.83882)tmp1=-81.2;
        else if(FT1<=1.84588)tmp1=-81.1;
        else if(FT1<=1.85296)tmp1=-81;
    }
    else if(FT1<=1.91762){
        if(FT1==1.85296)tmp1=-81;
        else if(FT1<=1.86007)tmp1=-80.9;
        else if(FT1<=1.86719)tmp1=-80.8;
        else if(FT1<=1.87433)tmp1=-80.7;
        else if(FT1<=1.8815)tmp1=-80.6;
        else if(FT1<=1.88868)tmp1=-80.5;
        else if(FT1<=1.89589)tmp1=-80.4;
        else if(FT1<=1.90311)tmp1=-80.3;
        else if(FT1<=1.91036)tmp1=-80.2;
        else if(FT1<=1.91762)tmp1=-80.1;
        else if(FT1<=1.92491)tmp1=-80;
    }
    else if(FT1<=1.99141){
        if(FT1==1.92491)tmp1=-80;
        else if(FT1<=1.93221)tmp1=-79.9;
        else if(FT1<=1.93954)tmp1=-79.8;
        else if(FT1<=1.94689)tmp1=-79.7;
        else if(FT1<=1.95426)tmp1=-79.6;
        else if(FT1<=1.96165)tmp1=-79.5;
        else if(FT1<=1.96905)tmp1=-79.4;
        else if(FT1<=1.97648)tmp1=-79.3;
        else if(FT1<=1.98394)tmp1=-79.2;
        else if(FT1<=1.99141)tmp1=-79.1;
        else if(FT1<=1.9989)tmp1=-79;
    }
    else if(FT1<=2.06727){
        if(FT1==1.9989)tmp1=-79;
        else if(FT1<=2.00641)tmp1=-78.9;
        else if(FT1<=2.01394)tmp1=-78.8;
        else if(FT1<=2.0215)tmp1=-78.7;
        else if(FT1<=2.02907)tmp1=-78.6;
        else if(FT1<=2.03667)tmp1=-78.5;
        else if(FT1<=2.04429)tmp1=-78.4;
        else if(FT1<=2.05193)tmp1=-78.3;
        else if(FT1<=2.05958)tmp1=-78.2;
        else if(FT1<=2.06727)tmp1=-78.1;
        else if(FT1<=2.07497)tmp1=-78;
    }
    else if(FT1<=2.14523){
        if(FT1==2.07497)tmp1=-78;
        else if(FT1<=2.08269)tmp1=-77.9;
        else if(FT1<=2.09043)tmp1=-77.8;
        else if(FT1<=2.0982)tmp1=-77.7;
        else if(FT1<=2.10598)tmp1=-77.6;
        else if(FT1<=2.11379)tmp1=-77.5;
        else if(FT1<=2.12162)tmp1=-77.4;
        else if(FT1<=2.12947)tmp1=-77.3;
        else if(FT1<=2.13734)tmp1=-77.2;
        else if(FT1<=2.14523)tmp1=-77.1;
        else if(FT1<=2.15315)tmp1=-77;
    }
    else if(FT1<=2.22534){
        if(FT1==2.15315)tmp1=-77;
        else if(FT1<=2.16108)tmp1=-76.9;
        else if(FT1<=2.16904)tmp1=-76.8;
        else if(FT1<=2.17702)tmp1=-76.7;
        else if(FT1<=2.18502)tmp1=-76.6;
        else if(FT1<=2.19304)tmp1=-76.5;
        else if(FT1<=2.20108)tmp1=-76.4;
        else if(FT1<=2.20915)tmp1=-76.3;
        else if(FT1<=2.21723)tmp1=-76.2;
        else if(FT1<=2.22534)tmp1=-76.1;
        else if(FT1<=2.23347)tmp1=-76;
    }
    else if(FT1<=2.30763){
        if(FT1==2.23347)tmp1=-76;
        else if(FT1<=2.24162)tmp1=-75.9;
        else if(FT1<=2.2498)tmp1=-75.8;
        else if(FT1<=2.25799)tmp1=-75.7;
        else if(FT1<=2.26621)tmp1=-75.6;
        else if(FT1<=2.27445)tmp1=-75.5;
        else if(FT1<=2.28271)tmp1=-75.4;
        else if(FT1<=2.29099)tmp1=-75.3;
        else if(FT1<=2.2993)tmp1=-75.2;
        else if(FT1<=2.30763)tmp1=-75.1;
        else if(FT1<=2.31598)tmp1=-75;
    }
    else if(FT1<=2.39212){
        if(FT1==2.31598)tmp1=-75;
        else if(FT1<=2.32435)tmp1=-74.9;
        else if(FT1<=2.33274)tmp1=-74.8;
        else if(FT1<=2.34116)tmp1=-74.7;
        else if(FT1<=2.3496)tmp1=-74.6;
        else if(FT1<=2.35806)tmp1=-74.5;
        else if(FT1<=2.36654)tmp1=-74.4;
        else if(FT1<=2.37504)tmp1=-74.3;
        else if(FT1<=2.38357)tmp1=-74.2;
        else if(FT1<=2.39212)tmp1=-74.1;
        else if(FT1<=2.40069)tmp1=-74;
    }
    else if(FT1<=2.47886){
        if(FT1==2.40069)tmp1=-74;
        else if(FT1<=2.40929)tmp1=-73.9;
        else if(FT1<=2.41791)tmp1=-73.8;
        else if(FT1<=2.42654)tmp1=-73.7;
        else if(FT1<=2.43521)tmp1=-73.6;
        else if(FT1<=2.44389)tmp1=-73.5;
        else if(FT1<=2.4526)tmp1=-73.4;
        else if(FT1<=2.46133)tmp1=-73.3;
        else if(FT1<=2.47008)tmp1=-73.2;
        else if(FT1<=2.47886)tmp1=-73.1;
        else if(FT1<=2.48766)tmp1=-73;
    }
    else if(FT1<=2.56787){
        if(FT1==2.48766)tmp1=-73;
        else if(FT1<=2.49648)tmp1=-72.9;
        else if(FT1<=2.50532)tmp1=-72.8;
        else if(FT1<=2.51419)tmp1=-72.7;
        else if(FT1<=2.52308)tmp1=-72.6;
        else if(FT1<=2.53199)tmp1=-72.5;
        else if(FT1<=2.54093)tmp1=-72.4;
        else if(FT1<=2.54988)tmp1=-72.3;
        else if(FT1<=2.55887)tmp1=-72.2;
        else if(FT1<=2.56787)tmp1=-72.1;
        else if(FT1<=2.5769)tmp1=-72;
    }
    else if(FT1<=2.65919){
        if(FT1==2.5769)tmp1=-72;
        else if(FT1<=2.58595)tmp1=-71.9;
        else if(FT1<=2.59502)tmp1=-71.8;
        else if(FT1<=2.60412)tmp1=-71.7;
        else if(FT1<=2.61324)tmp1=-71.6;
        else if(FT1<=2.62238)tmp1=-71.5;
        else if(FT1<=2.63155)tmp1=-71.4;
        else if(FT1<=2.64074)tmp1=-71.3;
        else if(FT1<=2.64996)tmp1=-71.2;
        else if(FT1<=2.65919)tmp1=-71.1;
        else if(FT1<=2.66845)tmp1=-71;
    }
    else if(FT1<=2.75286){
        if(FT1==2.66845)tmp1=-71;
        else if(FT1<=2.67774)tmp1=-70.9;
        else if(FT1<=2.68704)tmp1=-70.8;
        else if(FT1<=2.69638)tmp1=-70.7;
        else if(FT1<=2.70573)tmp1=-70.6;
        else if(FT1<=2.71511)tmp1=-70.5;
        else if(FT1<=2.72451)tmp1=-70.4;
        else if(FT1<=2.73394)tmp1=-70.3;
        else if(FT1<=2.74338)tmp1=-70.2;
        else if(FT1<=2.75286)tmp1=-70.1;
        else if(FT1<=2.76235)tmp1=-70;
    }
    else if(FT1<=2.8489){
        if(FT1==2.76235)tmp1=-70;
        else if(FT1<=2.77187)tmp1=-69.9;
        else if(FT1<=2.78142)tmp1=-69.8;
        else if(FT1<=2.79099)tmp1=-69.7;
        else if(FT1<=2.80058)tmp1=-69.6;
        else if(FT1<=2.81019)tmp1=-69.5;
        else if(FT1<=2.81983)tmp1=-69.4;
        else if(FT1<=2.8295)tmp1=-69.3;
        else if(FT1<=2.83919)tmp1=-69.2;
        else if(FT1<=2.8489)tmp1=-69.1;
        else if(FT1<=2.85863)tmp1=-69;
    }
    else if(FT1<=2.94735){
        if(FT1==2.85863)tmp1=-69;
        else if(FT1<=2.86839)tmp1=-68.9;
        else if(FT1<=2.87818)tmp1=-68.8;
        else if(FT1<=2.88799)tmp1=-68.7;
        else if(FT1<=2.89782)tmp1=-68.6;
        else if(FT1<=2.90768)tmp1=-68.5;
        else if(FT1<=2.91756)tmp1=-68.4;
        else if(FT1<=2.92746)tmp1=-68.3;
        else if(FT1<=2.93739)tmp1=-68.2;
        else if(FT1<=2.94735)tmp1=-68.1;
        else if(FT1<=2.95733)tmp1=-68;
    }
    else if(FT1<=3.04824){
        if(FT1==2.95733)tmp1=-68;
        else if(FT1<=2.96733)tmp1=-67.9;
        else if(FT1<=2.97736)tmp1=-67.8;
        else if(FT1<=2.98741)tmp1=-67.7;
        else if(FT1<=2.99748)tmp1=-67.6;
        else if(FT1<=3.00759)tmp1=-67.5;
        else if(FT1<=3.01771)tmp1=-67.4;
        else if(FT1<=3.02786)tmp1=-67.3;
        else if(FT1<=3.03804)tmp1=-67.2;
        else if(FT1<=3.04824)tmp1=-67.1;
        else if(FT1<=3.05846)tmp1=-67;
    }
    else if(FT1<=3.1516){
        if(FT1==3.05846)tmp1=-67;
        else if(FT1<=3.06871)tmp1=-66.9;
        else if(FT1<=3.07899)tmp1=-66.8;
        else if(FT1<=3.08928)tmp1=-66.7;
        else if(FT1<=3.09961)tmp1=-66.6;
        else if(FT1<=3.10996)tmp1=-66.5;
        else if(FT1<=3.12033)tmp1=-66.4;
        else if(FT1<=3.13073)tmp1=-66.3;
        else if(FT1<=3.14115)tmp1=-66.2;
        else if(FT1<=3.1516)tmp1=-66.1;
        else if(FT1<=3.16208)tmp1=-66;
    }
    else if(FT1<=3.25747){
        if(FT1==3.16208)tmp1=-66;
        else if(FT1<=3.17257)tmp1=-65.9;
        else if(FT1<=3.1831)tmp1=-65.8;
        else if(FT1<=3.19365)tmp1=-65.7;
        else if(FT1<=3.20422)tmp1=-65.6;
        else if(FT1<=3.21482)tmp1=-65.5;
        else if(FT1<=3.22545)tmp1=-65.4;
        else if(FT1<=3.2361)tmp1=-65.3;
        else if(FT1<=3.24677)tmp1=-65.2;
        else if(FT1<=3.25747)tmp1=-65.1;
        else if(FT1<=3.2682)tmp1=-65;
    }
    else if(FT1<=3.36589){
        if(FT1==3.2682)tmp1=-65;
        else if(FT1<=3.27895)tmp1=-64.9;
        else if(FT1<=3.28973)tmp1=-64.8;
        else if(FT1<=3.30053)tmp1=-64.7;
        else if(FT1<=3.31136)tmp1=-64.6;
        else if(FT1<=3.32221)tmp1=-64.5;
        else if(FT1<=3.33309)tmp1=-64.4;
        else if(FT1<=3.344)tmp1=-64.3;
        else if(FT1<=3.35493)tmp1=-64.2;
        else if(FT1<=3.36589)tmp1=-64.1;
        else if(FT1<=3.37687)tmp1=-64;
    }
    else if(FT1<=3.47687){
        if(FT1==3.37687)tmp1=-64;
        else if(FT1<=3.38788)tmp1=-63.9;
        else if(FT1<=3.39891)tmp1=-63.8;
        else if(FT1<=3.40997)tmp1=-63.7;
        else if(FT1<=3.42105)tmp1=-63.6;
        else if(FT1<=3.43217)tmp1=-63.5;
        else if(FT1<=3.4433)tmp1=-63.4;
        else if(FT1<=3.45447)tmp1=-63.3;
        else if(FT1<=3.46566)tmp1=-63.2;
        else if(FT1<=3.47687)tmp1=-63.1;
    }
    else if(FT1<=3.59046){
        if(FT1==3.48811)tmp1=-63;
        else if(FT1<=3.49938)tmp1=-62.9;
        else if(FT1<=3.51067)tmp1=-62.8;
        else if(FT1<=3.52199)tmp1=-62.7;
        else if(FT1<=3.53334)tmp1=-62.6;
        else if(FT1<=3.54471)tmp1=-62.5;
        else if(FT1<=3.55611)tmp1=-62.4;
        else if(FT1<=3.56753)tmp1=-62.3;
        else if(FT1<=3.57898)tmp1=-62.2;
        else if(FT1<=3.59046)tmp1=-62.1;
    }
    else if(FT1<=3.70668){
        if(FT1==3.60196)tmp1=-62;
        else if(FT1<=3.61349)tmp1=-61.9;
        else if(FT1<=3.62505)tmp1=-61.8;
        else if(FT1<=3.63663)tmp1=-61.7;
        else if(FT1<=3.64824)tmp1=-61.6;
        else if(FT1<=3.65988)tmp1=-61.5;
        else if(FT1<=3.67154)tmp1=-61.4;
        else if(FT1<=3.68323)tmp1=-61.3;
        else if(FT1<=3.69494)tmp1=-61.2;
        else if(FT1<=3.70668)tmp1=-61.1;
    }
    else if(FT1<=3.82558){
        if(FT1==3.71845)tmp1=-61;
        else if(FT1<=3.73025)tmp1=-60.9;
        else if(FT1<=3.74207)tmp1=-60.8;
        else if(FT1<=3.75392)tmp1=-60.7;
        else if(FT1<=3.76579)tmp1=-60.6;
        else if(FT1<=3.7777)tmp1=-60.5;
        else if(FT1<=3.78963)tmp1=-60.4;
        else if(FT1<=3.80158)tmp1=-60.3;
        else if(FT1<=3.81357)tmp1=-60.2;
        else if(FT1<=3.82558)tmp1=-60.1;
    }
    else if(FT1<=3.94717){
        if(FT1==3.83761)tmp1=-60;
        else if(FT1<=3.84968)tmp1=-59.9;
        else if(FT1<=3.86177)tmp1=-59.8;
        else if(FT1<=3.87389)tmp1=-59.7;
        else if(FT1<=3.88603)tmp1=-59.6;
        else if(FT1<=3.89821)tmp1=-59.5;
        else if(FT1<=3.91041)tmp1=-59.4;
        else if(FT1<=3.92264)tmp1=-59.3;
        else if(FT1<=3.93489)tmp1=-59.2;
        else if(FT1<=3.94717)tmp1=-59.1;
    }
    else if(FT1<=4.0715){
        if(FT1==3.95948)tmp1=-59;
        else if(FT1<=3.97182)tmp1=-58.9;
        else if(FT1<=3.98418)tmp1=-58.8;
        else if(FT1<=3.99657)tmp1=-58.7;
        else if(FT1<=4.00899)tmp1=-58.6;
        else if(FT1<=4.02144)tmp1=-58.5;
        else if(FT1<=4.03391)tmp1=-58.4;
        else if(FT1<=4.04641)tmp1=-58.3;
        else if(FT1<=4.05894)tmp1=-58.2;
        else if(FT1<=4.0715)tmp1=-58.1;
    }
    else if(FT1<=4.19859){
        if(FT1==4.08408)tmp1=-58;
        else if(FT1<=4.09669)tmp1=-57.9;
        else if(FT1<=4.10933)tmp1=-57.8;
        else if(FT1<=4.122)tmp1=-57.7;
        else if(FT1<=4.1347)tmp1=-57.6;
        else if(FT1<=4.14742)tmp1=-57.5;
        else if(FT1<=4.16017)tmp1=-57.4;
        else if(FT1<=4.17295)tmp1=-57.3;
        else if(FT1<=4.18575)tmp1=-57.2;
        else if(FT1<=4.19859)tmp1=-57.1;
    }
    else if(FT1<=4.32847){
        if(FT1==4.21145)tmp1=-57;
        else if(FT1<=4.22434)tmp1=-56.9;
        else if(FT1<=4.23726)tmp1=-56.8;
        else if(FT1<=4.2502)tmp1=-56.7;
        else if(FT1<=4.26318)tmp1=-56.6;
        else if(FT1<=4.27618)tmp1=-56.5;
        else if(FT1<=4.28921)tmp1=-56.4;
        else if(FT1<=4.30227)tmp1=-56.3;
        else if(FT1<=4.31536)tmp1=-56.2;
        else if(FT1<=4.32847)tmp1=-56.1;
    }
    else if(FT1<=4.46118){
        if(FT1==4.34162)tmp1=-56;
        else if(FT1<=4.35479)tmp1=-55.9;
        else if(FT1<=4.36799)tmp1=-55.8;
        else if(FT1<=4.38122)tmp1=-55.7;
        else if(FT1<=4.39447)tmp1=-55.6;
        else if(FT1<=4.40776)tmp1=-55.5;
        else if(FT1<=4.42107)tmp1=-55.4;
        else if(FT1<=4.43441)tmp1=-55.3;
        else if(FT1<=4.44778)tmp1=-55.2;
        else if(FT1<=4.46118)tmp1=-55.1;
    }
    else if(FT1<=4.59675){
        if(FT1==4.47461)tmp1=-55;
        else if(FT1<=4.48807)tmp1=-54.9;
        else if(FT1<=4.50155)tmp1=-54.8;
        else if(FT1<=4.51507)tmp1=-54.7;
        else if(FT1<=4.52861)tmp1=-54.6;
        else if(FT1<=4.54218)tmp1=-54.5;
        else if(FT1<=4.55578)tmp1=-54.4;
        else if(FT1<=4.56941)tmp1=-54.3;
        else if(FT1<=4.58306)tmp1=-54.2;
        else if(FT1<=4.59675)tmp1=-54.1;
    }
    else if(FT1<=4.73521){
        if(FT1==4.61047)tmp1=-54;
        else if(FT1<=4.62421)tmp1=-53.9;
        else if(FT1<=4.63798)tmp1=-53.8;
        else if(FT1<=4.65178)tmp1=-53.7;
        else if(FT1<=4.66562)tmp1=-53.6;
        else if(FT1<=4.67948)tmp1=-53.5;
        else if(FT1<=4.69336)tmp1=-53.4;
        else if(FT1<=4.70728)tmp1=-53.3;
        else if(FT1<=4.72123)tmp1=-53.2;
        else if(FT1<=4.73521)tmp1=-53.1;
    }
    else if(FT1<=4.87658){
        if(FT1==4.74921)tmp1=-53;
        else if(FT1<=4.76325)tmp1=-52.9;
        else if(FT1<=4.77731)tmp1=-52.8;
        else if(FT1<=4.7914)tmp1=-52.7;
        else if(FT1<=4.80553)tmp1=-52.6;
        else if(FT1<=4.81968)tmp1=-52.5;
        else if(FT1<=4.83386)tmp1=-52.4;
        else if(FT1<=4.84807)tmp1=-52.3;
        else if(FT1<=4.86231)tmp1=-52.2;
        else if(FT1<=4.87658)tmp1=-52.1;
    }
    else if(FT1<=5.0209){
        if(FT1==4.89088)tmp1=-52;
        else if(FT1<=4.90521)tmp1=-51.9;
        else if(FT1<=4.91957)tmp1=-51.8;
        else if(FT1<=4.93395)tmp1=-51.7;
        else if(FT1<=4.94837)tmp1=-51.6;
        else if(FT1<=4.96282)tmp1=-51.5;
        else if(FT1<=4.97729)tmp1=-51.4;
        else if(FT1<=4.9918)tmp1=-51.3;
        else if(FT1<=5.00634)tmp1=-51.2;
        else if(FT1<=5.0209)tmp1=-51.1;
    }
    else if(FT1<=5.1682){
        if(FT1==5.0355)tmp1=-51;
        else if(FT1<=5.05012)tmp1=-50.9;
        else if(FT1<=5.06478)tmp1=-50.8;
        else if(FT1<=5.07946)tmp1=-50.7;
        else if(FT1<=5.09418)tmp1=-50.6;
        else if(FT1<=5.10892)tmp1=-50.5;
        else if(FT1<=5.1237)tmp1=-50.4;
        else if(FT1<=5.1385)tmp1=-50.3;
        else if(FT1<=5.15334)tmp1=-50.2;
        else if(FT1<=5.1682)tmp1=-50.1;
    }
    else if(FT1<=5.31851){
        if(FT1==5.1831)tmp1=-50;
        else if(FT1<=5.19802)tmp1=-49.9;
        else if(FT1<=5.21298)tmp1=-49.8;
        else if(FT1<=5.22796)tmp1=-49.7;
        else if(FT1<=5.24298)tmp1=-49.6;
        else if(FT1<=5.25802)tmp1=-49.5;
        else if(FT1<=5.2731)tmp1=-49.4;
        else if(FT1<=5.28821)tmp1=-49.3;
        else if(FT1<=5.30334)tmp1=-49.2;
        else if(FT1<=5.31851)tmp1=-49.1;
    }
    else if(FT1<=5.47186){
        if(FT1==5.33371)tmp1=-49;
        else if(FT1<=5.34894)tmp1=-48.9;
        else if(FT1<=5.3642)tmp1=-48.8;
        else if(FT1<=5.37948)tmp1=-48.7;
        else if(FT1<=5.3948)tmp1=-48.6;
        else if(FT1<=5.41015)tmp1=-48.5;
        else if(FT1<=5.42553)tmp1=-48.4;
        else if(FT1<=5.44094)tmp1=-48.3;
        else if(FT1<=5.45639)tmp1=-48.2;
        else if(FT1<=5.47186)tmp1=-48.1;
    }
    else if(FT1<=5.62827){
        if(FT1==5.48736)tmp1=-48;
        else if(FT1<=5.5029)tmp1=-47.9;
        else if(FT1<=5.51846)tmp1=-47.8;
        else if(FT1<=5.53405)tmp1=-47.7;
        else if(FT1<=5.54968)tmp1=-47.6;
        else if(FT1<=5.56534)tmp1=-47.5;
        else if(FT1<=5.58103)tmp1=-47.4;
        else if(FT1<=5.59674)tmp1=-47.3;
        else if(FT1<=5.61249)tmp1=-47.2;
        else if(FT1<=5.62827)tmp1=-47.1;
    }
    else if(FT1<=5.78779){
        if(FT1==5.64408)tmp1=-47;
        else if(FT1<=5.65993)tmp1=-46.9;
        else if(FT1<=5.6758)tmp1=-46.8;
        else if(FT1<=5.69171)tmp1=-46.7;
        else if(FT1<=5.70764)tmp1=-46.6;
        else if(FT1<=5.72361)tmp1=-46.5;
        else if(FT1<=5.7396)tmp1=-46.4;
        else if(FT1<=5.75563)tmp1=-46.3;
        else if(FT1<=5.77169)tmp1=-46.2;
        else if(FT1<=5.78779)tmp1=-46.1;
    }
    else if(FT1<=5.95042){
        if(FT1==5.80391)tmp1=-46;
        else if(FT1<=5.82006)tmp1=-45.9;
        else if(FT1<=5.83625)tmp1=-45.8;
        else if(FT1<=5.85246)tmp1=-45.7;
        else if(FT1<=5.86871)tmp1=-45.6;
        else if(FT1<=5.88499)tmp1=-45.5;
        else if(FT1<=5.9013)tmp1=-45.4;
        else if(FT1<=5.91764)tmp1=-45.3;
        else if(FT1<=5.93402)tmp1=-45.2;
        else if(FT1<=5.95042)tmp1=-45.1;
    }
    else if(FT1<=6.11621){
        if(FT1==5.96686)tmp1=-45;
        else if(FT1<=5.98333)tmp1=-44.9;
        else if(FT1<=5.99983)tmp1=-44.8;
        else if(FT1<=6.01636)tmp1=-44.7;
        else if(FT1<=6.03292)tmp1=-44.6;
        else if(FT1<=6.04952)tmp1=-44.5;
        else if(FT1<=6.06614)tmp1=-44.4;
        else if(FT1<=6.0828)tmp1=-44.3;
        else if(FT1<=6.09949)tmp1=-44.2;
        else if(FT1<=6.11621)tmp1=-44.1;
    }
    else if(FT1<=6.28519){
        if(FT1==6.13297)tmp1=-44;
        else if(FT1<=6.14975)tmp1=-43.9;
        else if(FT1<=6.16657)tmp1=-43.8;
        else if(FT1<=6.18342)tmp1=-43.7;
        else if(FT1<=6.2003)tmp1=-43.6;
        else if(FT1<=6.21722)tmp1=-43.5;
        else if(FT1<=6.23416)tmp1=-43.4;
        else if(FT1<=6.25114)tmp1=-43.3;
        else if(FT1<=6.26815)tmp1=-43.2;
        else if(FT1<=6.28519)tmp1=-43.1;
    }
    else if(FT1<=6.45738){
        if(FT1==6.30226)tmp1=-43;
        else if(FT1<=6.31937)tmp1=-42.9;
        else if(FT1<=6.33651)tmp1=-42.8;
        else if(FT1<=6.35368)tmp1=-42.7;
        else if(FT1<=6.37088)tmp1=-42.6;
        else if(FT1<=6.38811)tmp1=-42.5;
        else if(FT1<=6.40538)tmp1=-42.4;
        else if(FT1<=6.42268)tmp1=-42.3;
        else if(FT1<=6.44001)tmp1=-42.2;
        else if(FT1<=6.45738)tmp1=-42.1;
    }
    else if(FT1<=6.6328){
        if(FT1==6.47477)tmp1=-42;
        else if(FT1<=6.4922)tmp1=-41.9;
        else if(FT1<=6.50966)tmp1=-41.8;
        else if(FT1<=6.52716)tmp1=-41.7;
        else if(FT1<=6.54468)tmp1=-41.6;
        else if(FT1<=6.56224)tmp1=-41.5;
        else if(FT1<=6.57983)tmp1=-41.4;
        else if(FT1<=6.59746)tmp1=-41.3;
        else if(FT1<=6.61511)tmp1=-41.2;
        else if(FT1<=6.6328)tmp1=-41.1;
    }
    else if(FT1<=6.81149){
        if(FT1==6.65052)tmp1=-41;
        else if(FT1<=6.66828)tmp1=-40.9;
        else if(FT1<=6.68606)tmp1=-40.8;
        else if(FT1<=6.70388)tmp1=-40.7;
        else if(FT1<=6.72174)tmp1=-40.6;
        else if(FT1<=6.73962)tmp1=-40.5;
        else if(FT1<=6.75754)tmp1=-40.4;
        else if(FT1<=6.77549)tmp1=-40.3;
        else if(FT1<=6.79348)tmp1=-40.2;
        else if(FT1<=6.81149)tmp1=-40.1;
    }
    else if(FT1<=6.99348){
        if(FT1==6.82954)tmp1=-40;
        else if(FT1<=6.84763)tmp1=-39.9;
        else if(FT1<=6.86574)tmp1=-39.8;
        else if(FT1<=6.88389)tmp1=-39.7;
        else if(FT1<=6.90207)tmp1=-39.6;
        else if(FT1<=6.92029)tmp1=-39.5;
        else if(FT1<=6.93854)tmp1=-39.4;
        else if(FT1<=6.95682)tmp1=-39.3;
        else if(FT1<=6.97513)tmp1=-39.2;
        else if(FT1<=6.99348)tmp1=-39.1;
    }
    else if(FT1<=7.17879){
        if(FT1==7.01186)tmp1=-39;
        else if(FT1<=7.03028)tmp1=-38.9;
        else if(FT1<=7.04872)tmp1=-38.8;
        else if(FT1<=7.0672)tmp1=-38.7;
        else if(FT1<=7.08572)tmp1=-38.6;
        else if(FT1<=7.10427)tmp1=-38.5;
        else if(FT1<=7.12285)tmp1=-38.4;
        else if(FT1<=7.14146)tmp1=-38.3;
        else if(FT1<=7.16011)tmp1=-38.2;
        else if(FT1<=7.17879)tmp1=-38.1;
    }
    else if(FT1<=7.36745){
        if(FT1==7.1975)tmp1=-38;
        else if(FT1<=7.21625)tmp1=-37.9;
        else if(FT1<=7.23503)tmp1=-37.8;
        else if(FT1<=7.25385)tmp1=-37.7;
        else if(FT1<=7.2727)tmp1=-37.6;
        else if(FT1<=7.29158)tmp1=-37.5;
        else if(FT1<=7.3105)tmp1=-37.4;
        else if(FT1<=7.32944)tmp1=-37.3;
        else if(FT1<=7.34843)tmp1=-37.2;
        else if(FT1<=7.36745)tmp1=-37.1;
    }
    else if(FT1<=7.55948){
        if(FT1==7.3865)tmp1=-37;
        else if(FT1<=7.40558)tmp1=-36.9;
        else if(FT1<=7.4247)tmp1=-36.8;
        else if(FT1<=7.44385)tmp1=-36.7;
        else if(FT1<=7.46304)tmp1=-36.6;
        else if(FT1<=7.48226)tmp1=-36.5;
        else if(FT1<=7.50151)tmp1=-36.4;
        else if(FT1<=7.5208)tmp1=-36.3;
        else if(FT1<=7.54012)tmp1=-36.2;
        else if(FT1<=7.55948)tmp1=-36.1;
    }
    else if(FT1<=7.75491){
        if(FT1==7.57887)tmp1=-36;
        else if(FT1<=7.59829)tmp1=-35.9;
        else if(FT1<=7.61775)tmp1=-35.8;
        else if(FT1<=7.63724)tmp1=-35.7;
        else if(FT1<=7.65677)tmp1=-35.6;
        else if(FT1<=7.67633)tmp1=-35.5;
        else if(FT1<=7.69592)tmp1=-35.4;
        else if(FT1<=7.71555)tmp1=-35.3;
        else if(FT1<=7.73521)tmp1=-35.2;
        else if(FT1<=7.75491)tmp1=-35.1;
    }
    else if(FT1<=7.95377){
        if(FT1==7.77464)tmp1=-35;
        else if(FT1<=7.79441)tmp1=-34.9;
        else if(FT1<=7.81421)tmp1=-34.8;
        else if(FT1<=7.83404)tmp1=-34.7;
        else if(FT1<=7.85391)tmp1=-34.6;
        else if(FT1<=7.87381)tmp1=-34.5;
        else if(FT1<=7.89375)tmp1=-34.4;
        else if(FT1<=7.91372)tmp1=-34.3;
        else if(FT1<=7.93373)tmp1=-34.2;
        else if(FT1<=7.95377)tmp1=-34.1;
    }
    else if(FT1<=8.15609){
        if(FT1==7.97385)tmp1=-34;
        else if(FT1<=7.99396)tmp1=-33.9;
        else if(FT1<=8.0141)tmp1=-33.8;
        else if(FT1<=8.03428)tmp1=-33.7;
        else if(FT1<=8.0545)tmp1=-33.6;
        else if(FT1<=8.07474)tmp1=-33.5;
        else if(FT1<=8.09503)tmp1=-33.4;
        else if(FT1<=8.11535)tmp1=-33.3;
        else if(FT1<=8.1357)tmp1=-33.2;
        else if(FT1<=8.15609)tmp1=-33.1;
    }
    else if(FT1<=8.36188){
        if(FT1==8.17651)tmp1=-33;
        else if(FT1<=8.19697)tmp1=-32.9;
        else if(FT1<=8.21746)tmp1=-32.8;
        else if(FT1<=8.23798)tmp1=-32.7;
        else if(FT1<=8.25855)tmp1=-32.6;
        else if(FT1<=8.27914)tmp1=-32.5;
        else if(FT1<=8.29978)tmp1=-32.4;
        else if(FT1<=8.32044)tmp1=-32.3;
        else if(FT1<=8.34114)tmp1=-32.2;
        else if(FT1<=8.36188)tmp1=-32.1;
    }
    else if(FT1<=8.57118){
        if(FT1==8.38265)tmp1=-32;
        else if(FT1<=8.40346)tmp1=-31.9;
        else if(FT1<=8.4243)tmp1=-31.8;
        else if(FT1<=8.44518)tmp1=-31.7;
        else if(FT1<=8.46609)tmp1=-31.6;
        else if(FT1<=8.48704)tmp1=-31.5;
        else if(FT1<=8.50802)tmp1=-31.4;
        else if(FT1<=8.52904)tmp1=-31.3;
        else if(FT1<=8.55009)tmp1=-31.2;
        else if(FT1<=8.57118)tmp1=-31.1;
    }
    else if(FT1<=8.78401){
        if(FT1==8.5923)tmp1=-31;
        else if(FT1<=8.61346)tmp1=-30.9;
        else if(FT1<=8.63465)tmp1=-30.8;
        else if(FT1<=8.65588)tmp1=-30.7;
        else if(FT1<=8.67715)tmp1=-30.6;
        else if(FT1<=8.69845)tmp1=-30.5;
        else if(FT1<=8.71979)tmp1=-30.4;
        else if(FT1<=8.74116)tmp1=-30.3;
        else if(FT1<=8.76256)tmp1=-30.2;
        else if(FT1<=8.78401)tmp1=-30.1;
    }
    else if(FT1<=9.00039){
        if(FT1==8.80548)tmp1=-30;
        else if(FT1<=8.827)tmp1=-29.9;
        else if(FT1<=8.84855)tmp1=-29.8;
        else if(FT1<=8.87013)tmp1=-29.7;
        else if(FT1<=8.89175)tmp1=-29.6;
        else if(FT1<=8.91341)tmp1=-29.5;
        else if(FT1<=8.9351)tmp1=-29.4;
        else if(FT1<=8.95682)tmp1=-29.3;
        else if(FT1<=8.97859)tmp1=-29.2;
        else if(FT1<=9.00039)tmp1=-29.1;
    }
    else if(FT1<=9.22035){
        if(FT1==9.02222)tmp1=-29;
        else if(FT1<=9.04409)tmp1=-28.9;
        else if(FT1<=9.066)tmp1=-28.8;
        else if(FT1<=9.08794)tmp1=-28.7;
        else if(FT1<=9.10992)tmp1=-28.6;
        else if(FT1<=9.13193)tmp1=-28.5;
        else if(FT1<=9.15398)tmp1=-28.4;
        else if(FT1<=9.17607)tmp1=-28.3;
        else if(FT1<=9.19819)tmp1=-28.2;
        else if(FT1<=9.22035)tmp1=-28.1;
    }
    else if(FT1<=9.44391){
        if(FT1==9.24254)tmp1=-28;
        else if(FT1<=9.26477)tmp1=-27.9;
        else if(FT1<=9.28704)tmp1=-27.8;
        else if(FT1<=9.30934)tmp1=-27.7;
        else if(FT1<=9.33168)tmp1=-27.6;
        else if(FT1<=9.35405)tmp1=-27.5;
        else if(FT1<=9.37646)tmp1=-27.4;
        else if(FT1<=9.39891)tmp1=-27.3;
        else if(FT1<=9.42139)tmp1=-27.2;
        else if(FT1<=9.44391)tmp1=-27.1;
    }
    else if(FT1<=9.6711){
        if(FT1==9.46646)tmp1=-27;
        else if(FT1<=9.48906)tmp1=-26.9;
        else if(FT1<=9.51168)tmp1=-26.8;
        else if(FT1<=9.53435)tmp1=-26.7;
        else if(FT1<=9.55705)tmp1=-26.6;
        else if(FT1<=9.57979)tmp1=-26.5;
        else if(FT1<=9.60256)tmp1=-26.4;
        else if(FT1<=9.62537)tmp1=-26.3;
        else if(FT1<=9.64822)tmp1=-26.2;
        else if(FT1<=9.6711)tmp1=-26.1;
    }
    else if(FT1<=9.90194){
        if(FT1==9.69402)tmp1=-26;
        else if(FT1<=9.71697)tmp1=-25.9;
        else if(FT1<=9.73997)tmp1=-25.8;
        else if(FT1<=9.76299)tmp1=-25.7;
        else if(FT1<=9.78606)tmp1=-25.6;
        else if(FT1<=9.80916)tmp1=-25.5;
        else if(FT1<=9.8323)tmp1=-25.4;
        else if(FT1<=9.85548)tmp1=-25.3;
        else if(FT1<=9.87869)tmp1=-25.2;
        else if(FT1<=9.90194)tmp1=-25.1;
    }
    else if(FT1<=10.1364){
        if(FT1==9.92522)tmp1=-25;
        else if(FT1<=9.94854)tmp1=-24.9;
        else if(FT1<=9.9719)tmp1=-24.8;
        else if(FT1<=9.9953)tmp1=-24.7;
        else if(FT1<=10.0187)tmp1=-24.6;
        else if(FT1<=10.0422)tmp1=-24.5;
        else if(FT1<=10.0657)tmp1=-24.4;
        else if(FT1<=10.0893)tmp1=-24.3;
        else if(FT1<=10.1128)tmp1=-24.2;
        else if(FT1<=10.1364)tmp1=-24.1;
    }
    else if(FT1<=10.3747){
        if(FT1==10.1601)tmp1=-24;
        else if(FT1<=10.1838)tmp1=-23.9;
        else if(FT1<=10.2075)tmp1=-23.8;
        else if(FT1<=10.2313)tmp1=-23.7;
        else if(FT1<=10.2551)tmp1=-23.6;
        else if(FT1<=10.2789)tmp1=-23.5;
        else if(FT1<=10.3028)tmp1=-23.4;
        else if(FT1<=10.3267)tmp1=-23.3;
        else if(FT1<=10.3507)tmp1=-23.2;
        else if(FT1<=10.3747)tmp1=-23.1;
    }
    else if(FT1<=10.6166){
        if(FT1==10.3987)tmp1=-23;
        else if(FT1<=10.4227)tmp1=-22.9;
        else if(FT1<=10.4468)tmp1=-22.8;
        else if(FT1<=10.471)tmp1=-22.7;
        else if(FT1<=10.4952)tmp1=-22.6;
        else if(FT1<=10.5194)tmp1=-22.5;
        else if(FT1<=10.5436)tmp1=-22.4;
        else if(FT1<=10.5679)tmp1=-22.3;
        else if(FT1<=10.5922)tmp1=-22.2;
        else if(FT1<=10.6166)tmp1=-22.1;
    }
    else if(FT1<=10.8623){
        if(FT1==10.641)tmp1=-22;
        else if(FT1<=10.6654)tmp1=-21.9;
        else if(FT1<=10.6899)tmp1=-21.8;
        else if(FT1<=10.7144)tmp1=-21.7;
        else if(FT1<=10.739)tmp1=-21.6;
        else if(FT1<=10.7635)tmp1=-21.5;
        else if(FT1<=10.7882)tmp1=-21.4;
        else if(FT1<=10.8128)tmp1=-21.3;
        else if(FT1<=10.8375)tmp1=-21.2;
        else if(FT1<=10.8623)tmp1=-21.1;
    }
    else if(FT1<=11.1117){
        if(FT1==10.887)tmp1=-21;
        else if(FT1<=10.9118)tmp1=-20.9;
        else if(FT1<=10.9367)tmp1=-20.8;
        else if(FT1<=10.9616)tmp1=-20.7;
        else if(FT1<=10.9865)tmp1=-20.6;
        else if(FT1<=11.0115)tmp1=-20.5;
        else if(FT1<=11.0365)tmp1=-20.4;
        else if(FT1<=11.0615)tmp1=-20.3;
        else if(FT1<=11.0866)tmp1=-20.2;
        else if(FT1<=11.1117)tmp1=-20.1;
    }
    else if(FT1<=11.3649){
        if(FT1==11.1368)tmp1=-20;
        else if(FT1<=11.162)tmp1=-19.9;
        else if(FT1<=11.1873)tmp1=-19.8;
        else if(FT1<=11.2125)tmp1=-19.7;
        else if(FT1<=11.2378)tmp1=-19.6;
        else if(FT1<=11.2632)tmp1=-19.5;
        else if(FT1<=11.2885)tmp1=-19.4;
        else if(FT1<=11.314)tmp1=-19.3;
        else if(FT1<=11.3394)tmp1=-19.2;
        else if(FT1<=11.3649)tmp1=-19.1;
    }
    else if(FT1<=11.6219){
        if(FT1==11.3904)tmp1=-19;
        else if(FT1<=11.416)tmp1=-18.9;
        else if(FT1<=11.4416)tmp1=-18.8;
        else if(FT1<=11.4673)tmp1=-18.7;
        else if(FT1<=11.4929)tmp1=-18.6;
        else if(FT1<=11.5187)tmp1=-18.5;
        else if(FT1<=11.5444)tmp1=-18.4;
        else if(FT1<=11.5702)tmp1=-18.3;
        else if(FT1<=11.5961)tmp1=-18.2;
        else if(FT1<=11.6219)tmp1=-18.1;
    }
    else if(FT1<=11.8828){
        if(FT1==11.6478)tmp1=-18;
        else if(FT1<=11.6738)tmp1=-17.9;
        else if(FT1<=11.6998)tmp1=-17.8;
        else if(FT1<=11.7258)tmp1=-17.7;
        else if(FT1<=11.7519)tmp1=-17.6;
        else if(FT1<=11.778)tmp1=-17.5;
        else if(FT1<=11.8041)tmp1=-17.4;
        else if(FT1<=11.8303)tmp1=-17.3;
        else if(FT1<=11.8565)tmp1=-17.2;
        else if(FT1<=11.8828)tmp1=-17.1;
    }
    else if(FT1<=12.1475){
        if(FT1==11.9091)tmp1=-17;
        else if(FT1<=11.9354)tmp1=-16.9;
        else if(FT1<=11.9618)tmp1=-16.8;
        else if(FT1<=11.9882)tmp1=-16.7;
        else if(FT1<=12.0146)tmp1=-16.6;
        else if(FT1<=12.0411)tmp1=-16.5;
        else if(FT1<=12.0677)tmp1=-16.4;
        else if(FT1<=12.0942)tmp1=-16.3;
        else if(FT1<=12.1208)tmp1=-16.2;
        else if(FT1<=12.1475)tmp1=-16.1;
    }
    else if(FT1<=12.416){
        if(FT1==12.1742)tmp1=-16;
        else if(FT1<=12.2009)tmp1=-15.9;
        else if(FT1<=12.2276)tmp1=-15.8;
        else if(FT1<=12.2544)tmp1=-15.7;
        else if(FT1<=12.2813)tmp1=-15.6;
        else if(FT1<=12.3082)tmp1=-15.5;
        else if(FT1<=12.3351)tmp1=-15.4;
        else if(FT1<=12.362)tmp1=-15.3;
        else if(FT1<=12.389)tmp1=-15.2;
        else if(FT1<=12.416)tmp1=-15.1;
        else if(FT1<=12.4431)tmp1=-15;
    }
    else if(FT1<=12.6885){
        if(FT1==12.4431)tmp1=-15;
        else if(FT1<=12.4702)tmp1=-14.9;
        else if(FT1<=12.4974)tmp1=-14.8;
        else if(FT1<=12.5246)tmp1=-14.7;
        else if(FT1<=12.5518)tmp1=-14.6;
        else if(FT1<=12.5791)tmp1=-14.5;
        else if(FT1<=12.6064)tmp1=-14.4;
        else if(FT1<=12.6337)tmp1=-14.3;
        else if(FT1<=12.6611)tmp1=-14.2;
        else if(FT1<=12.6885)tmp1=-14.1;
        else if(FT1<=12.716)tmp1=-14;
    }
    else if(FT1<=12.9649){
        if(FT1==12.716)tmp1=-14;
        else if(FT1<=12.7435)tmp1=-13.9;
        else if(FT1<=12.771)tmp1=-13.8;
        else if(FT1<=12.7986)tmp1=-13.7;
        else if(FT1<=12.8262)tmp1=-13.6;
        else if(FT1<=12.8539)tmp1=-13.5;
        else if(FT1<=12.8816)tmp1=-13.4;
        else if(FT1<=12.9093)tmp1=-13.3;
        else if(FT1<=12.9371)tmp1=-13.2;
        else if(FT1<=12.9649)tmp1=-13.1;
        else if(FT1<=12.9927)tmp1=-13;
    }
    else if(FT1<=13.2452){
        if(FT1==12.9927)tmp1=-13;
        else if(FT1<=13.0206)tmp1=-12.9;
        else if(FT1<=13.0485)tmp1=-12.8;
        else if(FT1<=13.0765)tmp1=-12.7;
        else if(FT1<=13.1045)tmp1=-12.6;
        else if(FT1<=13.1326)tmp1=-12.5;
        else if(FT1<=13.1607)tmp1=-12.4;
        else if(FT1<=13.1888)tmp1=-12.3;
        else if(FT1<=13.217)tmp1=-12.2;
        else if(FT1<=13.2452)tmp1=-12.1;
        else if(FT1<=13.2734)tmp1=-12;
    }
    else if(FT1<=13.5294){
        if(FT1==13.2734)tmp1=-12;
        else if(FT1<=13.3017)tmp1=-11.9;
        else if(FT1<=13.33)tmp1=-11.8;
        else if(FT1<=13.3584)tmp1=-11.7;
        else if(FT1<=13.3868)tmp1=-11.6;
        else if(FT1<=13.4152)tmp1=-11.5;
        else if(FT1<=13.4437)tmp1=-11.4;
        else if(FT1<=13.4722)tmp1=-11.3;
        else if(FT1<=13.5008)tmp1=-11.2;
        else if(FT1<=13.5294)tmp1=-11.1;
        else if(FT1<=13.5581)tmp1=-11;
    }
    else if(FT1<=13.8176){
        if(FT1==13.5581)tmp1=-11;
        else if(FT1<=13.5867)tmp1=-10.9;
        else if(FT1<=13.6155)tmp1=-10.8;
        else if(FT1<=13.6442)tmp1=-10.7;
        else if(FT1<=13.673)tmp1=-10.6;
        else if(FT1<=13.7019)tmp1=-10.5;
        else if(FT1<=13.7307)tmp1=-10.4;
        else if(FT1<=13.7597)tmp1=-10.3;
        else if(FT1<=13.7886)tmp1=-10.2;
        else if(FT1<=13.8176)tmp1=-10.1;
        else if(FT1<=13.8467)tmp1=-10;
    }
    else if(FT1<=14.1098){
        if(FT1==13.8467)tmp1=-10;
        else if(FT1<=13.8757)tmp1=-9.9;
        else if(FT1<=13.9049)tmp1=-9.8;
        else if(FT1<=13.934)tmp1=-9.7;
        else if(FT1<=13.9632)tmp1=-9.6;
        else if(FT1<=13.9925)tmp1=-9.5;
        else if(FT1<=14.0217)tmp1=-9.4;
        else if(FT1<=14.0511)tmp1=-9.3;
        else if(FT1<=14.0804)tmp1=-9.2;
        else if(FT1<=14.1098)tmp1=-9.1;
        else if(FT1<=14.1393)tmp1=-9;
    }
    else if(FT1<=14.406){
        if(FT1==14.1393)tmp1=-9;
        else if(FT1<=14.1687)tmp1=-8.9;
        else if(FT1<=14.1983)tmp1=-8.8;
        else if(FT1<=14.2278)tmp1=-8.7;
        else if(FT1<=14.2574)tmp1=-8.6;
        else if(FT1<=14.2871)tmp1=-8.5;
        else if(FT1<=14.3167)tmp1=-8.4;
        else if(FT1<=14.3465)tmp1=-8.3;
        else if(FT1<=14.3762)tmp1=-8.2;
        else if(FT1<=14.406)tmp1=-8.1;
        else if(FT1<=14.4359)tmp1=-8;
    }
    else if(FT1<=14.7063){
        if(FT1==14.4359)tmp1=-8;
        else if(FT1<=14.4657)tmp1=-7.9;
        else if(FT1<=14.4957)tmp1=-7.8;
        else if(FT1<=14.5256)tmp1=-7.7;
        else if(FT1<=14.5556)tmp1=-7.6;
        else if(FT1<=14.5857)tmp1=-7.5;
        else if(FT1<=14.6158)tmp1=-7.4;
        else if(FT1<=14.6459)tmp1=-7.3;
        else if(FT1<=14.676)tmp1=-7.2;
        else if(FT1<=14.7063)tmp1=-7.1;
        else if(FT1<=14.7365)tmp1=-7;
    }
    else if(FT1<=15.0105){
        if(FT1==14.7365)tmp1=-7;
        else if(FT1<=14.7668)tmp1=-6.9;
        else if(FT1<=14.7971)tmp1=-6.8;
        else if(FT1<=14.8275)tmp1=-6.7;
        else if(FT1<=14.8579)tmp1=-6.6;
        else if(FT1<=14.8883)tmp1=-6.5;
        else if(FT1<=14.9188)tmp1=-6.4;
        else if(FT1<=14.9493)tmp1=-6.3;
        else if(FT1<=14.9799)tmp1=-6.2;
        else if(FT1<=15.0105)tmp1=-6.1;
        else if(FT1<=15.0412)tmp1=-6;
    }
    else if(FT1<=15.3188){
        if(FT1==15.0412)tmp1=-6;
        else if(FT1<=15.0719)tmp1=-5.9;
        else if(FT1<=15.1026)tmp1=-5.8;
        else if(FT1<=15.1334)tmp1=-5.7;
        else if(FT1<=15.1642)tmp1=-5.6;
        else if(FT1<=15.195)tmp1=-5.5;
        else if(FT1<=15.2259)tmp1=-5.4;
        else if(FT1<=15.2568)tmp1=-5.3;
        else if(FT1<=15.2878)tmp1=-5.2;
        else if(FT1<=15.3188)tmp1=-5.1;
        else if(FT1<=15.3499)tmp1=-5;
    }
    else if(FT1<=15.6312){
        if(FT1==15.3499)tmp1=-5;
        else if(FT1<=15.381)tmp1=-4.9;
        else if(FT1<=15.4121)tmp1=-4.8;
        else if(FT1<=15.4433)tmp1=-4.7;
        else if(FT1<=15.4745)tmp1=-4.6;
        else if(FT1<=15.5058)tmp1=-4.5;
        else if(FT1<=15.5371)tmp1=-4.4;
        else if(FT1<=15.5684)tmp1=-4.3;
        else if(FT1<=15.5998)tmp1=-4.2;
        else if(FT1<=15.6312)tmp1=-4.1;
        else if(FT1<=15.6627)tmp1=-4;
    }
    else if(FT1<=15.9477){
        if(FT1==15.6627)tmp1=-4;
        else if(FT1<=15.6942)tmp1=-3.9;
        else if(FT1<=15.7258)tmp1=-3.8;
        else if(FT1<=15.7573)tmp1=-3.7;
        else if(FT1<=15.789)tmp1=-3.6;
        else if(FT1<=15.8206)tmp1=-3.5;
        else if(FT1<=15.8524)tmp1=-3.4;
        else if(FT1<=15.8841)tmp1=-3.3;
        else if(FT1<=15.9159)tmp1=-3.2;
        else if(FT1<=15.9477)tmp1=-3.1;
    }
    else if(FT1<=16.2683){
        if(FT1==15.9796)tmp1=-3;
        else if(FT1<=16.0115)tmp1=-2.9;
        else if(FT1<=16.0435)tmp1=-2.8;
        else if(FT1<=16.0755)tmp1=-2.7;
        else if(FT1<=16.1075)tmp1=-2.6;
        else if(FT1<=16.1396)tmp1=-2.5;
        else if(FT1<=16.1717)tmp1=-2.4;
        else if(FT1<=16.2039)tmp1=-2.3;
        else if(FT1<=16.2361)tmp1=-2.2;
        else if(FT1<=16.2683)tmp1=-2.1;
    }
    else if(FT1<=16.5931){
        if(FT1==16.3006)tmp1=-2;
        else if(FT1<=16.3329)tmp1=-1.9;
        else if(FT1<=16.3653)tmp1=-1.8;
        else if(FT1<=16.3977)tmp1=-1.7;
        else if(FT1<=16.4302)tmp1=-1.6;
        else if(FT1<=16.4627)tmp1=-1.5;
        else if(FT1<=16.4952)tmp1=-1.4;
        else if(FT1<=16.5278)tmp1=-1.3;
        else if(FT1<=16.5604)tmp1=-1.2;
        else if(FT1<=16.5931)tmp1=-1.1;
    }
    else if(FT1<=16.9219){
        if(FT1==16.6258)tmp1=-1;
        else if(FT1<=16.6585)tmp1=-0.9;
        else if(FT1<=16.6913)tmp1=-0.8;
        else if(FT1<=16.7241)tmp1=-0.7;
        else if(FT1<=16.757)tmp1=-0.6;
        else if(FT1<=16.7899)tmp1=-0.5;
        else if(FT1<=16.8228)tmp1=-0.4;
        else if(FT1<=16.8558)tmp1=-0.3;
        else if(FT1<=16.8888)tmp1=-0.2;
        else if(FT1<=16.9219)tmp1=-0.1;
        else if(FT1<=16.955)tmp1=-1.38778e-16;
    }
    else if(FT1<=17.2549){
        if(FT1==16.955)tmp1=0;
        else if(FT1<=16.9882)tmp1=0.1;
        else if(FT1<=17.0214)tmp1=0.2;
        else if(FT1<=17.0546)tmp1=0.3;
        else if(FT1<=17.0879)tmp1=0.4;
        else if(FT1<=17.1212)tmp1=0.5;
        else if(FT1<=17.1546)tmp1=0.6;
        else if(FT1<=17.188)tmp1=0.7;
        else if(FT1<=17.2215)tmp1=0.8;
        else if(FT1<=17.2549)tmp1=0.9;
        else if(FT1<=17.2885)tmp1=1;
    }
    else if(FT1<=17.5921){
        if(FT1==17.2885)tmp1=1;
        else if(FT1<=17.322)tmp1=1.1;
        else if(FT1<=17.3557)tmp1=1.2;
        else if(FT1<=17.3893)tmp1=1.3;
        else if(FT1<=17.423)tmp1=1.4;
        else if(FT1<=17.4568)tmp1=1.5;
        else if(FT1<=17.4905)tmp1=1.6;
        else if(FT1<=17.5244)tmp1=1.7;
        else if(FT1<=17.5582)tmp1=1.8;
        else if(FT1<=17.5921)tmp1=1.9;
    }
    else if(FT1<=17.9335){
        if(FT1==17.6261)tmp1=2;
        else if(FT1<=17.6601)tmp1=2.1;
        else if(FT1<=17.6941)tmp1=2.2;
        else if(FT1<=17.7282)tmp1=2.3;
        else if(FT1<=17.7623)tmp1=2.4;
        else if(FT1<=17.7965)tmp1=2.5;
        else if(FT1<=17.8307)tmp1=2.6;
        else if(FT1<=17.8649)tmp1=2.7;
        else if(FT1<=17.8992)tmp1=2.8;
        else if(FT1<=17.9335)tmp1=2.9;
    }
    else if(FT1<=18.2791){
        if(FT1==17.9679)tmp1=3;
        else if(FT1<=18.0023)tmp1=3.1;
        else if(FT1<=18.0368)tmp1=3.2;
        else if(FT1<=18.0712)tmp1=3.3;
        else if(FT1<=18.1058)tmp1=3.4;
        else if(FT1<=18.1404)tmp1=3.5;
        else if(FT1<=18.175)tmp1=3.6;
        else if(FT1<=18.2096)tmp1=3.7;
        else if(FT1<=18.2444)tmp1=3.8;
        else if(FT1<=18.2791)tmp1=3.9;
    }
    else if(FT1<=18.6289){
        if(FT1==18.3139)tmp1=4;
        else if(FT1<=18.3487)tmp1=4.1;
        else if(FT1<=18.3836)tmp1=4.2;
        else if(FT1<=18.4185)tmp1=4.3;
        else if(FT1<=18.4535)tmp1=4.4;
        else if(FT1<=18.4885)tmp1=4.5;
        else if(FT1<=18.5235)tmp1=4.6;
        else if(FT1<=18.5586)tmp1=4.7;
        else if(FT1<=18.5937)tmp1=4.8;
        else if(FT1<=18.6289)tmp1=4.9;
        else if(FT1<=18.6641)tmp1=5;
    }
    else if(FT1<=18.9829){
        if(FT1==18.6641)tmp1=5;
        else if(FT1<=18.6994)tmp1=5.1;
        else if(FT1<=18.7347)tmp1=5.2;
        else if(FT1<=18.77)tmp1=5.3;
        else if(FT1<=18.8054)tmp1=5.4;
        else if(FT1<=18.8408)tmp1=5.5;
        else if(FT1<=18.8763)tmp1=5.6;
        else if(FT1<=18.9118)tmp1=5.7;
        else if(FT1<=18.9473)tmp1=5.8;
        else if(FT1<=18.9829)tmp1=5.9;
        else if(FT1<=19.0186)tmp1=6;
    }
    else if(FT1<=19.3412){
        if(FT1==19.0186)tmp1=6;
        else if(FT1<=19.0543)tmp1=6.1;
        else if(FT1<=19.09)tmp1=6.2;
        else if(FT1<=19.1257)tmp1=6.3;
        else if(FT1<=19.1615)tmp1=6.4;
        else if(FT1<=19.1974)tmp1=6.5;
        else if(FT1<=19.2333)tmp1=6.6;
        else if(FT1<=19.2692)tmp1=6.7;
        else if(FT1<=19.3052)tmp1=6.8;
        else if(FT1<=19.3412)tmp1=6.9;
        else if(FT1<=19.3773)tmp1=7;
    }
    else if(FT1<=19.7038){
        if(FT1==19.3773)tmp1=7;
        else if(FT1<=19.4134)tmp1=7.1;
        else if(FT1<=19.4495)tmp1=7.2;
        else if(FT1<=19.4857)tmp1=7.3;
        else if(FT1<=19.522)tmp1=7.4;
        else if(FT1<=19.5582)tmp1=7.5;
        else if(FT1<=19.5945)tmp1=7.6;
        else if(FT1<=19.6309)tmp1=7.7;
        else if(FT1<=19.6673)tmp1=7.8;
        else if(FT1<=19.7038)tmp1=7.9;
        else if(FT1<=19.7402)tmp1=8;
    }
    else if(FT1<=20.0706){
        if(FT1==19.7402)tmp1=8;
        else if(FT1<=19.7768)tmp1=8.1;
        else if(FT1<=19.8133)tmp1=8.2;
        else if(FT1<=19.85)tmp1=8.3;
        else if(FT1<=19.8866)tmp1=8.4;
        else if(FT1<=19.9233)tmp1=8.5;
        else if(FT1<=19.9601)tmp1=8.6;
        else if(FT1<=19.9969)tmp1=8.7;
        else if(FT1<=20.0337)tmp1=8.8;
        else if(FT1<=20.0706)tmp1=8.9;
        else if(FT1<=20.1075)tmp1=9;
    }
    else if(FT1<=20.4417){
        if(FT1==20.1075)tmp1=9;
        else if(FT1<=20.1444)tmp1=9.1;
        else if(FT1<=20.1814)tmp1=9.2;
        else if(FT1<=20.2185)tmp1=9.3;
        else if(FT1<=20.2556)tmp1=9.4;
        else if(FT1<=20.2927)tmp1=9.5;
        else if(FT1<=20.3299)tmp1=9.6;
        else if(FT1<=20.3671)tmp1=9.7;
        else if(FT1<=20.4044)tmp1=9.8;
        else if(FT1<=20.4417)tmp1=9.9;
        else if(FT1<=20.479)tmp1=10;
    }
    else if(FT1<=20.817){
        if(FT1==20.479)tmp1=10;
        else if(FT1<=20.5164)tmp1=10.1;
        else if(FT1<=20.5538)tmp1=10.2;
        else if(FT1<=20.5913)tmp1=10.3;
        else if(FT1<=20.6288)tmp1=10.4;
        else if(FT1<=20.6664)tmp1=10.5;
        else if(FT1<=20.704)tmp1=10.6;
        else if(FT1<=20.7416)tmp1=10.7;
        else if(FT1<=20.7793)tmp1=10.8;
        else if(FT1<=20.817)tmp1=10.9;
        else if(FT1<=20.8548)tmp1=11;
    }
    else if(FT1<=21.1967){
        if(FT1==20.8548)tmp1=11;
        else if(FT1<=20.8926)tmp1=11.1;
        else if(FT1<=20.9305)tmp1=11.2;
        else if(FT1<=20.9684)tmp1=11.3;
        else if(FT1<=21.0064)tmp1=11.4;
        else if(FT1<=21.0444)tmp1=11.5;
        else if(FT1<=21.0824)tmp1=11.6;
        else if(FT1<=21.1205)tmp1=11.7;
        else if(FT1<=21.1586)tmp1=11.8;
        else if(FT1<=21.1967)tmp1=11.9;
        else if(FT1<=21.235)tmp1=12;
    }
    else if(FT1<=21.5808){
        if(FT1==21.235)tmp1=12;
        else if(FT1<=21.2732)tmp1=12.1;
        else if(FT1<=21.3115)tmp1=12.2;
        else if(FT1<=21.3498)tmp1=12.3;
        else if(FT1<=21.3882)tmp1=12.4;
        else if(FT1<=21.4266)tmp1=12.5;
        else if(FT1<=21.4651)tmp1=12.6;
        else if(FT1<=21.5036)tmp1=12.7;
        else if(FT1<=21.5422)tmp1=12.8;
        else if(FT1<=21.5808)tmp1=12.9;
        else if(FT1<=21.6194)tmp1=13;
    }
    else if(FT1<=21.9691){
        if(FT1==21.6194)tmp1=13;
        else if(FT1<=21.6581)tmp1=13.1;
        else if(FT1<=21.6968)tmp1=13.2;
        else if(FT1<=21.7356)tmp1=13.3;
        else if(FT1<=21.7744)tmp1=13.4;
        else if(FT1<=21.8133)tmp1=13.5;
        else if(FT1<=21.8522)tmp1=13.6;
        else if(FT1<=21.8911)tmp1=13.7;
        else if(FT1<=21.9301)tmp1=13.8;
        else if(FT1<=21.9691)tmp1=13.9;
        else if(FT1<=22.0082)tmp1=14;
    }
    else if(FT1<=22.3618){
        if(FT1==22.0082)tmp1=14;
        else if(FT1<=22.0473)tmp1=14.1;
        else if(FT1<=22.0865)tmp1=14.2;
        else if(FT1<=22.1257)tmp1=14.3;
        else if(FT1<=22.1649)tmp1=14.4;
        else if(FT1<=22.2042)tmp1=14.5;
        else if(FT1<=22.2436)tmp1=14.6;
        else if(FT1<=22.283)tmp1=14.7;
        else if(FT1<=22.3224)tmp1=14.8;
        else if(FT1<=22.3618)tmp1=14.9;
        else if(FT1<=22.4014)tmp1=15;
    }
    else if(FT1<=22.7589){
        if(FT1==22.4014)tmp1=15;
        else if(FT1<=22.4409)tmp1=15.1;
        else if(FT1<=22.4805)tmp1=15.2;
        else if(FT1<=22.5201)tmp1=15.3;
        else if(FT1<=22.5598)tmp1=15.4;
        else if(FT1<=22.5996)tmp1=15.5;
        else if(FT1<=22.6393)tmp1=15.6;
        else if(FT1<=22.6791)tmp1=15.7;
        else if(FT1<=22.719)tmp1=15.8;
        else if(FT1<=22.7589)tmp1=15.9;
        else if(FT1<=22.7989)tmp1=16;
    }
    else if(FT1<=23.1603){
        if(FT1==22.7989)tmp1=16;
        else if(FT1<=22.8388)tmp1=16.1;
        else if(FT1<=22.8789)tmp1=16.2;
        else if(FT1<=22.919)tmp1=16.3;
        else if(FT1<=22.9591)tmp1=16.4;
        else if(FT1<=22.9992)tmp1=16.5;
        else if(FT1<=23.0394)tmp1=16.6;
        else if(FT1<=23.0797)tmp1=16.7;
        else if(FT1<=23.12)tmp1=16.8;
        else if(FT1<=23.1603)tmp1=16.9;
    }
    else if(FT1<=23.5662){
        if(FT1==23.2007)tmp1=17;
        else if(FT1<=23.2411)tmp1=17.1;
        else if(FT1<=23.2816)tmp1=17.2;
        else if(FT1<=23.3221)tmp1=17.3;
        else if(FT1<=23.3627)tmp1=17.4;
        else if(FT1<=23.4033)tmp1=17.5;
        else if(FT1<=23.4439)tmp1=17.6;
        else if(FT1<=23.4846)tmp1=17.7;
        else if(FT1<=23.5254)tmp1=17.8;
        else if(FT1<=23.5662)tmp1=17.9;
    }
    else if(FT1<=23.9764){
        if(FT1==23.607)tmp1=18;
        else if(FT1<=23.6478)tmp1=18.1;
        else if(FT1<=23.6888)tmp1=18.2;
        else if(FT1<=23.7297)tmp1=18.3;
        else if(FT1<=23.7707)tmp1=18.4;
        else if(FT1<=23.8117)tmp1=18.5;
        else if(FT1<=23.8528)tmp1=18.6;
        else if(FT1<=23.894)tmp1=18.7;
        else if(FT1<=23.9351)tmp1=18.8;
        else if(FT1<=23.9764)tmp1=18.9;
    }
    else if(FT1<=24.391){
        if(FT1==24.0176)tmp1=19;
        else if(FT1<=24.0589)tmp1=19.1;
        else if(FT1<=24.1003)tmp1=19.2;
        else if(FT1<=24.1417)tmp1=19.3;
        else if(FT1<=24.1831)tmp1=19.4;
        else if(FT1<=24.2246)tmp1=19.5;
        else if(FT1<=24.2661)tmp1=19.6;
        else if(FT1<=24.3077)tmp1=19.7;
        else if(FT1<=24.3493)tmp1=19.8;
        else if(FT1<=24.391)tmp1=19.9;
    }
    else if(FT1<=24.81){
        if(FT1==24.4327)tmp1=20;
        else if(FT1<=24.4744)tmp1=20.1;
        else if(FT1<=24.5162)tmp1=20.2;
        else if(FT1<=24.558)tmp1=20.3;
        else if(FT1<=24.5999)tmp1=20.4;
        else if(FT1<=24.6418)tmp1=20.5;
        else if(FT1<=24.6838)tmp1=20.6;
        else if(FT1<=24.7258)tmp1=20.7;
        else if(FT1<=24.7679)tmp1=20.8;
        else if(FT1<=24.81)tmp1=20.9;
    }
    else if(FT1<=25.2334){
        if(FT1==24.8521)tmp1=21;
        else if(FT1<=24.8943)tmp1=21.1;
        else if(FT1<=24.9365)tmp1=21.2;
        else if(FT1<=24.9788)tmp1=21.3;
        else if(FT1<=25.0211)tmp1=21.4;
        else if(FT1<=25.0635)tmp1=21.5;
        else if(FT1<=25.1059)tmp1=21.6;
        else if(FT1<=25.1484)tmp1=21.7;
        else if(FT1<=25.1909)tmp1=21.8;
        else if(FT1<=25.2334)tmp1=21.9;
    }
    else if(FT1<=25.6613){
        if(FT1==25.276)tmp1=22;
        else if(FT1<=25.3186)tmp1=22.1;
        else if(FT1<=25.3613)tmp1=22.2;
        else if(FT1<=25.404)tmp1=22.3;
        else if(FT1<=25.4468)tmp1=22.4;
        else if(FT1<=25.4896)tmp1=22.5;
        else if(FT1<=25.5324)tmp1=22.6;
        else if(FT1<=25.5753)tmp1=22.7;
        else if(FT1<=25.6183)tmp1=22.8;
        else if(FT1<=25.6613)tmp1=22.9;
    }
    else if(FT1<=26.0936){
        if(FT1==25.7043)tmp1=23;
        else if(FT1<=25.7474)tmp1=23.1;
        else if(FT1<=25.7905)tmp1=23.2;
        else if(FT1<=25.8337)tmp1=23.3;
        else if(FT1<=25.8769)tmp1=23.4;
        else if(FT1<=25.9201)tmp1=23.5;
        else if(FT1<=25.9634)tmp1=23.6;
        else if(FT1<=26.0068)tmp1=23.7;
        else if(FT1<=26.0501)tmp1=23.8;
        else if(FT1<=26.0936)tmp1=23.9;
    }
    else if(FT1<=26.5303){
        if(FT1==26.137)tmp1=24;
        else if(FT1<=26.1806)tmp1=24.1;
        else if(FT1<=26.2241)tmp1=24.2;
        else if(FT1<=26.2677)tmp1=24.3;
        else if(FT1<=26.3114)tmp1=24.4;
        else if(FT1<=26.3551)tmp1=24.5;
        else if(FT1<=26.3988)tmp1=24.6;
        else if(FT1<=26.4426)tmp1=24.7;
        else if(FT1<=26.4865)tmp1=24.8;
        else if(FT1<=26.5303)tmp1=24.9;
    }
    else if(FT1<=26.9715){
        if(FT1==26.5742)tmp1=25;
        else if(FT1<=26.6182)tmp1=25.1;
        else if(FT1<=26.6622)tmp1=25.2;
        else if(FT1<=26.7063)tmp1=25.3;
        else if(FT1<=26.7504)tmp1=25.4;
        else if(FT1<=26.7945)tmp1=25.5;
        else if(FT1<=26.8387)tmp1=25.6;
        else if(FT1<=26.8829)tmp1=25.7;
        else if(FT1<=26.9272)tmp1=25.8;
        else if(FT1<=26.9715)tmp1=25.9;
    }
    else if(FT1<=27.4172){
        if(FT1==27.0159)tmp1=26;
        else if(FT1<=27.0603)tmp1=26.1;
        else if(FT1<=27.1048)tmp1=26.2;
        else if(FT1<=27.1493)tmp1=26.3;
        else if(FT1<=27.1938)tmp1=26.4;
        else if(FT1<=27.2384)tmp1=26.5;
        else if(FT1<=27.283)tmp1=26.6;
        else if(FT1<=27.3277)tmp1=26.7;
        else if(FT1<=27.3724)tmp1=26.8;
        else if(FT1<=27.4172)tmp1=26.9;
    }
    else if(FT1<=27.8674){
        if(FT1==27.462)tmp1=27;
        else if(FT1<=27.5069)tmp1=27.1;
        else if(FT1<=27.5518)tmp1=27.2;
        else if(FT1<=27.5967)tmp1=27.3;
        else if(FT1<=27.6417)tmp1=27.4;
        else if(FT1<=27.6868)tmp1=27.5;
        else if(FT1<=27.7318)tmp1=27.6;
        else if(FT1<=27.777)tmp1=27.7;
        else if(FT1<=27.8221)tmp1=27.8;
        else if(FT1<=27.8674)tmp1=27.9;
    }
    else if(FT1<=28.322){
        if(FT1==27.9126)tmp1=28;
        else if(FT1<=27.9579)tmp1=28.1;
        else if(FT1<=28.0033)tmp1=28.2;
        else if(FT1<=28.0487)tmp1=28.3;
        else if(FT1<=28.0941)tmp1=28.4;
        else if(FT1<=28.1396)tmp1=28.5;
        else if(FT1<=28.1851)tmp1=28.6;
        else if(FT1<=28.2307)tmp1=28.7;
        else if(FT1<=28.2763)tmp1=28.8;
        else if(FT1<=28.322)tmp1=28.9;
    }
    else if(FT1<=28.7811){
        if(FT1==28.3677)tmp1=29;
        else if(FT1<=28.4134)tmp1=29.1;
        else if(FT1<=28.4592)tmp1=29.2;
        else if(FT1<=28.5051)tmp1=29.3;
        else if(FT1<=28.551)tmp1=29.4;
        else if(FT1<=28.5969)tmp1=29.5;
        else if(FT1<=28.6429)tmp1=29.6;
        else if(FT1<=28.6889)tmp1=29.7;
        else if(FT1<=28.735)tmp1=29.8;
        else if(FT1<=28.7811)tmp1=29.9;
    }
    else if(FT1<=29.2447){
        if(FT1==28.8273)tmp1=30;
        else if(FT1<=28.8735)tmp1=30.1;
        else if(FT1<=28.9197)tmp1=30.2;
        else if(FT1<=28.966)tmp1=30.3;
        else if(FT1<=29.0123)tmp1=30.4;
        else if(FT1<=29.0587)tmp1=30.5;
        else if(FT1<=29.1052)tmp1=30.6;
        else if(FT1<=29.1516)tmp1=30.7;
        else if(FT1<=29.1981)tmp1=30.8;
        else if(FT1<=29.2447)tmp1=30.9;
    }
    else if(FT1<=29.7128){
        if(FT1==29.2913)tmp1=31;
        else if(FT1<=29.338)tmp1=31.1;
        else if(FT1<=29.3847)tmp1=31.2;
        else if(FT1<=29.4314)tmp1=31.3;
        else if(FT1<=29.4782)tmp1=31.4;
        else if(FT1<=29.525)tmp1=31.5;
        else if(FT1<=29.5719)tmp1=31.6;
        else if(FT1<=29.6188)tmp1=31.7;
        else if(FT1<=29.6658)tmp1=31.8;
        else if(FT1<=29.7128)tmp1=31.9;
    }
    else if(FT1<=30.1855){
        if(FT1==29.7599)tmp1=32;
        else if(FT1<=29.807)tmp1=32.1;
        else if(FT1<=29.8541)tmp1=32.2;
        else if(FT1<=29.9013)tmp1=32.3;
        else if(FT1<=29.9486)tmp1=32.4;
        else if(FT1<=29.9959)tmp1=32.5;
        else if(FT1<=30.0432)tmp1=32.6;
        else if(FT1<=30.0906)tmp1=32.7;
        else if(FT1<=30.138)tmp1=32.8;
        else if(FT1<=30.1855)tmp1=32.9;
    }
    else if(FT1<=30.6626){
        if(FT1==30.233)tmp1=33;
        else if(FT1<=30.2805)tmp1=33.1;
        else if(FT1<=30.3281)tmp1=33.2;
        else if(FT1<=30.3758)tmp1=33.3;
        else if(FT1<=30.4235)tmp1=33.4;
        else if(FT1<=30.4712)tmp1=33.5;
        else if(FT1<=30.519)tmp1=33.6;
        else if(FT1<=30.5668)tmp1=33.7;
        else if(FT1<=30.6147)tmp1=33.8;
        else if(FT1<=30.6626)tmp1=33.9;
    }
    else if(FT1<=31.1443){
        if(FT1==30.7106)tmp1=34;
        else if(FT1<=30.7586)tmp1=34.1;
        else if(FT1<=30.8066)tmp1=34.2;
        else if(FT1<=30.8547)tmp1=34.3;
        else if(FT1<=30.9029)tmp1=34.4;
        else if(FT1<=30.9511)tmp1=34.5;
        else if(FT1<=30.9993)tmp1=34.6;
        else if(FT1<=31.0476)tmp1=34.7;
        else if(FT1<=31.0959)tmp1=34.8;
        else if(FT1<=31.1443)tmp1=34.9;
    }
    else if(FT1<=31.6305){
        if(FT1==31.1927)tmp1=35;
        else if(FT1<=31.2411)tmp1=35.1;
        else if(FT1<=31.2897)tmp1=35.2;
        else if(FT1<=31.3382)tmp1=35.3;
        else if(FT1<=31.3868)tmp1=35.4;
        else if(FT1<=31.4354)tmp1=35.5;
        else if(FT1<=31.4841)tmp1=35.6;
        else if(FT1<=31.5329)tmp1=35.7;
        else if(FT1<=31.5816)tmp1=35.8;
        else if(FT1<=31.6305)tmp1=35.9;
    }
    else if(FT1<=32.1212){
        if(FT1==31.6793)tmp1=36;
        else if(FT1<=31.7283)tmp1=36.1;
        else if(FT1<=31.7772)tmp1=36.2;
        else if(FT1<=31.8262)tmp1=36.3;
        else if(FT1<=31.8753)tmp1=36.4;
        else if(FT1<=31.9244)tmp1=36.5;
        else if(FT1<=31.9735)tmp1=36.6;
        else if(FT1<=32.0227)tmp1=36.7;
        else if(FT1<=32.0719)tmp1=36.8;
        else if(FT1<=32.1212)tmp1=36.9;
    }
    else if(FT1<=32.6165){
        if(FT1==32.1705)tmp1=37;
        else if(FT1<=32.2199)tmp1=37.1;
        else if(FT1<=32.2693)tmp1=37.2;
        else if(FT1<=32.3188)tmp1=37.3;
        else if(FT1<=32.3683)tmp1=37.4;
        else if(FT1<=32.4178)tmp1=37.5;
        else if(FT1<=32.4674)tmp1=37.6;
        else if(FT1<=32.5171)tmp1=37.7;
        else if(FT1<=32.5667)tmp1=37.8;
        else if(FT1<=32.6165)tmp1=37.9;
    }
    else if(FT1<=33.1163){
        if(FT1==32.6663)tmp1=38;
        else if(FT1<=32.7161)tmp1=38.1;
        else if(FT1<=32.7659)tmp1=38.2;
        else if(FT1<=32.8159)tmp1=38.3;
        else if(FT1<=32.8658)tmp1=38.4;
        else if(FT1<=32.9158)tmp1=38.5;
        else if(FT1<=32.9659)tmp1=38.6;
        else if(FT1<=33.016)tmp1=38.7;
        else if(FT1<=33.0661)tmp1=38.8;
        else if(FT1<=33.1163)tmp1=38.9;
    }
    else if(FT1<=33.6207){
        if(FT1==33.1665)tmp1=39;
        else if(FT1<=33.2168)tmp1=39.1;
        else if(FT1<=33.2671)tmp1=39.2;
        else if(FT1<=33.3175)tmp1=39.3;
        else if(FT1<=33.3679)tmp1=39.4;
        else if(FT1<=33.4184)tmp1=39.5;
        else if(FT1<=33.4689)tmp1=39.6;
        else if(FT1<=33.5194)tmp1=39.7;
        else if(FT1<=33.57)tmp1=39.8;
        else if(FT1<=33.6207)tmp1=39.9;
    }
    else if(FT1<=34.1296){
        if(FT1==33.6714)tmp1=40;
        else if(FT1<=33.7221)tmp1=40.1;
        else if(FT1<=33.7729)tmp1=40.2;
        else if(FT1<=33.8237)tmp1=40.3;
        else if(FT1<=33.8746)tmp1=40.4;
        else if(FT1<=33.9255)tmp1=40.5;
        else if(FT1<=33.9764)tmp1=40.6;
        else if(FT1<=34.0275)tmp1=40.7;
        else if(FT1<=34.0785)tmp1=40.8;
        else if(FT1<=34.1296)tmp1=40.9;
    }
    else if(FT1<=34.6431){
        if(FT1==34.1807)tmp1=41;
        else if(FT1<=34.2319)tmp1=41.1;
        else if(FT1<=34.2832)tmp1=41.2;
        else if(FT1<=34.3345)tmp1=41.3;
        else if(FT1<=34.3858)tmp1=41.4;
        else if(FT1<=34.4372)tmp1=41.5;
        else if(FT1<=34.4886)tmp1=41.6;
        else if(FT1<=34.54)tmp1=41.7;
        else if(FT1<=34.5915)tmp1=41.8;
        else if(FT1<=34.6431)tmp1=41.9;
    }
    else if(FT1<=35.1612){
        if(FT1==34.6947)tmp1=42;
        else if(FT1<=34.7463)tmp1=42.1;
        else if(FT1<=34.798)tmp1=42.2;
        else if(FT1<=34.8498)tmp1=42.3;
        else if(FT1<=34.9016)tmp1=42.4;
        else if(FT1<=34.9534)tmp1=42.5;
        else if(FT1<=35.0053)tmp1=42.6;
        else if(FT1<=35.0572)tmp1=42.7;
        else if(FT1<=35.1091)tmp1=42.8;
        else if(FT1<=35.1612)tmp1=42.9;
    }
    else if(FT1<=35.6838){
        if(FT1==35.2132)tmp1=43;
        else if(FT1<=35.2653)tmp1=43.1;
        else if(FT1<=35.3175)tmp1=43.2;
        else if(FT1<=35.3697)tmp1=43.3;
        else if(FT1<=35.4219)tmp1=43.4;
        else if(FT1<=35.4742)tmp1=43.5;
        else if(FT1<=35.5265)tmp1=43.6;
        else if(FT1<=35.5789)tmp1=43.7;
        else if(FT1<=35.6313)tmp1=43.8;
        else if(FT1<=35.6838)tmp1=43.9;
    }
    else if(FT1<=36.211){
        if(FT1==35.7363)tmp1=44;
        else if(FT1<=35.7889)tmp1=44.1;
        else if(FT1<=35.8415)tmp1=44.2;
        else if(FT1<=35.8941)tmp1=44.3;
        else if(FT1<=35.9468)tmp1=44.4;
        else if(FT1<=35.9996)tmp1=44.5;
        else if(FT1<=36.0523)tmp1=44.6;
        else if(FT1<=36.1052)tmp1=44.7;
        else if(FT1<=36.1581)tmp1=44.8;
        else if(FT1<=36.211)tmp1=44.9;
    }
    else if(FT1<=36.7428){
        if(FT1==36.264)tmp1=45;
        else if(FT1<=36.317)tmp1=45.1;
        else if(FT1<=36.37)tmp1=45.2;
        else if(FT1<=36.4231)tmp1=45.3;
        else if(FT1<=36.4763)tmp1=45.4;
        else if(FT1<=36.5295)tmp1=45.5;
        else if(FT1<=36.5828)tmp1=45.6;
        else if(FT1<=36.6361)tmp1=45.7;
        else if(FT1<=36.6894)tmp1=45.8;
        else if(FT1<=36.7428)tmp1=45.9;
    }
    else if(FT1<=37.2791){
        if(FT1==36.7962)tmp1=46;
        else if(FT1<=36.8497)tmp1=46.1;
        else if(FT1<=36.9032)tmp1=46.2;
        else if(FT1<=36.9568)tmp1=46.3;
        else if(FT1<=37.0104)tmp1=46.4;
        else if(FT1<=37.064)tmp1=46.5;
        else if(FT1<=37.1178)tmp1=46.6;
        else if(FT1<=37.1715)tmp1=46.7;
        else if(FT1<=37.2253)tmp1=46.8;
        else if(FT1<=37.2791)tmp1=46.9;
    }
    else if(FT1<=37.8201){
        if(FT1==37.333)tmp1=47;
        else if(FT1<=37.387)tmp1=47.1;
        else if(FT1<=37.4409)tmp1=47.2;
        else if(FT1<=37.495)tmp1=47.3;
        else if(FT1<=37.549)tmp1=47.4;
        else if(FT1<=37.6032)tmp1=47.5;
        else if(FT1<=37.6573)tmp1=47.6;
        else if(FT1<=37.7115)tmp1=47.7;
        else if(FT1<=37.7658)tmp1=47.8;
        else if(FT1<=37.8201)tmp1=47.9;
    }
    else if(FT1<=38.3656){
        if(FT1==37.8744)tmp1=48;
        else if(FT1<=37.9288)tmp1=48.1;
        else if(FT1<=37.9833)tmp1=48.2;
        else if(FT1<=38.0378)tmp1=48.3;
        else if(FT1<=38.0923)tmp1=48.4;
        else if(FT1<=38.1469)tmp1=48.5;
        else if(FT1<=38.2015)tmp1=48.6;
        else if(FT1<=38.2562)tmp1=48.7;
        else if(FT1<=38.3109)tmp1=48.8;
        else if(FT1<=38.3656)tmp1=48.9;
    }
    else if(FT1<=38.9158){
        if(FT1==38.4204)tmp1=49;
        else if(FT1<=38.4753)tmp1=49.1;
        else if(FT1<=38.5302)tmp1=49.2;
        else if(FT1<=38.5851)tmp1=49.3;
        else if(FT1<=38.6401)tmp1=49.4;
        else if(FT1<=38.6952)tmp1=49.5;
        else if(FT1<=38.7502)tmp1=49.6;
        else if(FT1<=38.8054)tmp1=49.7;
        else if(FT1<=38.8605)tmp1=49.8;
        else if(FT1<=38.9158)tmp1=49.9;
    }
    else if(FT1<=39.4705){
        if(FT1==38.971)tmp1=50;
        else if(FT1<=39.0263)tmp1=50.1;
        else if(FT1<=39.0817)tmp1=50.2;
        else if(FT1<=39.1371)tmp1=50.3;
        else if(FT1<=39.1926)tmp1=50.4;
        else if(FT1<=39.2481)tmp1=50.5;
        else if(FT1<=39.3036)tmp1=50.6;
        else if(FT1<=39.3592)tmp1=50.7;
        else if(FT1<=39.4148)tmp1=50.8;
        else if(FT1<=39.4705)tmp1=50.9;
    }
    else if(FT1<=40.0298){
        if(FT1==39.5262)tmp1=51;
        else if(FT1<=39.582)tmp1=51.1;
        else if(FT1<=39.6378)tmp1=51.2;
        else if(FT1<=39.6937)tmp1=51.3;
        else if(FT1<=39.7496)tmp1=51.4;
        else if(FT1<=39.8055)tmp1=51.5;
        else if(FT1<=39.8615)tmp1=51.6;
        else if(FT1<=39.9176)tmp1=51.7;
        else if(FT1<=39.9737)tmp1=51.8;
        else if(FT1<=40.0298)tmp1=51.9;
    }
    else if(FT1<=40.5937){
        if(FT1==40.086)tmp1=52;
        else if(FT1<=40.1422)tmp1=52.1;
        else if(FT1<=40.1985)tmp1=52.2;
        else if(FT1<=40.2548)tmp1=52.3;
        else if(FT1<=40.3112)tmp1=52.4;
        else if(FT1<=40.3676)tmp1=52.5;
        else if(FT1<=40.4241)tmp1=52.6;
        else if(FT1<=40.4806)tmp1=52.7;
        else if(FT1<=40.5371)tmp1=52.8;
        else if(FT1<=40.5937)tmp1=52.9;
    }
    else if(FT1<=41.1623){
        if(FT1==40.6504)tmp1=53;
        else if(FT1<=40.7071)tmp1=53.1;
        else if(FT1<=40.7638)tmp1=53.2;
        else if(FT1<=40.8206)tmp1=53.3;
        else if(FT1<=40.8774)tmp1=53.4;
        else if(FT1<=40.9343)tmp1=53.5;
        else if(FT1<=40.9912)tmp1=53.6;
        else if(FT1<=41.0482)tmp1=53.7;
        else if(FT1<=41.1052)tmp1=53.8;
        else if(FT1<=41.1623)tmp1=53.9;
    }
    else if(FT1<=41.7354){
        if(FT1==41.2194)tmp1=54;
        else if(FT1<=41.2765)tmp1=54.1;
        else if(FT1<=41.3337)tmp1=54.2;
        else if(FT1<=41.391)tmp1=54.3;
        else if(FT1<=41.4483)tmp1=54.4;
        else if(FT1<=41.5056)tmp1=54.5;
        else if(FT1<=41.563)tmp1=54.6;
        else if(FT1<=41.6204)tmp1=54.7;
        else if(FT1<=41.6779)tmp1=54.8;
        else if(FT1<=41.7354)tmp1=54.9;
    }
    else if(FT1<=42.3131){
        if(FT1==41.793)tmp1=55;
        else if(FT1<=41.8506)tmp1=55.1;
        else if(FT1<=41.9082)tmp1=55.2;
        else if(FT1<=41.9659)tmp1=55.3;
        else if(FT1<=42.0237)tmp1=55.4;
        else if(FT1<=42.0815)tmp1=55.5;
        else if(FT1<=42.1393)tmp1=55.6;
        else if(FT1<=42.1972)tmp1=55.7;
        else if(FT1<=42.2552)tmp1=55.8;
        else if(FT1<=42.3131)tmp1=55.9;
    }
    else if(FT1<=42.8955){
        if(FT1==42.3712)tmp1=56;
        else if(FT1<=42.4292)tmp1=56.1;
        else if(FT1<=42.4874)tmp1=56.2;
        else if(FT1<=42.5455)tmp1=56.3;
        else if(FT1<=42.6037)tmp1=56.4;
        else if(FT1<=42.662)tmp1=56.5;
        else if(FT1<=42.7203)tmp1=56.6;
        else if(FT1<=42.7787)tmp1=56.7;
        else if(FT1<=42.837)tmp1=56.8;
        else if(FT1<=42.8955)tmp1=56.9;
    }
    else if(FT1<=43.4824){
        if(FT1==42.954)tmp1=57;
        else if(FT1<=43.0125)tmp1=57.1;
        else if(FT1<=43.0711)tmp1=57.2;
        else if(FT1<=43.1297)tmp1=57.3;
        else if(FT1<=43.1884)tmp1=57.4;
        else if(FT1<=43.2471)tmp1=57.5;
        else if(FT1<=43.3059)tmp1=57.6;
        else if(FT1<=43.3647)tmp1=57.7;
        else if(FT1<=43.4235)tmp1=57.8;
        else if(FT1<=43.4824)tmp1=57.9;
    }
    else if(FT1<=44.074){
        if(FT1==43.5414)tmp1=58;
        else if(FT1<=43.6004)tmp1=58.1;
        else if(FT1<=43.6594)tmp1=58.2;
        else if(FT1<=43.7185)tmp1=58.3;
        else if(FT1<=43.7777)tmp1=58.4;
        else if(FT1<=43.8368)tmp1=58.5;
        else if(FT1<=43.8961)tmp1=58.6;
        else if(FT1<=43.9553)tmp1=58.7;
        else if(FT1<=44.0146)tmp1=58.8;
        else if(FT1<=44.074)tmp1=58.9;
    }
    else if(FT1<=44.6702){
        if(FT1==44.1334)tmp1=59;
        else if(FT1<=44.1929)tmp1=59.1;
        else if(FT1<=44.2524)tmp1=59.2;
        else if(FT1<=44.3119)tmp1=59.3;
        else if(FT1<=44.3715)tmp1=59.4;
        else if(FT1<=44.4312)tmp1=59.5;
        else if(FT1<=44.4908)tmp1=59.6;
        else if(FT1<=44.5506)tmp1=59.7;
        else if(FT1<=44.6104)tmp1=59.8;
        else if(FT1<=44.6702)tmp1=59.9;
    }
    else if(FT1<=45.271){
        if(FT1==44.7301)tmp1=60;
        else if(FT1<=44.79)tmp1=60.1;
        else if(FT1<=44.8499)tmp1=60.2;
        else if(FT1<=44.9099)tmp1=60.3;
        else if(FT1<=44.97)tmp1=60.4;
        else if(FT1<=45.0301)tmp1=60.5;
        else if(FT1<=45.0903)tmp1=60.6;
        else if(FT1<=45.1504)tmp1=60.7;
        else if(FT1<=45.2107)tmp1=60.8;
        else if(FT1<=45.271)tmp1=60.9;
    }
    else if(FT1<=45.8764){
        if(FT1==45.3313)tmp1=61;
        else if(FT1<=45.3917)tmp1=61.1;
        else if(FT1<=45.4521)tmp1=61.2;
        else if(FT1<=45.5126)tmp1=61.3;
        else if(FT1<=45.5731)tmp1=61.4;
        else if(FT1<=45.6337)tmp1=61.5;
        else if(FT1<=45.6943)tmp1=61.6;
        else if(FT1<=45.7549)tmp1=61.7;
        else if(FT1<=45.8156)tmp1=61.8;
        else if(FT1<=45.8764)tmp1=61.9;
    }
    else if(FT1<=46.4864){
        if(FT1==45.9372)tmp1=62;
        else if(FT1<=45.998)tmp1=62.1;
        else if(FT1<=46.0589)tmp1=62.2;
        else if(FT1<=46.1198)tmp1=62.3;
        else if(FT1<=46.1808)tmp1=62.4;
        else if(FT1<=46.2418)tmp1=62.5;
        else if(FT1<=46.3029)tmp1=62.6;
        else if(FT1<=46.364)tmp1=62.7;
        else if(FT1<=46.4252)tmp1=62.8;
        else if(FT1<=46.4864)tmp1=62.9;
    }
    else if(FT1<=47.101){
        if(FT1==46.5476)tmp1=63;
        else if(FT1<=46.6089)tmp1=63.1;
        else if(FT1<=46.6703)tmp1=63.2;
        else if(FT1<=46.7317)tmp1=63.3;
        else if(FT1<=46.7931)tmp1=63.4;
        else if(FT1<=46.8546)tmp1=63.5;
        else if(FT1<=46.9161)tmp1=63.6;
        else if(FT1<=46.9777)tmp1=63.7;
        else if(FT1<=47.0393)tmp1=63.8;
        else if(FT1<=47.101)tmp1=63.9;
    }
    else if(FT1<=47.7203){
        if(FT1==47.1627)tmp1=64;
        else if(FT1<=47.2245)tmp1=64.1;
        else if(FT1<=47.2863)tmp1=64.2;
        else if(FT1<=47.3482)tmp1=64.3;
        else if(FT1<=47.4101)tmp1=64.4;
        else if(FT1<=47.472)tmp1=64.5;
        else if(FT1<=47.534)tmp1=64.6;
        else if(FT1<=47.596)tmp1=64.7;
        else if(FT1<=47.6581)tmp1=64.8;
        else if(FT1<=47.7203)tmp1=64.9;
        else if(FT1<=47.7824)tmp1=65;
    }
    else if(FT1<=48.3441){
        if(FT1==47.7824)tmp1=65;
        else if(FT1<=47.8447)tmp1=65.1;
        else if(FT1<=47.9069)tmp1=65.2;
        else if(FT1<=47.9692)tmp1=65.3;
        else if(FT1<=48.0316)tmp1=65.4;
        else if(FT1<=48.094)tmp1=65.5;
        else if(FT1<=48.1565)tmp1=65.6;
        else if(FT1<=48.219)tmp1=65.7;
        else if(FT1<=48.2815)tmp1=65.8;
        else if(FT1<=48.3441)tmp1=65.9;
        else if(FT1<=48.4067)tmp1=66;
    }
    else if(FT1<=48.9726){
        if(FT1==48.4067)tmp1=66;
        else if(FT1<=48.4694)tmp1=66.1;
        else if(FT1<=48.5322)tmp1=66.2;
        else if(FT1<=48.5949)tmp1=66.3;
        else if(FT1<=48.6578)tmp1=66.4;
        else if(FT1<=48.7206)tmp1=66.5;
        else if(FT1<=48.7836)tmp1=66.6;
        else if(FT1<=48.8465)tmp1=66.7;
        else if(FT1<=48.9095)tmp1=66.8;
        else if(FT1<=48.9726)tmp1=66.9;
        else if(FT1<=49.0357)tmp1=67;
    }
    else if(FT1<=49.6057){
        if(FT1==49.0357)tmp1=67;
        else if(FT1<=49.0988)tmp1=67.1;
        else if(FT1<=49.162)tmp1=67.2;
        else if(FT1<=49.2253)tmp1=67.3;
        else if(FT1<=49.2885)tmp1=67.4;
        else if(FT1<=49.3519)tmp1=67.5;
        else if(FT1<=49.4152)tmp1=67.6;
        else if(FT1<=49.4787)tmp1=67.7;
        else if(FT1<=49.5421)tmp1=67.8;
        else if(FT1<=49.6057)tmp1=67.9;
        else if(FT1<=49.6692)tmp1=68;
    }
    else if(FT1<=50.2433){
        if(FT1==49.6692)tmp1=68;
        else if(FT1<=49.7328)tmp1=68.1;
        else if(FT1<=49.7965)tmp1=68.2;
        else if(FT1<=49.8602)tmp1=68.3;
        else if(FT1<=49.9239)tmp1=68.4;
        else if(FT1<=49.9877)tmp1=68.5;
        else if(FT1<=50.0516)tmp1=68.6;
        else if(FT1<=50.1154)tmp1=68.7;
        else if(FT1<=50.1794)tmp1=68.8;
        else if(FT1<=50.2433)tmp1=68.9;
        else if(FT1<=50.3074)tmp1=69;
    }
    else if(FT1<=50.8857){
        if(FT1==50.3074)tmp1=69;
        else if(FT1<=50.3714)tmp1=69.1;
        else if(FT1<=50.4356)tmp1=69.2;
        else if(FT1<=50.4997)tmp1=69.3;
        else if(FT1<=50.5639)tmp1=69.4;
        else if(FT1<=50.6282)tmp1=69.5;
        else if(FT1<=50.6925)tmp1=69.6;
        else if(FT1<=50.7568)tmp1=69.7;
        else if(FT1<=50.8212)tmp1=69.8;
        else if(FT1<=50.8857)tmp1=69.9;
        else if(FT1<=50.9501)tmp1=70;
    }
    else if(FT1<=51.5326){
        if(FT1==50.9501)tmp1=70;
        else if(FT1<=51.0147)tmp1=70.1;
        else if(FT1<=51.0792)tmp1=70.2;
        else if(FT1<=51.1439)tmp1=70.3;
        else if(FT1<=51.2085)tmp1=70.4;
        else if(FT1<=51.2732)tmp1=70.5;
        else if(FT1<=51.338)tmp1=70.6;
        else if(FT1<=51.4028)tmp1=70.7;
        else if(FT1<=51.4677)tmp1=70.8;
        else if(FT1<=51.5326)tmp1=70.9;
        else if(FT1<=51.5975)tmp1=71;
    }
    else if(FT1<=52.1841){
        if(FT1==51.5975)tmp1=71;
        else if(FT1<=51.6625)tmp1=71.1;
        else if(FT1<=51.7275)tmp1=71.2;
        else if(FT1<=51.7926)tmp1=71.3;
        else if(FT1<=51.8578)tmp1=71.4;
        else if(FT1<=51.9229)tmp1=71.5;
        else if(FT1<=51.9882)tmp1=71.6;
        else if(FT1<=52.0534)tmp1=71.7;
        else if(FT1<=52.1187)tmp1=71.8;
        else if(FT1<=52.1841)tmp1=71.9;
        else if(FT1<=52.2495)tmp1=72;
    }
    else if(FT1<=52.8402){
        if(FT1==52.2495)tmp1=72;
        else if(FT1<=52.3149)tmp1=72.1;
        else if(FT1<=52.3804)tmp1=72.2;
        else if(FT1<=52.446)tmp1=72.3;
        else if(FT1<=52.5116)tmp1=72.4;
        else if(FT1<=52.5772)tmp1=72.5;
        else if(FT1<=52.6429)tmp1=72.6;
        else if(FT1<=52.7086)tmp1=72.7;
        else if(FT1<=52.7744)tmp1=72.8;
        else if(FT1<=52.8402)tmp1=72.9;
        else if(FT1<=52.9061)tmp1=73;
    }
    else if(FT1<=53.501){
        if(FT1==52.9061)tmp1=73;
        else if(FT1<=52.972)tmp1=73.1;
        else if(FT1<=53.038)tmp1=73.2;
        else if(FT1<=53.104)tmp1=73.3;
        else if(FT1<=53.17)tmp1=73.4;
        else if(FT1<=53.2361)tmp1=73.5;
        else if(FT1<=53.3023)tmp1=73.6;
        else if(FT1<=53.3684)tmp1=73.7;
        else if(FT1<=53.4347)tmp1=73.8;
        else if(FT1<=53.501)tmp1=73.9;
        else if(FT1<=53.5673)tmp1=74;
    }
    else if(FT1<=54.1663){
        if(FT1==53.5673)tmp1=74;
        else if(FT1<=53.6337)tmp1=74.1;
        else if(FT1<=53.7001)tmp1=74.2;
        else if(FT1<=53.7665)tmp1=74.3;
        else if(FT1<=53.8331)tmp1=74.4;
        else if(FT1<=53.8996)tmp1=74.5;
        else if(FT1<=53.9662)tmp1=74.6;
        else if(FT1<=54.0329)tmp1=74.7;
        else if(FT1<=54.0996)tmp1=74.8;
        else if(FT1<=54.1663)tmp1=74.9;
        else if(FT1<=54.2331)tmp1=75;
    }
    else if(FT1<=54.8363){
        if(FT1==54.2331)tmp1=75;
        else if(FT1<=54.2999)tmp1=75.1;
        else if(FT1<=54.3668)tmp1=75.2;
        else if(FT1<=54.4337)tmp1=75.3;
        else if(FT1<=54.5007)tmp1=75.4;
        else if(FT1<=54.5677)tmp1=75.5;
        else if(FT1<=54.6348)tmp1=75.6;
        else if(FT1<=54.7019)tmp1=75.7;
        else if(FT1<=54.7691)tmp1=75.8;
        else if(FT1<=54.8363)tmp1=75.9;
        else if(FT1<=54.9035)tmp1=76;
    }
    else if(FT1<=55.5108){
        if(FT1==54.9035)tmp1=76;
        else if(FT1<=54.9708)tmp1=76.1;
        else if(FT1<=55.0381)tmp1=76.2;
        else if(FT1<=55.1055)tmp1=76.3;
        else if(FT1<=55.173)tmp1=76.4;
        else if(FT1<=55.2404)tmp1=76.5;
        else if(FT1<=55.308)tmp1=76.6;
        else if(FT1<=55.3755)tmp1=76.7;
        else if(FT1<=55.4431)tmp1=76.8;
        else if(FT1<=55.5108)tmp1=76.9;
        else if(FT1<=55.5785)tmp1=77;
    }
    else if(FT1<=56.19){
        if(FT1==55.5785)tmp1=77;
        else if(FT1<=55.6463)tmp1=77.1;
        else if(FT1<=55.7141)tmp1=77.2;
        else if(FT1<=55.7819)tmp1=77.3;
        else if(FT1<=55.8498)tmp1=77.4;
        else if(FT1<=55.9177)tmp1=77.5;
        else if(FT1<=55.9857)tmp1=77.6;
        else if(FT1<=56.0538)tmp1=77.7;
        else if(FT1<=56.1218)tmp1=77.8;
        else if(FT1<=56.19)tmp1=77.9;
        else if(FT1<=56.2581)tmp1=78;
    }
    else if(FT1<=56.8737){
        if(FT1==56.2581)tmp1=78;
        else if(FT1<=56.3263)tmp1=78.1;
        else if(FT1<=56.3946)tmp1=78.2;
        else if(FT1<=56.4629)tmp1=78.3;
        else if(FT1<=56.5313)tmp1=78.4;
        else if(FT1<=56.5996)tmp1=78.5;
        else if(FT1<=56.6681)tmp1=78.6;
        else if(FT1<=56.7366)tmp1=78.7;
        else if(FT1<=56.8051)tmp1=78.8;
        else if(FT1<=56.8737)tmp1=78.9;
        else if(FT1<=56.9423)tmp1=79;
    }
    else if(FT1<=57.562){
        if(FT1==56.9423)tmp1=79;
        else if(FT1<=57.011)tmp1=79.1;
        else if(FT1<=57.0797)tmp1=79.2;
        else if(FT1<=57.1485)tmp1=79.3;
        else if(FT1<=57.2173)tmp1=79.4;
        else if(FT1<=57.2862)tmp1=79.5;
        else if(FT1<=57.3551)tmp1=79.6;
        else if(FT1<=57.424)tmp1=79.7;
        else if(FT1<=57.493)tmp1=79.8;
        else if(FT1<=57.562)tmp1=79.9;
        else if(FT1<=57.6311)tmp1=80;
    }
    else if(FT1<=58.255){
        if(FT1==57.6311)tmp1=80;
        else if(FT1<=57.7003)tmp1=80.1;
        else if(FT1<=57.7694)tmp1=80.2;
        else if(FT1<=57.8387)tmp1=80.3;
        else if(FT1<=57.9079)tmp1=80.4;
        else if(FT1<=57.9772)tmp1=80.5;
        else if(FT1<=58.0466)tmp1=80.6;
        else if(FT1<=58.116)tmp1=80.7;
        else if(FT1<=58.1855)tmp1=80.8;
        else if(FT1<=58.255)tmp1=80.9;
        else if(FT1<=58.3245)tmp1=81;
    }
    else if(FT1<=58.9525){
        if(FT1==58.3245)tmp1=81;
        else if(FT1<=58.3941)tmp1=81.1;
        else if(FT1<=58.4637)tmp1=81.2;
        else if(FT1<=58.5334)tmp1=81.3;
        else if(FT1<=58.6032)tmp1=81.4;
        else if(FT1<=58.6729)tmp1=81.5;
        else if(FT1<=58.7428)tmp1=81.6;
        else if(FT1<=58.8126)tmp1=81.7;
        else if(FT1<=58.8825)tmp1=81.8;
        else if(FT1<=58.9525)tmp1=81.9;
        else if(FT1<=59.0225)tmp1=82;
    }
    else if(FT1<=59.6546){
        if(FT1==59.0225)tmp1=82;
        else if(FT1<=59.0926)tmp1=82.1;
        else if(FT1<=59.1626)tmp1=82.2;
        else if(FT1<=59.2328)tmp1=82.3;
        else if(FT1<=59.303)tmp1=82.4;
        else if(FT1<=59.3732)tmp1=82.5;
        else if(FT1<=59.4435)tmp1=82.6;
        else if(FT1<=59.5138)tmp1=82.7;
        else if(FT1<=59.5842)tmp1=82.8;
        else if(FT1<=59.6546)tmp1=82.9;
        else if(FT1<=59.7251)tmp1=83;
    }
    else if(FT1<=60.3613){
        if(FT1==59.7251)tmp1=83;
        else if(FT1<=59.7956)tmp1=83.1;
        else if(FT1<=59.8661)tmp1=83.2;
        else if(FT1<=59.9367)tmp1=83.3;
        else if(FT1<=60.0074)tmp1=83.4;
        else if(FT1<=60.0781)tmp1=83.5;
        else if(FT1<=60.1488)tmp1=83.6;
        else if(FT1<=60.2196)tmp1=83.7;
        else if(FT1<=60.2904)tmp1=83.8;
        else if(FT1<=60.3613)tmp1=83.9;
        else if(FT1<=60.4322)tmp1=84;
    }
    else if(FT1<=61.0726){
        if(FT1==60.4322)tmp1=84;
        else if(FT1<=60.5032)tmp1=84.1;
        else if(FT1<=60.5742)tmp1=84.2;
        else if(FT1<=60.6453)tmp1=84.3;
        else if(FT1<=60.7164)tmp1=84.4;
        else if(FT1<=60.7875)tmp1=84.5;
        else if(FT1<=60.8587)tmp1=84.6;
        else if(FT1<=60.93)tmp1=84.7;
        else if(FT1<=61.0012)tmp1=84.8;
        else if(FT1<=61.0726)tmp1=84.9;
        else if(FT1<=61.144)tmp1=85;
    }
    else if(FT1<=61.7884){
        if(FT1==61.144)tmp1=85;
        else if(FT1<=61.2154)tmp1=85.1;
        else if(FT1<=61.2869)tmp1=85.2;
        else if(FT1<=61.3584)tmp1=85.3;
        else if(FT1<=61.4299)tmp1=85.4;
        else if(FT1<=61.5015)tmp1=85.5;
        else if(FT1<=61.5732)tmp1=85.6;
        else if(FT1<=61.6449)tmp1=85.7;
        else if(FT1<=61.7166)tmp1=85.8;
        else if(FT1<=61.7884)tmp1=85.9;
        else if(FT1<=61.8603)tmp1=86;
    }
    else if(FT1<=62.5089){
        if(FT1==61.8603)tmp1=86;
        else if(FT1<=61.9322)tmp1=86.1;
        else if(FT1<=62.0041)tmp1=86.2;
        else if(FT1<=62.0761)tmp1=86.3;
        else if(FT1<=62.1481)tmp1=86.4;
        else if(FT1<=62.2201)tmp1=86.5;
        else if(FT1<=62.2923)tmp1=86.6;
        else if(FT1<=62.3644)tmp1=86.7;
        else if(FT1<=62.4366)tmp1=86.8;
        else if(FT1<=62.5089)tmp1=86.9;
        else if(FT1<=62.5812)tmp1=87;
    }
    else if(FT1<=63.2339){
        if(FT1==62.5812)tmp1=87;
        else if(FT1<=62.6535)tmp1=87.1;
        else if(FT1<=62.7259)tmp1=87.2;
        else if(FT1<=62.7983)tmp1=87.3;
        else if(FT1<=62.8708)tmp1=87.4;
        else if(FT1<=62.9433)tmp1=87.5;
        else if(FT1<=63.0159)tmp1=87.6;
        else if(FT1<=63.0885)tmp1=87.7;
        else if(FT1<=63.1612)tmp1=87.8;
        else if(FT1<=63.2339)tmp1=87.9;
        else if(FT1<=63.3066)tmp1=88;
    }
    else if(FT1<=63.9634){
        if(FT1==63.3066)tmp1=88;
        else if(FT1<=63.3794)tmp1=88.1;
        else if(FT1<=63.4523)tmp1=88.2;
        else if(FT1<=63.5251)tmp1=88.3;
        else if(FT1<=63.5981)tmp1=88.4;
        else if(FT1<=63.6711)tmp1=88.5;
        else if(FT1<=63.7441)tmp1=88.6;
        else if(FT1<=63.8172)tmp1=88.7;
        else if(FT1<=63.8903)tmp1=88.8;
        else if(FT1<=63.9634)tmp1=88.9;
        else if(FT1<=64.0366)tmp1=89;
    }
    else if(FT1<=64.6976){
        if(FT1==64.0366)tmp1=89;
        else if(FT1<=64.1099)tmp1=89.1;
        else if(FT1<=64.1832)tmp1=89.2;
        else if(FT1<=64.2565)tmp1=89.3;
        else if(FT1<=64.3299)tmp1=89.4;
        else if(FT1<=64.4034)tmp1=89.5;
        else if(FT1<=64.4768)tmp1=89.6;
        else if(FT1<=64.5504)tmp1=89.7;
        else if(FT1<=64.6239)tmp1=89.8;
        else if(FT1<=64.6976)tmp1=89.9;
        else if(FT1<=64.7712)tmp1=90;
    }
    else if(FT1<=65.4363){
        if(FT1==64.7712)tmp1=90;
        else if(FT1<=64.8449)tmp1=90.1;
        else if(FT1<=64.9187)tmp1=90.2;
        else if(FT1<=64.9925)tmp1=90.3;
        else if(FT1<=65.0663)tmp1=90.4;
        else if(FT1<=65.1402)tmp1=90.5;
        else if(FT1<=65.2142)tmp1=90.6;
        else if(FT1<=65.2882)tmp1=90.7;
        else if(FT1<=65.3622)tmp1=90.8;
        else if(FT1<=65.4363)tmp1=90.9;
        else if(FT1<=65.5104)tmp1=91;
    }
    else if(FT1<=66.1795){
        if(FT1==65.5104)tmp1=91;
        else if(FT1<=65.5845)tmp1=91.1;
        else if(FT1<=65.6588)tmp1=91.2;
        else if(FT1<=65.733)tmp1=91.3;
        else if(FT1<=65.8073)tmp1=91.4;
        else if(FT1<=65.8817)tmp1=91.5;
        else if(FT1<=65.9561)tmp1=91.6;
        else if(FT1<=66.0305)tmp1=91.7;
        else if(FT1<=66.105)tmp1=91.8;
        else if(FT1<=66.1795)tmp1=91.9;
        else if(FT1<=66.2541)tmp1=92;
    }
    else if(FT1<=66.9273){
        if(FT1==66.2541)tmp1=92;
        else if(FT1<=66.3287)tmp1=92.1;
        else if(FT1<=66.4034)tmp1=92.2;
        else if(FT1<=66.4781)tmp1=92.3;
        else if(FT1<=66.5528)tmp1=92.4;
        else if(FT1<=66.6276)tmp1=92.5;
        else if(FT1<=66.7025)tmp1=92.6;
        else if(FT1<=66.7774)tmp1=92.7;
        else if(FT1<=66.8523)tmp1=92.8;
        else if(FT1<=66.9273)tmp1=92.9;
        else if(FT1<=67.0023)tmp1=93;
    }
    else if(FT1<=67.6797){
        if(FT1==67.0023)tmp1=93;
        else if(FT1<=67.0774)tmp1=93.1;
        else if(FT1<=67.1525)tmp1=93.2;
        else if(FT1<=67.2277)tmp1=93.3;
        else if(FT1<=67.3029)tmp1=93.4;
        else if(FT1<=67.3782)tmp1=93.5;
        else if(FT1<=67.4535)tmp1=93.6;
        else if(FT1<=67.5288)tmp1=93.7;
        else if(FT1<=67.6042)tmp1=93.8;
        else if(FT1<=67.6797)tmp1=93.9;
        else if(FT1<=67.7551)tmp1=94;
    }
    else if(FT1<=68.4365){
        if(FT1==67.7551)tmp1=94;
        else if(FT1<=67.8307)tmp1=94.1;
        else if(FT1<=67.9062)tmp1=94.2;
        else if(FT1<=67.9819)tmp1=94.3;
        else if(FT1<=68.0575)tmp1=94.4;
        else if(FT1<=68.1332)tmp1=94.5;
        else if(FT1<=68.209)tmp1=94.6;
        else if(FT1<=68.2848)tmp1=94.7;
        else if(FT1<=68.3607)tmp1=94.8;
        else if(FT1<=68.4365)tmp1=94.9;
        else if(FT1<=68.5125)tmp1=95;
    }
    else if(FT1<=69.198){
        if(FT1==68.5125)tmp1=95;
        else if(FT1<=68.5885)tmp1=95.1;
        else if(FT1<=68.6645)tmp1=95.2;
        else if(FT1<=68.7406)tmp1=95.3;
        else if(FT1<=68.8167)tmp1=95.4;
        else if(FT1<=68.8929)tmp1=95.5;
        else if(FT1<=68.9691)tmp1=95.6;
        else if(FT1<=69.0453)tmp1=95.7;
        else if(FT1<=69.1216)tmp1=95.8;
        else if(FT1<=69.198)tmp1=95.9;
        else if(FT1<=69.2744)tmp1=96;
    }
    else if(FT1<=69.9639){
        if(FT1==69.2744)tmp1=96;
        else if(FT1<=69.3508)tmp1=96.1;
        else if(FT1<=69.4273)tmp1=96.2;
        else if(FT1<=69.5038)tmp1=96.3;
        else if(FT1<=69.5804)tmp1=96.4;
        else if(FT1<=69.657)tmp1=96.5;
        else if(FT1<=69.7337)tmp1=96.6;
        else if(FT1<=69.8104)tmp1=96.7;
        else if(FT1<=69.8871)tmp1=96.8;
        else if(FT1<=69.9639)tmp1=96.9;
        else if(FT1<=70.0408)tmp1=97;
    }
    else if(FT1<=70.7344){
        if(FT1==70.0408)tmp1=97;
        else if(FT1<=70.1177)tmp1=97.1;
        else if(FT1<=70.1946)tmp1=97.2;
        else if(FT1<=70.2716)tmp1=97.3;
        else if(FT1<=70.3486)tmp1=97.4;
        else if(FT1<=70.4257)tmp1=97.5;
        else if(FT1<=70.5028)tmp1=97.6;
        else if(FT1<=70.58)tmp1=97.7;
        else if(FT1<=70.6572)tmp1=97.8;
        else if(FT1<=70.7344)tmp1=97.9;
        else if(FT1<=70.8117)tmp1=98;
    }
    else if(FT1<=71.5095){
        if(FT1==70.8117)tmp1=98;
        else if(FT1<=70.8891)tmp1=98.1;
        else if(FT1<=70.9665)tmp1=98.2;
        else if(FT1<=71.0439)tmp1=98.3;
        else if(FT1<=71.1214)tmp1=98.4;
        else if(FT1<=71.1989)tmp1=98.5;
        else if(FT1<=71.2765)tmp1=98.6;
        else if(FT1<=71.3541)tmp1=98.7;
        else if(FT1<=71.4318)tmp1=98.8;
        else if(FT1<=71.5095)tmp1=98.9;
        else if(FT1<=71.5872)tmp1=99;
    }
    else if(FT1<=72.289){
        if(FT1==71.5872)tmp1=99;
        else if(FT1<=71.665)tmp1=99.1;
        else if(FT1<=71.7429)tmp1=99.2;
        else if(FT1<=71.8207)tmp1=99.3;
        else if(FT1<=71.8987)tmp1=99.4;
        else if(FT1<=71.9766)tmp1=99.5;
        else if(FT1<=72.0547)tmp1=99.6;
        else if(FT1<=72.1327)tmp1=99.7;
        else if(FT1<=72.2108)tmp1=99.8;
        else if(FT1<=72.289)tmp1=99.9;
        else if(FT1<=72.3672)tmp1=100;
    }
    else if(FT1<=73.0731){
        if(FT1==72.3672)tmp1=100;
        else if(FT1<=72.4455)tmp1=100.1;
        else if(FT1<=72.5238)tmp1=100.2;
        else if(FT1<=72.6021)tmp1=100.3;
        else if(FT1<=72.6805)tmp1=100.4;
        else if(FT1<=72.7589)tmp1=100.5;
        else if(FT1<=72.8374)tmp1=100.6;
        else if(FT1<=72.9159)tmp1=100.7;
        else if(FT1<=72.9945)tmp1=100.8;
        else if(FT1<=73.0731)tmp1=100.9;
        else if(FT1<=73.1517)tmp1=101;
    }
    else if(FT1<=73.8616){
        if(FT1==73.1517)tmp1=101;
        else if(FT1<=73.2304)tmp1=101.1;
        else if(FT1<=73.3092)tmp1=101.2;
        else if(FT1<=73.388)tmp1=101.3;
        else if(FT1<=73.4668)tmp1=101.4;
        else if(FT1<=73.5457)tmp1=101.5;
        else if(FT1<=73.6246)tmp1=101.6;
        else if(FT1<=73.7036)tmp1=101.7;
        else if(FT1<=73.7826)tmp1=101.8;
        else if(FT1<=73.8616)tmp1=101.9;
        else if(FT1<=73.9407)tmp1=102;
    }
    else if(FT1<=74.6547){
        if(FT1==73.9407)tmp1=102;
        else if(FT1<=74.0199)tmp1=102.1;
        else if(FT1<=74.0991)tmp1=102.2;
        else if(FT1<=74.1783)tmp1=102.3;
        else if(FT1<=74.2576)tmp1=102.4;
        else if(FT1<=74.3369)tmp1=102.5;
        else if(FT1<=74.4163)tmp1=102.6;
        else if(FT1<=74.4957)tmp1=102.7;
        else if(FT1<=74.5752)tmp1=102.8;
        else if(FT1<=74.6547)tmp1=102.9;
        else if(FT1<=74.7343)tmp1=103;
    }
    else if(FT1<=75.4523){
        if(FT1==74.7343)tmp1=103;
        else if(FT1<=74.8139)tmp1=103.1;
        else if(FT1<=74.8935)tmp1=103.2;
        else if(FT1<=74.9732)tmp1=103.3;
        else if(FT1<=75.0529)tmp1=103.4;
        else if(FT1<=75.1327)tmp1=103.5;
        else if(FT1<=75.2125)tmp1=103.6;
        else if(FT1<=75.2924)tmp1=103.7;
        else if(FT1<=75.3723)tmp1=103.8;
        else if(FT1<=75.4523)tmp1=103.9;
        else if(FT1<=75.5323)tmp1=104;
    }
    else if(FT1<=76.2543){
        if(FT1==75.5323)tmp1=104;
        else if(FT1<=75.6123)tmp1=104.1;
        else if(FT1<=75.6924)tmp1=104.2;
        else if(FT1<=75.7726)tmp1=104.3;
        else if(FT1<=75.8527)tmp1=104.4;
        else if(FT1<=75.933)tmp1=104.5;
        else if(FT1<=76.0132)tmp1=104.6;
        else if(FT1<=76.0936)tmp1=104.7;
        else if(FT1<=76.1739)tmp1=104.8;
        else if(FT1<=76.2543)tmp1=104.9;
        else if(FT1<=76.3348)tmp1=105;
    }
    else if(FT1<=77.0609){
        if(FT1==76.3348)tmp1=105;
        else if(FT1<=76.4153)tmp1=105.1;
        else if(FT1<=76.4958)tmp1=105.2;
        else if(FT1<=76.5764)tmp1=105.3;
        else if(FT1<=76.6571)tmp1=105.4;
        else if(FT1<=76.7377)tmp1=105.5;
        else if(FT1<=76.8185)tmp1=105.6;
        else if(FT1<=76.8992)tmp1=105.7;
        else if(FT1<=76.98)tmp1=105.8;
        else if(FT1<=77.0609)tmp1=105.9;
        else if(FT1<=77.1418)tmp1=106;
    }
    else if(FT1<=77.8719){
        if(FT1==77.1418)tmp1=106;
        else if(FT1<=77.2227)tmp1=106.1;
        else if(FT1<=77.3037)tmp1=106.2;
        else if(FT1<=77.3848)tmp1=106.3;
        else if(FT1<=77.4659)tmp1=106.4;
        else if(FT1<=77.547)tmp1=106.5;
        else if(FT1<=77.6282)tmp1=106.6;
        else if(FT1<=77.7094)tmp1=106.7;
        else if(FT1<=77.7906)tmp1=106.8;
        else if(FT1<=77.8719)tmp1=106.9;
        else if(FT1<=77.9533)tmp1=107;
    }
    else if(FT1<=78.6874){
        if(FT1==77.9533)tmp1=107;
        else if(FT1<=78.0347)tmp1=107.1;
        else if(FT1<=78.1161)tmp1=107.2;
        else if(FT1<=78.1976)tmp1=107.3;
        else if(FT1<=78.2791)tmp1=107.4;
        else if(FT1<=78.3607)tmp1=107.5;
        else if(FT1<=78.4423)tmp1=107.6;
        else if(FT1<=78.524)tmp1=107.7;
        else if(FT1<=78.6057)tmp1=107.8;
        else if(FT1<=78.6874)tmp1=107.9;
        else if(FT1<=78.7692)tmp1=108;
    }
    else if(FT1<=79.5074){
        if(FT1==78.7692)tmp1=108;
        else if(FT1<=78.8511)tmp1=108.1;
        else if(FT1<=78.933)tmp1=108.2;
        else if(FT1<=79.0149)tmp1=108.3;
        else if(FT1<=79.0969)tmp1=108.4;
        else if(FT1<=79.1789)tmp1=108.5;
        else if(FT1<=79.261)tmp1=108.6;
        else if(FT1<=79.3431)tmp1=108.7;
        else if(FT1<=79.4252)tmp1=108.8;
        else if(FT1<=79.5074)tmp1=108.9;
        else if(FT1<=79.5897)tmp1=109;
    }
    else if(FT1<=80.3319){
        if(FT1==79.5897)tmp1=109;
        else if(FT1<=79.672)tmp1=109.1;
        else if(FT1<=79.7543)tmp1=109.2;
        else if(FT1<=79.8367)tmp1=109.3;
        else if(FT1<=79.9191)tmp1=109.4;
        else if(FT1<=80.0016)tmp1=109.5;
        else if(FT1<=80.0841)tmp1=109.6;
        else if(FT1<=80.1666)tmp1=109.7;
        else if(FT1<=80.2492)tmp1=109.8;
        else if(FT1<=80.3319)tmp1=109.9;
        else if(FT1<=80.4146)tmp1=110;
    }
    else if(FT1<=81.1608){
        if(FT1==80.4146)tmp1=110;
        else if(FT1<=80.4973)tmp1=110.1;
        else if(FT1<=80.5801)tmp1=110.2;
        else if(FT1<=80.6629)tmp1=110.3;
        else if(FT1<=80.7458)tmp1=110.4;
        else if(FT1<=80.8287)tmp1=110.5;
        else if(FT1<=80.9117)tmp1=110.6;
        else if(FT1<=80.9947)tmp1=110.7;
        else if(FT1<=81.0777)tmp1=110.8;
        else if(FT1<=81.1608)tmp1=110.9;
        else if(FT1<=81.2439)tmp1=111;
    }
    else if(FT1<=81.9942){
        if(FT1==81.2439)tmp1=111;
        else if(FT1<=81.3271)tmp1=111.1;
        else if(FT1<=81.4103)tmp1=111.2;
        else if(FT1<=81.4936)tmp1=111.3;
        else if(FT1<=81.5769)tmp1=111.4;
        else if(FT1<=81.6603)tmp1=111.5;
        else if(FT1<=81.7437)tmp1=111.6;
        else if(FT1<=81.8271)tmp1=111.7;
        else if(FT1<=81.9106)tmp1=111.8;
        else if(FT1<=81.9942)tmp1=111.9;
        else if(FT1<=82.0777)tmp1=112;
    }
    else if(FT1<=82.832){
        if(FT1==82.0777)tmp1=112;
        else if(FT1<=82.1614)tmp1=112.1;
        else if(FT1<=82.245)tmp1=112.2;
        else if(FT1<=82.3288)tmp1=112.3;
        else if(FT1<=82.4125)tmp1=112.4;
        else if(FT1<=82.4963)tmp1=112.5;
        else if(FT1<=82.5802)tmp1=112.6;
        else if(FT1<=82.6641)tmp1=112.7;
        else if(FT1<=82.748)tmp1=112.8;
        else if(FT1<=82.832)tmp1=112.9;
        else if(FT1<=82.916)tmp1=113;
    }
    else if(FT1<=83.6742){
        if(FT1==82.916)tmp1=113;
        else if(FT1<=83.0001)tmp1=113.1;
        else if(FT1<=83.0842)tmp1=113.2;
        else if(FT1<=83.1683)tmp1=113.3;
        else if(FT1<=83.2525)tmp1=113.4;
        else if(FT1<=83.3368)tmp1=113.5;
        else if(FT1<=83.4211)tmp1=113.6;
        else if(FT1<=83.5054)tmp1=113.7;
        else if(FT1<=83.5898)tmp1=113.8;
        else if(FT1<=83.6742)tmp1=113.9;
        else if(FT1<=83.7587)tmp1=114;
    }
    else if(FT1<=84.5209){
        if(FT1==83.7587)tmp1=114;
        else if(FT1<=83.8432)tmp1=114.1;
        else if(FT1<=83.9278)tmp1=114.2;
        else if(FT1<=84.0124)tmp1=114.3;
        else if(FT1<=84.097)tmp1=114.4;
        else if(FT1<=84.1817)tmp1=114.5;
        else if(FT1<=84.2664)tmp1=114.6;
        else if(FT1<=84.3512)tmp1=114.7;
        else if(FT1<=84.436)tmp1=114.8;
        else if(FT1<=84.5209)tmp1=114.9;
        else if(FT1<=84.6058)tmp1=115;
    }
    else if(FT1<=85.372){
        if(FT1==84.6058)tmp1=115;
        else if(FT1<=84.6908)tmp1=115.1;
        else if(FT1<=84.7758)tmp1=115.2;
        else if(FT1<=84.8608)tmp1=115.3;
        else if(FT1<=84.9459)tmp1=115.4;
        else if(FT1<=85.0311)tmp1=115.5;
        else if(FT1<=85.1162)tmp1=115.6;
        else if(FT1<=85.2015)tmp1=115.7;
        else if(FT1<=85.2867)tmp1=115.8;
        else if(FT1<=85.372)tmp1=115.9;
        else if(FT1<=85.4574)tmp1=116;
    }
    else if(FT1<=86.2276){
        if(FT1==85.4574)tmp1=116;
        else if(FT1<=85.5428)tmp1=116.1;
        else if(FT1<=85.6282)tmp1=116.2;
        else if(FT1<=85.7137)tmp1=116.3;
        else if(FT1<=85.7993)tmp1=116.4;
        else if(FT1<=85.8848)tmp1=116.5;
        else if(FT1<=85.9704)tmp1=116.6;
        else if(FT1<=86.0561)tmp1=116.7;
        else if(FT1<=86.1418)tmp1=116.8;
        else if(FT1<=86.2276)tmp1=116.9;
        else if(FT1<=86.3134)tmp1=117;
    }
    else if(FT1<=87.0875){
        if(FT1==86.3134)tmp1=117;
        else if(FT1<=86.3992)tmp1=117.1;
        else if(FT1<=86.4851)tmp1=117.2;
        else if(FT1<=86.571)tmp1=117.3;
        else if(FT1<=86.657)tmp1=117.4;
        else if(FT1<=86.743)tmp1=117.5;
        else if(FT1<=86.8291)tmp1=117.6;
        else if(FT1<=86.9152)tmp1=117.7;
        else if(FT1<=87.0013)tmp1=117.8;
        else if(FT1<=87.0875)tmp1=117.9;
        else if(FT1<=87.1738)tmp1=118;
    }
    else if(FT1<=87.9519){
        if(FT1==87.1738)tmp1=118;
        else if(FT1<=87.26)tmp1=118.1;
        else if(FT1<=87.3464)tmp1=118.2;
        else if(FT1<=87.4327)tmp1=118.3;
        else if(FT1<=87.5192)tmp1=118.4;
        else if(FT1<=87.6056)tmp1=118.5;
        else if(FT1<=87.6921)tmp1=118.6;
        else if(FT1<=87.7787)tmp1=118.7;
        else if(FT1<=87.8653)tmp1=118.8;
        else if(FT1<=87.9519)tmp1=118.9;
        else if(FT1<=88.0386)tmp1=119;
    }
    else if(FT1<=88.8207){
        if(FT1==88.0386)tmp1=119;
        else if(FT1<=88.1253)tmp1=119.1;
        else if(FT1<=88.2121)tmp1=119.2;
        else if(FT1<=88.2989)tmp1=119.3;
        else if(FT1<=88.3857)tmp1=119.4;
        else if(FT1<=88.4726)tmp1=119.5;
        else if(FT1<=88.5596)tmp1=119.6;
        else if(FT1<=88.6465)tmp1=119.7;
        else if(FT1<=88.7336)tmp1=119.8;
        else if(FT1<=88.8207)tmp1=119.9;
        else if(FT1<=88.9078)tmp1=120;
    }
    else if(FT1<=89.6938){
        if(FT1==88.9078)tmp1=120;
        else if(FT1<=88.9949)tmp1=120.1;
        else if(FT1<=89.0821)tmp1=120.2;
        else if(FT1<=89.1694)tmp1=120.3;
        else if(FT1<=89.2567)tmp1=120.4;
        else if(FT1<=89.344)tmp1=120.5;
        else if(FT1<=89.4314)tmp1=120.6;
        else if(FT1<=89.5188)tmp1=120.7;
        else if(FT1<=89.6063)tmp1=120.8;
        else if(FT1<=89.6938)tmp1=120.9;
        else if(FT1<=89.7814)tmp1=121;
    }
    else if(FT1<=90.5714){
        if(FT1==89.7814)tmp1=121;
        else if(FT1<=89.869)tmp1=121.1;
        else if(FT1<=89.9566)tmp1=121.2;
        else if(FT1<=90.0443)tmp1=121.3;
        else if(FT1<=90.132)tmp1=121.4;
        else if(FT1<=90.2198)tmp1=121.5;
        else if(FT1<=90.3076)tmp1=121.6;
        else if(FT1<=90.3955)tmp1=121.7;
        else if(FT1<=90.4834)tmp1=121.8;
        else if(FT1<=90.5714)tmp1=121.9;
        else if(FT1<=90.6593)tmp1=122;
    }
    else if(FT1<=91.4533){
        if(FT1==90.6593)tmp1=122;
        else if(FT1<=90.7474)tmp1=122.1;
        else if(FT1<=90.8355)tmp1=122.2;
        else if(FT1<=90.9236)tmp1=122.3;
        else if(FT1<=91.0118)tmp1=122.4;
        else if(FT1<=91.1)tmp1=122.5;
        else if(FT1<=91.1882)tmp1=122.6;
        else if(FT1<=91.2765)tmp1=122.7;
        else if(FT1<=91.3649)tmp1=122.8;
        else if(FT1<=91.4533)tmp1=122.9;
        else if(FT1<=91.5417)tmp1=123;
    }
    else if(FT1<=92.3396){
        if(FT1==91.5417)tmp1=123;
        else if(FT1<=91.6302)tmp1=123.1;
        else if(FT1<=91.7187)tmp1=123.2;
        else if(FT1<=91.8073)tmp1=123.3;
        else if(FT1<=91.8959)tmp1=123.4;
        else if(FT1<=91.9845)tmp1=123.5;
        else if(FT1<=92.0732)tmp1=123.6;
        else if(FT1<=92.162)tmp1=123.7;
        else if(FT1<=92.2508)tmp1=123.8;
        else if(FT1<=92.3396)tmp1=123.9;
        else if(FT1<=92.4285)tmp1=124;
    }
    else if(FT1<=93.2302){
        if(FT1==92.4285)tmp1=124;
        else if(FT1<=92.5174)tmp1=124.1;
        else if(FT1<=92.6063)tmp1=124.2;
        else if(FT1<=92.6953)tmp1=124.3;
        else if(FT1<=92.7844)tmp1=124.4;
        else if(FT1<=92.8735)tmp1=124.5;
        else if(FT1<=92.9626)tmp1=124.6;
        else if(FT1<=93.0518)tmp1=124.7;
        else if(FT1<=93.141)tmp1=124.8;
        else if(FT1<=93.2302)tmp1=124.9;
        else if(FT1<=93.3196)tmp1=125;
    }
    else if(FT1<=94.1253){
        if(FT1==93.3196)tmp1=125;
        else if(FT1<=93.4089)tmp1=125.1;
        else if(FT1<=93.4983)tmp1=125.2;
        else if(FT1<=93.5877)tmp1=125.3;
        else if(FT1<=93.6772)tmp1=125.4;
        else if(FT1<=93.7667)tmp1=125.5;
        else if(FT1<=93.8563)tmp1=125.6;
        else if(FT1<=93.9459)tmp1=125.7;
        else if(FT1<=94.0356)tmp1=125.8;
        else if(FT1<=94.1253)tmp1=125.9;
        else if(FT1<=94.215)tmp1=126;
    }
    else if(FT1<=95.0247){
        if(FT1==94.215)tmp1=126;
        else if(FT1<=94.3048)tmp1=126.1;
        else if(FT1<=94.3946)tmp1=126.2;
        else if(FT1<=94.4845)tmp1=126.3;
        else if(FT1<=94.5744)tmp1=126.4;
        else if(FT1<=94.6644)tmp1=126.5;
        else if(FT1<=94.7544)tmp1=126.6;
        else if(FT1<=94.8444)tmp1=126.7;
        else if(FT1<=94.9345)tmp1=126.8;
        else if(FT1<=95.0247)tmp1=126.9;
        else if(FT1<=95.1148)tmp1=127;
    }
    else if(FT1<=95.9284){
        if(FT1==95.1148)tmp1=127;
        else if(FT1<=95.2051)tmp1=127.1;
        else if(FT1<=95.2953)tmp1=127.2;
        else if(FT1<=95.3856)tmp1=127.3;
        else if(FT1<=95.476)tmp1=127.4;
        else if(FT1<=95.5664)tmp1=127.5;
        else if(FT1<=95.6568)tmp1=127.6;
        else if(FT1<=95.7473)tmp1=127.7;
        else if(FT1<=95.8378)tmp1=127.8;
        else if(FT1<=95.9284)tmp1=127.9;
        else if(FT1<=96.019)tmp1=128;
    }
    else if(FT1<=96.8364){
        if(FT1==96.019)tmp1=128;
        else if(FT1<=96.1096)tmp1=128.1;
        else if(FT1<=96.2003)tmp1=128.2;
        else if(FT1<=96.2911)tmp1=128.3;
        else if(FT1<=96.3819)tmp1=128.4;
        else if(FT1<=96.4727)tmp1=128.5;
        else if(FT1<=96.5636)tmp1=128.6;
        else if(FT1<=96.6545)tmp1=128.7;
        else if(FT1<=96.7454)tmp1=128.8;
        else if(FT1<=96.8364)tmp1=128.9;
        else if(FT1<=96.9275)tmp1=129;
    }
    else if(FT1<=97.7488){
        if(FT1==96.9275)tmp1=129;
        else if(FT1<=97.0186)tmp1=129.1;
        else if(FT1<=97.1097)tmp1=129.2;
        else if(FT1<=97.2009)tmp1=129.3;
        else if(FT1<=97.2921)tmp1=129.4;
        else if(FT1<=97.3834)tmp1=129.5;
        else if(FT1<=97.4747)tmp1=129.6;
        else if(FT1<=97.566)tmp1=129.7;
        else if(FT1<=97.6574)tmp1=129.8;
        else if(FT1<=97.7488)tmp1=129.9;
        else if(FT1<=97.8403)tmp1=130;
    }
    else if(FT1<=98.6655){
        if(FT1==97.8403)tmp1=130;
        else if(FT1<=97.9318)tmp1=130.1;
        else if(FT1<=98.0234)tmp1=130.2;
        else if(FT1<=98.115)tmp1=130.3;
        else if(FT1<=98.2066)tmp1=130.4;
        else if(FT1<=98.2983)tmp1=130.5;
        else if(FT1<=98.3901)tmp1=130.6;
        else if(FT1<=98.4819)tmp1=130.7;
        else if(FT1<=98.5737)tmp1=130.8;
        else if(FT1<=98.6655)tmp1=130.9;
        else if(FT1<=98.7575)tmp1=131;
    }
    else if(FT1<=99.5866){
        if(FT1==98.7575)tmp1=131;
        else if(FT1<=98.8494)tmp1=131.1;
        else if(FT1<=98.9414)tmp1=131.2;
        else if(FT1<=99.0334)tmp1=131.3;
        else if(FT1<=99.1255)tmp1=131.4;
        else if(FT1<=99.2176)tmp1=131.5;
        else if(FT1<=99.3098)tmp1=131.6;
        else if(FT1<=99.402)tmp1=131.7;
        else if(FT1<=99.4943)tmp1=131.8;
        else if(FT1<=99.5866)tmp1=131.9;
        else if(FT1<=99.6789)tmp1=132;
    }
    else if(FT1<=100.512){
        if(FT1==99.6789)tmp1=132;
        else if(FT1<=99.7713)tmp1=132.1;
        else if(FT1<=99.8637)tmp1=132.2;
        else if(FT1<=99.9562)tmp1=132.3;
        else if(FT1<=100.049)tmp1=132.4;
        else if(FT1<=100.141)tmp1=132.5;
        else if(FT1<=100.234)tmp1=132.6;
        else if(FT1<=100.326)tmp1=132.7;
        else if(FT1<=100.419)tmp1=132.8;
        else if(FT1<=100.512)tmp1=132.9;
        else if(FT1<=100.605)tmp1=133;
    }
    else if(FT1<=101.442){
        if(FT1==100.605)tmp1=133;
        else if(FT1<=100.697)tmp1=133.1;
        else if(FT1<=100.79)tmp1=133.2;
        else if(FT1<=100.883)tmp1=133.3;
        else if(FT1<=100.976)tmp1=133.4;
        else if(FT1<=101.069)tmp1=133.5;
        else if(FT1<=101.162)tmp1=133.6;
        else if(FT1<=101.255)tmp1=133.7;
        else if(FT1<=101.348)tmp1=133.8;
        else if(FT1<=101.442)tmp1=133.9;
        else if(FT1<=101.535)tmp1=134;
    }
    else if(FT1<=102.375){
        if(FT1==101.535)tmp1=134;
        else if(FT1<=101.628)tmp1=134.1;
        else if(FT1<=101.721)tmp1=134.2;
        else if(FT1<=101.815)tmp1=134.3;
        else if(FT1<=101.908)tmp1=134.4;
        else if(FT1<=102.001)tmp1=134.5;
        else if(FT1<=102.095)tmp1=134.6;
        else if(FT1<=102.188)tmp1=134.7;
        else if(FT1<=102.282)tmp1=134.8;
        else if(FT1<=102.375)tmp1=134.9;
        else if(FT1<=102.469)tmp1=135;
    }
    else if(FT1<=103.314){
        if(FT1==102.469)tmp1=135;
        else if(FT1<=102.563)tmp1=135.1;
        else if(FT1<=102.656)tmp1=135.2;
        else if(FT1<=102.75)tmp1=135.3;
        else if(FT1<=102.844)tmp1=135.4;
        else if(FT1<=102.938)tmp1=135.5;
        else if(FT1<=103.032)tmp1=135.6;
        else if(FT1<=103.126)tmp1=135.7;
        else if(FT1<=103.22)tmp1=135.8;
        else if(FT1<=103.314)tmp1=135.9;
        else if(FT1<=103.408)tmp1=136;
    }
    else if(FT1<=104.256){
        if(FT1==103.408)tmp1=136;
        else if(FT1<=103.502)tmp1=136.1;
        else if(FT1<=103.596)tmp1=136.2;
        else if(FT1<=103.69)tmp1=136.3;
        else if(FT1<=103.784)tmp1=136.4;
        else if(FT1<=103.879)tmp1=136.5;
        else if(FT1<=103.973)tmp1=136.6;
        else if(FT1<=104.067)tmp1=136.7;
        else if(FT1<=104.162)tmp1=136.8;
        else if(FT1<=104.256)tmp1=136.9;
        else if(FT1<=104.351)tmp1=137;
    }
    else if(FT1<=105.203){
        if(FT1==104.351)tmp1=137;
        else if(FT1<=104.445)tmp1=137.1;
        else if(FT1<=104.54)tmp1=137.2;
        else if(FT1<=104.634)tmp1=137.3;
        else if(FT1<=104.729)tmp1=137.4;
        else if(FT1<=104.824)tmp1=137.5;
        else if(FT1<=104.918)tmp1=137.6;
        else if(FT1<=105.013)tmp1=137.7;
        else if(FT1<=105.108)tmp1=137.8;
        else if(FT1<=105.203)tmp1=137.9;
        else if(FT1<=105.298)tmp1=138;
    }
    else if(FT1<=106.154){
        if(FT1==105.298)tmp1=138;
        else if(FT1<=105.393)tmp1=138.1;
        else if(FT1<=105.488)tmp1=138.2;
        else if(FT1<=105.583)tmp1=138.3;
        else if(FT1<=105.678)tmp1=138.4;
        else if(FT1<=105.773)tmp1=138.5;
        else if(FT1<=105.868)tmp1=138.6;
        else if(FT1<=105.963)tmp1=138.7;
        else if(FT1<=106.059)tmp1=138.8;
        else if(FT1<=106.154)tmp1=138.9;
        else if(FT1<=106.249)tmp1=139;
    }
    else if(FT1<=107.109){
        if(FT1==106.249)tmp1=139;
        else if(FT1<=106.345)tmp1=139.1;
        else if(FT1<=106.44)tmp1=139.2;
        else if(FT1<=106.536)tmp1=139.3;
        else if(FT1<=106.631)tmp1=139.4;
        else if(FT1<=106.727)tmp1=139.5;
        else if(FT1<=106.822)tmp1=139.6;
        else if(FT1<=106.918)tmp1=139.7;
        else if(FT1<=107.013)tmp1=139.8;
        else if(FT1<=107.109)tmp1=139.9;
        else if(FT1<=107.205)tmp1=140;
    }
    else if(FT1<=108.069){
        if(FT1==107.205)tmp1=140;
        else if(FT1<=107.301)tmp1=140.1;
        else if(FT1<=107.397)tmp1=140.2;
        else if(FT1<=107.492)tmp1=140.3;
        else if(FT1<=107.588)tmp1=140.4;
        else if(FT1<=107.684)tmp1=140.5;
        else if(FT1<=107.78)tmp1=140.6;
        else if(FT1<=107.876)tmp1=140.7;
        else if(FT1<=107.973)tmp1=140.8;
        else if(FT1<=108.069)tmp1=140.9;
        else if(FT1<=108.165)tmp1=141;
    }
    else if(FT1<=109.032){
        if(FT1==108.165)tmp1=141;
        else if(FT1<=108.261)tmp1=141.1;
        else if(FT1<=108.357)tmp1=141.2;
        else if(FT1<=108.454)tmp1=141.3;
        else if(FT1<=108.55)tmp1=141.4;
        else if(FT1<=108.646)tmp1=141.5;
        else if(FT1<=108.743)tmp1=141.6;
        else if(FT1<=108.839)tmp1=141.7;
        else if(FT1<=108.936)tmp1=141.8;
        else if(FT1<=109.032)tmp1=141.9;
        else if(FT1<=109.129)tmp1=142;
    }
    else if(FT1<=110){
        if(FT1==109.129)tmp1=142;
        else if(FT1<=109.226)tmp1=142.1;
        else if(FT1<=109.322)tmp1=142.2;
        else if(FT1<=109.419)tmp1=142.3;
        else if(FT1<=109.516)tmp1=142.4;
        else if(FT1<=109.613)tmp1=142.5;
        else if(FT1<=109.71)tmp1=142.6;
        else if(FT1<=109.806)tmp1=142.7;
        else if(FT1<=109.903)tmp1=142.8;
        else if(FT1<=110)tmp1=142.9;
        else if(FT1<=110.097)tmp1=143;
    }
    else if(FT1<=110.973){
        if(FT1==110.097)tmp1=143;
        else if(FT1<=110.195)tmp1=143.1;
        else if(FT1<=110.292)tmp1=143.2;
        else if(FT1<=110.389)tmp1=143.3;
        else if(FT1<=110.486)tmp1=143.4;
        else if(FT1<=110.583)tmp1=143.5;
        else if(FT1<=110.68)tmp1=143.6;
        else if(FT1<=110.778)tmp1=143.7;
        else if(FT1<=110.875)tmp1=143.8;
        else if(FT1<=110.973)tmp1=143.9;
        else if(FT1<=111.07)tmp1=144;
    }
    else if(FT1<=111.949){
        if(FT1==111.07)tmp1=144;
        else if(FT1<=111.168)tmp1=144.1;
        else if(FT1<=111.265)tmp1=144.2;
        else if(FT1<=111.363)tmp1=144.3;
        else if(FT1<=111.46)tmp1=144.4;
        else if(FT1<=111.558)tmp1=144.5;
        else if(FT1<=111.656)tmp1=144.6;
        else if(FT1<=111.753)tmp1=144.7;
        else if(FT1<=111.851)tmp1=144.8;
        else if(FT1<=111.949)tmp1=144.9;
        else if(FT1<=112.047)tmp1=145;
    }
    else if(FT1<=112.93){
        if(FT1==112.047)tmp1=145;
        else if(FT1<=112.145)tmp1=145.1;
        else if(FT1<=112.243)tmp1=145.2;
        else if(FT1<=112.341)tmp1=145.3;
        else if(FT1<=112.439)tmp1=145.4;
        else if(FT1<=112.537)tmp1=145.5;
        else if(FT1<=112.635)tmp1=145.6;
        else if(FT1<=112.733)tmp1=145.7;
        else if(FT1<=112.831)tmp1=145.8;
        else if(FT1<=112.93)tmp1=145.9;
        else if(FT1<=113.028)tmp1=146;
    }
    else if(FT1<=113.914){
        if(FT1==113.028)tmp1=146;
        else if(FT1<=113.126)tmp1=146.1;
        else if(FT1<=113.225)tmp1=146.2;
        else if(FT1<=113.323)tmp1=146.3;
        else if(FT1<=113.421)tmp1=146.4;
        else if(FT1<=113.52)tmp1=146.5;
        else if(FT1<=113.619)tmp1=146.6;
        else if(FT1<=113.717)tmp1=146.7;
        else if(FT1<=113.816)tmp1=146.8;
        else if(FT1<=113.914)tmp1=146.9;
        else if(FT1<=114.013)tmp1=147;
    }
    else if(FT1<=114.903){
        if(FT1==114.013)tmp1=147;
        else if(FT1<=114.112)tmp1=147.1;
        else if(FT1<=114.211)tmp1=147.2;
        else if(FT1<=114.309)tmp1=147.3;
        else if(FT1<=114.408)tmp1=147.4;
        else if(FT1<=114.507)tmp1=147.5;
        else if(FT1<=114.606)tmp1=147.6;
        else if(FT1<=114.705)tmp1=147.7;
        else if(FT1<=114.804)tmp1=147.8;
        else if(FT1<=114.903)tmp1=147.9;
        else if(FT1<=115.003)tmp1=148;
    }
    else if(FT1<=115.897){
        if(FT1==115.003)tmp1=148;
        else if(FT1<=115.102)tmp1=148.1;
        else if(FT1<=115.201)tmp1=148.2;
        else if(FT1<=115.3)tmp1=148.3;
        else if(FT1<=115.399)tmp1=148.4;
        else if(FT1<=115.499)tmp1=148.5;
        else if(FT1<=115.598)tmp1=148.6;
        else if(FT1<=115.698)tmp1=148.7;
        else if(FT1<=115.797)tmp1=148.8;
        else if(FT1<=115.897)tmp1=148.9;
        else if(FT1<=115.996)tmp1=149;
    }
    else if(FT1<=116.894){
        if(FT1==115.996)tmp1=149;
        else if(FT1<=116.096)tmp1=149.1;
        else if(FT1<=116.195)tmp1=149.2;
        else if(FT1<=116.295)tmp1=149.3;
        else if(FT1<=116.395)tmp1=149.4;
        else if(FT1<=116.494)tmp1=149.5;
        else if(FT1<=116.594)tmp1=149.6;
        else if(FT1<=116.694)tmp1=149.7;
        else if(FT1<=116.794)tmp1=149.8;
        else if(FT1<=116.894)tmp1=149.9;
        else if(FT1<=116.994)tmp1=150;
    }
    else if(FT1<=117.895){
        if(FT1==116.994)tmp1=150;
        else if(FT1<=117.094)tmp1=150.1;
        else if(FT1<=117.194)tmp1=150.2;
        else if(FT1<=117.294)tmp1=150.3;
        else if(FT1<=117.394)tmp1=150.4;
        else if(FT1<=117.494)tmp1=150.5;
        else if(FT1<=117.594)tmp1=150.6;
        else if(FT1<=117.695)tmp1=150.7;
        else if(FT1<=117.795)tmp1=150.8;
        else if(FT1<=117.895)tmp1=150.9;
        else if(FT1<=117.996)tmp1=151;
    }
    else if(FT1<=118.901){
        if(FT1==117.996)tmp1=151;
        else if(FT1<=118.096)tmp1=151.1;
        else if(FT1<=118.197)tmp1=151.2;
        else if(FT1<=118.297)tmp1=151.3;
        else if(FT1<=118.398)tmp1=151.4;
        else if(FT1<=118.498)tmp1=151.5;
        else if(FT1<=118.599)tmp1=151.6;
        else if(FT1<=118.7)tmp1=151.7;
        else if(FT1<=118.8)tmp1=151.8;
        else if(FT1<=118.901)tmp1=151.9;
        else if(FT1<=119.002)tmp1=152;
    }
    else if(FT1<=119.911){
        if(FT1==119.002)tmp1=152;
        else if(FT1<=119.103)tmp1=152.1;
        else if(FT1<=119.203)tmp1=152.2;
        else if(FT1<=119.304)tmp1=152.3;
        else if(FT1<=119.405)tmp1=152.4;
        else if(FT1<=119.506)tmp1=152.5;
        else if(FT1<=119.607)tmp1=152.6;
        else if(FT1<=119.708)tmp1=152.7;
        else if(FT1<=119.81)tmp1=152.8;
        else if(FT1<=119.911)tmp1=152.9;
        else if(FT1<=120.012)tmp1=153;
    }
    else if(FT1<=120.925){
        if(FT1==120.012)tmp1=153;
        else if(FT1<=120.113)tmp1=153.1;
        else if(FT1<=120.215)tmp1=153.2;
        else if(FT1<=120.316)tmp1=153.3;
        else if(FT1<=120.417)tmp1=153.4;
        else if(FT1<=120.519)tmp1=153.5;
        else if(FT1<=120.62)tmp1=153.6;
        else if(FT1<=120.722)tmp1=153.7;
        else if(FT1<=120.823)tmp1=153.8;
        else if(FT1<=120.925)tmp1=153.9;
        else if(FT1<=121.026)tmp1=154;
    }
    else if(FT1<=121.943){
        if(FT1==121.026)tmp1=154;
        else if(FT1<=121.128)tmp1=154.1;
        else if(FT1<=121.23)tmp1=154.2;
        else if(FT1<=121.331)tmp1=154.3;
        else if(FT1<=121.433)tmp1=154.4;
        else if(FT1<=121.535)tmp1=154.5;
        else if(FT1<=121.637)tmp1=154.6;
        else if(FT1<=121.739)tmp1=154.7;
        else if(FT1<=121.841)tmp1=154.8;
        else if(FT1<=121.943)tmp1=154.9;
        else if(FT1<=122.045)tmp1=155;
    }
    else if(FT1<=122.965){
        if(FT1==122.045)tmp1=155;
        else if(FT1<=122.147)tmp1=155.1;
        else if(FT1<=122.249)tmp1=155.2;
        else if(FT1<=122.351)tmp1=155.3;
        else if(FT1<=122.453)tmp1=155.4;
        else if(FT1<=122.556)tmp1=155.5;
        else if(FT1<=122.658)tmp1=155.6;
        else if(FT1<=122.76)tmp1=155.7;
        else if(FT1<=122.862)tmp1=155.8;
        else if(FT1<=122.965)tmp1=155.9;
        else if(FT1<=123.067)tmp1=156;
    }
    else if(FT1<=123.991){
        if(FT1==123.067)tmp1=156;
        else if(FT1<=123.17)tmp1=156.1;
        else if(FT1<=123.272)tmp1=156.2;
        else if(FT1<=123.375)tmp1=156.3;
        else if(FT1<=123.477)tmp1=156.4;
        else if(FT1<=123.58)tmp1=156.5;
        else if(FT1<=123.683)tmp1=156.6;
        else if(FT1<=123.786)tmp1=156.7;
        else if(FT1<=123.888)tmp1=156.8;
        else if(FT1<=123.991)tmp1=156.9;
        else if(FT1<=124.094)tmp1=157;
    }
    else if(FT1<=125.022){
        if(FT1==124.094)tmp1=157;
        else if(FT1<=124.197)tmp1=157.1;
        else if(FT1<=124.3)tmp1=157.2;
        else if(FT1<=124.403)tmp1=157.3;
        else if(FT1<=124.506)tmp1=157.4;
        else if(FT1<=124.609)tmp1=157.5;
        else if(FT1<=124.712)tmp1=157.6;
        else if(FT1<=124.815)tmp1=157.7;
        else if(FT1<=124.918)tmp1=157.8;
        else if(FT1<=125.022)tmp1=157.9;
        else if(FT1<=125.125)tmp1=158;
    }
    else if(FT1<=126.056){
        if(FT1==125.125)tmp1=158;
        else if(FT1<=125.228)tmp1=158.1;
        else if(FT1<=125.331)tmp1=158.2;
        else if(FT1<=125.435)tmp1=158.3;
        else if(FT1<=125.538)tmp1=158.4;
        else if(FT1<=125.642)tmp1=158.5;
        else if(FT1<=125.745)tmp1=158.6;
        else if(FT1<=125.849)tmp1=158.7;
        else if(FT1<=125.952)tmp1=158.8;
        else if(FT1<=126.056)tmp1=158.9;
        else if(FT1<=126.16)tmp1=159;
    }
    else if(FT1<=127.094){
        if(FT1==126.16)tmp1=159;
        else if(FT1<=126.263)tmp1=159.1;
        else if(FT1<=126.367)tmp1=159.2;
        else if(FT1<=126.471)tmp1=159.3;
        else if(FT1<=126.575)tmp1=159.4;
        else if(FT1<=126.679)tmp1=159.5;
        else if(FT1<=126.782)tmp1=159.6;
        else if(FT1<=126.886)tmp1=159.7;
        else if(FT1<=126.99)tmp1=159.8;
        else if(FT1<=127.094)tmp1=159.9;
        else if(FT1<=127.199)tmp1=160;
    }
    else if(FT1<=128.137){
        if(FT1==127.199)tmp1=160;
        else if(FT1<=127.303)tmp1=160.1;
        else if(FT1<=127.407)tmp1=160.2;
        else if(FT1<=127.511)tmp1=160.3;
        else if(FT1<=127.615)tmp1=160.4;
        else if(FT1<=127.72)tmp1=160.5;
        else if(FT1<=127.824)tmp1=160.6;
        else if(FT1<=127.928)tmp1=160.7;
        else if(FT1<=128.033)tmp1=160.8;
        else if(FT1<=128.137)tmp1=160.9;
        else if(FT1<=128.242)tmp1=161;
    }
    else if(FT1<=129.184){
        if(FT1==128.242)tmp1=161;
        else if(FT1<=128.346)tmp1=161.1;
        else if(FT1<=128.451)tmp1=161.2;
        else if(FT1<=128.555)tmp1=161.3;
        else if(FT1<=128.66)tmp1=161.4;
        else if(FT1<=128.765)tmp1=161.5;
        else if(FT1<=128.869)tmp1=161.6;
        else if(FT1<=128.974)tmp1=161.7;
        else if(FT1<=129.079)tmp1=161.8;
        else if(FT1<=129.184)tmp1=161.9;
        else if(FT1<=129.289)tmp1=162;
    }
    else if(FT1<=130.234){
        if(FT1==129.289)tmp1=162;
        else if(FT1<=129.394)tmp1=162.1;
        else if(FT1<=129.499)tmp1=162.2;
        else if(FT1<=129.604)tmp1=162.3;
        else if(FT1<=129.709)tmp1=162.4;
        else if(FT1<=129.814)tmp1=162.5;
        else if(FT1<=129.919)tmp1=162.6;
        else if(FT1<=130.024)tmp1=162.7;
        else if(FT1<=130.129)tmp1=162.8;
        else if(FT1<=130.234)tmp1=162.9;
        else if(FT1<=130.34)tmp1=163;
    }
    else if(FT1<=131.289){
        if(FT1==130.34)tmp1=163;
        else if(FT1<=130.445)tmp1=163.1;
        else if(FT1<=130.55)tmp1=163.2;
        else if(FT1<=130.656)tmp1=163.3;
        else if(FT1<=130.761)tmp1=163.4;
        else if(FT1<=130.867)tmp1=163.5;
        else if(FT1<=130.972)tmp1=163.6;
        else if(FT1<=131.078)tmp1=163.7;
        else if(FT1<=131.184)tmp1=163.8;
        else if(FT1<=131.289)tmp1=163.9;
        else if(FT1<=131.395)tmp1=164;
    }
    else if(FT1<=132.348){
        if(FT1==131.395)tmp1=164;
        else if(FT1<=131.501)tmp1=164.1;
        else if(FT1<=131.606)tmp1=164.2;
        else if(FT1<=131.712)tmp1=164.3;
        else if(FT1<=131.818)tmp1=164.4;
        else if(FT1<=131.924)tmp1=164.5;
        else if(FT1<=132.03)tmp1=164.6;
        else if(FT1<=132.136)tmp1=164.7;
        else if(FT1<=132.242)tmp1=164.8;
        else if(FT1<=132.348)tmp1=164.9;
        else if(FT1<=132.454)tmp1=165;
    }
    else if(FT1<=133.411){
        if(FT1==132.454)tmp1=165;
        else if(FT1<=132.56)tmp1=165.1;
        else if(FT1<=132.666)tmp1=165.2;
        else if(FT1<=132.773)tmp1=165.3;
        else if(FT1<=132.879)tmp1=165.4;
        else if(FT1<=132.985)tmp1=165.5;
        else if(FT1<=133.092)tmp1=165.6;
        else if(FT1<=133.198)tmp1=165.7;
        else if(FT1<=133.304)tmp1=165.8;
        else if(FT1<=133.411)tmp1=165.9;
        else if(FT1<=133.517)tmp1=166;
    }
    else if(FT1<=134.478){
        if(FT1==133.517)tmp1=166;
        else if(FT1<=133.624)tmp1=166.1;
        else if(FT1<=133.731)tmp1=166.2;
        else if(FT1<=133.837)tmp1=166.3;
        else if(FT1<=133.944)tmp1=166.4;
        else if(FT1<=134.05)tmp1=166.5;
        else if(FT1<=134.157)tmp1=166.6;
        else if(FT1<=134.264)tmp1=166.7;
        else if(FT1<=134.371)tmp1=166.8;
        else if(FT1<=134.478)tmp1=166.9;
        else if(FT1<=134.585)tmp1=167;
    }
    else if(FT1<=135.549){
        if(FT1==134.585)tmp1=167;
        else if(FT1<=134.692)tmp1=167.1;
        else if(FT1<=134.799)tmp1=167.2;
        else if(FT1<=134.906)tmp1=167.3;
        else if(FT1<=135.013)tmp1=167.4;
        else if(FT1<=135.12)tmp1=167.5;
        else if(FT1<=135.227)tmp1=167.6;
        else if(FT1<=135.334)tmp1=167.7;
        else if(FT1<=135.441)tmp1=167.8;
        else if(FT1<=135.549)tmp1=167.9;
        else if(FT1<=135.656)tmp1=168;
    }
    else if(FT1<=136.623){
        if(FT1==135.656)tmp1=168;
        else if(FT1<=135.763)tmp1=168.1;
        else if(FT1<=135.871)tmp1=168.2;
        else if(FT1<=135.978)tmp1=168.3;
        else if(FT1<=136.085)tmp1=168.4;
        else if(FT1<=136.193)tmp1=168.5;
        else if(FT1<=136.301)tmp1=168.6;
        else if(FT1<=136.408)tmp1=168.7;
        else if(FT1<=136.516)tmp1=168.8;
        else if(FT1<=136.623)tmp1=168.9;
        else if(FT1<=136.731)tmp1=169;
    }
    else if(FT1<=137.702){
        if(FT1==136.731)tmp1=169;
        else if(FT1<=136.839)tmp1=169.1;
        else if(FT1<=136.947)tmp1=169.2;
        else if(FT1<=137.054)tmp1=169.3;
        else if(FT1<=137.162)tmp1=169.4;
        else if(FT1<=137.27)tmp1=169.5;
        else if(FT1<=137.378)tmp1=169.6;
        else if(FT1<=137.486)tmp1=169.7;
        else if(FT1<=137.594)tmp1=169.8;
        else if(FT1<=137.702)tmp1=169.9;
        else if(FT1<=137.81)tmp1=170;
    }
    else if(FT1<=138.785){
        if(FT1==137.81)tmp1=170;
        else if(FT1<=137.919)tmp1=170.1;
        else if(FT1<=138.027)tmp1=170.2;
        else if(FT1<=138.135)tmp1=170.3;
        else if(FT1<=138.243)tmp1=170.4;
        else if(FT1<=138.351)tmp1=170.5;
        else if(FT1<=138.46)tmp1=170.6;
        else if(FT1<=138.568)tmp1=170.7;
        else if(FT1<=138.677)tmp1=170.8;
        else if(FT1<=138.785)tmp1=170.9;
        else if(FT1<=138.894)tmp1=171;
    }
    else if(FT1<=139.872){
        if(FT1==138.894)tmp1=171;
        else if(FT1<=139.002)tmp1=171.1;
        else if(FT1<=139.111)tmp1=171.2;
        else if(FT1<=139.219)tmp1=171.3;
        else if(FT1<=139.328)tmp1=171.4;
        else if(FT1<=139.437)tmp1=171.5;
        else if(FT1<=139.545)tmp1=171.6;
        else if(FT1<=139.654)tmp1=171.7;
        else if(FT1<=139.763)tmp1=171.8;
        else if(FT1<=139.872)tmp1=171.9;
        else if(FT1<=139.981)tmp1=172;
    }
    else if(FT1<=140.963){
        if(FT1==139.981)tmp1=172;
        else if(FT1<=140.09)tmp1=172.1;
        else if(FT1<=140.199)tmp1=172.2;
        else if(FT1<=140.308)tmp1=172.3;
        else if(FT1<=140.417)tmp1=172.4;
        else if(FT1<=140.526)tmp1=172.5;
        else if(FT1<=140.635)tmp1=172.6;
        else if(FT1<=140.744)tmp1=172.7;
        else if(FT1<=140.853)tmp1=172.8;
        else if(FT1<=140.963)tmp1=172.9;
        else if(FT1<=141.072)tmp1=173;
    }
    else if(FT1<=142.057){
        if(FT1==141.072)tmp1=173;
        else if(FT1<=141.181)tmp1=173.1;
        else if(FT1<=141.291)tmp1=173.2;
        else if(FT1<=141.4)tmp1=173.3;
        else if(FT1<=141.51)tmp1=173.4;
        else if(FT1<=141.619)tmp1=173.5;
        else if(FT1<=141.729)tmp1=173.6;
        else if(FT1<=141.838)tmp1=173.7;
        else if(FT1<=141.948)tmp1=173.8;
        else if(FT1<=142.057)tmp1=173.9;
        else if(FT1<=142.167)tmp1=174;
    }
    else if(FT1<=143.156){
        if(FT1==142.167)tmp1=174;
        else if(FT1<=142.277)tmp1=174.1;
        else if(FT1<=142.387)tmp1=174.2;
        else if(FT1<=142.496)tmp1=174.3;
        else if(FT1<=142.606)tmp1=174.4;
        else if(FT1<=142.716)tmp1=174.5;
        else if(FT1<=142.826)tmp1=174.6;
        else if(FT1<=142.936)tmp1=174.7;
        else if(FT1<=143.046)tmp1=174.8;
        else if(FT1<=143.156)tmp1=174.9;
        else if(FT1<=143.266)tmp1=175;
    }
    else if(FT1<=144.259){
        if(FT1==143.266)tmp1=175;
        else if(FT1<=143.376)tmp1=175.1;
        else if(FT1<=143.486)tmp1=175.2;
        else if(FT1<=143.597)tmp1=175.3;
        else if(FT1<=143.707)tmp1=175.4;
        else if(FT1<=143.817)tmp1=175.5;
        else if(FT1<=143.927)tmp1=175.6;
        else if(FT1<=144.038)tmp1=175.7;
        else if(FT1<=144.148)tmp1=175.8;
        else if(FT1<=144.259)tmp1=175.9;
        else if(FT1<=144.369)tmp1=176;
    }
    else if(FT1<=145.365){
        if(FT1==144.369)tmp1=176;
        else if(FT1<=144.48)tmp1=176.1;
        else if(FT1<=144.59)tmp1=176.2;
        else if(FT1<=144.701)tmp1=176.3;
        else if(FT1<=144.811)tmp1=176.4;
        else if(FT1<=144.922)tmp1=176.5;
        else if(FT1<=145.033)tmp1=176.6;
        else if(FT1<=145.144)tmp1=176.7;
        else if(FT1<=145.254)tmp1=176.8;
        else if(FT1<=145.365)tmp1=176.9;
        else if(FT1<=145.476)tmp1=177;
    }
    else if(FT1<=146.476){
        if(FT1==145.476)tmp1=177;
        else if(FT1<=145.587)tmp1=177.1;
        else if(FT1<=145.698)tmp1=177.2;
        else if(FT1<=145.809)tmp1=177.3;
        else if(FT1<=145.92)tmp1=177.4;
        else if(FT1<=146.031)tmp1=177.5;
        else if(FT1<=146.142)tmp1=177.6;
        else if(FT1<=146.253)tmp1=177.7;
        else if(FT1<=146.364)tmp1=177.8;
        else if(FT1<=146.476)tmp1=177.9;
        else if(FT1<=146.587)tmp1=178;
    }
    else if(FT1<=147.59){
        if(FT1==146.587)tmp1=178;
        else if(FT1<=146.698)tmp1=178.1;
        else if(FT1<=146.809)tmp1=178.2;
        else if(FT1<=146.921)tmp1=178.3;
        else if(FT1<=147.032)tmp1=178.4;
        else if(FT1<=147.144)tmp1=178.5;
        else if(FT1<=147.255)tmp1=178.6;
        else if(FT1<=147.367)tmp1=178.7;
        else if(FT1<=147.478)tmp1=178.8;
        else if(FT1<=147.59)tmp1=178.9;
        else if(FT1<=147.702)tmp1=179;
    }
    else if(FT1<=148.708){
        if(FT1==147.702)tmp1=179;
        else if(FT1<=147.813)tmp1=179.1;
        else if(FT1<=147.925)tmp1=179.2;
        else if(FT1<=148.037)tmp1=179.3;
        else if(FT1<=148.149)tmp1=179.4;
        else if(FT1<=148.26)tmp1=179.5;
        else if(FT1<=148.372)tmp1=179.6;
        else if(FT1<=148.484)tmp1=179.7;
        else if(FT1<=148.596)tmp1=179.8;
        else if(FT1<=148.708)tmp1=179.9;
        else if(FT1<=148.82)tmp1=180;
    }
    else if(FT1<=149.83){
        if(FT1==148.82)tmp1=180;
        else if(FT1<=148.932)tmp1=180.1;
        else if(FT1<=149.044)tmp1=180.2;
        else if(FT1<=149.157)tmp1=180.3;
        else if(FT1<=149.269)tmp1=180.4;
        else if(FT1<=149.381)tmp1=180.5;
        else if(FT1<=149.493)tmp1=180.6;
        else if(FT1<=149.606)tmp1=180.7;
        else if(FT1<=149.718)tmp1=180.8;
        else if(FT1<=149.83)tmp1=180.9;
        else if(FT1<=149.943)tmp1=181;
    }
    else if(FT1<=150.956){
        if(FT1==149.943)tmp1=181;
        else if(FT1<=150.055)tmp1=181.1;
        else if(FT1<=150.168)tmp1=181.2;
        else if(FT1<=150.28)tmp1=181.3;
        else if(FT1<=150.393)tmp1=181.4;
        else if(FT1<=150.505)tmp1=181.5;
        else if(FT1<=150.618)tmp1=181.6;
        else if(FT1<=150.731)tmp1=181.7;
        else if(FT1<=150.843)tmp1=181.8;
        else if(FT1<=150.956)tmp1=181.9;
        else if(FT1<=151.069)tmp1=182;
    }
    else if(FT1<=152.086){
        if(FT1==151.069)tmp1=182;
        else if(FT1<=151.182)tmp1=182.1;
        else if(FT1<=151.295)tmp1=182.2;
        else if(FT1<=151.408)tmp1=182.3;
        else if(FT1<=151.521)tmp1=182.4;
        else if(FT1<=151.634)tmp1=182.5;
        else if(FT1<=151.747)tmp1=182.6;
        else if(FT1<=151.86)tmp1=182.7;
        else if(FT1<=151.973)tmp1=182.8;
        else if(FT1<=152.086)tmp1=182.9;
        else if(FT1<=152.199)tmp1=183;
    }
    else if(FT1<=153.22){
        if(FT1==152.199)tmp1=183;
        else if(FT1<=152.313)tmp1=183.1;
        else if(FT1<=152.426)tmp1=183.2;
        else if(FT1<=152.539)tmp1=183.3;
        else if(FT1<=152.653)tmp1=183.4;
        else if(FT1<=152.766)tmp1=183.5;
        else if(FT1<=152.879)tmp1=183.6;
        else if(FT1<=152.993)tmp1=183.7;
        else if(FT1<=153.106)tmp1=183.8;
        else if(FT1<=153.22)tmp1=183.9;
        else if(FT1<=153.333)tmp1=184;
    }
    else if(FT1<=154.357){
        if(FT1==153.333)tmp1=184;
        else if(FT1<=153.447)tmp1=184.1;
        else if(FT1<=153.561)tmp1=184.2;
        else if(FT1<=153.674)tmp1=184.3;
        else if(FT1<=153.788)tmp1=184.4;
        else if(FT1<=153.902)tmp1=184.5;
        else if(FT1<=154.016)tmp1=184.6;
        else if(FT1<=154.13)tmp1=184.7;
        else if(FT1<=154.244)tmp1=184.8;
        else if(FT1<=154.357)tmp1=184.9;
        else if(FT1<=154.471)tmp1=185;
    }
    else if(FT1<=155.499){
        if(FT1==154.471)tmp1=185;
        else if(FT1<=154.585)tmp1=185.1;
        else if(FT1<=154.699)tmp1=185.2;
        else if(FT1<=154.814)tmp1=185.3;
        else if(FT1<=154.928)tmp1=185.4;
        else if(FT1<=155.042)tmp1=185.5;
        else if(FT1<=155.156)tmp1=185.6;
        else if(FT1<=155.27)tmp1=185.7;
        else if(FT1<=155.385)tmp1=185.8;
        else if(FT1<=155.499)tmp1=185.9;
        else if(FT1<=155.613)tmp1=186;
    }
    else if(FT1<=156.644){
        if(FT1==155.613)tmp1=186;
        else if(FT1<=155.728)tmp1=186.1;
        else if(FT1<=155.842)tmp1=186.2;
        else if(FT1<=155.957)tmp1=186.3;
        else if(FT1<=156.071)tmp1=186.4;
        else if(FT1<=156.186)tmp1=186.5;
        else if(FT1<=156.3)tmp1=186.6;
        else if(FT1<=156.415)tmp1=186.7;
        else if(FT1<=156.529)tmp1=186.8;
        else if(FT1<=156.644)tmp1=186.9;
        else if(FT1<=156.759)tmp1=187;
    }
    else if(FT1<=157.793){
        if(FT1==156.759)tmp1=187;
        else if(FT1<=156.874)tmp1=187.1;
        else if(FT1<=156.988)tmp1=187.2;
        else if(FT1<=157.103)tmp1=187.3;
        else if(FT1<=157.218)tmp1=187.4;
        else if(FT1<=157.333)tmp1=187.5;
        else if(FT1<=157.448)tmp1=187.6;
        else if(FT1<=157.563)tmp1=187.7;
        else if(FT1<=157.678)tmp1=187.8;
        else if(FT1<=157.793)tmp1=187.9;
        else if(FT1<=157.908)tmp1=188;
    }
    else if(FT1<=158.946){
        if(FT1==157.908)tmp1=188;
        else if(FT1<=158.023)tmp1=188.1;
        else if(FT1<=158.139)tmp1=188.2;
        else if(FT1<=158.254)tmp1=188.3;
        else if(FT1<=158.369)tmp1=188.4;
        else if(FT1<=158.484)tmp1=188.5;
        else if(FT1<=158.6)tmp1=188.6;
        else if(FT1<=158.715)tmp1=188.7;
        else if(FT1<=158.831)tmp1=188.8;
        else if(FT1<=158.946)tmp1=188.9;
        else if(FT1<=159.062)tmp1=189;
    }
    else if(FT1<=160.103){
        if(FT1==159.062)tmp1=189;
        else if(FT1<=159.177)tmp1=189.1;
        else if(FT1<=159.293)tmp1=189.2;
        else if(FT1<=159.408)tmp1=189.3;
        else if(FT1<=159.524)tmp1=189.4;
        else if(FT1<=159.64)tmp1=189.5;
        else if(FT1<=159.755)tmp1=189.6;
        else if(FT1<=159.871)tmp1=189.7;
        else if(FT1<=159.987)tmp1=189.8;
        else if(FT1<=160.103)tmp1=189.9;
        else if(FT1<=160.219)tmp1=190;
    }
    else if(FT1<=161.263){
        if(FT1==160.219)tmp1=190;
        else if(FT1<=160.335)tmp1=190.1;
        else if(FT1<=160.451)tmp1=190.2;
        else if(FT1<=160.567)tmp1=190.3;
        else if(FT1<=160.683)tmp1=190.4;
        else if(FT1<=160.799)tmp1=190.5;
        else if(FT1<=160.915)tmp1=190.6;
        else if(FT1<=161.031)tmp1=190.7;
        else if(FT1<=161.147)tmp1=190.8;
        else if(FT1<=161.263)tmp1=190.9;
        else if(FT1<=161.38)tmp1=191;
    }
    else if(FT1<=162.428){
        if(FT1==161.38)tmp1=191;
        else if(FT1<=161.496)tmp1=191.1;
        else if(FT1<=161.612)tmp1=191.2;
        else if(FT1<=161.729)tmp1=191.3;
        else if(FT1<=161.845)tmp1=191.4;
        else if(FT1<=161.961)tmp1=191.5;
        else if(FT1<=162.078)tmp1=191.6;
        else if(FT1<=162.194)tmp1=191.7;
        else if(FT1<=162.311)tmp1=191.8;
        else if(FT1<=162.428)tmp1=191.9;
        else if(FT1<=162.544)tmp1=192;
    }
    else if(FT1<=163.596){
        if(FT1==162.544)tmp1=192;
        else if(FT1<=162.661)tmp1=192.1;
        else if(FT1<=162.778)tmp1=192.2;
        else if(FT1<=162.894)tmp1=192.3;
        else if(FT1<=163.011)tmp1=192.4;
        else if(FT1<=163.128)tmp1=192.5;
        else if(FT1<=163.245)tmp1=192.6;
        else if(FT1<=163.362)tmp1=192.7;
        else if(FT1<=163.479)tmp1=192.8;
        else if(FT1<=163.596)tmp1=192.9;
        else if(FT1<=163.713)tmp1=193;
    }
    else if(FT1<=164.767){
        if(FT1==163.713)tmp1=193;
        else if(FT1<=163.83)tmp1=193.1;
        else if(FT1<=163.947)tmp1=193.2;
        else if(FT1<=164.064)tmp1=193.3;
        else if(FT1<=164.181)tmp1=193.4;
        else if(FT1<=164.298)tmp1=193.5;
        else if(FT1<=164.415)tmp1=193.6;
        else if(FT1<=164.533)tmp1=193.7;
        else if(FT1<=164.65)tmp1=193.8;
        else if(FT1<=164.767)tmp1=193.9;
        else if(FT1<=164.885)tmp1=194;
    }
    else if(FT1<=165.943){
        if(FT1==164.885)tmp1=194;
        else if(FT1<=165.002)tmp1=194.1;
        else if(FT1<=165.12)tmp1=194.2;
        else if(FT1<=165.237)tmp1=194.3;
        else if(FT1<=165.355)tmp1=194.4;
        else if(FT1<=165.472)tmp1=194.5;
        else if(FT1<=165.59)tmp1=194.6;
        else if(FT1<=165.708)tmp1=194.7;
        else if(FT1<=165.825)tmp1=194.8;
        else if(FT1<=165.943)tmp1=194.9;
        else if(FT1<=166.061)tmp1=195;
    }
    else if(FT1<=167.122){
        if(FT1==166.061)tmp1=195;
        else if(FT1<=166.179)tmp1=195.1;
        else if(FT1<=166.296)tmp1=195.2;
        else if(FT1<=166.414)tmp1=195.3;
        else if(FT1<=166.532)tmp1=195.4;
        else if(FT1<=166.65)tmp1=195.5;
        else if(FT1<=166.768)tmp1=195.6;
        else if(FT1<=166.886)tmp1=195.7;
        else if(FT1<=167.004)tmp1=195.8;
        else if(FT1<=167.122)tmp1=195.9;
        else if(FT1<=167.24)tmp1=196;
    }
    else if(FT1<=168.305){
        if(FT1==167.24)tmp1=196;
        else if(FT1<=167.359)tmp1=196.1;
        else if(FT1<=167.477)tmp1=196.2;
        else if(FT1<=167.595)tmp1=196.3;
        else if(FT1<=167.713)tmp1=196.4;
        else if(FT1<=167.832)tmp1=196.5;
        else if(FT1<=167.95)tmp1=196.6;
        else if(FT1<=168.068)tmp1=196.7;
        else if(FT1<=168.187)tmp1=196.8;
        else if(FT1<=168.305)tmp1=196.9;
        else if(FT1<=168.424)tmp1=197;
    }
    else if(FT1<=169.492){
        if(FT1==168.424)tmp1=197;
        else if(FT1<=168.542)tmp1=197.1;
        else if(FT1<=168.661)tmp1=197.2;
        else if(FT1<=168.78)tmp1=197.3;
        else if(FT1<=168.898)tmp1=197.4;
        else if(FT1<=169.017)tmp1=197.5;
        else if(FT1<=169.136)tmp1=197.6;
        else if(FT1<=169.254)tmp1=197.7;
        else if(FT1<=169.373)tmp1=197.8;
        else if(FT1<=169.492)tmp1=197.9;
        else if(FT1<=169.611)tmp1=198;
    }
    else if(FT1<=170.683){
        if(FT1==169.611)tmp1=198;
        else if(FT1<=169.73)tmp1=198.1;
        else if(FT1<=169.849)tmp1=198.2;
        else if(FT1<=169.968)tmp1=198.3;
        else if(FT1<=170.087)tmp1=198.4;
        else if(FT1<=170.206)tmp1=198.5;
        else if(FT1<=170.325)tmp1=198.6;
        else if(FT1<=170.444)tmp1=198.7;
        else if(FT1<=170.563)tmp1=198.8;
        else if(FT1<=170.683)tmp1=198.9;
        else if(FT1<=170.802)tmp1=199;
    }
    else if(FT1<=171.877){
        if(FT1==170.802)tmp1=199;
        else if(FT1<=170.921)tmp1=199.1;
        else if(FT1<=171.04)tmp1=199.2;
        else if(FT1<=171.16)tmp1=199.3;
        else if(FT1<=171.279)tmp1=199.4;
        else if(FT1<=171.399)tmp1=199.5;
        else if(FT1<=171.518)tmp1=199.6;
        else if(FT1<=171.638)tmp1=199.7;
        else if(FT1<=171.757)tmp1=199.8;
        else if(FT1<=171.877)tmp1=199.9;
        else if(FT1<=171.996)tmp1=200;
    }

    return tmp1;
}


double ShowImage::gettemperature_colli_furnace(double FT1){

    double tmp1=0;

    if(FT1<=0)tmp1=-123.15;
    else if(FT1<=0.03752){
        if(FT1==0.03465)tmp1=-150;
        else if(FT1<=0.03496)tmp1=-149.9;
        else if(FT1<=0.03527)tmp1=-149.8;
        else if(FT1<=0.03558)tmp1=-149.7;
        else if(FT1<=0.0359)tmp1=-149.6;
        else if(FT1<=0.03622)tmp1=-149.5;
        else if(FT1<=0.03654)tmp1=-149.4;
        else if(FT1<=0.03686)tmp1=-149.3;
        else if(FT1<=0.03719)tmp1=-149.2;
        else if(FT1<=0.03752)tmp1=-149.1;
        else if(FT1<=0.03785)tmp1=-149;
    }
    else if(FT1<=0.04094){
        if(FT1==0.03785)tmp1=-149;
        else if(FT1<=0.03818)tmp1=-148.9;
        else if(FT1<=0.03852)tmp1=-148.8;
        else if(FT1<=0.03886)tmp1=-148.7;
        else if(FT1<=0.0392)tmp1=-148.6;
        else if(FT1<=0.03954)tmp1=-148.5;
        else if(FT1<=0.03989)tmp1=-148.4;
        else if(FT1<=0.04024)tmp1=-148.3;
        else if(FT1<=0.04059)tmp1=-148.2;
        else if(FT1<=0.04094)tmp1=-148.1;
        else if(FT1<=0.0413)tmp1=-148;
    }
    else if(FT1<=0.04462){
        if(FT1==0.0413)tmp1=-148;
        else if(FT1<=0.04165)tmp1=-147.9;
        else if(FT1<=0.04202)tmp1=-147.8;
        else if(FT1<=0.04238)tmp1=-147.7;
        else if(FT1<=0.04275)tmp1=-147.6;
        else if(FT1<=0.04311)tmp1=-147.5;
        else if(FT1<=0.04349)tmp1=-147.4;
        else if(FT1<=0.04386)tmp1=-147.3;
        else if(FT1<=0.04424)tmp1=-147.2;
        else if(FT1<=0.04462)tmp1=-147.1;
        else if(FT1<=0.045)tmp1=-147;
    }
    else if(FT1<=0.04856){
        if(FT1==0.045)tmp1=-147;
        else if(FT1<=0.04538)tmp1=-146.9;
        else if(FT1<=0.04577)tmp1=-146.8;
        else if(FT1<=0.04616)tmp1=-146.7;
        else if(FT1<=0.04655)tmp1=-146.6;
        else if(FT1<=0.04695)tmp1=-146.5;
        else if(FT1<=0.04735)tmp1=-146.4;
        else if(FT1<=0.04775)tmp1=-146.3;
        else if(FT1<=0.04815)tmp1=-146.2;
        else if(FT1<=0.04856)tmp1=-146.1;
        else if(FT1<=0.04897)tmp1=-146;
    }
    else if(FT1<=0.05278){
        if(FT1==0.04897)tmp1=-146;
        else if(FT1<=0.04938)tmp1=-145.9;
        else if(FT1<=0.04979)tmp1=-145.8;
        else if(FT1<=0.05021)tmp1=-145.7;
        else if(FT1<=0.05063)tmp1=-145.6;
        else if(FT1<=0.05106)tmp1=-145.5;
        else if(FT1<=0.05148)tmp1=-145.4;
        else if(FT1<=0.05191)tmp1=-145.3;
        else if(FT1<=0.05235)tmp1=-145.2;
        else if(FT1<=0.05278)tmp1=-145.1;
        else if(FT1<=0.05322)tmp1=-145;
    }
    else if(FT1<=0.0573){
        if(FT1==0.05322)tmp1=-145;
        else if(FT1<=0.05366)tmp1=-144.9;
        else if(FT1<=0.05411)tmp1=-144.8;
        else if(FT1<=0.05455)tmp1=-144.7;
        else if(FT1<=0.055)tmp1=-144.6;
        else if(FT1<=0.05546)tmp1=-144.5;
        else if(FT1<=0.05591)tmp1=-144.4;
        else if(FT1<=0.05637)tmp1=-144.3;
        else if(FT1<=0.05684)tmp1=-144.2;
        else if(FT1<=0.0573)tmp1=-144.1;
        else if(FT1<=0.05777)tmp1=-144;
    }
    else if(FT1<=0.06214){
        if(FT1==0.05777)tmp1=-144;
        else if(FT1<=0.05824)tmp1=-143.9;
        else if(FT1<=0.05872)tmp1=-143.8;
        else if(FT1<=0.0592)tmp1=-143.7;
        else if(FT1<=0.05968)tmp1=-143.6;
        else if(FT1<=0.06017)tmp1=-143.5;
        else if(FT1<=0.06065)tmp1=-143.4;
        else if(FT1<=0.06114)tmp1=-143.3;
        else if(FT1<=0.06164)tmp1=-143.2;
        else if(FT1<=0.06214)tmp1=-143.1;
        else if(FT1<=0.06264)tmp1=-143;
    }
    else if(FT1<=0.0673){
        if(FT1==0.06264)tmp1=-143;
        else if(FT1<=0.06314)tmp1=-142.9;
        else if(FT1<=0.06365)tmp1=-142.8;
        else if(FT1<=0.06416)tmp1=-142.7;
        else if(FT1<=0.06468)tmp1=-142.6;
        else if(FT1<=0.06519)tmp1=-142.5;
        else if(FT1<=0.06572)tmp1=-142.4;
        else if(FT1<=0.06624)tmp1=-142.3;
        else if(FT1<=0.06677)tmp1=-142.2;
        else if(FT1<=0.0673)tmp1=-142.1;
        else if(FT1<=0.06784)tmp1=-142;
    }
    else if(FT1<=0.07281){
        if(FT1==0.06784)tmp1=-142;
        else if(FT1<=0.06837)tmp1=-141.9;
        else if(FT1<=0.06892)tmp1=-141.8;
        else if(FT1<=0.06946)tmp1=-141.7;
        else if(FT1<=0.07001)tmp1=-141.6;
        else if(FT1<=0.07056)tmp1=-141.5;
        else if(FT1<=0.07112)tmp1=-141.4;
        else if(FT1<=0.07168)tmp1=-141.3;
        else if(FT1<=0.07224)tmp1=-141.2;
        else if(FT1<=0.07281)tmp1=-141.1;
        else if(FT1<=0.07338)tmp1=-141;
    }
    else if(FT1<=0.07868){
        if(FT1==0.07338)tmp1=-141;
        else if(FT1<=0.07395)tmp1=-140.9;
        else if(FT1<=0.07453)tmp1=-140.8;
        else if(FT1<=0.07511)tmp1=-140.7;
        else if(FT1<=0.0757)tmp1=-140.6;
        else if(FT1<=0.07629)tmp1=-140.5;
        else if(FT1<=0.07688)tmp1=-140.4;
        else if(FT1<=0.07748)tmp1=-140.3;
        else if(FT1<=0.07808)tmp1=-140.2;
        else if(FT1<=0.07868)tmp1=-140.1;
        else if(FT1<=0.07929)tmp1=-140;
    }
    else if(FT1<=0.08493){
        if(FT1==0.07929)tmp1=-140;
        else if(FT1<=0.0799)tmp1=-139.9;
        else if(FT1<=0.08052)tmp1=-139.8;
        else if(FT1<=0.08113)tmp1=-139.7;
        else if(FT1<=0.08176)tmp1=-139.6;
        else if(FT1<=0.08239)tmp1=-139.5;
        else if(FT1<=0.08302)tmp1=-139.4;
        else if(FT1<=0.08365)tmp1=-139.3;
        else if(FT1<=0.08429)tmp1=-139.2;
        else if(FT1<=0.08493)tmp1=-139.1;
        else if(FT1<=0.08558)tmp1=-139;
    }
    else if(FT1<=0.09158){
        if(FT1==0.08558)tmp1=-139;
        else if(FT1<=0.08623)tmp1=-138.9;
        else if(FT1<=0.08688)tmp1=-138.8;
        else if(FT1<=0.08754)tmp1=-138.7;
        else if(FT1<=0.08821)tmp1=-138.6;
        else if(FT1<=0.08887)tmp1=-138.5;
        else if(FT1<=0.08954)tmp1=-138.4;
        else if(FT1<=0.09022)tmp1=-138.3;
        else if(FT1<=0.0909)tmp1=-138.2;
        else if(FT1<=0.09158)tmp1=-138.1;
        else if(FT1<=0.09227)tmp1=-138;
    }
    else if(FT1<=0.09865){
        if(FT1==0.09227)tmp1=-138;
        else if(FT1<=0.09296)tmp1=-137.9;
        else if(FT1<=0.09366)tmp1=-137.8;
        else if(FT1<=0.09436)tmp1=-137.7;
        else if(FT1<=0.09506)tmp1=-137.6;
        else if(FT1<=0.09577)tmp1=-137.5;
        else if(FT1<=0.09648)tmp1=-137.4;
        else if(FT1<=0.0972)tmp1=-137.3;
        else if(FT1<=0.09792)tmp1=-137.2;
        else if(FT1<=0.09865)tmp1=-137.1;
        else if(FT1<=0.09938)tmp1=-137;
    }
    else if(FT1<=0.10615){
        if(FT1==0.09938)tmp1=-137;
        else if(FT1<=0.10011)tmp1=-136.9;
        else if(FT1<=0.10085)tmp1=-136.8;
        else if(FT1<=0.1016)tmp1=-136.7;
        else if(FT1<=0.10235)tmp1=-136.6;
        else if(FT1<=0.1031)tmp1=-136.5;
        else if(FT1<=0.10385)tmp1=-136.4;
        else if(FT1<=0.10462)tmp1=-136.3;
        else if(FT1<=0.10538)tmp1=-136.2;
        else if(FT1<=0.10615)tmp1=-136.1;
        else if(FT1<=0.10693)tmp1=-136;
    }
    else if(FT1<=0.11411){
        if(FT1==0.10693)tmp1=-136;
        else if(FT1<=0.10771)tmp1=-135.9;
        else if(FT1<=0.10849)tmp1=-135.8;
        else if(FT1<=0.10928)tmp1=-135.7;
        else if(FT1<=0.11007)tmp1=-135.6;
        else if(FT1<=0.11087)tmp1=-135.5;
        else if(FT1<=0.11167)tmp1=-135.4;
        else if(FT1<=0.11248)tmp1=-135.3;
        else if(FT1<=0.11329)tmp1=-135.2;
        else if(FT1<=0.11411)tmp1=-135.1;
        else if(FT1<=0.11493)tmp1=-135;
    }
    else if(FT1<=0.12255){
        if(FT1==0.11493)tmp1=-135;
        else if(FT1<=0.11576)tmp1=-134.9;
        else if(FT1<=0.11659)tmp1=-134.8;
        else if(FT1<=0.11743)tmp1=-134.7;
        else if(FT1<=0.11827)tmp1=-134.6;
        else if(FT1<=0.11911)tmp1=-134.5;
        else if(FT1<=0.11996)tmp1=-134.4;
        else if(FT1<=0.12082)tmp1=-134.3;
        else if(FT1<=0.12168)tmp1=-134.2;
        else if(FT1<=0.12255)tmp1=-134.1;
        else if(FT1<=0.12342)tmp1=-134;
    }
    else if(FT1<=0.13148){
        if(FT1==0.12342)tmp1=-134;
        else if(FT1<=0.12429)tmp1=-133.9;
        else if(FT1<=0.12517)tmp1=-133.8;
        else if(FT1<=0.12606)tmp1=-133.7;
        else if(FT1<=0.12695)tmp1=-133.6;
        else if(FT1<=0.12784)tmp1=-133.5;
        else if(FT1<=0.12874)tmp1=-133.4;
        else if(FT1<=0.12965)tmp1=-133.3;
        else if(FT1<=0.13056)tmp1=-133.2;
        else if(FT1<=0.13148)tmp1=-133.1;
        else if(FT1<=0.1324)tmp1=-133;
    }
    else if(FT1<=0.14093){
        if(FT1==0.1324)tmp1=-133;
        else if(FT1<=0.13332)tmp1=-132.9;
        else if(FT1<=0.13426)tmp1=-132.8;
        else if(FT1<=0.13519)tmp1=-132.7;
        else if(FT1<=0.13613)tmp1=-132.6;
        else if(FT1<=0.13708)tmp1=-132.5;
        else if(FT1<=0.13804)tmp1=-132.4;
        else if(FT1<=0.13899)tmp1=-132.3;
        else if(FT1<=0.13996)tmp1=-132.2;
        else if(FT1<=0.14093)tmp1=-132.1;
        else if(FT1<=0.1419)tmp1=-132;
    }
    else if(FT1<=0.15091){
        if(FT1==0.1419)tmp1=-132;
        else if(FT1<=0.14288)tmp1=-131.9;
        else if(FT1<=0.14386)tmp1=-131.8;
        else if(FT1<=0.14485)tmp1=-131.7;
        else if(FT1<=0.14585)tmp1=-131.6;
        else if(FT1<=0.14685)tmp1=-131.5;
        else if(FT1<=0.14786)tmp1=-131.4;
        else if(FT1<=0.14887)tmp1=-131.3;
        else if(FT1<=0.14989)tmp1=-131.2;
        else if(FT1<=0.15091)tmp1=-131.1;
        else if(FT1<=0.15194)tmp1=-131;
    }
    else if(FT1<=0.16146){
        if(FT1==0.15194)tmp1=-131;
        else if(FT1<=0.15298)tmp1=-130.9;
        else if(FT1<=0.15402)tmp1=-130.8;
        else if(FT1<=0.15506)tmp1=-130.7;
        else if(FT1<=0.15612)tmp1=-130.6;
        else if(FT1<=0.15717)tmp1=-130.5;
        else if(FT1<=0.15824)tmp1=-130.4;
        else if(FT1<=0.15931)tmp1=-130.3;
        else if(FT1<=0.16038)tmp1=-130.2;
        else if(FT1<=0.16146)tmp1=-130.1;
        else if(FT1<=0.16255)tmp1=-130;
    }
    else if(FT1<=0.17259){
        if(FT1==0.16255)tmp1=-130;
        else if(FT1<=0.16364)tmp1=-129.9;
        else if(FT1<=0.16474)tmp1=-129.8;
        else if(FT1<=0.16584)tmp1=-129.7;
        else if(FT1<=0.16695)tmp1=-129.6;
        else if(FT1<=0.16807)tmp1=-129.5;
        else if(FT1<=0.16919)tmp1=-129.4;
        else if(FT1<=0.17032)tmp1=-129.3;
        else if(FT1<=0.17145)tmp1=-129.2;
        else if(FT1<=0.17259)tmp1=-129.1;
        else if(FT1<=0.17374)tmp1=-129;
    }
    else if(FT1<=0.18433){
        if(FT1==0.17374)tmp1=-129;
        else if(FT1<=0.17489)tmp1=-128.9;
        else if(FT1<=0.17605)tmp1=-128.8;
        else if(FT1<=0.17721)tmp1=-128.7;
        else if(FT1<=0.17838)tmp1=-128.6;
        else if(FT1<=0.17956)tmp1=-128.5;
        else if(FT1<=0.18074)tmp1=-128.4;
        else if(FT1<=0.18193)tmp1=-128.3;
        else if(FT1<=0.18313)tmp1=-128.2;
        else if(FT1<=0.18433)tmp1=-128.1;
        else if(FT1<=0.18554)tmp1=-128;
    }
    else if(FT1<=0.1967){
        if(FT1==0.18554)tmp1=-128;
        else if(FT1<=0.18675)tmp1=-127.9;
        else if(FT1<=0.18797)tmp1=-127.8;
        else if(FT1<=0.1892)tmp1=-127.7;
        else if(FT1<=0.19044)tmp1=-127.6;
        else if(FT1<=0.19168)tmp1=-127.5;
        else if(FT1<=0.19292)tmp1=-127.4;
        else if(FT1<=0.19417)tmp1=-127.3;
        else if(FT1<=0.19543)tmp1=-127.2;
        else if(FT1<=0.1967)tmp1=-127.1;
        else if(FT1<=0.19797)tmp1=-127;
    }
    else if(FT1<=0.20972){
        if(FT1==0.19797)tmp1=-127;
        else if(FT1<=0.19925)tmp1=-126.9;
        else if(FT1<=0.20054)tmp1=-126.8;
        else if(FT1<=0.20183)tmp1=-126.7;
        else if(FT1<=0.20313)tmp1=-126.6;
        else if(FT1<=0.20443)tmp1=-126.5;
        else if(FT1<=0.20575)tmp1=-126.4;
        else if(FT1<=0.20706)tmp1=-126.3;
        else if(FT1<=0.20839)tmp1=-126.2;
        else if(FT1<=0.20972)tmp1=-126.1;
        else if(FT1<=0.21106)tmp1=-126;
    }
    else if(FT1<=0.22342){
        if(FT1==0.21106)tmp1=-126;
        else if(FT1<=0.21241)tmp1=-125.9;
        else if(FT1<=0.21376)tmp1=-125.8;
        else if(FT1<=0.21512)tmp1=-125.7;
        else if(FT1<=0.21649)tmp1=-125.6;
        else if(FT1<=0.21786)tmp1=-125.5;
        else if(FT1<=0.21924)tmp1=-125.4;
        else if(FT1<=0.22063)tmp1=-125.3;
        else if(FT1<=0.22202)tmp1=-125.2;
        else if(FT1<=0.22342)tmp1=-125.1;
        else if(FT1<=0.22483)tmp1=-125;
    }
    else if(FT1<=0.23783){
        if(FT1==0.22483)tmp1=-125;
        else if(FT1<=0.22625)tmp1=-124.9;
        else if(FT1<=0.22767)tmp1=-124.8;
        else if(FT1<=0.2291)tmp1=-124.7;
        else if(FT1<=0.23054)tmp1=-124.6;
        else if(FT1<=0.23198)tmp1=-124.5;
        else if(FT1<=0.23343)tmp1=-124.4;
        else if(FT1<=0.23489)tmp1=-124.3;
        else if(FT1<=0.23636)tmp1=-124.2;
        else if(FT1<=0.23783)tmp1=-124.1;
        else if(FT1<=0.23931)tmp1=-124;
    }
    else if(FT1<=0.25296){
        if(FT1==0.23931)tmp1=-124;
        else if(FT1<=0.2408)tmp1=-123.9;
        else if(FT1<=0.24229)tmp1=-123.8;
        else if(FT1<=0.24379)tmp1=-123.7;
        else if(FT1<=0.2453)tmp1=-123.6;
        else if(FT1<=0.24682)tmp1=-123.5;
        else if(FT1<=0.24834)tmp1=-123.4;
        else if(FT1<=0.24987)tmp1=-123.3;
        else if(FT1<=0.25141)tmp1=-123.2;
        else if(FT1<=0.25296)tmp1=-123.1;
        else if(FT1<=0.25452)tmp1=-123;
    }
    else if(FT1<=0.26885){
        if(FT1==0.25452)tmp1=-123;
        else if(FT1<=0.25608)tmp1=-122.9;
        else if(FT1<=0.25765)tmp1=-122.8;
        else if(FT1<=0.25922)tmp1=-122.7;
        else if(FT1<=0.26081)tmp1=-122.6;
        else if(FT1<=0.2624)tmp1=-122.5;
        else if(FT1<=0.264)tmp1=-122.4;
        else if(FT1<=0.26561)tmp1=-122.3;
        else if(FT1<=0.26722)tmp1=-122.2;
        else if(FT1<=0.26885)tmp1=-122.1;
        else if(FT1<=0.27048)tmp1=-122;
    }
    else if(FT1<=0.28552){
        if(FT1==0.27048)tmp1=-122;
        else if(FT1<=0.27212)tmp1=-121.9;
        else if(FT1<=0.27377)tmp1=-121.8;
        else if(FT1<=0.27542)tmp1=-121.7;
        else if(FT1<=0.27708)tmp1=-121.6;
        else if(FT1<=0.27875)tmp1=-121.5;
        else if(FT1<=0.28043)tmp1=-121.4;
        else if(FT1<=0.28212)tmp1=-121.3;
        else if(FT1<=0.28381)tmp1=-121.2;
        else if(FT1<=0.28552)tmp1=-121.1;
        else if(FT1<=0.28723)tmp1=-121;
    }
    else if(FT1<=0.30299){
        if(FT1==0.28723)tmp1=-121;
        else if(FT1<=0.28895)tmp1=-120.9;
        else if(FT1<=0.29067)tmp1=-120.8;
        else if(FT1<=0.29241)tmp1=-120.7;
        else if(FT1<=0.29415)tmp1=-120.6;
        else if(FT1<=0.2959)tmp1=-120.5;
        else if(FT1<=0.29766)tmp1=-120.4;
        else if(FT1<=0.29943)tmp1=-120.3;
        else if(FT1<=0.30121)tmp1=-120.2;
        else if(FT1<=0.30299)tmp1=-120.1;
        else if(FT1<=0.30478)tmp1=-120;
    }
    else if(FT1<=0.3213){
        if(FT1==0.30478)tmp1=-120;
        else if(FT1<=0.30658)tmp1=-119.9;
        else if(FT1<=0.30839)tmp1=-119.8;
        else if(FT1<=0.31021)tmp1=-119.7;
        else if(FT1<=0.31204)tmp1=-119.6;
        else if(FT1<=0.31387)tmp1=-119.5;
        else if(FT1<=0.31572)tmp1=-119.4;
        else if(FT1<=0.31757)tmp1=-119.3;
        else if(FT1<=0.31943)tmp1=-119.2;
        else if(FT1<=0.3213)tmp1=-119.1;
        else if(FT1<=0.32318)tmp1=-119;
    }
    else if(FT1<=0.34047){
        if(FT1==0.32318)tmp1=-119;
        else if(FT1<=0.32506)tmp1=-118.9;
        else if(FT1<=0.32696)tmp1=-118.8;
        else if(FT1<=0.32886)tmp1=-118.7;
        else if(FT1<=0.33077)tmp1=-118.6;
        else if(FT1<=0.3327)tmp1=-118.5;
        else if(FT1<=0.33463)tmp1=-118.4;
        else if(FT1<=0.33656)tmp1=-118.3;
        else if(FT1<=0.33851)tmp1=-118.2;
        else if(FT1<=0.34047)tmp1=-118.1;
        else if(FT1<=0.34243)tmp1=-118;
    }
    else if(FT1<=0.36053){
        if(FT1==0.34243)tmp1=-118;
        else if(FT1<=0.34441)tmp1=-117.9;
        else if(FT1<=0.34639)tmp1=-117.8;
        else if(FT1<=0.34838)tmp1=-117.7;
        else if(FT1<=0.35039)tmp1=-117.6;
        else if(FT1<=0.3524)tmp1=-117.5;
        else if(FT1<=0.35441)tmp1=-117.4;
        else if(FT1<=0.35644)tmp1=-117.3;
        else if(FT1<=0.35848)tmp1=-117.2;
        else if(FT1<=0.36053)tmp1=-117.1;
        else if(FT1<=0.36258)tmp1=-117;
    }
    else if(FT1<=0.3815){
        if(FT1==0.36258)tmp1=-117;
        else if(FT1<=0.36465)tmp1=-116.9;
        else if(FT1<=0.36672)tmp1=-116.8;
        else if(FT1<=0.36881)tmp1=-116.7;
        else if(FT1<=0.3709)tmp1=-116.6;
        else if(FT1<=0.373)tmp1=-116.5;
        else if(FT1<=0.37511)tmp1=-116.4;
        else if(FT1<=0.37723)tmp1=-116.3;
        else if(FT1<=0.37936)tmp1=-116.2;
        else if(FT1<=0.3815)tmp1=-116.1;
        else if(FT1<=0.38365)tmp1=-116;
    }
    else if(FT1<=0.40343){
        if(FT1==0.38365)tmp1=-116;
        else if(FT1<=0.38581)tmp1=-115.9;
        else if(FT1<=0.38798)tmp1=-115.8;
        else if(FT1<=0.39016)tmp1=-115.7;
        else if(FT1<=0.39235)tmp1=-115.6;
        else if(FT1<=0.39454)tmp1=-115.5;
        else if(FT1<=0.39675)tmp1=-115.4;
        else if(FT1<=0.39896)tmp1=-115.3;
        else if(FT1<=0.40119)tmp1=-115.2;
        else if(FT1<=0.40343)tmp1=-115.1;
        else if(FT1<=0.40567)tmp1=-115;
    }
    else if(FT1<=0.42632){
        if(FT1==0.40567)tmp1=-115;
        else if(FT1<=0.40793)tmp1=-114.9;
        else if(FT1<=0.41019)tmp1=-114.8;
        else if(FT1<=0.41247)tmp1=-114.7;
        else if(FT1<=0.41475)tmp1=-114.6;
        else if(FT1<=0.41705)tmp1=-114.5;
        else if(FT1<=0.41935)tmp1=-114.4;
        else if(FT1<=0.42166)tmp1=-114.3;
        else if(FT1<=0.42399)tmp1=-114.2;
        else if(FT1<=0.42632)tmp1=-114.1;
        else if(FT1<=0.42867)tmp1=-114;
    }
    else if(FT1<=0.45022){
        if(FT1==0.42867)tmp1=-114;
        else if(FT1<=0.43102)tmp1=-113.9;
        else if(FT1<=0.43339)tmp1=-113.8;
        else if(FT1<=0.43576)tmp1=-113.7;
        else if(FT1<=0.43815)tmp1=-113.6;
        else if(FT1<=0.44054)tmp1=-113.5;
        else if(FT1<=0.44295)tmp1=-113.4;
        else if(FT1<=0.44536)tmp1=-113.3;
        else if(FT1<=0.44779)tmp1=-113.2;
        else if(FT1<=0.45022)tmp1=-113.1;
        else if(FT1<=0.45267)tmp1=-113;
    }
    else if(FT1<=0.47516){
        if(FT1==0.45267)tmp1=-113;
        else if(FT1<=0.45513)tmp1=-112.9;
        else if(FT1<=0.4576)tmp1=-112.8;
        else if(FT1<=0.46007)tmp1=-112.7;
        else if(FT1<=0.46256)tmp1=-112.6;
        else if(FT1<=0.46506)tmp1=-112.5;
        else if(FT1<=0.46757)tmp1=-112.4;
        else if(FT1<=0.47009)tmp1=-112.3;
        else if(FT1<=0.47262)tmp1=-112.2;
        else if(FT1<=0.47516)tmp1=-112.1;
        else if(FT1<=0.47771)tmp1=-112;
    }
    else if(FT1<=0.50116){
        if(FT1==0.47771)tmp1=-112;
        else if(FT1<=0.48027)tmp1=-111.9;
        else if(FT1<=0.48285)tmp1=-111.8;
        else if(FT1<=0.48543)tmp1=-111.7;
        else if(FT1<=0.48802)tmp1=-111.6;
        else if(FT1<=0.49063)tmp1=-111.5;
        else if(FT1<=0.49325)tmp1=-111.4;
        else if(FT1<=0.49587)tmp1=-111.3;
        else if(FT1<=0.49851)tmp1=-111.2;
        else if(FT1<=0.50116)tmp1=-111.1;
        else if(FT1<=0.50382)tmp1=-111;
    }
    else if(FT1<=0.52825){
        if(FT1==0.50382)tmp1=-111;
        else if(FT1<=0.50649)tmp1=-110.9;
        else if(FT1<=0.50917)tmp1=-110.8;
        else if(FT1<=0.51186)tmp1=-110.7;
        else if(FT1<=0.51457)tmp1=-110.6;
        else if(FT1<=0.51728)tmp1=-110.5;
        else if(FT1<=0.52001)tmp1=-110.4;
        else if(FT1<=0.52274)tmp1=-110.3;
        else if(FT1<=0.52549)tmp1=-110.2;
        else if(FT1<=0.52825)tmp1=-110.1;
        else if(FT1<=0.53102)tmp1=-110;
    }
    else if(FT1<=0.55647){
        if(FT1==0.53102)tmp1=-110;
        else if(FT1<=0.53381)tmp1=-109.9;
        else if(FT1<=0.5366)tmp1=-109.8;
        else if(FT1<=0.5394)tmp1=-109.7;
        else if(FT1<=0.54222)tmp1=-109.6;
        else if(FT1<=0.54505)tmp1=-109.5;
        else if(FT1<=0.54789)tmp1=-109.4;
        else if(FT1<=0.55074)tmp1=-109.3;
        else if(FT1<=0.5536)tmp1=-109.2;
        else if(FT1<=0.55647)tmp1=-109.1;
        else if(FT1<=0.55936)tmp1=-109;
    }
    else if(FT1<=0.58584){
        if(FT1==0.55936)tmp1=-109;
        else if(FT1<=0.56225)tmp1=-108.9;
        else if(FT1<=0.56516)tmp1=-108.8;
        else if(FT1<=0.56808)tmp1=-108.7;
        else if(FT1<=0.57101)tmp1=-108.6;
        else if(FT1<=0.57395)tmp1=-108.5;
        else if(FT1<=0.57691)tmp1=-108.4;
        else if(FT1<=0.57988)tmp1=-108.3;
        else if(FT1<=0.58285)tmp1=-108.2;
        else if(FT1<=0.58584)tmp1=-108.1;
        else if(FT1<=0.58885)tmp1=-108;
    }
    else if(FT1<=0.61641){
        if(FT1==0.58885)tmp1=-108;
        else if(FT1<=0.59186)tmp1=-107.9;
        else if(FT1<=0.59489)tmp1=-107.8;
        else if(FT1<=0.59792)tmp1=-107.7;
        else if(FT1<=0.60097)tmp1=-107.6;
        else if(FT1<=0.60404)tmp1=-107.5;
        else if(FT1<=0.60711)tmp1=-107.4;
        else if(FT1<=0.6102)tmp1=-107.3;
        else if(FT1<=0.61329)tmp1=-107.2;
        else if(FT1<=0.61641)tmp1=-107.1;
        else if(FT1<=0.61953)tmp1=-107;
    }
    else if(FT1<=0.64818){
        if(FT1==0.61953)tmp1=-107;
        else if(FT1<=0.62266)tmp1=-106.9;
        else if(FT1<=0.62581)tmp1=-106.8;
        else if(FT1<=0.62897)tmp1=-106.7;
        else if(FT1<=0.63214)tmp1=-106.6;
        else if(FT1<=0.63532)tmp1=-106.5;
        else if(FT1<=0.63852)tmp1=-106.4;
        else if(FT1<=0.64173)tmp1=-106.3;
        else if(FT1<=0.64495)tmp1=-106.2;
        else if(FT1<=0.64818)tmp1=-106.1;
        else if(FT1<=0.65143)tmp1=-106;
    }
    else if(FT1<=0.68121){
        if(FT1==0.65143)tmp1=-106;
        else if(FT1<=0.65469)tmp1=-105.9;
        else if(FT1<=0.65796)tmp1=-105.8;
        else if(FT1<=0.66124)tmp1=-105.7;
        else if(FT1<=0.66454)tmp1=-105.6;
        else if(FT1<=0.66785)tmp1=-105.5;
        else if(FT1<=0.67117)tmp1=-105.4;
        else if(FT1<=0.67451)tmp1=-105.3;
        else if(FT1<=0.67785)tmp1=-105.2;
        else if(FT1<=0.68121)tmp1=-105.1;
        else if(FT1<=0.68459)tmp1=-105;
    }
    else if(FT1<=0.71553){
        if(FT1==0.68459)tmp1=-105;
        else if(FT1<=0.68797)tmp1=-104.9;
        else if(FT1<=0.69137)tmp1=-104.8;
        else if(FT1<=0.69478)tmp1=-104.7;
        else if(FT1<=0.69821)tmp1=-104.6;
        else if(FT1<=0.70165)tmp1=-104.5;
        else if(FT1<=0.7051)tmp1=-104.4;
        else if(FT1<=0.70856)tmp1=-104.3;
        else if(FT1<=0.71204)tmp1=-104.2;
        else if(FT1<=0.71553)tmp1=-104.1;
        else if(FT1<=0.71903)tmp1=-104;
    }
    else if(FT1<=0.75115){
        if(FT1==0.71903)tmp1=-104;
        else if(FT1<=0.72255)tmp1=-103.9;
        else if(FT1<=0.72608)tmp1=-103.8;
        else if(FT1<=0.72962)tmp1=-103.7;
        else if(FT1<=0.73317)tmp1=-103.6;
        else if(FT1<=0.73674)tmp1=-103.5;
        else if(FT1<=0.74033)tmp1=-103.4;
        else if(FT1<=0.74392)tmp1=-103.3;
        else if(FT1<=0.74753)tmp1=-103.2;
        else if(FT1<=0.75115)tmp1=-103.1;
        else if(FT1<=0.75479)tmp1=-103;
    }
    else if(FT1<=0.78813){
        if(FT1==0.75479)tmp1=-103;
        else if(FT1<=0.75844)tmp1=-102.9;
        else if(FT1<=0.7621)tmp1=-102.8;
        else if(FT1<=0.76578)tmp1=-102.7;
        else if(FT1<=0.76947)tmp1=-102.6;
        else if(FT1<=0.77318)tmp1=-102.5;
        else if(FT1<=0.77689)tmp1=-102.4;
        else if(FT1<=0.78063)tmp1=-102.3;
        else if(FT1<=0.78437)tmp1=-102.2;
        else if(FT1<=0.78813)tmp1=-102.1;
        else if(FT1<=0.7919)tmp1=-102;
    }
    else if(FT1<=0.82648){
        if(FT1==0.7919)tmp1=-102;
        else if(FT1<=0.79569)tmp1=-101.9;
        else if(FT1<=0.79949)tmp1=-101.8;
        else if(FT1<=0.8033)tmp1=-101.7;
        else if(FT1<=0.80713)tmp1=-101.6;
        else if(FT1<=0.81097)tmp1=-101.5;
        else if(FT1<=0.81483)tmp1=-101.4;
        else if(FT1<=0.8187)tmp1=-101.3;
        else if(FT1<=0.82259)tmp1=-101.2;
        else if(FT1<=0.82648)tmp1=-101.1;
        else if(FT1<=0.8304)tmp1=-101;
    }
    else if(FT1<=0.86625){
        if(FT1==0.8304)tmp1=-101;
        else if(FT1<=0.83432)tmp1=-100.9;
        else if(FT1<=0.83827)tmp1=-100.8;
        else if(FT1<=0.84222)tmp1=-100.7;
        else if(FT1<=0.84619)tmp1=-100.6;
        else if(FT1<=0.85017)tmp1=-100.5;
        else if(FT1<=0.85417)tmp1=-100.4;
        else if(FT1<=0.85818)tmp1=-100.3;
        else if(FT1<=0.86221)tmp1=-100.2;
        else if(FT1<=0.86625)tmp1=-100.1;
        else if(FT1<=0.87031)tmp1=-100;
    }
    else if(FT1<=0.90747){
        if(FT1==0.87031)tmp1=-100;
        else if(FT1<=0.87438)tmp1=-99.9;
        else if(FT1<=0.87846)tmp1=-99.8;
        else if(FT1<=0.88256)tmp1=-99.7;
        else if(FT1<=0.88668)tmp1=-99.6;
        else if(FT1<=0.89081)tmp1=-99.5;
        else if(FT1<=0.89495)tmp1=-99.4;
        else if(FT1<=0.89911)tmp1=-99.3;
        else if(FT1<=0.90328)tmp1=-99.2;
        else if(FT1<=0.90747)tmp1=-99.1;
        else if(FT1<=0.91167)tmp1=-99;
    }
    else if(FT1<=0.95016){
        if(FT1==0.91167)tmp1=-99;
        else if(FT1<=0.91589)tmp1=-98.9;
        else if(FT1<=0.92012)tmp1=-98.8;
        else if(FT1<=0.92437)tmp1=-98.7;
        else if(FT1<=0.92863)tmp1=-98.6;
        else if(FT1<=0.93291)tmp1=-98.5;
        else if(FT1<=0.9372)tmp1=-98.4;
        else if(FT1<=0.94151)tmp1=-98.3;
        else if(FT1<=0.94583)tmp1=-98.2;
        else if(FT1<=0.95016)tmp1=-98.1;
        else if(FT1<=0.95452)tmp1=-98;
    }
    else if(FT1<=0.99437){
        if(FT1==0.95452)tmp1=-98;
        else if(FT1<=0.95888)tmp1=-97.9;
        else if(FT1<=0.96327)tmp1=-97.8;
        else if(FT1<=0.96766)tmp1=-97.7;
        else if(FT1<=0.97208)tmp1=-97.6;
        else if(FT1<=0.97651)tmp1=-97.5;
        else if(FT1<=0.98095)tmp1=-97.4;
        else if(FT1<=0.98541)tmp1=-97.3;
        else if(FT1<=0.98988)tmp1=-97.2;
        else if(FT1<=0.99437)tmp1=-97.1;
        else if(FT1<=0.99888)tmp1=-97;
    }
    else if(FT1<=1.04013){
        if(FT1==0.99888)tmp1=-97;
        else if(FT1<=1.0034)tmp1=-96.9;
        else if(FT1<=1.00794)tmp1=-96.8;
        else if(FT1<=1.01249)tmp1=-96.7;
        else if(FT1<=1.01706)tmp1=-96.6;
        else if(FT1<=1.02164)tmp1=-96.5;
        else if(FT1<=1.02624)tmp1=-96.4;
        else if(FT1<=1.03085)tmp1=-96.3;
        else if(FT1<=1.03548)tmp1=-96.2;
        else if(FT1<=1.04013)tmp1=-96.1;
        else if(FT1<=1.04479)tmp1=-96;
    }
    else if(FT1<=1.08747){
        if(FT1==1.04479)tmp1=-96;
        else if(FT1<=1.04947)tmp1=-95.9;
        else if(FT1<=1.05417)tmp1=-95.8;
        else if(FT1<=1.05888)tmp1=-95.7;
        else if(FT1<=1.0636)tmp1=-95.6;
        else if(FT1<=1.06834)tmp1=-95.5;
        else if(FT1<=1.0731)tmp1=-95.4;
        else if(FT1<=1.07787)tmp1=-95.3;
        else if(FT1<=1.08266)tmp1=-95.2;
        else if(FT1<=1.08747)tmp1=-95.1;
        else if(FT1<=1.09229)tmp1=-95;
    }
    else if(FT1<=1.13643){
        if(FT1==1.09229)tmp1=-95;
        else if(FT1<=1.09713)tmp1=-94.9;
        else if(FT1<=1.10199)tmp1=-94.8;
        else if(FT1<=1.10686)tmp1=-94.7;
        else if(FT1<=1.11174)tmp1=-94.6;
        else if(FT1<=1.11665)tmp1=-94.5;
        else if(FT1<=1.12157)tmp1=-94.4;
        else if(FT1<=1.1265)tmp1=-94.3;
        else if(FT1<=1.13146)tmp1=-94.2;
        else if(FT1<=1.13643)tmp1=-94.1;
        else if(FT1<=1.14141)tmp1=-94;
    }
    else if(FT1<=1.18703){
        if(FT1==1.14141)tmp1=-94;
        else if(FT1<=1.14641)tmp1=-93.9;
        else if(FT1<=1.15143)tmp1=-93.8;
        else if(FT1<=1.15647)tmp1=-93.7;
        else if(FT1<=1.16152)tmp1=-93.6;
        else if(FT1<=1.16659)tmp1=-93.5;
        else if(FT1<=1.17167)tmp1=-93.4;
        else if(FT1<=1.17678)tmp1=-93.3;
        else if(FT1<=1.1819)tmp1=-93.2;
        else if(FT1<=1.18703)tmp1=-93.1;
        else if(FT1<=1.19218)tmp1=-93;
    }
    else if(FT1<=1.23932){
        if(FT1==1.19218)tmp1=-93;
        else if(FT1<=1.19735)tmp1=-92.9;
        else if(FT1<=1.20254)tmp1=-92.8;
        else if(FT1<=1.20774)tmp1=-92.7;
        else if(FT1<=1.21296)tmp1=-92.6;
        else if(FT1<=1.2182)tmp1=-92.5;
        else if(FT1<=1.22346)tmp1=-92.4;
        else if(FT1<=1.22873)tmp1=-92.3;
        else if(FT1<=1.23402)tmp1=-92.2;
        else if(FT1<=1.23932)tmp1=-92.1;
        else if(FT1<=1.24464)tmp1=-92;
    }
    else if(FT1<=1.29333){
        if(FT1==1.24464)tmp1=-92;
        else if(FT1<=1.24998)tmp1=-91.9;
        else if(FT1<=1.25534)tmp1=-91.8;
        else if(FT1<=1.26072)tmp1=-91.7;
        else if(FT1<=1.26611)tmp1=-91.6;
        else if(FT1<=1.27152)tmp1=-91.5;
        else if(FT1<=1.27695)tmp1=-91.4;
        else if(FT1<=1.28239)tmp1=-91.3;
        else if(FT1<=1.28785)tmp1=-91.2;
        else if(FT1<=1.29333)tmp1=-91.1;
        else if(FT1<=1.29883)tmp1=-91;
    }
    else if(FT1<=1.3491){
        if(FT1==1.29883)tmp1=-91;
        else if(FT1<=1.30434)tmp1=-90.9;
        else if(FT1<=1.30987)tmp1=-90.8;
        else if(FT1<=1.31542)tmp1=-90.7;
        else if(FT1<=1.32099)tmp1=-90.6;
        else if(FT1<=1.32658)tmp1=-90.5;
        else if(FT1<=1.33218)tmp1=-90.4;
        else if(FT1<=1.3378)tmp1=-90.3;
        else if(FT1<=1.34344)tmp1=-90.2;
        else if(FT1<=1.3491)tmp1=-90.1;
        else if(FT1<=1.35477)tmp1=-90;
    }
    else if(FT1<=1.40665){
        if(FT1==1.35477)tmp1=-90;
        else if(FT1<=1.36046)tmp1=-89.9;
        else if(FT1<=1.36617)tmp1=-89.8;
        else if(FT1<=1.3719)tmp1=-89.7;
        else if(FT1<=1.37765)tmp1=-89.6;
        else if(FT1<=1.38341)tmp1=-89.5;
        else if(FT1<=1.38919)tmp1=-89.4;
        else if(FT1<=1.39499)tmp1=-89.3;
        else if(FT1<=1.40081)tmp1=-89.2;
        else if(FT1<=1.40665)tmp1=-89.1;
        else if(FT1<=1.4125)tmp1=-89;
    }
    else if(FT1<=1.46602){
        if(FT1==1.4125)tmp1=-89;
        else if(FT1<=1.41838)tmp1=-88.9;
        else if(FT1<=1.42427)tmp1=-88.8;
        else if(FT1<=1.43018)tmp1=-88.7;
        else if(FT1<=1.43611)tmp1=-88.6;
        else if(FT1<=1.44205)tmp1=-88.5;
        else if(FT1<=1.44802)tmp1=-88.4;
        else if(FT1<=1.454)tmp1=-88.3;
        else if(FT1<=1.46)tmp1=-88.2;
        else if(FT1<=1.46602)tmp1=-88.1;
        else if(FT1<=1.47206)tmp1=-88;
    }
    else if(FT1<=1.52726){
        if(FT1==1.47206)tmp1=-88;
        else if(FT1<=1.47812)tmp1=-87.9;
        else if(FT1<=1.4842)tmp1=-87.8;
        else if(FT1<=1.49029)tmp1=-87.7;
        else if(FT1<=1.49641)tmp1=-87.6;
        else if(FT1<=1.50254)tmp1=-87.5;
        else if(FT1<=1.50869)tmp1=-87.4;
        else if(FT1<=1.51486)tmp1=-87.3;
        else if(FT1<=1.52105)tmp1=-87.2;
        else if(FT1<=1.52726)tmp1=-87.1;
        else if(FT1<=1.53349)tmp1=-87;
    }
    else if(FT1<=1.59039){
        if(FT1==1.53349)tmp1=-87;
        else if(FT1<=1.53973)tmp1=-86.9;
        else if(FT1<=1.546)tmp1=-86.8;
        else if(FT1<=1.55228)tmp1=-86.7;
        else if(FT1<=1.55859)tmp1=-86.6;
        else if(FT1<=1.56491)tmp1=-86.5;
        else if(FT1<=1.57125)tmp1=-86.4;
        else if(FT1<=1.57761)tmp1=-86.3;
        else if(FT1<=1.58399)tmp1=-86.2;
        else if(FT1<=1.59039)tmp1=-86.1;
        else if(FT1<=1.59681)tmp1=-86;
    }
    else if(FT1<=1.65545){
        if(FT1==1.59681)tmp1=-86;
        else if(FT1<=1.60325)tmp1=-85.9;
        else if(FT1<=1.6097)tmp1=-85.8;
        else if(FT1<=1.61618)tmp1=-85.7;
        else if(FT1<=1.62268)tmp1=-85.6;
        else if(FT1<=1.62919)tmp1=-85.5;
        else if(FT1<=1.63573)tmp1=-85.4;
        else if(FT1<=1.64228)tmp1=-85.3;
        else if(FT1<=1.64886)tmp1=-85.2;
        else if(FT1<=1.65545)tmp1=-85.1;
        else if(FT1<=1.66206)tmp1=-85;
    }
    else if(FT1<=1.72247){
        if(FT1==1.66206)tmp1=-85;
        else if(FT1<=1.6687)tmp1=-84.9;
        else if(FT1<=1.67535)tmp1=-84.8;
        else if(FT1<=1.68202)tmp1=-84.7;
        else if(FT1<=1.68871)tmp1=-84.6;
        else if(FT1<=1.69542)tmp1=-84.5;
        else if(FT1<=1.70216)tmp1=-84.4;
        else if(FT1<=1.70891)tmp1=-84.3;
        else if(FT1<=1.71568)tmp1=-84.2;
        else if(FT1<=1.72247)tmp1=-84.1;
        else if(FT1<=1.72928)tmp1=-84;
    }
    else if(FT1<=1.79149){
        if(FT1==1.72928)tmp1=-84;
        else if(FT1<=1.73612)tmp1=-83.9;
        else if(FT1<=1.74297)tmp1=-83.8;
        else if(FT1<=1.74984)tmp1=-83.7;
        else if(FT1<=1.75673)tmp1=-83.6;
        else if(FT1<=1.76364)tmp1=-83.5;
        else if(FT1<=1.77058)tmp1=-83.4;
        else if(FT1<=1.77753)tmp1=-83.3;
        else if(FT1<=1.7845)tmp1=-83.2;
        else if(FT1<=1.79149)tmp1=-83.1;
        else if(FT1<=1.79851)tmp1=-83;
    }
    else if(FT1<=1.86255){
        if(FT1==1.79851)tmp1=-83;
        else if(FT1<=1.80554)tmp1=-82.9;
        else if(FT1<=1.8126)tmp1=-82.8;
        else if(FT1<=1.81967)tmp1=-82.7;
        else if(FT1<=1.82677)tmp1=-82.6;
        else if(FT1<=1.83388)tmp1=-82.5;
        else if(FT1<=1.84102)tmp1=-82.4;
        else if(FT1<=1.84818)tmp1=-82.3;
        else if(FT1<=1.85535)tmp1=-82.2;
        else if(FT1<=1.86255)tmp1=-82.1;
        else if(FT1<=1.86977)tmp1=-82;
    }
    else if(FT1<=1.93568){
        if(FT1==1.86977)tmp1=-82;
        else if(FT1<=1.87701)tmp1=-81.9;
        else if(FT1<=1.88427)tmp1=-81.8;
        else if(FT1<=1.89155)tmp1=-81.7;
        else if(FT1<=1.89885)tmp1=-81.6;
        else if(FT1<=1.90618)tmp1=-81.5;
        else if(FT1<=1.91352)tmp1=-81.4;
        else if(FT1<=1.92089)tmp1=-81.3;
        else if(FT1<=1.92827)tmp1=-81.2;
        else if(FT1<=1.93568)tmp1=-81.1;
        else if(FT1<=1.94311)tmp1=-81;
    }
    else if(FT1<=2.01091){
        if(FT1==1.94311)tmp1=-81;
        else if(FT1<=1.95056)tmp1=-80.9;
        else if(FT1<=1.95803)tmp1=-80.8;
        else if(FT1<=1.96552)tmp1=-80.7;
        else if(FT1<=1.97303)tmp1=-80.6;
        else if(FT1<=1.98056)tmp1=-80.5;
        else if(FT1<=1.98812)tmp1=-80.4;
        else if(FT1<=1.9957)tmp1=-80.3;
        else if(FT1<=2.00329)tmp1=-80.2;
        else if(FT1<=2.01091)tmp1=-80.1;
        else if(FT1<=2.01855)tmp1=-80;
    }
    else if(FT1<=2.08829){
        if(FT1==2.01855)tmp1=-80;
        else if(FT1<=2.02621)tmp1=-79.9;
        else if(FT1<=2.0339)tmp1=-79.8;
        else if(FT1<=2.0416)tmp1=-79.7;
        else if(FT1<=2.04933)tmp1=-79.6;
        else if(FT1<=2.05708)tmp1=-79.5;
        else if(FT1<=2.06485)tmp1=-79.4;
        else if(FT1<=2.07264)tmp1=-79.3;
        else if(FT1<=2.08045)tmp1=-79.2;
        else if(FT1<=2.08829)tmp1=-79.1;
        else if(FT1<=2.09614)tmp1=-79;
    }
    else if(FT1<=2.16783){
        if(FT1==2.09614)tmp1=-79;
        else if(FT1<=2.10402)tmp1=-78.9;
        else if(FT1<=2.11192)tmp1=-78.8;
        else if(FT1<=2.11984)tmp1=-78.7;
        else if(FT1<=2.12779)tmp1=-78.6;
        else if(FT1<=2.13575)tmp1=-78.5;
        else if(FT1<=2.14374)tmp1=-78.4;
        else if(FT1<=2.15175)tmp1=-78.3;
        else if(FT1<=2.15978)tmp1=-78.2;
        else if(FT1<=2.16783)tmp1=-78.1;
        else if(FT1<=2.17591)tmp1=-78;
    }
    else if(FT1<=2.2496){
        if(FT1==2.17591)tmp1=-78;
        else if(FT1<=2.18401)tmp1=-77.9;
        else if(FT1<=2.19213)tmp1=-77.8;
        else if(FT1<=2.20027)tmp1=-77.7;
        else if(FT1<=2.20844)tmp1=-77.6;
        else if(FT1<=2.21662)tmp1=-77.5;
        else if(FT1<=2.22483)tmp1=-77.4;
        else if(FT1<=2.23306)tmp1=-77.3;
        else if(FT1<=2.24132)tmp1=-77.2;
        else if(FT1<=2.2496)tmp1=-77.1;
        else if(FT1<=2.25789)tmp1=-77;
    }
    else if(FT1<=2.3336){
        if(FT1==2.25789)tmp1=-77;
        else if(FT1<=2.26622)tmp1=-76.9;
        else if(FT1<=2.27456)tmp1=-76.8;
        else if(FT1<=2.28293)tmp1=-76.7;
        else if(FT1<=2.29132)tmp1=-76.6;
        else if(FT1<=2.29973)tmp1=-76.5;
        else if(FT1<=2.30816)tmp1=-76.4;
        else if(FT1<=2.31662)tmp1=-76.3;
        else if(FT1<=2.3251)tmp1=-76.2;
        else if(FT1<=2.3336)tmp1=-76.1;
        else if(FT1<=2.34213)tmp1=-76;
    }
    else if(FT1<=2.41989){
        if(FT1==2.34213)tmp1=-76;
        else if(FT1<=2.35068)tmp1=-75.9;
        else if(FT1<=2.35925)tmp1=-75.8;
        else if(FT1<=2.36784)tmp1=-75.7;
        else if(FT1<=2.37646)tmp1=-75.6;
        else if(FT1<=2.3851)tmp1=-75.5;
        else if(FT1<=2.39376)tmp1=-75.4;
        else if(FT1<=2.40245)tmp1=-75.3;
        else if(FT1<=2.41116)tmp1=-75.2;
        else if(FT1<=2.41989)tmp1=-75.1;
        else if(FT1<=2.42865)tmp1=-75;
    }
    else if(FT1<=2.50849){
        if(FT1==2.42865)tmp1=-75;
        else if(FT1<=2.43742)tmp1=-74.9;
        else if(FT1<=2.44623)tmp1=-74.8;
        else if(FT1<=2.45505)tmp1=-74.7;
        else if(FT1<=2.4639)tmp1=-74.6;
        else if(FT1<=2.47277)tmp1=-74.5;
        else if(FT1<=2.48167)tmp1=-74.4;
        else if(FT1<=2.49059)tmp1=-74.3;
        else if(FT1<=2.49953)tmp1=-74.2;
        else if(FT1<=2.50849)tmp1=-74.1;
        else if(FT1<=2.51748)tmp1=-74;
    }
    else if(FT1<=2.59945){
        if(FT1==2.51748)tmp1=-74;
        else if(FT1<=2.5265)tmp1=-73.9;
        else if(FT1<=2.53553)tmp1=-73.8;
        else if(FT1<=2.54459)tmp1=-73.7;
        else if(FT1<=2.55368)tmp1=-73.6;
        else if(FT1<=2.56278)tmp1=-73.5;
        else if(FT1<=2.57192)tmp1=-73.4;
        else if(FT1<=2.58107)tmp1=-73.3;
        else if(FT1<=2.59025)tmp1=-73.2;
        else if(FT1<=2.59945)tmp1=-73.1;
        else if(FT1<=2.60868)tmp1=-73;
    }
    else if(FT1<=2.69279){
        if(FT1==2.60868)tmp1=-73;
        else if(FT1<=2.61793)tmp1=-72.9;
        else if(FT1<=2.6272)tmp1=-72.8;
        else if(FT1<=2.6365)tmp1=-72.7;
        else if(FT1<=2.64582)tmp1=-72.6;
        else if(FT1<=2.65517)tmp1=-72.5;
        else if(FT1<=2.66454)tmp1=-72.4;
        else if(FT1<=2.67393)tmp1=-72.3;
        else if(FT1<=2.68335)tmp1=-72.2;
        else if(FT1<=2.69279)tmp1=-72.1;
        else if(FT1<=2.70226)tmp1=-72;
    }
    else if(FT1<=2.78856){
        if(FT1==2.70226)tmp1=-72;
        else if(FT1<=2.71175)tmp1=-71.9;
        else if(FT1<=2.72127)tmp1=-71.8;
        else if(FT1<=2.73081)tmp1=-71.7;
        else if(FT1<=2.74037)tmp1=-71.6;
        else if(FT1<=2.74996)tmp1=-71.5;
        else if(FT1<=2.75957)tmp1=-71.4;
        else if(FT1<=2.76921)tmp1=-71.3;
        else if(FT1<=2.77887)tmp1=-71.2;
        else if(FT1<=2.78856)tmp1=-71.1;
        else if(FT1<=2.79827)tmp1=-71;
    }
    else if(FT1<=2.88678){
        if(FT1==2.79827)tmp1=-71;
        else if(FT1<=2.80801)tmp1=-70.9;
        else if(FT1<=2.81777)tmp1=-70.8;
        else if(FT1<=2.82755)tmp1=-70.7;
        else if(FT1<=2.83736)tmp1=-70.6;
        else if(FT1<=2.84719)tmp1=-70.5;
        else if(FT1<=2.85705)tmp1=-70.4;
        else if(FT1<=2.86694)tmp1=-70.3;
        else if(FT1<=2.87685)tmp1=-70.2;
        else if(FT1<=2.88678)tmp1=-70.1;
        else if(FT1<=2.89674)tmp1=-70;
    }
    else if(FT1<=2.98749){
        if(FT1==2.89674)tmp1=-70;
        else if(FT1<=2.90672)tmp1=-69.9;
        else if(FT1<=2.91673)tmp1=-69.8;
        else if(FT1<=2.92676)tmp1=-69.7;
        else if(FT1<=2.93682)tmp1=-69.6;
        else if(FT1<=2.94691)tmp1=-69.5;
        else if(FT1<=2.95702)tmp1=-69.4;
        else if(FT1<=2.96715)tmp1=-69.3;
        else if(FT1<=2.97731)tmp1=-69.2;
        else if(FT1<=2.98749)tmp1=-69.1;
        else if(FT1<=2.9977)tmp1=-69;
    }
    else if(FT1<=3.09073){
        if(FT1==2.9977)tmp1=-69;
        else if(FT1<=3.00794)tmp1=-68.9;
        else if(FT1<=3.0182)tmp1=-68.8;
        else if(FT1<=3.02848)tmp1=-68.7;
        else if(FT1<=3.03879)tmp1=-68.6;
        else if(FT1<=3.04913)tmp1=-68.5;
        else if(FT1<=3.05949)tmp1=-68.4;
        else if(FT1<=3.06988)tmp1=-68.3;
        else if(FT1<=3.08029)tmp1=-68.2;
        else if(FT1<=3.09073)tmp1=-68.1;
        else if(FT1<=3.1012)tmp1=-68;
    }
    else if(FT1<=3.19653){
        if(FT1==3.1012)tmp1=-68;
        else if(FT1<=3.11168)tmp1=-67.9;
        else if(FT1<=3.1222)tmp1=-67.8;
        else if(FT1<=3.13274)tmp1=-67.7;
        else if(FT1<=3.14331)tmp1=-67.6;
        else if(FT1<=3.1539)tmp1=-67.5;
        else if(FT1<=3.16452)tmp1=-67.4;
        else if(FT1<=3.17516)tmp1=-67.3;
        else if(FT1<=3.18583)tmp1=-67.2;
        else if(FT1<=3.19653)tmp1=-67.1;
        else if(FT1<=3.20725)tmp1=-67;
    }
    else if(FT1<=3.30492){
        if(FT1==3.20725)tmp1=-67;
        else if(FT1<=3.218)tmp1=-66.9;
        else if(FT1<=3.22877)tmp1=-66.8;
        else if(FT1<=3.23957)tmp1=-66.7;
        else if(FT1<=3.2504)tmp1=-66.6;
        else if(FT1<=3.26125)tmp1=-66.5;
        else if(FT1<=3.27213)tmp1=-66.4;
        else if(FT1<=3.28304)tmp1=-66.3;
        else if(FT1<=3.29397)tmp1=-66.2;
        else if(FT1<=3.30492)tmp1=-66.1;
        else if(FT1<=3.31591)tmp1=-66;
    }
    else if(FT1<=3.41595){
        if(FT1==3.31591)tmp1=-66;
        else if(FT1<=3.32692)tmp1=-65.9;
        else if(FT1<=3.33795)tmp1=-65.8;
        else if(FT1<=3.34901)tmp1=-65.7;
        else if(FT1<=3.3601)tmp1=-65.6;
        else if(FT1<=3.37122)tmp1=-65.5;
        else if(FT1<=3.38236)tmp1=-65.4;
        else if(FT1<=3.39353)tmp1=-65.3;
        else if(FT1<=3.40472)tmp1=-65.2;
        else if(FT1<=3.41595)tmp1=-65.1;
        else if(FT1<=3.42719)tmp1=-65;
    }
    else if(FT1<=3.52963){
        if(FT1==3.42719)tmp1=-65;
        else if(FT1<=3.43847)tmp1=-64.9;
        else if(FT1<=3.44977)tmp1=-64.8;
        else if(FT1<=3.4611)tmp1=-64.7;
        else if(FT1<=3.47245)tmp1=-64.6;
        else if(FT1<=3.48384)tmp1=-64.5;
        else if(FT1<=3.49524)tmp1=-64.4;
        else if(FT1<=3.50668)tmp1=-64.3;
        else if(FT1<=3.51814)tmp1=-64.2;
        else if(FT1<=3.52963)tmp1=-64.1;
        else if(FT1<=3.54115)tmp1=-64;
    }
    else if(FT1<=3.64602){
        if(FT1==3.54115)tmp1=-64;
        else if(FT1<=3.55269)tmp1=-63.9;
        else if(FT1<=3.56426)tmp1=-63.8;
        else if(FT1<=3.57586)tmp1=-63.7;
        else if(FT1<=3.58748)tmp1=-63.6;
        else if(FT1<=3.59914)tmp1=-63.5;
        else if(FT1<=3.61081)tmp1=-63.4;
        else if(FT1<=3.62252)tmp1=-63.3;
        else if(FT1<=3.63425)tmp1=-63.2;
        else if(FT1<=3.64602)tmp1=-63.1;
    }
    else if(FT1<=3.76513){
        if(FT1==3.6578)tmp1=-63;
        else if(FT1<=3.66962)tmp1=-62.9;
        else if(FT1<=3.68146)tmp1=-62.8;
        else if(FT1<=3.69333)tmp1=-62.7;
        else if(FT1<=3.70523)tmp1=-62.6;
        else if(FT1<=3.71715)tmp1=-62.5;
        else if(FT1<=3.72911)tmp1=-62.4;
        else if(FT1<=3.74109)tmp1=-62.3;
        else if(FT1<=3.75309)tmp1=-62.2;
        else if(FT1<=3.76513)tmp1=-62.1;
    }
    else if(FT1<=3.88701){
        if(FT1==3.77719)tmp1=-62;
        else if(FT1<=3.78928)tmp1=-61.9;
        else if(FT1<=3.8014)tmp1=-61.8;
        else if(FT1<=3.81355)tmp1=-61.7;
        else if(FT1<=3.82572)tmp1=-61.6;
        else if(FT1<=3.83792)tmp1=-61.5;
        else if(FT1<=3.85015)tmp1=-61.4;
        else if(FT1<=3.86241)tmp1=-61.3;
        else if(FT1<=3.8747)tmp1=-61.2;
        else if(FT1<=3.88701)tmp1=-61.1;
    }
    else if(FT1<=4.01169){
        if(FT1==3.89935)tmp1=-61;
        else if(FT1<=3.91172)tmp1=-60.9;
        else if(FT1<=3.92412)tmp1=-60.8;
        else if(FT1<=3.93654)tmp1=-60.7;
        else if(FT1<=3.949)tmp1=-60.6;
        else if(FT1<=3.96148)tmp1=-60.5;
        else if(FT1<=3.97399)tmp1=-60.4;
        else if(FT1<=3.98653)tmp1=-60.3;
        else if(FT1<=3.99909)tmp1=-60.2;
        else if(FT1<=4.01169)tmp1=-60.1;
    }
    else if(FT1<=4.1392){
        if(FT1==4.02431)tmp1=-60;
        else if(FT1<=4.03696)tmp1=-59.9;
        else if(FT1<=4.04964)tmp1=-59.8;
        else if(FT1<=4.06235)tmp1=-59.7;
        else if(FT1<=4.07509)tmp1=-59.6;
        else if(FT1<=4.08785)tmp1=-59.5;
        else if(FT1<=4.10064)tmp1=-59.4;
        else if(FT1<=4.11347)tmp1=-59.3;
        else if(FT1<=4.12632)tmp1=-59.2;
        else if(FT1<=4.1392)tmp1=-59.1;
    }
    else if(FT1<=4.26957){
        if(FT1==4.1521)tmp1=-59;
        else if(FT1<=4.16504)tmp1=-58.9;
        else if(FT1<=4.17801)tmp1=-58.8;
        else if(FT1<=4.191)tmp1=-58.7;
        else if(FT1<=4.20402)tmp1=-58.6;
        else if(FT1<=4.21707)tmp1=-58.5;
        else if(FT1<=4.23016)tmp1=-58.4;
        else if(FT1<=4.24326)tmp1=-58.3;
        else if(FT1<=4.2564)tmp1=-58.2;
        else if(FT1<=4.26957)tmp1=-58.1;
    }
    else if(FT1<=4.40284){
        if(FT1==4.28277)tmp1=-58;
        else if(FT1<=4.29599)tmp1=-57.9;
        else if(FT1<=4.30925)tmp1=-57.8;
        else if(FT1<=4.32253)tmp1=-57.7;
        else if(FT1<=4.33584)tmp1=-57.6;
        else if(FT1<=4.34918)tmp1=-57.5;
        else if(FT1<=4.36256)tmp1=-57.4;
        else if(FT1<=4.37596)tmp1=-57.3;
        else if(FT1<=4.38938)tmp1=-57.2;
        else if(FT1<=4.40284)tmp1=-57.1;
    }
    else if(FT1<=4.53905){
        if(FT1==4.41633)tmp1=-57;
        else if(FT1<=4.42985)tmp1=-56.9;
        else if(FT1<=4.44339)tmp1=-56.8;
        else if(FT1<=4.45697)tmp1=-56.7;
        else if(FT1<=4.47058)tmp1=-56.6;
        else if(FT1<=4.48421)tmp1=-56.5;
        else if(FT1<=4.49788)tmp1=-56.4;
        else if(FT1<=4.51157)tmp1=-56.3;
        else if(FT1<=4.52529)tmp1=-56.2;
        else if(FT1<=4.53905)tmp1=-56.1;
    }
    else if(FT1<=4.67821){
        if(FT1==4.55283)tmp1=-56;
        else if(FT1<=4.56664)tmp1=-55.9;
        else if(FT1<=4.58048)tmp1=-55.8;
        else if(FT1<=4.59436)tmp1=-55.7;
        else if(FT1<=4.60826)tmp1=-55.6;
        else if(FT1<=4.62219)tmp1=-55.5;
        else if(FT1<=4.63615)tmp1=-55.4;
        else if(FT1<=4.65014)tmp1=-55.3;
        else if(FT1<=4.66416)tmp1=-55.2;
        else if(FT1<=4.67821)tmp1=-55.1;
    }
    else if(FT1<=4.82038){
        if(FT1==4.69229)tmp1=-55;
        else if(FT1<=4.70641)tmp1=-54.9;
        else if(FT1<=4.72055)tmp1=-54.8;
        else if(FT1<=4.73472)tmp1=-54.7;
        else if(FT1<=4.74892)tmp1=-54.6;
        else if(FT1<=4.76315)tmp1=-54.5;
        else if(FT1<=4.77741)tmp1=-54.4;
        else if(FT1<=4.7917)tmp1=-54.3;
        else if(FT1<=4.80602)tmp1=-54.2;
        else if(FT1<=4.82038)tmp1=-54.1;
    }
    else if(FT1<=4.96557){
        if(FT1==4.83476)tmp1=-54;
        else if(FT1<=4.84917)tmp1=-53.9;
        else if(FT1<=4.86361)tmp1=-53.8;
        else if(FT1<=4.87809)tmp1=-53.7;
        else if(FT1<=4.89259)tmp1=-53.6;
        else if(FT1<=4.90713)tmp1=-53.5;
        else if(FT1<=4.92169)tmp1=-53.4;
        else if(FT1<=4.93629)tmp1=-53.3;
        else if(FT1<=4.95091)tmp1=-53.2;
        else if(FT1<=4.96557)tmp1=-53.1;
    }
    else if(FT1<=5.11382){
        if(FT1==4.98025)tmp1=-53;
        else if(FT1<=4.99497)tmp1=-52.9;
        else if(FT1<=5.00972)tmp1=-52.8;
        else if(FT1<=5.0245)tmp1=-52.7;
        else if(FT1<=5.03931)tmp1=-52.6;
        else if(FT1<=5.05415)tmp1=-52.5;
        else if(FT1<=5.06902)tmp1=-52.4;
        else if(FT1<=5.08392)tmp1=-52.3;
        else if(FT1<=5.09885)tmp1=-52.2;
        else if(FT1<=5.11382)tmp1=-52.1;
    }
    else if(FT1<=5.26516){
        if(FT1==5.12881)tmp1=-52;
        else if(FT1<=5.14384)tmp1=-51.9;
        else if(FT1<=5.1589)tmp1=-51.8;
        else if(FT1<=5.17398)tmp1=-51.7;
        else if(FT1<=5.1891)tmp1=-51.6;
        else if(FT1<=5.20425)tmp1=-51.5;
        else if(FT1<=5.21943)tmp1=-51.4;
        else if(FT1<=5.23464)tmp1=-51.3;
        else if(FT1<=5.24989)tmp1=-51.2;
        else if(FT1<=5.26516)tmp1=-51.1;
    }
    else if(FT1<=5.41963){
        if(FT1==5.28047)tmp1=-51;
        else if(FT1<=5.2958)tmp1=-50.9;
        else if(FT1<=5.31117)tmp1=-50.8;
        else if(FT1<=5.32657)tmp1=-50.7;
        else if(FT1<=5.342)tmp1=-50.6;
        else if(FT1<=5.35746)tmp1=-50.5;
        else if(FT1<=5.37296)tmp1=-50.4;
        else if(FT1<=5.38848)tmp1=-50.3;
        else if(FT1<=5.40404)tmp1=-50.2;
        else if(FT1<=5.41963)tmp1=-50.1;
    }
    else if(FT1<=5.57725){
        if(FT1==5.43525)tmp1=-50;
        else if(FT1<=5.4509)tmp1=-49.9;
        else if(FT1<=5.46658)tmp1=-49.8;
        else if(FT1<=5.4823)tmp1=-49.7;
        else if(FT1<=5.49804)tmp1=-49.6;
        else if(FT1<=5.51382)tmp1=-49.5;
        else if(FT1<=5.52963)tmp1=-49.4;
        else if(FT1<=5.54547)tmp1=-49.3;
        else if(FT1<=5.56134)tmp1=-49.2;
        else if(FT1<=5.57725)tmp1=-49.1;
    }
    else if(FT1<=5.73806){
        if(FT1==5.59319)tmp1=-49;
        else if(FT1<=5.60916)tmp1=-48.9;
        else if(FT1<=5.62516)tmp1=-48.8;
        else if(FT1<=5.64119)tmp1=-48.7;
        else if(FT1<=5.65725)tmp1=-48.6;
        else if(FT1<=5.67335)tmp1=-48.5;
        else if(FT1<=5.68948)tmp1=-48.4;
        else if(FT1<=5.70564)tmp1=-48.3;
        else if(FT1<=5.72183)tmp1=-48.2;
        else if(FT1<=5.73806)tmp1=-48.1;
    }
    else if(FT1<=5.90208){
        if(FT1==5.75431)tmp1=-48;
        else if(FT1<=5.7706)tmp1=-47.9;
        else if(FT1<=5.78693)tmp1=-47.8;
        else if(FT1<=5.80328)tmp1=-47.7;
        else if(FT1<=5.81967)tmp1=-47.6;
        else if(FT1<=5.83608)tmp1=-47.5;
        else if(FT1<=5.85253)tmp1=-47.4;
        else if(FT1<=5.86902)tmp1=-47.3;
        else if(FT1<=5.88553)tmp1=-47.2;
        else if(FT1<=5.90208)tmp1=-47.1;
    }
    else if(FT1<=6.06935){
        if(FT1==5.91866)tmp1=-47;
        else if(FT1<=5.93528)tmp1=-46.9;
        else if(FT1<=5.95192)tmp1=-46.8;
        else if(FT1<=5.9686)tmp1=-46.7;
        else if(FT1<=5.98531)tmp1=-46.6;
        else if(FT1<=6.00205)tmp1=-46.5;
        else if(FT1<=6.01883)tmp1=-46.4;
        else if(FT1<=6.03564)tmp1=-46.3;
        else if(FT1<=6.05248)tmp1=-46.2;
        else if(FT1<=6.06935)tmp1=-46.1;
    }
    else if(FT1<=6.2399){
        if(FT1==6.08626)tmp1=-46;
        else if(FT1<=6.1032)tmp1=-45.9;
        else if(FT1<=6.12017)tmp1=-45.8;
        else if(FT1<=6.13718)tmp1=-45.7;
        else if(FT1<=6.15422)tmp1=-45.6;
        else if(FT1<=6.17129)tmp1=-45.5;
        else if(FT1<=6.18839)tmp1=-45.4;
        else if(FT1<=6.20553)tmp1=-45.3;
        else if(FT1<=6.2227)tmp1=-45.2;
        else if(FT1<=6.2399)tmp1=-45.1;
    }
    else if(FT1<=6.41376){
        if(FT1==6.25714)tmp1=-45;
        else if(FT1<=6.27441)tmp1=-44.9;
        else if(FT1<=6.29171)tmp1=-44.8;
        else if(FT1<=6.30905)tmp1=-44.7;
        else if(FT1<=6.32642)tmp1=-44.6;
        else if(FT1<=6.34382)tmp1=-44.5;
        else if(FT1<=6.36125)tmp1=-44.4;
        else if(FT1<=6.37872)tmp1=-44.3;
        else if(FT1<=6.39622)tmp1=-44.2;
        else if(FT1<=6.41376)tmp1=-44.1;
    }
    else if(FT1<=6.59096){
        if(FT1==6.43133)tmp1=-44;
        else if(FT1<=6.44893)tmp1=-43.9;
        else if(FT1<=6.46657)tmp1=-43.8;
        else if(FT1<=6.48424)tmp1=-43.7;
        else if(FT1<=6.50194)tmp1=-43.6;
        else if(FT1<=6.51968)tmp1=-43.5;
        else if(FT1<=6.53744)tmp1=-43.4;
        else if(FT1<=6.55525)tmp1=-43.3;
        else if(FT1<=6.57309)tmp1=-43.2;
        else if(FT1<=6.59096)tmp1=-43.1;
    }
    else if(FT1<=6.77152){
        if(FT1==6.60886)tmp1=-43;
        else if(FT1<=6.6268)tmp1=-42.9;
        else if(FT1<=6.64477)tmp1=-42.8;
        else if(FT1<=6.66278)tmp1=-42.7;
        else if(FT1<=6.68081)tmp1=-42.6;
        else if(FT1<=6.69889)tmp1=-42.5;
        else if(FT1<=6.71699)tmp1=-42.4;
        else if(FT1<=6.73514)tmp1=-42.3;
        else if(FT1<=6.75331)tmp1=-42.2;
        else if(FT1<=6.77152)tmp1=-42.1;
    }
    else if(FT1<=6.95548){
        if(FT1==6.78976)tmp1=-42;
        else if(FT1<=6.80804)tmp1=-41.9;
        else if(FT1<=6.82635)tmp1=-41.8;
        else if(FT1<=6.84469)tmp1=-41.7;
        else if(FT1<=6.86307)tmp1=-41.6;
        else if(FT1<=6.88148)tmp1=-41.5;
        else if(FT1<=6.89993)tmp1=-41.4;
        else if(FT1<=6.91841)tmp1=-41.3;
        else if(FT1<=6.93693)tmp1=-41.2;
        else if(FT1<=6.95548)tmp1=-41.1;
    }
    else if(FT1<=7.14286){
        if(FT1==6.97406)tmp1=-41;
        else if(FT1<=6.99268)tmp1=-40.9;
        else if(FT1<=7.01133)tmp1=-40.8;
        else if(FT1<=7.03002)tmp1=-40.7;
        else if(FT1<=7.04874)tmp1=-40.6;
        else if(FT1<=7.0675)tmp1=-40.5;
        else if(FT1<=7.08629)tmp1=-40.4;
        else if(FT1<=7.10511)tmp1=-40.3;
        else if(FT1<=7.12397)tmp1=-40.2;
        else if(FT1<=7.14286)tmp1=-40.1;
    }
    else if(FT1<=7.3337){
        if(FT1==7.16179)tmp1=-40;
        else if(FT1<=7.18075)tmp1=-39.9;
        else if(FT1<=7.19975)tmp1=-39.8;
        else if(FT1<=7.21878)tmp1=-39.7;
        else if(FT1<=7.23785)tmp1=-39.6;
        else if(FT1<=7.25695)tmp1=-39.5;
        else if(FT1<=7.27609)tmp1=-39.4;
        else if(FT1<=7.29526)tmp1=-39.3;
        else if(FT1<=7.31446)tmp1=-39.2;
        else if(FT1<=7.3337)tmp1=-39.1;
    }
    else if(FT1<=7.52803){
        if(FT1==7.35298)tmp1=-39;
        else if(FT1<=7.37229)tmp1=-38.9;
        else if(FT1<=7.39163)tmp1=-38.8;
        else if(FT1<=7.41101)tmp1=-38.7;
        else if(FT1<=7.43043)tmp1=-38.6;
        else if(FT1<=7.44988)tmp1=-38.5;
        else if(FT1<=7.46936)tmp1=-38.4;
        else if(FT1<=7.48888)tmp1=-38.3;
        else if(FT1<=7.50844)tmp1=-38.2;
        else if(FT1<=7.52803)tmp1=-38.1;
    }
    else if(FT1<=7.72586){
        if(FT1==7.54765)tmp1=-38;
        else if(FT1<=7.56731)tmp1=-37.9;
        else if(FT1<=7.58701)tmp1=-37.8;
        else if(FT1<=7.60674)tmp1=-37.7;
        else if(FT1<=7.6265)tmp1=-37.6;
        else if(FT1<=7.6463)tmp1=-37.5;
        else if(FT1<=7.66614)tmp1=-37.4;
        else if(FT1<=7.68601)tmp1=-37.3;
        else if(FT1<=7.70592)tmp1=-37.2;
        else if(FT1<=7.72586)tmp1=-37.1;
    }
    else if(FT1<=7.92724){
        if(FT1==7.74584)tmp1=-37;
        else if(FT1<=7.76585)tmp1=-36.9;
        else if(FT1<=7.7859)tmp1=-36.8;
        else if(FT1<=7.80598)tmp1=-36.7;
        else if(FT1<=7.8261)tmp1=-36.6;
        else if(FT1<=7.84626)tmp1=-36.5;
        else if(FT1<=7.86645)tmp1=-36.4;
        else if(FT1<=7.88668)tmp1=-36.3;
        else if(FT1<=7.90694)tmp1=-36.2;
        else if(FT1<=7.92724)tmp1=-36.1;
    }
    else if(FT1<=8.13218){
        if(FT1==7.94757)tmp1=-36;
        else if(FT1<=7.96794)tmp1=-35.9;
        else if(FT1<=7.98834)tmp1=-35.8;
        else if(FT1<=8.00878)tmp1=-35.7;
        else if(FT1<=8.02926)tmp1=-35.6;
        else if(FT1<=8.04977)tmp1=-35.5;
        else if(FT1<=8.07032)tmp1=-35.4;
        else if(FT1<=8.0909)tmp1=-35.3;
        else if(FT1<=8.11152)tmp1=-35.2;
        else if(FT1<=8.13218)tmp1=-35.1;
    }
    else if(FT1<=8.34071){
        if(FT1==8.15287)tmp1=-35;
        else if(FT1<=8.1736)tmp1=-34.9;
        else if(FT1<=8.19436)tmp1=-34.8;
        else if(FT1<=8.21516)tmp1=-34.7;
        else if(FT1<=8.23599)tmp1=-34.6;
        else if(FT1<=8.25686)tmp1=-34.5;
        else if(FT1<=8.27777)tmp1=-34.4;
        else if(FT1<=8.29872)tmp1=-34.3;
        else if(FT1<=8.3197)tmp1=-34.2;
        else if(FT1<=8.34071)tmp1=-34.1;
    }
    else if(FT1<=8.55287){
        if(FT1==8.36176)tmp1=-34;
        else if(FT1<=8.38285)tmp1=-33.9;
        else if(FT1<=8.40398)tmp1=-33.8;
        else if(FT1<=8.42514)tmp1=-33.7;
        else if(FT1<=8.44634)tmp1=-33.6;
        else if(FT1<=8.46757)tmp1=-33.5;
        else if(FT1<=8.48884)tmp1=-33.4;
        else if(FT1<=8.51015)tmp1=-33.3;
        else if(FT1<=8.53149)tmp1=-33.2;
        else if(FT1<=8.55287)tmp1=-33.1;
    }
    else if(FT1<=8.76867){
        if(FT1==8.57428)tmp1=-33;
        else if(FT1<=8.59574)tmp1=-32.9;
        else if(FT1<=8.61723)tmp1=-32.8;
        else if(FT1<=8.63875)tmp1=-32.7;
        else if(FT1<=8.66031)tmp1=-32.6;
        else if(FT1<=8.68191)tmp1=-32.5;
        else if(FT1<=8.70355)tmp1=-32.4;
        else if(FT1<=8.72522)tmp1=-32.3;
        else if(FT1<=8.74693)tmp1=-32.2;
        else if(FT1<=8.76867)tmp1=-32.1;
    }
    else if(FT1<=8.98815){
        if(FT1==8.79046)tmp1=-32;
        else if(FT1<=8.81228)tmp1=-31.9;
        else if(FT1<=8.83413)tmp1=-31.8;
        else if(FT1<=8.85602)tmp1=-31.7;
        else if(FT1<=8.87795)tmp1=-31.6;
        else if(FT1<=8.89992)tmp1=-31.5;
        else if(FT1<=8.92192)tmp1=-31.4;
        else if(FT1<=8.94396)tmp1=-31.3;
        else if(FT1<=8.96604)tmp1=-31.2;
        else if(FT1<=8.98815)tmp1=-31.1;
    }
    else if(FT1<=9.21134){
        if(FT1==9.01031)tmp1=-31;
        else if(FT1<=9.03249)tmp1=-30.9;
        else if(FT1<=9.05472)tmp1=-30.8;
        else if(FT1<=9.07698)tmp1=-30.7;
        else if(FT1<=9.09928)tmp1=-30.6;
        else if(FT1<=9.12162)tmp1=-30.5;
        else if(FT1<=9.14399)tmp1=-30.4;
        else if(FT1<=9.1664)tmp1=-30.3;
        else if(FT1<=9.18885)tmp1=-30.2;
        else if(FT1<=9.21134)tmp1=-30.1;
    }
    else if(FT1<=9.43824){
        if(FT1==9.23386)tmp1=-30;
        else if(FT1<=9.25642)tmp1=-29.9;
        else if(FT1<=9.27902)tmp1=-29.8;
        else if(FT1<=9.30165)tmp1=-29.7;
        else if(FT1<=9.32432)tmp1=-29.6;
        else if(FT1<=9.34703)tmp1=-29.5;
        else if(FT1<=9.36978)tmp1=-29.4;
        else if(FT1<=9.39256)tmp1=-29.3;
        else if(FT1<=9.41538)tmp1=-29.2;
        else if(FT1<=9.43824)tmp1=-29.1;
    }
    else if(FT1<=9.6689){
        if(FT1==9.46114)tmp1=-29;
        else if(FT1<=9.48407)tmp1=-28.9;
        else if(FT1<=9.50705)tmp1=-28.8;
        else if(FT1<=9.53006)tmp1=-28.7;
        else if(FT1<=9.5531)tmp1=-28.6;
        else if(FT1<=9.57619)tmp1=-28.5;
        else if(FT1<=9.59931)tmp1=-28.4;
        else if(FT1<=9.62247)tmp1=-28.3;
        else if(FT1<=9.64567)tmp1=-28.2;
        else if(FT1<=9.6689)tmp1=-28.1;
    }
    else if(FT1<=9.90334){
        if(FT1==9.69218)tmp1=-28;
        else if(FT1<=9.71549)tmp1=-27.9;
        else if(FT1<=9.73884)tmp1=-27.8;
        else if(FT1<=9.76222)tmp1=-27.7;
        else if(FT1<=9.78565)tmp1=-27.6;
        else if(FT1<=9.80911)tmp1=-27.5;
        else if(FT1<=9.83261)tmp1=-27.4;
        else if(FT1<=9.85615)tmp1=-27.3;
        else if(FT1<=9.87973)tmp1=-27.2;
        else if(FT1<=9.90334)tmp1=-27.1;
    }
    else if(FT1<=10.1416){
        if(FT1==9.927)tmp1=-27;
        else if(FT1<=9.95069)tmp1=-26.9;
        else if(FT1<=9.97441)tmp1=-26.8;
        else if(FT1<=9.99818)tmp1=-26.7;
        else if(FT1<=10.022)tmp1=-26.6;
        else if(FT1<=10.0458)tmp1=-26.5;
        else if(FT1<=10.0697)tmp1=-26.4;
        else if(FT1<=10.0936)tmp1=-26.3;
        else if(FT1<=10.1176)tmp1=-26.2;
        else if(FT1<=10.1416)tmp1=-26.1;
    }
    else if(FT1<=10.3837){
        if(FT1==10.1656)tmp1=-26;
        else if(FT1<=10.1897)tmp1=-25.9;
        else if(FT1<=10.2138)tmp1=-25.8;
        else if(FT1<=10.238)tmp1=-25.7;
        else if(FT1<=10.2621)tmp1=-25.6;
        else if(FT1<=10.2864)tmp1=-25.5;
        else if(FT1<=10.3106)tmp1=-25.4;
        else if(FT1<=10.3349)tmp1=-25.3;
        else if(FT1<=10.3593)tmp1=-25.2;
        else if(FT1<=10.3837)tmp1=-25.1;
    }
    else if(FT1<=10.6296){
        if(FT1==10.4081)tmp1=-25;
        else if(FT1<=10.4325)tmp1=-24.9;
        else if(FT1<=10.457)tmp1=-24.8;
        else if(FT1<=10.4816)tmp1=-24.7;
        else if(FT1<=10.5061)tmp1=-24.6;
        else if(FT1<=10.5307)tmp1=-24.5;
        else if(FT1<=10.5554)tmp1=-24.4;
        else if(FT1<=10.5801)tmp1=-24.3;
        else if(FT1<=10.6048)tmp1=-24.2;
        else if(FT1<=10.6296)tmp1=-24.1;
    }
    else if(FT1<=10.8794){
        if(FT1==10.6544)tmp1=-24;
        else if(FT1<=10.6792)tmp1=-23.9;
        else if(FT1<=10.7041)tmp1=-23.8;
        else if(FT1<=10.729)tmp1=-23.7;
        else if(FT1<=10.754)tmp1=-23.6;
        else if(FT1<=10.779)tmp1=-23.5;
        else if(FT1<=10.804)tmp1=-23.4;
        else if(FT1<=10.8291)tmp1=-23.3;
        else if(FT1<=10.8542)tmp1=-23.2;
        else if(FT1<=10.8794)tmp1=-23.1;
    }
    else if(FT1<=11.1331){
        if(FT1==10.9046)tmp1=-23;
        else if(FT1<=10.9298)tmp1=-22.9;
        else if(FT1<=10.9551)tmp1=-22.8;
        else if(FT1<=10.9804)tmp1=-22.7;
        else if(FT1<=11.0057)tmp1=-22.6;
        else if(FT1<=11.0311)tmp1=-22.5;
        else if(FT1<=11.0566)tmp1=-22.4;
        else if(FT1<=11.082)tmp1=-22.3;
        else if(FT1<=11.1075)tmp1=-22.2;
        else if(FT1<=11.1331)tmp1=-22.1;
    }
    else if(FT1<=11.3907){
        if(FT1==11.1587)tmp1=-22;
        else if(FT1<=11.1843)tmp1=-21.9;
        else if(FT1<=11.2099)tmp1=-21.8;
        else if(FT1<=11.2356)tmp1=-21.7;
        else if(FT1<=11.2614)tmp1=-21.6;
        else if(FT1<=11.2872)tmp1=-21.5;
        else if(FT1<=11.313)tmp1=-21.4;
        else if(FT1<=11.3389)tmp1=-21.3;
        else if(FT1<=11.3648)tmp1=-21.2;
        else if(FT1<=11.3907)tmp1=-21.1;
    }
    else if(FT1<=11.6523){
        if(FT1==11.4167)tmp1=-21;
        else if(FT1<=11.4427)tmp1=-20.9;
        else if(FT1<=11.4687)tmp1=-20.8;
        else if(FT1<=11.4948)tmp1=-20.7;
        else if(FT1<=11.521)tmp1=-20.6;
        else if(FT1<=11.5472)tmp1=-20.5;
        else if(FT1<=11.5734)tmp1=-20.4;
        else if(FT1<=11.5996)tmp1=-20.3;
        else if(FT1<=11.6259)tmp1=-20.2;
        else if(FT1<=11.6523)tmp1=-20.1;
    }
    else if(FT1<=11.9178){
        if(FT1==11.6786)tmp1=-20;
        else if(FT1<=11.705)tmp1=-19.9;
        else if(FT1<=11.7315)tmp1=-19.8;
        else if(FT1<=11.758)tmp1=-19.7;
        else if(FT1<=11.7845)tmp1=-19.6;
        else if(FT1<=11.8111)tmp1=-19.5;
        else if(FT1<=11.8377)tmp1=-19.4;
        else if(FT1<=11.8644)tmp1=-19.3;
        else if(FT1<=11.8911)tmp1=-19.2;
        else if(FT1<=11.9178)tmp1=-19.1;
    }
    else if(FT1<=12.1873){
        if(FT1==11.9446)tmp1=-19;
        else if(FT1<=11.9714)tmp1=-18.9;
        else if(FT1<=11.9982)tmp1=-18.8;
        else if(FT1<=12.0251)tmp1=-18.7;
        else if(FT1<=12.0521)tmp1=-18.6;
        else if(FT1<=12.079)tmp1=-18.5;
        else if(FT1<=12.106)tmp1=-18.4;
        else if(FT1<=12.1331)tmp1=-18.3;
        else if(FT1<=12.1602)tmp1=-18.2;
        else if(FT1<=12.1873)tmp1=-18.1;
    }
    else if(FT1<=12.4609){
        if(FT1==12.2145)tmp1=-18;
        else if(FT1<=12.2417)tmp1=-17.9;
        else if(FT1<=12.269)tmp1=-17.8;
        else if(FT1<=12.2963)tmp1=-17.7;
        else if(FT1<=12.3236)tmp1=-17.6;
        else if(FT1<=12.351)tmp1=-17.5;
        else if(FT1<=12.3784)tmp1=-17.4;
        else if(FT1<=12.4058)tmp1=-17.3;
        else if(FT1<=12.4333)tmp1=-17.2;
        else if(FT1<=12.4609)tmp1=-17.1;
    }
    else if(FT1<=12.7384){
        if(FT1==12.4884)tmp1=-17;
        else if(FT1<=12.5161)tmp1=-16.9;
        else if(FT1<=12.5437)tmp1=-16.8;
        else if(FT1<=12.5714)tmp1=-16.7;
        else if(FT1<=12.5991)tmp1=-16.6;
        else if(FT1<=12.6269)tmp1=-16.5;
        else if(FT1<=12.6547)tmp1=-16.4;
        else if(FT1<=12.6826)tmp1=-16.3;
        else if(FT1<=12.7105)tmp1=-16.2;
        else if(FT1<=12.7384)tmp1=-16.1;
    }
    else if(FT1<=13.0201){
        if(FT1==12.7664)tmp1=-16;
        else if(FT1<=12.7944)tmp1=-15.9;
        else if(FT1<=12.8225)tmp1=-15.8;
        else if(FT1<=12.8506)tmp1=-15.7;
        else if(FT1<=12.8787)tmp1=-15.6;
        else if(FT1<=12.9069)tmp1=-15.5;
        else if(FT1<=12.9352)tmp1=-15.4;
        else if(FT1<=12.9634)tmp1=-15.3;
        else if(FT1<=12.9917)tmp1=-15.2;
        else if(FT1<=13.0201)tmp1=-15.1;
        else if(FT1<=13.0485)tmp1=-15;
    }
    else if(FT1<=13.3058){
        if(FT1==13.0485)tmp1=-15;
        else if(FT1<=13.0769)tmp1=-14.9;
        else if(FT1<=13.1054)tmp1=-14.8;
        else if(FT1<=13.1339)tmp1=-14.7;
        else if(FT1<=13.1624)tmp1=-14.6;
        else if(FT1<=13.191)tmp1=-14.5;
        else if(FT1<=13.2196)tmp1=-14.4;
        else if(FT1<=13.2483)tmp1=-14.3;
        else if(FT1<=13.277)tmp1=-14.2;
        else if(FT1<=13.3058)tmp1=-14.1;
        else if(FT1<=13.3346)tmp1=-14;
    }
    else if(FT1<=13.5956){
        if(FT1==13.3346)tmp1=-14;
        else if(FT1<=13.3634)tmp1=-13.9;
        else if(FT1<=13.3923)tmp1=-13.8;
        else if(FT1<=13.4212)tmp1=-13.7;
        else if(FT1<=13.4502)tmp1=-13.6;
        else if(FT1<=13.4792)tmp1=-13.5;
        else if(FT1<=13.5082)tmp1=-13.4;
        else if(FT1<=13.5373)tmp1=-13.3;
        else if(FT1<=13.5664)tmp1=-13.2;
        else if(FT1<=13.5956)tmp1=-13.1;
        else if(FT1<=13.6248)tmp1=-13;
    }
    else if(FT1<=13.8895){
        if(FT1==13.6248)tmp1=-13;
        else if(FT1<=13.6541)tmp1=-12.9;
        else if(FT1<=13.6833)tmp1=-12.8;
        else if(FT1<=13.7127)tmp1=-12.7;
        else if(FT1<=13.742)tmp1=-12.6;
        else if(FT1<=13.7715)tmp1=-12.5;
        else if(FT1<=13.8009)tmp1=-12.4;
        else if(FT1<=13.8304)tmp1=-12.3;
        else if(FT1<=13.8599)tmp1=-12.2;
        else if(FT1<=13.8895)tmp1=-12.1;
        else if(FT1<=13.9191)tmp1=-12;
    }
    else if(FT1<=14.1876){
        if(FT1==13.9191)tmp1=-12;
        else if(FT1<=13.9488)tmp1=-11.9;
        else if(FT1<=13.9785)tmp1=-11.8;
        else if(FT1<=14.0083)tmp1=-11.7;
        else if(FT1<=14.038)tmp1=-11.6;
        else if(FT1<=14.0679)tmp1=-11.5;
        else if(FT1<=14.0977)tmp1=-11.4;
        else if(FT1<=14.1277)tmp1=-11.3;
        else if(FT1<=14.1576)tmp1=-11.2;
        else if(FT1<=14.1876)tmp1=-11.1;
        else if(FT1<=14.2176)tmp1=-11;
    }
    else if(FT1<=14.4898){
        if(FT1==14.2176)tmp1=-11;
        else if(FT1<=14.2477)tmp1=-10.9;
        else if(FT1<=14.2778)tmp1=-10.8;
        else if(FT1<=14.308)tmp1=-10.7;
        else if(FT1<=14.3382)tmp1=-10.6;
        else if(FT1<=14.3684)tmp1=-10.5;
        else if(FT1<=14.3987)tmp1=-10.4;
        else if(FT1<=14.4291)tmp1=-10.3;
        else if(FT1<=14.4594)tmp1=-10.2;
        else if(FT1<=14.4898)tmp1=-10.1;
        else if(FT1<=14.5203)tmp1=-10;
    }
    else if(FT1<=14.7962){
        if(FT1==14.5203)tmp1=-10;
        else if(FT1<=14.5508)tmp1=-9.9;
        else if(FT1<=14.5813)tmp1=-9.8;
        else if(FT1<=14.6119)tmp1=-9.7;
        else if(FT1<=14.6425)tmp1=-9.6;
        else if(FT1<=14.6732)tmp1=-9.5;
        else if(FT1<=14.7039)tmp1=-9.4;
        else if(FT1<=14.7346)tmp1=-9.3;
        else if(FT1<=14.7654)tmp1=-9.2;
        else if(FT1<=14.7962)tmp1=-9.1;
        else if(FT1<=14.8271)tmp1=-9;
    }
    else if(FT1<=15.1069){
        if(FT1==14.8271)tmp1=-9;
        else if(FT1<=14.858)tmp1=-8.9;
        else if(FT1<=14.889)tmp1=-8.8;
        else if(FT1<=14.92)tmp1=-8.7;
        else if(FT1<=14.951)tmp1=-8.6;
        else if(FT1<=14.9821)tmp1=-8.5;
        else if(FT1<=15.0132)tmp1=-8.4;
        else if(FT1<=15.0444)tmp1=-8.3;
        else if(FT1<=15.0756)tmp1=-8.2;
        else if(FT1<=15.1069)tmp1=-8.1;
        else if(FT1<=15.1382)tmp1=-8;
    }
    else if(FT1<=15.4217){
        if(FT1==15.1382)tmp1=-8;
        else if(FT1<=15.1695)tmp1=-7.9;
        else if(FT1<=15.2009)tmp1=-7.8;
        else if(FT1<=15.2323)tmp1=-7.7;
        else if(FT1<=15.2637)tmp1=-7.6;
        else if(FT1<=15.2952)tmp1=-7.5;
        else if(FT1<=15.3268)tmp1=-7.4;
        else if(FT1<=15.3584)tmp1=-7.3;
        else if(FT1<=15.39)tmp1=-7.2;
        else if(FT1<=15.4217)tmp1=-7.1;
        else if(FT1<=15.4534)tmp1=-7;
    }
    else if(FT1<=15.7408){
        if(FT1==15.4534)tmp1=-7;
        else if(FT1<=15.4852)tmp1=-6.9;
        else if(FT1<=15.517)tmp1=-6.8;
        else if(FT1<=15.5488)tmp1=-6.7;
        else if(FT1<=15.5807)tmp1=-6.6;
        else if(FT1<=15.6126)tmp1=-6.5;
        else if(FT1<=15.6446)tmp1=-6.4;
        else if(FT1<=15.6766)tmp1=-6.3;
        else if(FT1<=15.7087)tmp1=-6.2;
        else if(FT1<=15.7408)tmp1=-6.1;
        else if(FT1<=15.7729)tmp1=-6;
    }
    else if(FT1<=16.0641){
        if(FT1==15.7729)tmp1=-6;
        else if(FT1<=15.8051)tmp1=-5.9;
        else if(FT1<=15.8373)tmp1=-5.8;
        else if(FT1<=15.8696)tmp1=-5.7;
        else if(FT1<=15.9019)tmp1=-5.6;
        else if(FT1<=15.9342)tmp1=-5.5;
        else if(FT1<=15.9666)tmp1=-5.4;
        else if(FT1<=15.9991)tmp1=-5.3;
        else if(FT1<=16.0316)tmp1=-5.2;
        else if(FT1<=16.0641)tmp1=-5.1;
        else if(FT1<=16.0966)tmp1=-5;
    }
    else if(FT1<=16.3917){
        if(FT1==16.0966)tmp1=-5;
        else if(FT1<=16.1293)tmp1=-4.9;
        else if(FT1<=16.1619)tmp1=-4.8;
        else if(FT1<=16.1946)tmp1=-4.7;
        else if(FT1<=16.2273)tmp1=-4.6;
        else if(FT1<=16.2601)tmp1=-4.5;
        else if(FT1<=16.2929)tmp1=-4.4;
        else if(FT1<=16.3258)tmp1=-4.3;
        else if(FT1<=16.3587)tmp1=-4.2;
        else if(FT1<=16.3917)tmp1=-4.1;
        else if(FT1<=16.4247)tmp1=-4;
    }
    else if(FT1<=16.7236){
        if(FT1==16.4247)tmp1=-4;
        else if(FT1<=16.4577)tmp1=-3.9;
        else if(FT1<=16.4908)tmp1=-3.8;
        else if(FT1<=16.5239)tmp1=-3.7;
        else if(FT1<=16.5571)tmp1=-3.6;
        else if(FT1<=16.5903)tmp1=-3.5;
        else if(FT1<=16.6235)tmp1=-3.4;
        else if(FT1<=16.6568)tmp1=-3.3;
        else if(FT1<=16.6902)tmp1=-3.2;
        else if(FT1<=16.7236)tmp1=-3.1;
    }
    else if(FT1<=17.0598){
        if(FT1==16.757)tmp1=-3;
        else if(FT1<=16.7904)tmp1=-2.9;
        else if(FT1<=16.824)tmp1=-2.8;
        else if(FT1<=16.8575)tmp1=-2.7;
        else if(FT1<=16.8911)tmp1=-2.6;
        else if(FT1<=16.9248)tmp1=-2.5;
        else if(FT1<=16.9584)tmp1=-2.4;
        else if(FT1<=16.9922)tmp1=-2.3;
        else if(FT1<=17.0259)tmp1=-2.2;
        else if(FT1<=17.0598)tmp1=-2.1;
    }
    else if(FT1<=17.4003){
        if(FT1==17.0936)tmp1=-2;
        else if(FT1<=17.1275)tmp1=-1.9;
        else if(FT1<=17.1615)tmp1=-1.8;
        else if(FT1<=17.1954)tmp1=-1.7;
        else if(FT1<=17.2295)tmp1=-1.6;
        else if(FT1<=17.2636)tmp1=-1.5;
        else if(FT1<=17.2977)tmp1=-1.4;
        else if(FT1<=17.3318)tmp1=-1.3;
        else if(FT1<=17.366)tmp1=-1.2;
        else if(FT1<=17.4003)tmp1=-1.1;
    }
    else if(FT1<=17.7451){
        if(FT1==17.4346)tmp1=-1;
        else if(FT1<=17.4689)tmp1=-0.9;
        else if(FT1<=17.5033)tmp1=-0.8;
        else if(FT1<=17.5377)tmp1=-0.7;
        else if(FT1<=17.5722)tmp1=-0.6;
        else if(FT1<=17.6067)tmp1=-0.5;
        else if(FT1<=17.6412)tmp1=-0.4;
        else if(FT1<=17.6758)tmp1=-0.3;
        else if(FT1<=17.7105)tmp1=-0.2;
        else if(FT1<=17.7451)tmp1=-0.1;
        else if(FT1<=17.7799)tmp1=-1.38778e-16;
    }
    else if(FT1<=18.0944){
        if(FT1==17.7799)tmp1=0;
        else if(FT1<=17.8146)tmp1=0.1;
        else if(FT1<=17.8495)tmp1=0.2;
        else if(FT1<=17.8843)tmp1=0.3;
        else if(FT1<=17.9192)tmp1=0.4;
        else if(FT1<=17.9542)tmp1=0.5;
        else if(FT1<=17.9891)tmp1=0.6;
        else if(FT1<=18.0242)tmp1=0.7;
        else if(FT1<=18.0593)tmp1=0.8;
        else if(FT1<=18.0944)tmp1=0.9;
        else if(FT1<=18.1295)tmp1=1;
    }
    else if(FT1<=18.448){
        if(FT1==18.1295)tmp1=1;
        else if(FT1<=18.1647)tmp1=1.1;
        else if(FT1<=18.2)tmp1=1.2;
        else if(FT1<=18.2353)tmp1=1.3;
        else if(FT1<=18.2706)tmp1=1.4;
        else if(FT1<=18.306)tmp1=1.5;
        else if(FT1<=18.3414)tmp1=1.6;
        else if(FT1<=18.3769)tmp1=1.7;
        else if(FT1<=18.4124)tmp1=1.8;
        else if(FT1<=18.448)tmp1=1.9;
    }
    else if(FT1<=18.806){
        if(FT1==18.4836)tmp1=2;
        else if(FT1<=18.5192)tmp1=2.1;
        else if(FT1<=18.5549)tmp1=2.2;
        else if(FT1<=18.5906)tmp1=2.3;
        else if(FT1<=18.6264)tmp1=2.4;
        else if(FT1<=18.6622)tmp1=2.5;
        else if(FT1<=18.6981)tmp1=2.6;
        else if(FT1<=18.734)tmp1=2.7;
        else if(FT1<=18.77)tmp1=2.8;
        else if(FT1<=18.806)tmp1=2.9;
    }
    else if(FT1<=19.1684){
        if(FT1==18.842)tmp1=3;
        else if(FT1<=18.8781)tmp1=3.1;
        else if(FT1<=18.9142)tmp1=3.2;
        else if(FT1<=18.9504)tmp1=3.3;
        else if(FT1<=18.9866)tmp1=3.4;
        else if(FT1<=19.0229)tmp1=3.5;
        else if(FT1<=19.0592)tmp1=3.6;
        else if(FT1<=19.0955)tmp1=3.7;
        else if(FT1<=19.1319)tmp1=3.8;
        else if(FT1<=19.1684)tmp1=3.9;
    }
    else if(FT1<=19.5352){
        if(FT1==19.2048)tmp1=4;
        else if(FT1<=19.2414)tmp1=4.1;
        else if(FT1<=19.2779)tmp1=4.2;
        else if(FT1<=19.3146)tmp1=4.3;
        else if(FT1<=19.3512)tmp1=4.4;
        else if(FT1<=19.3879)tmp1=4.5;
        else if(FT1<=19.4247)tmp1=4.6;
        else if(FT1<=19.4615)tmp1=4.7;
        else if(FT1<=19.4983)tmp1=4.8;
        else if(FT1<=19.5352)tmp1=4.9;
        else if(FT1<=19.5721)tmp1=5;
    }
    else if(FT1<=19.9064){
        if(FT1==19.5721)tmp1=5;
        else if(FT1<=19.6091)tmp1=5.1;
        else if(FT1<=19.6461)tmp1=5.2;
        else if(FT1<=19.6831)tmp1=5.3;
        else if(FT1<=19.7202)tmp1=5.4;
        else if(FT1<=19.7574)tmp1=5.5;
        else if(FT1<=19.7946)tmp1=5.6;
        else if(FT1<=19.8318)tmp1=5.7;
        else if(FT1<=19.8691)tmp1=5.8;
        else if(FT1<=19.9064)tmp1=5.9;
        else if(FT1<=19.9438)tmp1=6;
    }
    else if(FT1<=20.2821){
        if(FT1==19.9438)tmp1=6;
        else if(FT1<=19.9812)tmp1=6.1;
        else if(FT1<=20.0187)tmp1=6.2;
        else if(FT1<=20.0562)tmp1=6.3;
        else if(FT1<=20.0937)tmp1=6.4;
        else if(FT1<=20.1313)tmp1=6.5;
        else if(FT1<=20.169)tmp1=6.6;
        else if(FT1<=20.2066)tmp1=6.7;
        else if(FT1<=20.2444)tmp1=6.8;
        else if(FT1<=20.2821)tmp1=6.9;
        else if(FT1<=20.32)tmp1=7;
    }
    else if(FT1<=20.6623){
        if(FT1==20.32)tmp1=7;
        else if(FT1<=20.3578)tmp1=7.1;
        else if(FT1<=20.3957)tmp1=7.2;
        else if(FT1<=20.4337)tmp1=7.3;
        else if(FT1<=20.4717)tmp1=7.4;
        else if(FT1<=20.5097)tmp1=7.5;
        else if(FT1<=20.5478)tmp1=7.6;
        else if(FT1<=20.5859)tmp1=7.7;
        else if(FT1<=20.6241)tmp1=7.8;
        else if(FT1<=20.6623)tmp1=7.9;
        else if(FT1<=20.7006)tmp1=8;
    }
    else if(FT1<=21.047){
        if(FT1==20.7006)tmp1=8;
        else if(FT1<=20.7389)tmp1=8.1;
        else if(FT1<=20.7772)tmp1=8.2;
        else if(FT1<=20.8156)tmp1=8.3;
        else if(FT1<=20.8541)tmp1=8.4;
        else if(FT1<=20.8926)tmp1=8.5;
        else if(FT1<=20.9311)tmp1=8.6;
        else if(FT1<=20.9697)tmp1=8.7;
        else if(FT1<=21.0083)tmp1=8.8;
        else if(FT1<=21.047)tmp1=8.9;
        else if(FT1<=21.0857)tmp1=9;
    }
    else if(FT1<=21.4361){
        if(FT1==21.0857)tmp1=9;
        else if(FT1<=21.1244)tmp1=9.1;
        else if(FT1<=21.1632)tmp1=9.2;
        else if(FT1<=21.2021)tmp1=9.3;
        else if(FT1<=21.241)tmp1=9.4;
        else if(FT1<=21.2799)tmp1=9.5;
        else if(FT1<=21.3189)tmp1=9.6;
        else if(FT1<=21.3579)tmp1=9.7;
        else if(FT1<=21.397)tmp1=9.8;
        else if(FT1<=21.4361)tmp1=9.9;
        else if(FT1<=21.4753)tmp1=10;
    }
    else if(FT1<=21.8298){
        if(FT1==21.4753)tmp1=10;
        else if(FT1<=21.5145)tmp1=10.1;
        else if(FT1<=21.5537)tmp1=10.2;
        else if(FT1<=21.593)tmp1=10.3;
        else if(FT1<=21.6324)tmp1=10.4;
        else if(FT1<=21.6718)tmp1=10.5;
        else if(FT1<=21.7112)tmp1=10.6;
        else if(FT1<=21.7507)tmp1=10.7;
        else if(FT1<=21.7902)tmp1=10.8;
        else if(FT1<=21.8298)tmp1=10.9;
        else if(FT1<=21.8694)tmp1=11;
    }
    else if(FT1<=22.2279){
        if(FT1==21.8694)tmp1=11;
        else if(FT1<=21.909)tmp1=11.1;
        else if(FT1<=21.9487)tmp1=11.2;
        else if(FT1<=21.9885)tmp1=11.3;
        else if(FT1<=22.0283)tmp1=11.4;
        else if(FT1<=22.0681)tmp1=11.5;
        else if(FT1<=22.108)tmp1=11.6;
        else if(FT1<=22.1479)tmp1=11.7;
        else if(FT1<=22.1879)tmp1=11.8;
        else if(FT1<=22.2279)tmp1=11.9;
        else if(FT1<=22.268)tmp1=12;
    }
    else if(FT1<=22.6307){
        if(FT1==22.268)tmp1=12;
        else if(FT1<=22.3081)tmp1=12.1;
        else if(FT1<=22.3483)tmp1=12.2;
        else if(FT1<=22.3885)tmp1=12.3;
        else if(FT1<=22.4287)tmp1=12.4;
        else if(FT1<=22.469)tmp1=12.5;
        else if(FT1<=22.5094)tmp1=12.6;
        else if(FT1<=22.5497)tmp1=12.7;
        else if(FT1<=22.5902)tmp1=12.8;
        else if(FT1<=22.6307)tmp1=12.9;
        else if(FT1<=22.6712)tmp1=13;
    }
    else if(FT1<=23.0379){
        if(FT1==22.6712)tmp1=13;
        else if(FT1<=22.7117)tmp1=13.1;
        else if(FT1<=22.7523)tmp1=13.2;
        else if(FT1<=22.793)tmp1=13.3;
        else if(FT1<=22.8337)tmp1=13.4;
        else if(FT1<=22.8745)tmp1=13.5;
        else if(FT1<=22.9152)tmp1=13.6;
        else if(FT1<=22.9561)tmp1=13.7;
        else if(FT1<=22.997)tmp1=13.8;
        else if(FT1<=23.0379)tmp1=13.9;
        else if(FT1<=23.0789)tmp1=14;
    }
    else if(FT1<=23.4497){
        if(FT1==23.0789)tmp1=14;
        else if(FT1<=23.1199)tmp1=14.1;
        else if(FT1<=23.161)tmp1=14.2;
        else if(FT1<=23.2021)tmp1=14.3;
        else if(FT1<=23.2432)tmp1=14.4;
        else if(FT1<=23.2844)tmp1=14.5;
        else if(FT1<=23.3257)tmp1=14.6;
        else if(FT1<=23.367)tmp1=14.7;
        else if(FT1<=23.4083)tmp1=14.8;
        else if(FT1<=23.4497)tmp1=14.9;
        else if(FT1<=23.4911)tmp1=15;
    }
    else if(FT1<=23.8661){
        if(FT1==23.4911)tmp1=15;
        else if(FT1<=23.5326)tmp1=15.1;
        else if(FT1<=23.5741)tmp1=15.2;
        else if(FT1<=23.6157)tmp1=15.3;
        else if(FT1<=23.6573)tmp1=15.4;
        else if(FT1<=23.699)tmp1=15.5;
        else if(FT1<=23.7407)tmp1=15.6;
        else if(FT1<=23.7825)tmp1=15.7;
        else if(FT1<=23.8243)tmp1=15.8;
        else if(FT1<=23.8661)tmp1=15.9;
        else if(FT1<=23.908)tmp1=16;
    }
    else if(FT1<=24.2871){
        if(FT1==23.908)tmp1=16;
        else if(FT1<=23.9499)tmp1=16.1;
        else if(FT1<=23.9919)tmp1=16.2;
        else if(FT1<=24.0339)tmp1=16.3;
        else if(FT1<=24.076)tmp1=16.4;
        else if(FT1<=24.1181)tmp1=16.5;
        else if(FT1<=24.1603)tmp1=16.6;
        else if(FT1<=24.2025)tmp1=16.7;
        else if(FT1<=24.2448)tmp1=16.8;
        else if(FT1<=24.2871)tmp1=16.9;
    }
    else if(FT1<=24.7126){
        if(FT1==24.3294)tmp1=17;
        else if(FT1<=24.3718)tmp1=17.1;
        else if(FT1<=24.4142)tmp1=17.2;
        else if(FT1<=24.4567)tmp1=17.3;
        else if(FT1<=24.4993)tmp1=17.4;
        else if(FT1<=24.5418)tmp1=17.5;
        else if(FT1<=24.5845)tmp1=17.6;
        else if(FT1<=24.6271)tmp1=17.7;
        else if(FT1<=24.6698)tmp1=17.8;
        else if(FT1<=24.7126)tmp1=17.9;
    }
    else if(FT1<=25.1428){
        if(FT1==24.7554)tmp1=18;
        else if(FT1<=24.7983)tmp1=18.1;
        else if(FT1<=24.8412)tmp1=18.2;
        else if(FT1<=24.8841)tmp1=18.3;
        else if(FT1<=24.9271)tmp1=18.4;
        else if(FT1<=24.9702)tmp1=18.5;
        else if(FT1<=25.0132)tmp1=18.6;
        else if(FT1<=25.0564)tmp1=18.7;
        else if(FT1<=25.0995)tmp1=18.8;
        else if(FT1<=25.1428)tmp1=18.9;
    }
    else if(FT1<=25.5775){
        if(FT1==25.186)tmp1=19;
        else if(FT1<=25.2294)tmp1=19.1;
        else if(FT1<=25.2727)tmp1=19.2;
        else if(FT1<=25.3161)tmp1=19.3;
        else if(FT1<=25.3596)tmp1=19.4;
        else if(FT1<=25.4031)tmp1=19.5;
        else if(FT1<=25.4466)tmp1=19.6;
        else if(FT1<=25.4902)tmp1=19.7;
        else if(FT1<=25.5339)tmp1=19.8;
        else if(FT1<=25.5775)tmp1=19.9;
    }
    else if(FT1<=26.0169){
        if(FT1==25.6213)tmp1=20;
        else if(FT1<=25.6651)tmp1=20.1;
        else if(FT1<=25.7089)tmp1=20.2;
        else if(FT1<=25.7527)tmp1=20.3;
        else if(FT1<=25.7967)tmp1=20.4;
        else if(FT1<=25.8406)tmp1=20.5;
        else if(FT1<=25.8846)tmp1=20.6;
        else if(FT1<=25.9287)tmp1=20.7;
        else if(FT1<=25.9728)tmp1=20.8;
        else if(FT1<=26.0169)tmp1=20.9;
    }
    else if(FT1<=26.461){
        if(FT1==26.0611)tmp1=21;
        else if(FT1<=26.1054)tmp1=21.1;
        else if(FT1<=26.1497)tmp1=21.2;
        else if(FT1<=26.194)tmp1=21.3;
        else if(FT1<=26.2384)tmp1=21.4;
        else if(FT1<=26.2828)tmp1=21.5;
        else if(FT1<=26.3273)tmp1=21.6;
        else if(FT1<=26.3718)tmp1=21.7;
        else if(FT1<=26.4164)tmp1=21.8;
        else if(FT1<=26.461)tmp1=21.9;
    }
    else if(FT1<=26.9097){
        if(FT1==26.5056)tmp1=22;
        else if(FT1<=26.5503)tmp1=22.1;
        else if(FT1<=26.5951)tmp1=22.2;
        else if(FT1<=26.6399)tmp1=22.3;
        else if(FT1<=26.6847)tmp1=22.4;
        else if(FT1<=26.7296)tmp1=22.5;
        else if(FT1<=26.7746)tmp1=22.6;
        else if(FT1<=26.8195)tmp1=22.7;
        else if(FT1<=26.8646)tmp1=22.8;
        else if(FT1<=26.9097)tmp1=22.9;
    }
    else if(FT1<=27.363){
        if(FT1==26.9548)tmp1=23;
        else if(FT1<=27)tmp1=23.1;
        else if(FT1<=27.0452)tmp1=23.2;
        else if(FT1<=27.0904)tmp1=23.3;
        else if(FT1<=27.1357)tmp1=23.4;
        else if(FT1<=27.1811)tmp1=23.5;
        else if(FT1<=27.2265)tmp1=23.6;
        else if(FT1<=27.272)tmp1=23.7;
        else if(FT1<=27.3174)tmp1=23.8;
        else if(FT1<=27.363)tmp1=23.9;
    }
    else if(FT1<=27.821){
        if(FT1==27.4086)tmp1=24;
        else if(FT1<=27.4542)tmp1=24.1;
        else if(FT1<=27.4999)tmp1=24.2;
        else if(FT1<=27.5456)tmp1=24.3;
        else if(FT1<=27.5914)tmp1=24.4;
        else if(FT1<=27.6372)tmp1=24.5;
        else if(FT1<=27.6831)tmp1=24.6;
        else if(FT1<=27.729)tmp1=24.7;
        else if(FT1<=27.775)tmp1=24.8;
        else if(FT1<=27.821)tmp1=24.9;
    }
    else if(FT1<=28.2837){
        if(FT1==27.867)tmp1=25;
        else if(FT1<=27.9132)tmp1=25.1;
        else if(FT1<=27.9593)tmp1=25.2;
        else if(FT1<=28.0055)tmp1=25.3;
        else if(FT1<=28.0517)tmp1=25.4;
        else if(FT1<=28.098)tmp1=25.5;
        else if(FT1<=28.1444)tmp1=25.6;
        else if(FT1<=28.1908)tmp1=25.7;
        else if(FT1<=28.2372)tmp1=25.8;
        else if(FT1<=28.2837)tmp1=25.9;
    }
    else if(FT1<=28.751){
        if(FT1==28.3302)tmp1=26;
        else if(FT1<=28.3768)tmp1=26.1;
        else if(FT1<=28.4234)tmp1=26.2;
        else if(FT1<=28.47)tmp1=26.3;
        else if(FT1<=28.5168)tmp1=26.4;
        else if(FT1<=28.5635)tmp1=26.5;
        else if(FT1<=28.6103)tmp1=26.6;
        else if(FT1<=28.6572)tmp1=26.7;
        else if(FT1<=28.7041)tmp1=26.8;
        else if(FT1<=28.751)tmp1=26.9;
    }
    else if(FT1<=29.2231){
        if(FT1==28.798)tmp1=27;
        else if(FT1<=28.8451)tmp1=27.1;
        else if(FT1<=28.8921)tmp1=27.2;
        else if(FT1<=28.9393)tmp1=27.3;
        else if(FT1<=28.9865)tmp1=27.4;
        else if(FT1<=29.0337)tmp1=27.5;
        else if(FT1<=29.081)tmp1=27.6;
        else if(FT1<=29.1283)tmp1=27.7;
        else if(FT1<=29.1756)tmp1=27.8;
        else if(FT1<=29.2231)tmp1=27.9;
    }
    else if(FT1<=29.6998){
        if(FT1==29.2705)tmp1=28;
        else if(FT1<=29.318)tmp1=28.1;
        else if(FT1<=29.3656)tmp1=28.2;
        else if(FT1<=29.4132)tmp1=28.3;
        else if(FT1<=29.4608)tmp1=28.4;
        else if(FT1<=29.5085)tmp1=28.5;
        else if(FT1<=29.5563)tmp1=28.6;
        else if(FT1<=29.6041)tmp1=28.7;
        else if(FT1<=29.6519)tmp1=28.8;
        else if(FT1<=29.6998)tmp1=28.9;
    }
    else if(FT1<=30.1813){
        if(FT1==29.7477)tmp1=29;
        else if(FT1<=29.7957)tmp1=29.1;
        else if(FT1<=29.8438)tmp1=29.2;
        else if(FT1<=29.8918)tmp1=29.3;
        else if(FT1<=29.9399)tmp1=29.4;
        else if(FT1<=29.9881)tmp1=29.5;
        else if(FT1<=30.0363)tmp1=29.6;
        else if(FT1<=30.0846)tmp1=29.7;
        else if(FT1<=30.1329)tmp1=29.8;
        else if(FT1<=30.1813)tmp1=29.9;
    }
    else if(FT1<=30.6674){
        if(FT1==30.2297)tmp1=30;
        else if(FT1<=30.2781)tmp1=30.1;
        else if(FT1<=30.3266)tmp1=30.2;
        else if(FT1<=30.3752)tmp1=30.3;
        else if(FT1<=30.4238)tmp1=30.4;
        else if(FT1<=30.4724)tmp1=30.5;
        else if(FT1<=30.5211)tmp1=30.6;
        else if(FT1<=30.5698)tmp1=30.7;
        else if(FT1<=30.6186)tmp1=30.8;
        else if(FT1<=30.6674)tmp1=30.9;
    }
    else if(FT1<=31.1583){
        if(FT1==30.7163)tmp1=31;
        else if(FT1<=30.7652)tmp1=31.1;
        else if(FT1<=30.8142)tmp1=31.2;
        else if(FT1<=30.8632)tmp1=31.3;
        else if(FT1<=30.9123)tmp1=31.4;
        else if(FT1<=30.9614)tmp1=31.5;
        else if(FT1<=31.0106)tmp1=31.6;
        else if(FT1<=31.0598)tmp1=31.7;
        else if(FT1<=31.109)tmp1=31.8;
        else if(FT1<=31.1583)tmp1=31.9;
    }
    else if(FT1<=31.6539){
        if(FT1==31.2077)tmp1=32;
        else if(FT1<=31.2571)tmp1=32.1;
        else if(FT1<=31.3065)tmp1=32.2;
        else if(FT1<=31.356)tmp1=32.3;
        else if(FT1<=31.4055)tmp1=32.4;
        else if(FT1<=31.4551)tmp1=32.5;
        else if(FT1<=31.5048)tmp1=32.6;
        else if(FT1<=31.5544)tmp1=32.7;
        else if(FT1<=31.6042)tmp1=32.8;
        else if(FT1<=31.6539)tmp1=32.9;
    }
    else if(FT1<=32.1543){
        if(FT1==31.7038)tmp1=33;
        else if(FT1<=31.7536)tmp1=33.1;
        else if(FT1<=31.8035)tmp1=33.2;
        else if(FT1<=31.8535)tmp1=33.3;
        else if(FT1<=31.9035)tmp1=33.4;
        else if(FT1<=31.9536)tmp1=33.5;
        else if(FT1<=32.0037)tmp1=33.6;
        else if(FT1<=32.0538)tmp1=33.7;
        else if(FT1<=32.104)tmp1=33.8;
        else if(FT1<=32.1543)tmp1=33.9;
    }
    else if(FT1<=32.6594){
        if(FT1==32.2046)tmp1=34;
        else if(FT1<=32.2549)tmp1=34.1;
        else if(FT1<=32.3053)tmp1=34.2;
        else if(FT1<=32.3558)tmp1=34.3;
        else if(FT1<=32.4062)tmp1=34.4;
        else if(FT1<=32.4568)tmp1=34.5;
        else if(FT1<=32.5074)tmp1=34.6;
        else if(FT1<=32.558)tmp1=34.7;
        else if(FT1<=32.6087)tmp1=34.8;
        else if(FT1<=32.6594)tmp1=34.9;
    }
    else if(FT1<=33.1692){
        if(FT1==32.7102)tmp1=35;
        else if(FT1<=32.761)tmp1=35.1;
        else if(FT1<=32.8118)tmp1=35.2;
        else if(FT1<=32.8628)tmp1=35.3;
        else if(FT1<=32.9137)tmp1=35.4;
        else if(FT1<=32.9647)tmp1=35.5;
        else if(FT1<=33.0158)tmp1=35.6;
        else if(FT1<=33.0669)tmp1=35.7;
        else if(FT1<=33.118)tmp1=35.8;
        else if(FT1<=33.1692)tmp1=35.9;
    }
    else if(FT1<=33.6838){
        if(FT1==33.2205)tmp1=36;
        else if(FT1<=33.2718)tmp1=36.1;
        else if(FT1<=33.3231)tmp1=36.2;
        else if(FT1<=33.3745)tmp1=36.3;
        else if(FT1<=33.426)tmp1=36.4;
        else if(FT1<=33.4774)tmp1=36.5;
        else if(FT1<=33.529)tmp1=36.6;
        else if(FT1<=33.5806)tmp1=36.7;
        else if(FT1<=33.6322)tmp1=36.8;
        else if(FT1<=33.6838)tmp1=36.9;
    }
    else if(FT1<=34.2032){
        if(FT1==33.7356)tmp1=37;
        else if(FT1<=33.7873)tmp1=37.1;
        else if(FT1<=33.8392)tmp1=37.2;
        else if(FT1<=33.891)tmp1=37.3;
        else if(FT1<=33.9429)tmp1=37.4;
        else if(FT1<=33.9949)tmp1=37.5;
        else if(FT1<=34.0469)tmp1=37.6;
        else if(FT1<=34.099)tmp1=37.7;
        else if(FT1<=34.1511)tmp1=37.8;
        else if(FT1<=34.2032)tmp1=37.9;
    }
    else if(FT1<=34.7274){
        if(FT1==34.2554)tmp1=38;
        else if(FT1<=34.3077)tmp1=38.1;
        else if(FT1<=34.36)tmp1=38.2;
        else if(FT1<=34.4123)tmp1=38.3;
        else if(FT1<=34.4647)tmp1=38.4;
        else if(FT1<=34.5171)tmp1=38.5;
        else if(FT1<=34.5696)tmp1=38.6;
        else if(FT1<=34.6222)tmp1=38.7;
        else if(FT1<=34.6747)tmp1=38.8;
        else if(FT1<=34.7274)tmp1=38.9;
    }
    else if(FT1<=35.2563){
        if(FT1==34.78)tmp1=39;
        else if(FT1<=34.8328)tmp1=39.1;
        else if(FT1<=34.8855)tmp1=39.2;
        else if(FT1<=34.9383)tmp1=39.3;
        else if(FT1<=34.9912)tmp1=39.4;
        else if(FT1<=35.0441)tmp1=39.5;
        else if(FT1<=35.0971)tmp1=39.6;
        else if(FT1<=35.1501)tmp1=39.7;
        else if(FT1<=35.2032)tmp1=39.8;
        else if(FT1<=35.2563)tmp1=39.9;
    }
    else if(FT1<=35.79){
        if(FT1==35.3094)tmp1=40;
        else if(FT1<=35.3626)tmp1=40.1;
        else if(FT1<=35.4159)tmp1=40.2;
        else if(FT1<=35.4692)tmp1=40.3;
        else if(FT1<=35.5225)tmp1=40.4;
        else if(FT1<=35.5759)tmp1=40.5;
        else if(FT1<=35.6294)tmp1=40.6;
        else if(FT1<=35.6828)tmp1=40.7;
        else if(FT1<=35.7364)tmp1=40.8;
        else if(FT1<=35.79)tmp1=40.9;
    }
    else if(FT1<=36.3284){
        if(FT1==35.8436)tmp1=41;
        else if(FT1<=35.8973)tmp1=41.1;
        else if(FT1<=35.951)tmp1=41.2;
        else if(FT1<=36.0048)tmp1=41.3;
        else if(FT1<=36.0586)tmp1=41.4;
        else if(FT1<=36.1125)tmp1=41.5;
        else if(FT1<=36.1664)tmp1=41.6;
        else if(FT1<=36.2204)tmp1=41.7;
        else if(FT1<=36.2744)tmp1=41.8;
        else if(FT1<=36.3284)tmp1=41.9;
    }
    else if(FT1<=36.8717){
        if(FT1==36.3825)tmp1=42;
        else if(FT1<=36.4367)tmp1=42.1;
        else if(FT1<=36.4909)tmp1=42.2;
        else if(FT1<=36.5452)tmp1=42.3;
        else if(FT1<=36.5995)tmp1=42.4;
        else if(FT1<=36.6538)tmp1=42.5;
        else if(FT1<=36.7082)tmp1=42.6;
        else if(FT1<=36.7627)tmp1=42.7;
        else if(FT1<=36.8172)tmp1=42.8;
        else if(FT1<=36.8717)tmp1=42.9;
    }
    else if(FT1<=37.4198){
        if(FT1==36.9263)tmp1=43;
        else if(FT1<=36.9809)tmp1=43.1;
        else if(FT1<=37.0356)tmp1=43.2;
        else if(FT1<=37.0903)tmp1=43.3;
        else if(FT1<=37.1451)tmp1=43.4;
        else if(FT1<=37.2)tmp1=43.5;
        else if(FT1<=37.2548)tmp1=43.6;
        else if(FT1<=37.3098)tmp1=43.7;
        else if(FT1<=37.3647)tmp1=43.8;
        else if(FT1<=37.4198)tmp1=43.9;
    }
    else if(FT1<=37.9726){
        if(FT1==37.4748)tmp1=44;
        else if(FT1<=37.5299)tmp1=44.1;
        else if(FT1<=37.5851)tmp1=44.2;
        else if(FT1<=37.6403)tmp1=44.3;
        else if(FT1<=37.6956)tmp1=44.4;
        else if(FT1<=37.7509)tmp1=44.5;
        else if(FT1<=37.8062)tmp1=44.6;
        else if(FT1<=37.8617)tmp1=44.7;
        else if(FT1<=37.9171)tmp1=44.8;
        else if(FT1<=37.9726)tmp1=44.9;
    }
    else if(FT1<=38.5303){
        if(FT1==38.0282)tmp1=45;
        else if(FT1<=38.0838)tmp1=45.1;
        else if(FT1<=38.1394)tmp1=45.2;
        else if(FT1<=38.1951)tmp1=45.3;
        else if(FT1<=38.2508)tmp1=45.4;
        else if(FT1<=38.3066)tmp1=45.5;
        else if(FT1<=38.3625)tmp1=45.6;
        else if(FT1<=38.4183)tmp1=45.7;
        else if(FT1<=38.4743)tmp1=45.8;
        else if(FT1<=38.5303)tmp1=45.9;
    }
    else if(FT1<=39.0927){
        if(FT1==38.5863)tmp1=46;
        else if(FT1<=38.6424)tmp1=46.1;
        else if(FT1<=38.6985)tmp1=46.2;
        else if(FT1<=38.7547)tmp1=46.3;
        else if(FT1<=38.8109)tmp1=46.4;
        else if(FT1<=38.8672)tmp1=46.5;
        else if(FT1<=38.9235)tmp1=46.6;
        else if(FT1<=38.9798)tmp1=46.7;
        else if(FT1<=39.0363)tmp1=46.8;
        else if(FT1<=39.0927)tmp1=46.9;
    }
    else if(FT1<=39.66){
        if(FT1==39.1492)tmp1=47;
        else if(FT1<=39.2058)tmp1=47.1;
        else if(FT1<=39.2624)tmp1=47.2;
        else if(FT1<=39.3191)tmp1=47.3;
        else if(FT1<=39.3758)tmp1=47.4;
        else if(FT1<=39.4325)tmp1=47.5;
        else if(FT1<=39.4893)tmp1=47.6;
        else if(FT1<=39.5462)tmp1=47.7;
        else if(FT1<=39.603)tmp1=47.8;
        else if(FT1<=39.66)tmp1=47.9;
    }
    else if(FT1<=40.2321){
        if(FT1==39.717)tmp1=48;
        else if(FT1<=39.774)tmp1=48.1;
        else if(FT1<=39.8311)tmp1=48.2;
        else if(FT1<=39.8882)tmp1=48.3;
        else if(FT1<=39.9454)tmp1=48.4;
        else if(FT1<=40.0027)tmp1=48.5;
        else if(FT1<=40.0599)tmp1=48.6;
        else if(FT1<=40.1173)tmp1=48.7;
        else if(FT1<=40.1746)tmp1=48.8;
        else if(FT1<=40.2321)tmp1=48.9;
    }
    else if(FT1<=40.809){
        if(FT1==40.2895)tmp1=49;
        else if(FT1<=40.3471)tmp1=49.1;
        else if(FT1<=40.4046)tmp1=49.2;
        else if(FT1<=40.4623)tmp1=49.3;
        else if(FT1<=40.5199)tmp1=49.4;
        else if(FT1<=40.5776)tmp1=49.5;
        else if(FT1<=40.6354)tmp1=49.6;
        else if(FT1<=40.6932)tmp1=49.7;
        else if(FT1<=40.7511)tmp1=49.8;
        else if(FT1<=40.809)tmp1=49.9;
    }
    else if(FT1<=41.3907){
        if(FT1==40.8669)tmp1=50;
        else if(FT1<=40.9249)tmp1=50.1;
        else if(FT1<=40.983)tmp1=50.2;
        else if(FT1<=41.0411)tmp1=50.3;
        else if(FT1<=41.0992)tmp1=50.4;
        else if(FT1<=41.1574)tmp1=50.5;
        else if(FT1<=41.2157)tmp1=50.6;
        else if(FT1<=41.274)tmp1=50.7;
        else if(FT1<=41.3323)tmp1=50.8;
        else if(FT1<=41.3907)tmp1=50.9;
    }
    else if(FT1<=41.9772){
        if(FT1==41.4491)tmp1=51;
        else if(FT1<=41.5076)tmp1=51.1;
        else if(FT1<=41.5661)tmp1=51.2;
        else if(FT1<=41.6247)tmp1=51.3;
        else if(FT1<=41.6833)tmp1=51.4;
        else if(FT1<=41.742)tmp1=51.5;
        else if(FT1<=41.8008)tmp1=51.6;
        else if(FT1<=41.8595)tmp1=51.7;
        else if(FT1<=41.9183)tmp1=51.8;
        else if(FT1<=41.9772)tmp1=51.9;
    }
    else if(FT1<=42.5686){
        if(FT1==42.0361)tmp1=52;
        else if(FT1<=42.0951)tmp1=52.1;
        else if(FT1<=42.1541)tmp1=52.2;
        else if(FT1<=42.2132)tmp1=52.3;
        else if(FT1<=42.2723)tmp1=52.4;
        else if(FT1<=42.3315)tmp1=52.5;
        else if(FT1<=42.3907)tmp1=52.6;
        else if(FT1<=42.4499)tmp1=52.7;
        else if(FT1<=42.5092)tmp1=52.8;
        else if(FT1<=42.5686)tmp1=52.9;
    }
    else if(FT1<=43.1648){
        if(FT1==42.628)tmp1=53;
        else if(FT1<=42.6874)tmp1=53.1;
        else if(FT1<=42.7469)tmp1=53.2;
        else if(FT1<=42.8065)tmp1=53.3;
        else if(FT1<=42.8661)tmp1=53.4;
        else if(FT1<=42.9257)tmp1=53.5;
        else if(FT1<=42.9854)tmp1=53.6;
        else if(FT1<=43.0451)tmp1=53.7;
        else if(FT1<=43.1049)tmp1=53.8;
        else if(FT1<=43.1648)tmp1=53.9;
    }
    else if(FT1<=43.7658){
        if(FT1==43.2246)tmp1=54;
        else if(FT1<=43.2846)tmp1=54.1;
        else if(FT1<=43.3446)tmp1=54.2;
        else if(FT1<=43.4046)tmp1=54.3;
        else if(FT1<=43.4647)tmp1=54.4;
        else if(FT1<=43.5248)tmp1=54.5;
        else if(FT1<=43.585)tmp1=54.6;
        else if(FT1<=43.6452)tmp1=54.7;
        else if(FT1<=43.7055)tmp1=54.8;
        else if(FT1<=43.7658)tmp1=54.9;
    }
    else if(FT1<=44.3716){
        if(FT1==43.8261)tmp1=55;
        else if(FT1<=43.8866)tmp1=55.1;
        else if(FT1<=43.947)tmp1=55.2;
        else if(FT1<=44.0075)tmp1=55.3;
        else if(FT1<=44.0681)tmp1=55.4;
        else if(FT1<=44.1287)tmp1=55.5;
        else if(FT1<=44.1894)tmp1=55.6;
        else if(FT1<=44.2501)tmp1=55.7;
        else if(FT1<=44.3108)tmp1=55.8;
        else if(FT1<=44.3716)tmp1=55.9;
    }
    else if(FT1<=44.9823){
        if(FT1==44.4325)tmp1=56;
        else if(FT1<=44.4934)tmp1=56.1;
        else if(FT1<=44.5543)tmp1=56.2;
        else if(FT1<=44.6153)tmp1=56.3;
        else if(FT1<=44.6764)tmp1=56.4;
        else if(FT1<=44.7374)tmp1=56.5;
        else if(FT1<=44.7986)tmp1=56.6;
        else if(FT1<=44.8598)tmp1=56.7;
        else if(FT1<=44.921)tmp1=56.8;
        else if(FT1<=44.9823)tmp1=56.9;
    }
    else if(FT1<=45.5978){
        if(FT1==45.0436)tmp1=57;
        else if(FT1<=45.105)tmp1=57.1;
        else if(FT1<=45.1664)tmp1=57.2;
        else if(FT1<=45.2279)tmp1=57.3;
        else if(FT1<=45.2894)tmp1=57.4;
        else if(FT1<=45.351)tmp1=57.5;
        else if(FT1<=45.4126)tmp1=57.6;
        else if(FT1<=45.4743)tmp1=57.7;
        else if(FT1<=45.536)tmp1=57.8;
        else if(FT1<=45.5978)tmp1=57.9;
    }
    else if(FT1<=46.2182){
        if(FT1==45.6596)tmp1=58;
        else if(FT1<=45.7215)tmp1=58.1;
        else if(FT1<=45.7834)tmp1=58.2;
        else if(FT1<=45.8454)tmp1=58.3;
        else if(FT1<=45.9074)tmp1=58.4;
        else if(FT1<=45.9694)tmp1=58.5;
        else if(FT1<=46.0315)tmp1=58.6;
        else if(FT1<=46.0937)tmp1=58.7;
        else if(FT1<=46.1559)tmp1=58.8;
        else if(FT1<=46.2182)tmp1=58.9;
    }
    else if(FT1<=46.8433){
        if(FT1==46.2805)tmp1=59;
        else if(FT1<=46.3428)tmp1=59.1;
        else if(FT1<=46.4052)tmp1=59.2;
        else if(FT1<=46.4676)tmp1=59.3;
        else if(FT1<=46.5301)tmp1=59.4;
        else if(FT1<=46.5927)tmp1=59.5;
        else if(FT1<=46.6553)tmp1=59.6;
        else if(FT1<=46.7179)tmp1=59.7;
        else if(FT1<=46.7806)tmp1=59.8;
        else if(FT1<=46.8433)tmp1=59.9;
    }
    else if(FT1<=47.4733){
        if(FT1==46.9061)tmp1=60;
        else if(FT1<=46.9689)tmp1=60.1;
        else if(FT1<=47.0318)tmp1=60.2;
        else if(FT1<=47.0948)tmp1=60.3;
        else if(FT1<=47.1577)tmp1=60.4;
        else if(FT1<=47.2208)tmp1=60.5;
        else if(FT1<=47.2838)tmp1=60.6;
        else if(FT1<=47.347)tmp1=60.7;
        else if(FT1<=47.4101)tmp1=60.8;
        else if(FT1<=47.4733)tmp1=60.9;
    }
    else if(FT1<=48.1082){
        if(FT1==47.5366)tmp1=61;
        else if(FT1<=47.5999)tmp1=61.1;
        else if(FT1<=47.6633)tmp1=61.2;
        else if(FT1<=47.7267)tmp1=61.3;
        else if(FT1<=47.7902)tmp1=61.4;
        else if(FT1<=47.8537)tmp1=61.5;
        else if(FT1<=47.9172)tmp1=61.6;
        else if(FT1<=47.9808)tmp1=61.7;
        else if(FT1<=48.0445)tmp1=61.8;
        else if(FT1<=48.1082)tmp1=61.9;
    }
    else if(FT1<=48.7479){
        if(FT1==48.1719)tmp1=62;
        else if(FT1<=48.2357)tmp1=62.1;
        else if(FT1<=48.2996)tmp1=62.2;
        else if(FT1<=48.3635)tmp1=62.3;
        else if(FT1<=48.4274)tmp1=62.4;
        else if(FT1<=48.4914)tmp1=62.5;
        else if(FT1<=48.5555)tmp1=62.6;
        else if(FT1<=48.6196)tmp1=62.7;
        else if(FT1<=48.6837)tmp1=62.8;
        else if(FT1<=48.7479)tmp1=62.9;
    }
    else if(FT1<=49.3924){
        if(FT1==48.8121)tmp1=63;
        else if(FT1<=48.8764)tmp1=63.1;
        else if(FT1<=48.9407)tmp1=63.2;
        else if(FT1<=49.0051)tmp1=63.3;
        else if(FT1<=49.0695)tmp1=63.4;
        else if(FT1<=49.134)tmp1=63.5;
        else if(FT1<=49.1985)tmp1=63.6;
        else if(FT1<=49.2631)tmp1=63.7;
        else if(FT1<=49.3277)tmp1=63.8;
        else if(FT1<=49.3924)tmp1=63.9;
    }
    else if(FT1<=50.0418){
        if(FT1==49.4571)tmp1=64;
        else if(FT1<=49.5219)tmp1=64.1;
        else if(FT1<=49.5867)tmp1=64.2;
        else if(FT1<=49.6516)tmp1=64.3;
        else if(FT1<=49.7165)tmp1=64.4;
        else if(FT1<=49.7815)tmp1=64.5;
        else if(FT1<=49.8465)tmp1=64.6;
        else if(FT1<=49.9115)tmp1=64.7;
        else if(FT1<=49.9766)tmp1=64.8;
        else if(FT1<=50.0418)tmp1=64.9;
        else if(FT1<=50.107)tmp1=65;
    }
    else if(FT1<=50.696){
        if(FT1==50.107)tmp1=65;
        else if(FT1<=50.1722)tmp1=65.1;
        else if(FT1<=50.2375)tmp1=65.2;
        else if(FT1<=50.3029)tmp1=65.3;
        else if(FT1<=50.3683)tmp1=65.4;
        else if(FT1<=50.4337)tmp1=65.5;
        else if(FT1<=50.4992)tmp1=65.6;
        else if(FT1<=50.5648)tmp1=65.7;
        else if(FT1<=50.6303)tmp1=65.8;
        else if(FT1<=50.696)tmp1=65.9;
        else if(FT1<=50.7617)tmp1=66;
    }
    else if(FT1<=51.355){
        if(FT1==50.7617)tmp1=66;
        else if(FT1<=50.8274)tmp1=66.1;
        else if(FT1<=50.8932)tmp1=66.2;
        else if(FT1<=50.959)tmp1=66.3;
        else if(FT1<=51.0249)tmp1=66.4;
        else if(FT1<=51.0908)tmp1=66.5;
        else if(FT1<=51.1568)tmp1=66.6;
        else if(FT1<=51.2228)tmp1=66.7;
        else if(FT1<=51.2889)tmp1=66.8;
        else if(FT1<=51.355)tmp1=66.9;
        else if(FT1<=51.4212)tmp1=67;
    }
    else if(FT1<=52.0189){
        if(FT1==51.4212)tmp1=67;
        else if(FT1<=51.4874)tmp1=67.1;
        else if(FT1<=51.5537)tmp1=67.2;
        else if(FT1<=51.62)tmp1=67.3;
        else if(FT1<=51.6864)tmp1=67.4;
        else if(FT1<=51.7528)tmp1=67.5;
        else if(FT1<=51.8192)tmp1=67.6;
        else if(FT1<=51.8857)tmp1=67.7;
        else if(FT1<=51.9523)tmp1=67.8;
        else if(FT1<=52.0189)tmp1=67.9;
        else if(FT1<=52.0856)tmp1=68;
    }
    else if(FT1<=52.6876){
        if(FT1==52.0856)tmp1=68;
        else if(FT1<=52.1523)tmp1=68.1;
        else if(FT1<=52.219)tmp1=68.2;
        else if(FT1<=52.2858)tmp1=68.3;
        else if(FT1<=52.3527)tmp1=68.4;
        else if(FT1<=52.4196)tmp1=68.5;
        else if(FT1<=52.4865)tmp1=68.6;
        else if(FT1<=52.5535)tmp1=68.7;
        else if(FT1<=52.6205)tmp1=68.8;
        else if(FT1<=52.6876)tmp1=68.9;
        else if(FT1<=52.7548)tmp1=69;
    }
    else if(FT1<=53.3612){
        if(FT1==52.7548)tmp1=69;
        else if(FT1<=52.8219)tmp1=69.1;
        else if(FT1<=52.8892)tmp1=69.2;
        else if(FT1<=52.9565)tmp1=69.3;
        else if(FT1<=53.0238)tmp1=69.4;
        else if(FT1<=53.0912)tmp1=69.5;
        else if(FT1<=53.1586)tmp1=69.6;
        else if(FT1<=53.2261)tmp1=69.7;
        else if(FT1<=53.2936)tmp1=69.8;
        else if(FT1<=53.3612)tmp1=69.9;
        else if(FT1<=53.4288)tmp1=70;
    }
    else if(FT1<=54.0396){
        if(FT1==53.4288)tmp1=70;
        else if(FT1<=53.4965)tmp1=70.1;
        else if(FT1<=53.5642)tmp1=70.2;
        else if(FT1<=53.6319)tmp1=70.3;
        else if(FT1<=53.6998)tmp1=70.4;
        else if(FT1<=53.7676)tmp1=70.5;
        else if(FT1<=53.8355)tmp1=70.6;
        else if(FT1<=53.9035)tmp1=70.7;
        else if(FT1<=53.9715)tmp1=70.8;
        else if(FT1<=54.0396)tmp1=70.9;
        else if(FT1<=54.1077)tmp1=71;
    }
    else if(FT1<=54.7228){
        if(FT1==54.1077)tmp1=71;
        else if(FT1<=54.1758)tmp1=71.1;
        else if(FT1<=54.244)tmp1=71.2;
        else if(FT1<=54.3123)tmp1=71.3;
        else if(FT1<=54.3806)tmp1=71.4;
        else if(FT1<=54.4489)tmp1=71.5;
        else if(FT1<=54.5173)tmp1=71.6;
        else if(FT1<=54.5857)tmp1=71.7;
        else if(FT1<=54.6542)tmp1=71.8;
        else if(FT1<=54.7228)tmp1=71.9;
        else if(FT1<=54.7914)tmp1=72;
    }
    else if(FT1<=55.4108){
        if(FT1==54.7914)tmp1=72;
        else if(FT1<=54.86)tmp1=72.1;
        else if(FT1<=54.9287)tmp1=72.2;
        else if(FT1<=54.9974)tmp1=72.3;
        else if(FT1<=55.0662)tmp1=72.4;
        else if(FT1<=55.135)tmp1=72.5;
        else if(FT1<=55.2039)tmp1=72.6;
        else if(FT1<=55.2728)tmp1=72.7;
        else if(FT1<=55.3418)tmp1=72.8;
        else if(FT1<=55.4108)tmp1=72.9;
        else if(FT1<=55.4799)tmp1=73;
    }
    else if(FT1<=56.1037){
        if(FT1==55.4799)tmp1=73;
        else if(FT1<=55.549)tmp1=73.1;
        else if(FT1<=55.6182)tmp1=73.2;
        else if(FT1<=55.6874)tmp1=73.3;
        else if(FT1<=55.7567)tmp1=73.4;
        else if(FT1<=55.826)tmp1=73.5;
        else if(FT1<=55.8953)tmp1=73.6;
        else if(FT1<=55.9647)tmp1=73.7;
        else if(FT1<=56.0342)tmp1=73.8;
        else if(FT1<=56.1037)tmp1=73.9;
        else if(FT1<=56.1733)tmp1=74;
    }
    else if(FT1<=56.8014){
        if(FT1==56.1733)tmp1=74;
        else if(FT1<=56.2429)tmp1=74.1;
        else if(FT1<=56.3125)tmp1=74.2;
        else if(FT1<=56.3822)tmp1=74.3;
        else if(FT1<=56.452)tmp1=74.4;
        else if(FT1<=56.5218)tmp1=74.5;
        else if(FT1<=56.5916)tmp1=74.6;
        else if(FT1<=56.6615)tmp1=74.7;
        else if(FT1<=56.7314)tmp1=74.8;
        else if(FT1<=56.8014)tmp1=74.9;
        else if(FT1<=56.8715)tmp1=75;
    }
    else if(FT1<=57.504){
        if(FT1==56.8715)tmp1=75;
        else if(FT1<=56.9415)tmp1=75.1;
        else if(FT1<=57.0117)tmp1=75.2;
        else if(FT1<=57.0819)tmp1=75.3;
        else if(FT1<=57.1521)tmp1=75.4;
        else if(FT1<=57.2224)tmp1=75.5;
        else if(FT1<=57.2927)tmp1=75.6;
        else if(FT1<=57.3631)tmp1=75.7;
        else if(FT1<=57.4335)tmp1=75.8;
        else if(FT1<=57.504)tmp1=75.9;
        else if(FT1<=57.5745)tmp1=76;
    }
    else if(FT1<=58.2113){
        if(FT1==57.5745)tmp1=76;
        else if(FT1<=57.6451)tmp1=76.1;
        else if(FT1<=57.7157)tmp1=76.2;
        else if(FT1<=57.7863)tmp1=76.3;
        else if(FT1<=57.857)tmp1=76.4;
        else if(FT1<=57.9278)tmp1=76.5;
        else if(FT1<=57.9986)tmp1=76.6;
        else if(FT1<=58.0695)tmp1=76.7;
        else if(FT1<=58.1404)tmp1=76.8;
        else if(FT1<=58.2113)tmp1=76.9;
        else if(FT1<=58.2823)tmp1=77;
    }
    else if(FT1<=58.9235){
        if(FT1==58.2823)tmp1=77;
        else if(FT1<=58.3534)tmp1=77.1;
        else if(FT1<=58.4245)tmp1=77.2;
        else if(FT1<=58.4956)tmp1=77.3;
        else if(FT1<=58.5668)tmp1=77.4;
        else if(FT1<=58.6381)tmp1=77.5;
        else if(FT1<=58.7094)tmp1=77.6;
        else if(FT1<=58.7807)tmp1=77.7;
        else if(FT1<=58.8521)tmp1=77.8;
        else if(FT1<=58.9235)tmp1=77.9;
        else if(FT1<=58.995)tmp1=78;
    }
    else if(FT1<=59.6405){
        if(FT1==58.995)tmp1=78;
        else if(FT1<=59.0665)tmp1=78.1;
        else if(FT1<=59.1381)tmp1=78.2;
        else if(FT1<=59.2097)tmp1=78.3;
        else if(FT1<=59.2814)tmp1=78.4;
        else if(FT1<=59.3531)tmp1=78.5;
        else if(FT1<=59.4249)tmp1=78.6;
        else if(FT1<=59.4967)tmp1=78.7;
        else if(FT1<=59.5686)tmp1=78.8;
        else if(FT1<=59.6405)tmp1=78.9;
        else if(FT1<=59.7125)tmp1=79;
    }
    else if(FT1<=60.3624){
        if(FT1==59.7125)tmp1=79;
        else if(FT1<=59.7845)tmp1=79.1;
        else if(FT1<=59.8566)tmp1=79.2;
        else if(FT1<=59.9287)tmp1=79.3;
        else if(FT1<=60.0008)tmp1=79.4;
        else if(FT1<=60.073)tmp1=79.5;
        else if(FT1<=60.1453)tmp1=79.6;
        else if(FT1<=60.2176)tmp1=79.7;
        else if(FT1<=60.29)tmp1=79.8;
        else if(FT1<=60.3624)tmp1=79.9;
        else if(FT1<=60.4348)tmp1=80;
    }
    else if(FT1<=61.089){
        if(FT1==60.4348)tmp1=80;
        else if(FT1<=60.5073)tmp1=80.1;
        else if(FT1<=60.5798)tmp1=80.2;
        else if(FT1<=60.6524)tmp1=80.3;
        else if(FT1<=60.7251)tmp1=80.4;
        else if(FT1<=60.7978)tmp1=80.5;
        else if(FT1<=60.8705)tmp1=80.6;
        else if(FT1<=60.9433)tmp1=80.7;
        else if(FT1<=61.0161)tmp1=80.8;
        else if(FT1<=61.089)tmp1=80.9;
        else if(FT1<=61.1619)tmp1=81;
    }
    else if(FT1<=61.8205){
        if(FT1==61.1619)tmp1=81;
        else if(FT1<=61.2349)tmp1=81.1;
        else if(FT1<=61.3079)tmp1=81.2;
        else if(FT1<=61.381)tmp1=81.3;
        else if(FT1<=61.4541)tmp1=81.4;
        else if(FT1<=61.5273)tmp1=81.5;
        else if(FT1<=61.6005)tmp1=81.6;
        else if(FT1<=61.6738)tmp1=81.7;
        else if(FT1<=61.7471)tmp1=81.8;
        else if(FT1<=61.8205)tmp1=81.9;
        else if(FT1<=61.8939)tmp1=82;
    }
    else if(FT1<=62.5567){
        if(FT1==61.8939)tmp1=82;
        else if(FT1<=61.9673)tmp1=82.1;
        else if(FT1<=62.0408)tmp1=82.2;
        else if(FT1<=62.1144)tmp1=82.3;
        else if(FT1<=62.188)tmp1=82.4;
        else if(FT1<=62.2616)tmp1=82.5;
        else if(FT1<=62.3353)tmp1=82.6;
        else if(FT1<=62.4091)tmp1=82.7;
        else if(FT1<=62.4829)tmp1=82.8;
        else if(FT1<=62.5567)tmp1=82.9;
        else if(FT1<=62.6306)tmp1=83;
    }
    else if(FT1<=63.2978){
        if(FT1==62.6306)tmp1=83;
        else if(FT1<=62.7046)tmp1=83.1;
        else if(FT1<=62.7785)tmp1=83.2;
        else if(FT1<=62.8526)tmp1=83.3;
        else if(FT1<=62.9267)tmp1=83.4;
        else if(FT1<=63.0008)tmp1=83.5;
        else if(FT1<=63.075)tmp1=83.6;
        else if(FT1<=63.1492)tmp1=83.7;
        else if(FT1<=63.2235)tmp1=83.8;
        else if(FT1<=63.2978)tmp1=83.9;
        else if(FT1<=63.3722)tmp1=84;
    }
    else if(FT1<=64.0437){
        if(FT1==63.3722)tmp1=84;
        else if(FT1<=63.4466)tmp1=84.1;
        else if(FT1<=63.5211)tmp1=84.2;
        else if(FT1<=63.5956)tmp1=84.3;
        else if(FT1<=63.6701)tmp1=84.4;
        else if(FT1<=63.7447)tmp1=84.5;
        else if(FT1<=63.8194)tmp1=84.6;
        else if(FT1<=63.8941)tmp1=84.7;
        else if(FT1<=63.9689)tmp1=84.8;
        else if(FT1<=64.0437)tmp1=84.9;
        else if(FT1<=64.1185)tmp1=85;
    }
    else if(FT1<=64.7944){
        if(FT1==64.1185)tmp1=85;
        else if(FT1<=64.1934)tmp1=85.1;
        else if(FT1<=64.2684)tmp1=85.2;
        else if(FT1<=64.3434)tmp1=85.3;
        else if(FT1<=64.4184)tmp1=85.4;
        else if(FT1<=64.4935)tmp1=85.5;
        else if(FT1<=64.5686)tmp1=85.6;
        else if(FT1<=64.6438)tmp1=85.7;
        else if(FT1<=64.7191)tmp1=85.8;
        else if(FT1<=64.7944)tmp1=85.9;
        else if(FT1<=64.8697)tmp1=86;
    }
    else if(FT1<=65.5498){
        if(FT1==64.8697)tmp1=86;
        else if(FT1<=64.9451)tmp1=86.1;
        else if(FT1<=65.0205)tmp1=86.2;
        else if(FT1<=65.096)tmp1=86.3;
        else if(FT1<=65.1715)tmp1=86.4;
        else if(FT1<=65.2471)tmp1=86.5;
        else if(FT1<=65.3227)tmp1=86.6;
        else if(FT1<=65.3984)tmp1=86.7;
        else if(FT1<=65.4741)tmp1=86.8;
        else if(FT1<=65.5498)tmp1=86.9;
        else if(FT1<=65.6256)tmp1=87;
    }
    else if(FT1<=66.3101){
        if(FT1==65.6256)tmp1=87;
        else if(FT1<=65.7015)tmp1=87.1;
        else if(FT1<=65.7774)tmp1=87.2;
        else if(FT1<=65.8534)tmp1=87.3;
        else if(FT1<=65.9294)tmp1=87.4;
        else if(FT1<=66.0054)tmp1=87.5;
        else if(FT1<=66.0815)tmp1=87.6;
        else if(FT1<=66.1577)tmp1=87.7;
        else if(FT1<=66.2339)tmp1=87.8;
        else if(FT1<=66.3101)tmp1=87.9;
        else if(FT1<=66.3864)tmp1=88;
    }
    else if(FT1<=67.0752){
        if(FT1==66.3864)tmp1=88;
        else if(FT1<=66.4627)tmp1=88.1;
        else if(FT1<=66.5391)tmp1=88.2;
        else if(FT1<=66.6155)tmp1=88.3;
        else if(FT1<=66.692)tmp1=88.4;
        else if(FT1<=66.7686)tmp1=88.5;
        else if(FT1<=66.8451)tmp1=88.6;
        else if(FT1<=66.9218)tmp1=88.7;
        else if(FT1<=66.9984)tmp1=88.8;
        else if(FT1<=67.0752)tmp1=88.9;
        else if(FT1<=67.1519)tmp1=89;
    }
    else if(FT1<=67.845){
        if(FT1==67.1519)tmp1=89;
        else if(FT1<=67.2288)tmp1=89.1;
        else if(FT1<=67.3056)tmp1=89.2;
        else if(FT1<=67.3825)tmp1=89.3;
        else if(FT1<=67.4595)tmp1=89.4;
        else if(FT1<=67.5365)tmp1=89.5;
        else if(FT1<=67.6136)tmp1=89.6;
        else if(FT1<=67.6907)tmp1=89.7;
        else if(FT1<=67.7678)tmp1=89.8;
        else if(FT1<=67.845)tmp1=89.9;
        else if(FT1<=67.9223)tmp1=90;
    }
    else if(FT1<=68.6196){
        if(FT1==67.9223)tmp1=90;
        else if(FT1<=67.9995)tmp1=90.1;
        else if(FT1<=68.0769)tmp1=90.2;
        else if(FT1<=68.1543)tmp1=90.3;
        else if(FT1<=68.2317)tmp1=90.4;
        else if(FT1<=68.3092)tmp1=90.5;
        else if(FT1<=68.3868)tmp1=90.6;
        else if(FT1<=68.4643)tmp1=90.7;
        else if(FT1<=68.542)tmp1=90.8;
        else if(FT1<=68.6196)tmp1=90.9;
        else if(FT1<=68.6974)tmp1=91;
    }
    else if(FT1<=69.399){
        if(FT1==68.6974)tmp1=91;
        else if(FT1<=68.7751)tmp1=91.1;
        else if(FT1<=68.853)tmp1=91.2;
        else if(FT1<=68.9308)tmp1=91.3;
        else if(FT1<=69.0087)tmp1=91.4;
        else if(FT1<=69.0867)tmp1=91.5;
        else if(FT1<=69.1647)tmp1=91.6;
        else if(FT1<=69.2428)tmp1=91.7;
        else if(FT1<=69.3209)tmp1=91.8;
        else if(FT1<=69.399)tmp1=91.9;
        else if(FT1<=69.4773)tmp1=92;
    }
    else if(FT1<=70.1832){
        if(FT1==69.4773)tmp1=92;
        else if(FT1<=69.5555)tmp1=92.1;
        else if(FT1<=69.6338)tmp1=92.2;
        else if(FT1<=69.7121)tmp1=92.3;
        else if(FT1<=69.7905)tmp1=92.4;
        else if(FT1<=69.869)tmp1=92.5;
        else if(FT1<=69.9475)tmp1=92.6;
        else if(FT1<=70.026)tmp1=92.7;
        else if(FT1<=70.1046)tmp1=92.8;
        else if(FT1<=70.1832)tmp1=92.9;
        else if(FT1<=70.2619)tmp1=93;
    }
    else if(FT1<=70.9722){
        if(FT1==70.2619)tmp1=93;
        else if(FT1<=70.3406)tmp1=93.1;
        else if(FT1<=70.4194)tmp1=93.2;
        else if(FT1<=70.4982)tmp1=93.3;
        else if(FT1<=70.5771)tmp1=93.4;
        else if(FT1<=70.656)tmp1=93.5;
        else if(FT1<=70.735)tmp1=93.6;
        else if(FT1<=70.814)tmp1=93.7;
        else if(FT1<=70.8931)tmp1=93.8;
        else if(FT1<=70.9722)tmp1=93.9;
        else if(FT1<=71.0513)tmp1=94;
    }
    else if(FT1<=71.7659){
        if(FT1==71.0513)tmp1=94;
        else if(FT1<=71.1305)tmp1=94.1;
        else if(FT1<=71.2098)tmp1=94.2;
        else if(FT1<=71.2891)tmp1=94.3;
        else if(FT1<=71.3684)tmp1=94.4;
        else if(FT1<=71.4478)tmp1=94.5;
        else if(FT1<=71.5273)tmp1=94.6;
        else if(FT1<=71.6068)tmp1=94.7;
        else if(FT1<=71.6863)tmp1=94.8;
        else if(FT1<=71.7659)tmp1=94.9;
        else if(FT1<=71.8455)tmp1=95;
    }
    else if(FT1<=72.5644){
        if(FT1==71.8455)tmp1=95;
        else if(FT1<=71.9252)tmp1=95.1;
        else if(FT1<=72.0049)tmp1=95.2;
        else if(FT1<=72.0847)tmp1=95.3;
        else if(FT1<=72.1645)tmp1=95.4;
        else if(FT1<=72.2444)tmp1=95.5;
        else if(FT1<=72.3243)tmp1=95.6;
        else if(FT1<=72.4043)tmp1=95.7;
        else if(FT1<=72.4843)tmp1=95.8;
        else if(FT1<=72.5644)tmp1=95.9;
        else if(FT1<=72.6445)tmp1=96;
    }
    else if(FT1<=73.3676){
        if(FT1==72.6445)tmp1=96;
        else if(FT1<=72.7246)tmp1=96.1;
        else if(FT1<=72.8048)tmp1=96.2;
        else if(FT1<=72.8851)tmp1=96.3;
        else if(FT1<=72.9654)tmp1=96.4;
        else if(FT1<=73.0457)tmp1=96.5;
        else if(FT1<=73.1261)tmp1=96.6;
        else if(FT1<=73.2066)tmp1=96.7;
        else if(FT1<=73.2871)tmp1=96.8;
        else if(FT1<=73.3676)tmp1=96.9;
        else if(FT1<=73.4482)tmp1=97;
    }
    else if(FT1<=74.1756){
        if(FT1==73.4482)tmp1=97;
        else if(FT1<=73.5288)tmp1=97.1;
        else if(FT1<=73.6095)tmp1=97.2;
        else if(FT1<=73.6902)tmp1=97.3;
        else if(FT1<=73.771)tmp1=97.4;
        else if(FT1<=73.8518)tmp1=97.5;
        else if(FT1<=73.9327)tmp1=97.6;
        else if(FT1<=74.0136)tmp1=97.7;
        else if(FT1<=74.0946)tmp1=97.8;
        else if(FT1<=74.1756)tmp1=97.9;
        else if(FT1<=74.2566)tmp1=98;
    }
    else if(FT1<=74.9883){
        if(FT1==74.2566)tmp1=98;
        else if(FT1<=74.3377)tmp1=98.1;
        else if(FT1<=74.4189)tmp1=98.2;
        else if(FT1<=74.5001)tmp1=98.3;
        else if(FT1<=74.5813)tmp1=98.4;
        else if(FT1<=74.6626)tmp1=98.5;
        else if(FT1<=74.744)tmp1=98.6;
        else if(FT1<=74.8254)tmp1=98.7;
        else if(FT1<=74.9068)tmp1=98.8;
        else if(FT1<=74.9883)tmp1=98.9;
        else if(FT1<=75.0698)tmp1=99;
    }
    else if(FT1<=75.8058){
        if(FT1==75.0698)tmp1=99;
        else if(FT1<=75.1514)tmp1=99.1;
        else if(FT1<=75.233)tmp1=99.2;
        else if(FT1<=75.3147)tmp1=99.3;
        else if(FT1<=75.3964)tmp1=99.4;
        else if(FT1<=75.4782)tmp1=99.5;
        else if(FT1<=75.56)tmp1=99.6;
        else if(FT1<=75.6419)tmp1=99.7;
        else if(FT1<=75.7238)tmp1=99.8;
        else if(FT1<=75.8058)tmp1=99.9;
        else if(FT1<=75.8878)tmp1=100;
    }
    else if(FT1<=76.628){
        if(FT1==75.8878)tmp1=100;
        else if(FT1<=75.9698)tmp1=100.1;
        else if(FT1<=76.0519)tmp1=100.2;
        else if(FT1<=76.1341)tmp1=100.3;
        else if(FT1<=76.2163)tmp1=100.4;
        else if(FT1<=76.2985)tmp1=100.5;
        else if(FT1<=76.3808)tmp1=100.6;
        else if(FT1<=76.4632)tmp1=100.7;
        else if(FT1<=76.5455)tmp1=100.8;
        else if(FT1<=76.628)tmp1=100.9;
        else if(FT1<=76.7104)tmp1=101;
    }
    else if(FT1<=77.4549){
        if(FT1==76.7104)tmp1=101;
        else if(FT1<=76.793)tmp1=101.1;
        else if(FT1<=76.8756)tmp1=101.2;
        else if(FT1<=76.9582)tmp1=101.3;
        else if(FT1<=77.0408)tmp1=101.4;
        else if(FT1<=77.1236)tmp1=101.5;
        else if(FT1<=77.2063)tmp1=101.6;
        else if(FT1<=77.2891)tmp1=101.7;
        else if(FT1<=77.372)tmp1=101.8;
        else if(FT1<=77.4549)tmp1=101.9;
        else if(FT1<=77.5379)tmp1=102;
    }
    else if(FT1<=78.2866){
        if(FT1==77.5379)tmp1=102;
        else if(FT1<=77.6209)tmp1=102.1;
        else if(FT1<=77.7039)tmp1=102.2;
        else if(FT1<=77.787)tmp1=102.3;
        else if(FT1<=77.8701)tmp1=102.4;
        else if(FT1<=77.9533)tmp1=102.5;
        else if(FT1<=78.0366)tmp1=102.6;
        else if(FT1<=78.1198)tmp1=102.7;
        else if(FT1<=78.2032)tmp1=102.8;
        else if(FT1<=78.2866)tmp1=102.9;
        else if(FT1<=78.37)tmp1=103;
    }
    else if(FT1<=79.1229){
        if(FT1==78.37)tmp1=103;
        else if(FT1<=78.4535)tmp1=103.1;
        else if(FT1<=78.537)tmp1=103.2;
        else if(FT1<=78.6205)tmp1=103.3;
        else if(FT1<=78.7042)tmp1=103.4;
        else if(FT1<=78.7878)tmp1=103.5;
        else if(FT1<=78.8715)tmp1=103.6;
        else if(FT1<=78.9553)tmp1=103.7;
        else if(FT1<=79.0391)tmp1=103.8;
        else if(FT1<=79.1229)tmp1=103.9;
        else if(FT1<=79.2068)tmp1=104;
    }
    else if(FT1<=79.964){
        if(FT1==79.2068)tmp1=104;
        else if(FT1<=79.2908)tmp1=104.1;
        else if(FT1<=79.3748)tmp1=104.2;
        else if(FT1<=79.4588)tmp1=104.3;
        else if(FT1<=79.5429)tmp1=104.4;
        else if(FT1<=79.627)tmp1=104.5;
        else if(FT1<=79.7112)tmp1=104.6;
        else if(FT1<=79.7954)tmp1=104.7;
        else if(FT1<=79.8797)tmp1=104.8;
        else if(FT1<=79.964)tmp1=104.9;
        else if(FT1<=80.0484)tmp1=105;
    }
    else if(FT1<=80.8098){
        if(FT1==80.0484)tmp1=105;
        else if(FT1<=80.1328)tmp1=105.1;
        else if(FT1<=80.2173)tmp1=105.2;
        else if(FT1<=80.3018)tmp1=105.3;
        else if(FT1<=80.3863)tmp1=105.4;
        else if(FT1<=80.4709)tmp1=105.5;
        else if(FT1<=80.5556)tmp1=105.6;
        else if(FT1<=80.6403)tmp1=105.7;
        else if(FT1<=80.725)tmp1=105.8;
        else if(FT1<=80.8098)tmp1=105.9;
        else if(FT1<=80.8946)tmp1=106;
    }
    else if(FT1<=81.6603){
        if(FT1==80.8946)tmp1=106;
        else if(FT1<=80.9795)tmp1=106.1;
        else if(FT1<=81.0645)tmp1=106.2;
        else if(FT1<=81.1494)tmp1=106.3;
        else if(FT1<=81.2345)tmp1=106.4;
        else if(FT1<=81.3195)tmp1=106.5;
        else if(FT1<=81.4047)tmp1=106.6;
        else if(FT1<=81.4898)tmp1=106.7;
        else if(FT1<=81.575)tmp1=106.8;
        else if(FT1<=81.6603)tmp1=106.9;
        else if(FT1<=81.7456)tmp1=107;
    }
    else if(FT1<=82.5155){
        if(FT1==81.7456)tmp1=107;
        else if(FT1<=81.831)tmp1=107.1;
        else if(FT1<=81.9164)tmp1=107.2;
        else if(FT1<=82.0018)tmp1=107.3;
        else if(FT1<=82.0873)tmp1=107.4;
        else if(FT1<=82.1728)tmp1=107.5;
        else if(FT1<=82.2584)tmp1=107.6;
        else if(FT1<=82.3441)tmp1=107.7;
        else if(FT1<=82.4298)tmp1=107.8;
        else if(FT1<=82.5155)tmp1=107.9;
        else if(FT1<=82.6013)tmp1=108;
    }
    else if(FT1<=83.3754){
        if(FT1==82.6013)tmp1=108;
        else if(FT1<=82.6871)tmp1=108.1;
        else if(FT1<=82.773)tmp1=108.2;
        else if(FT1<=82.8589)tmp1=108.3;
        else if(FT1<=82.9448)tmp1=108.4;
        else if(FT1<=83.0309)tmp1=108.5;
        else if(FT1<=83.1169)tmp1=108.6;
        else if(FT1<=83.203)tmp1=108.7;
        else if(FT1<=83.2892)tmp1=108.8;
        else if(FT1<=83.3754)tmp1=108.9;
        else if(FT1<=83.4616)tmp1=109;
    }
    else if(FT1<=84.2399){
        if(FT1==83.4616)tmp1=109;
        else if(FT1<=83.5479)tmp1=109.1;
        else if(FT1<=83.6342)tmp1=109.2;
        else if(FT1<=83.7206)tmp1=109.3;
        else if(FT1<=83.8071)tmp1=109.4;
        else if(FT1<=83.8935)tmp1=109.5;
        else if(FT1<=83.9801)tmp1=109.6;
        else if(FT1<=84.0666)tmp1=109.7;
        else if(FT1<=84.1532)tmp1=109.8;
        else if(FT1<=84.2399)tmp1=109.9;
        else if(FT1<=84.3266)tmp1=110;
    }
    else if(FT1<=85.1092){
        if(FT1==84.3266)tmp1=110;
        else if(FT1<=84.4134)tmp1=110.1;
        else if(FT1<=84.5002)tmp1=110.2;
        else if(FT1<=84.5871)tmp1=110.3;
        else if(FT1<=84.674)tmp1=110.4;
        else if(FT1<=84.7609)tmp1=110.5;
        else if(FT1<=84.8479)tmp1=110.6;
        else if(FT1<=84.9349)tmp1=110.7;
        else if(FT1<=85.022)tmp1=110.8;
        else if(FT1<=85.1092)tmp1=110.9;
        else if(FT1<=85.1963)tmp1=111;
    }
    else if(FT1<=85.9831){
        if(FT1==85.1963)tmp1=111;
        else if(FT1<=85.2836)tmp1=111.1;
        else if(FT1<=85.3708)tmp1=111.2;
        else if(FT1<=85.4582)tmp1=111.3;
        else if(FT1<=85.5455)tmp1=111.4;
        else if(FT1<=85.6329)tmp1=111.5;
        else if(FT1<=85.7204)tmp1=111.6;
        else if(FT1<=85.8079)tmp1=111.7;
        else if(FT1<=85.8955)tmp1=111.8;
        else if(FT1<=85.9831)tmp1=111.9;
        else if(FT1<=86.0707)tmp1=112;
    }
    else if(FT1<=86.8616){
        if(FT1==86.0707)tmp1=112;
        else if(FT1<=86.1584)tmp1=112.1;
        else if(FT1<=86.2462)tmp1=112.2;
        else if(FT1<=86.3339)tmp1=112.3;
        else if(FT1<=86.4218)tmp1=112.4;
        else if(FT1<=86.5097)tmp1=112.5;
        else if(FT1<=86.5976)tmp1=112.6;
        else if(FT1<=86.6856)tmp1=112.7;
        else if(FT1<=86.7736)tmp1=112.8;
        else if(FT1<=86.8616)tmp1=112.9;
        else if(FT1<=86.9498)tmp1=113;
    }
    else if(FT1<=87.7449){
        if(FT1==86.9498)tmp1=113;
        else if(FT1<=87.0379)tmp1=113.1;
        else if(FT1<=87.1261)tmp1=113.2;
        else if(FT1<=87.2144)tmp1=113.3;
        else if(FT1<=87.3027)tmp1=113.4;
        else if(FT1<=87.391)tmp1=113.5;
        else if(FT1<=87.4794)tmp1=113.6;
        else if(FT1<=87.5679)tmp1=113.7;
        else if(FT1<=87.6563)tmp1=113.8;
        else if(FT1<=87.7449)tmp1=113.9;
        else if(FT1<=87.8334)tmp1=114;
    }
    else if(FT1<=88.6327){
        if(FT1==87.8334)tmp1=114;
        else if(FT1<=87.9221)tmp1=114.1;
        else if(FT1<=88.0107)tmp1=114.2;
        else if(FT1<=88.0995)tmp1=114.3;
        else if(FT1<=88.1882)tmp1=114.4;
        else if(FT1<=88.277)tmp1=114.5;
        else if(FT1<=88.3659)tmp1=114.6;
        else if(FT1<=88.4548)tmp1=114.7;
        else if(FT1<=88.5437)tmp1=114.8;
        else if(FT1<=88.6327)tmp1=114.9;
        else if(FT1<=88.7218)tmp1=115;
    }
    else if(FT1<=89.5253){
        if(FT1==88.7218)tmp1=115;
        else if(FT1<=88.8109)tmp1=115.1;
        else if(FT1<=88.9)tmp1=115.2;
        else if(FT1<=88.9892)tmp1=115.3;
        else if(FT1<=89.0784)tmp1=115.4;
        else if(FT1<=89.1677)tmp1=115.5;
        else if(FT1<=89.257)tmp1=115.6;
        else if(FT1<=89.3464)tmp1=115.7;
        else if(FT1<=89.4358)tmp1=115.8;
        else if(FT1<=89.5253)tmp1=115.9;
        else if(FT1<=89.6148)tmp1=116;
    }
    else if(FT1<=90.4224){
        if(FT1==89.6148)tmp1=116;
        else if(FT1<=89.7043)tmp1=116.1;
        else if(FT1<=89.7939)tmp1=116.2;
        else if(FT1<=89.8836)tmp1=116.3;
        else if(FT1<=89.9733)tmp1=116.4;
        else if(FT1<=90.063)tmp1=116.5;
        else if(FT1<=90.1528)tmp1=116.6;
        else if(FT1<=90.2426)tmp1=116.7;
        else if(FT1<=90.3325)tmp1=116.8;
        else if(FT1<=90.4224)tmp1=116.9;
        else if(FT1<=90.5124)tmp1=117;
    }
    else if(FT1<=91.3242){
        if(FT1==90.5124)tmp1=117;
        else if(FT1<=90.6024)tmp1=117.1;
        else if(FT1<=90.6925)tmp1=117.2;
        else if(FT1<=90.7826)tmp1=117.3;
        else if(FT1<=90.8727)tmp1=117.4;
        else if(FT1<=90.9629)tmp1=117.5;
        else if(FT1<=91.0532)tmp1=117.6;
        else if(FT1<=91.1435)tmp1=117.7;
        else if(FT1<=91.2338)tmp1=117.8;
        else if(FT1<=91.3242)tmp1=117.9;
        else if(FT1<=91.4146)tmp1=118;
    }
    else if(FT1<=92.2306){
        if(FT1==91.4146)tmp1=118;
        else if(FT1<=91.5051)tmp1=118.1;
        else if(FT1<=91.5957)tmp1=118.2;
        else if(FT1<=91.6862)tmp1=118.3;
        else if(FT1<=91.7768)tmp1=118.4;
        else if(FT1<=91.8675)tmp1=118.5;
        else if(FT1<=91.9582)tmp1=118.6;
        else if(FT1<=92.049)tmp1=118.7;
        else if(FT1<=92.1398)tmp1=118.8;
        else if(FT1<=92.2306)tmp1=118.9;
        else if(FT1<=92.3215)tmp1=119;
    }
    else if(FT1<=93.1417){
        if(FT1==92.3215)tmp1=119;
        else if(FT1<=92.4125)tmp1=119.1;
        else if(FT1<=92.5035)tmp1=119.2;
        else if(FT1<=92.5945)tmp1=119.3;
        else if(FT1<=92.6856)tmp1=119.4;
        else if(FT1<=92.7767)tmp1=119.5;
        else if(FT1<=92.8679)tmp1=119.6;
        else if(FT1<=92.9591)tmp1=119.7;
        else if(FT1<=93.0503)tmp1=119.8;
        else if(FT1<=93.1417)tmp1=119.9;
        else if(FT1<=93.233)tmp1=120;
    }
    else if(FT1<=94.0573){
        if(FT1==93.233)tmp1=120;
        else if(FT1<=93.3244)tmp1=120.1;
        else if(FT1<=93.4159)tmp1=120.2;
        else if(FT1<=93.5074)tmp1=120.3;
        else if(FT1<=93.5989)tmp1=120.4;
        else if(FT1<=93.6905)tmp1=120.5;
        else if(FT1<=93.7821)tmp1=120.6;
        else if(FT1<=93.8738)tmp1=120.7;
        else if(FT1<=93.9655)tmp1=120.8;
        else if(FT1<=94.0573)tmp1=120.9;
        else if(FT1<=94.1491)tmp1=121;
    }
    else if(FT1<=94.9775){
        if(FT1==94.1491)tmp1=121;
        else if(FT1<=94.241)tmp1=121.1;
        else if(FT1<=94.3329)tmp1=121.2;
        else if(FT1<=94.4248)tmp1=121.3;
        else if(FT1<=94.5168)tmp1=121.4;
        else if(FT1<=94.6089)tmp1=121.5;
        else if(FT1<=94.701)tmp1=121.6;
        else if(FT1<=94.7931)tmp1=121.7;
        else if(FT1<=94.8853)tmp1=121.8;
        else if(FT1<=94.9775)tmp1=121.9;
        else if(FT1<=95.0698)tmp1=122;
    }
    else if(FT1<=95.9024){
        if(FT1==95.0698)tmp1=122;
        else if(FT1<=95.1621)tmp1=122.1;
        else if(FT1<=95.2545)tmp1=122.2;
        else if(FT1<=95.3469)tmp1=122.3;
        else if(FT1<=95.4394)tmp1=122.4;
        else if(FT1<=95.5319)tmp1=122.5;
        else if(FT1<=95.6244)tmp1=122.6;
        else if(FT1<=95.717)tmp1=122.7;
        else if(FT1<=95.8097)tmp1=122.8;
        else if(FT1<=95.9024)tmp1=122.9;
        else if(FT1<=95.9951)tmp1=123;
    }
    else if(FT1<=96.8318){
        if(FT1==95.9951)tmp1=123;
        else if(FT1<=96.0879)tmp1=123.1;
        else if(FT1<=96.1807)tmp1=123.2;
        else if(FT1<=96.2736)tmp1=123.3;
        else if(FT1<=96.3665)tmp1=123.4;
        else if(FT1<=96.4595)tmp1=123.5;
        else if(FT1<=96.5525)tmp1=123.6;
        else if(FT1<=96.6455)tmp1=123.7;
        else if(FT1<=96.7386)tmp1=123.8;
        else if(FT1<=96.8318)tmp1=123.9;
        else if(FT1<=96.925)tmp1=124;
    }
    else if(FT1<=97.7658){
        if(FT1==96.925)tmp1=124;
        else if(FT1<=97.0182)tmp1=124.1;
        else if(FT1<=97.1115)tmp1=124.2;
        else if(FT1<=97.2048)tmp1=124.3;
        else if(FT1<=97.2982)tmp1=124.4;
        else if(FT1<=97.3916)tmp1=124.5;
        else if(FT1<=97.4851)tmp1=124.6;
        else if(FT1<=97.5786)tmp1=124.7;
        else if(FT1<=97.6722)tmp1=124.8;
        else if(FT1<=97.7658)tmp1=124.9;
        else if(FT1<=97.8594)tmp1=125;
    }
    else if(FT1<=98.7043){
        if(FT1==97.8594)tmp1=125;
        else if(FT1<=97.9531)tmp1=125.1;
        else if(FT1<=98.0469)tmp1=125.2;
        else if(FT1<=98.1407)tmp1=125.3;
        else if(FT1<=98.2345)tmp1=125.4;
        else if(FT1<=98.3284)tmp1=125.5;
        else if(FT1<=98.4223)tmp1=125.6;
        else if(FT1<=98.5163)tmp1=125.7;
        else if(FT1<=98.6103)tmp1=125.8;
        else if(FT1<=98.7043)tmp1=125.9;
        else if(FT1<=98.7985)tmp1=126;
    }
    else if(FT1<=99.6475){
        if(FT1==98.7985)tmp1=126;
        else if(FT1<=98.8926)tmp1=126.1;
        else if(FT1<=98.9868)tmp1=126.2;
        else if(FT1<=99.081)tmp1=126.3;
        else if(FT1<=99.1753)tmp1=126.4;
        else if(FT1<=99.2697)tmp1=126.5;
        else if(FT1<=99.3641)tmp1=126.6;
        else if(FT1<=99.4585)tmp1=126.7;
        else if(FT1<=99.553)tmp1=126.8;
        else if(FT1<=99.6475)tmp1=126.9;
        else if(FT1<=99.742)tmp1=127;
    }
    else if(FT1<=100.595){
        if(FT1==99.742)tmp1=127;
        else if(FT1<=99.8366)tmp1=127.1;
        else if(FT1<=99.9313)tmp1=127.2;
        else if(FT1<=100.026)tmp1=127.3;
        else if(FT1<=100.121)tmp1=127.4;
        else if(FT1<=100.216)tmp1=127.5;
        else if(FT1<=100.31)tmp1=127.6;
        else if(FT1<=100.405)tmp1=127.7;
        else if(FT1<=100.5)tmp1=127.8;
        else if(FT1<=100.595)tmp1=127.9;
        else if(FT1<=100.69)tmp1=128;
    }
    else if(FT1<=101.547){
        if(FT1==100.69)tmp1=128;
        else if(FT1<=100.785)tmp1=128.1;
        else if(FT1<=100.88)tmp1=128.2;
        else if(FT1<=100.976)tmp1=128.3;
        else if(FT1<=101.071)tmp1=128.4;
        else if(FT1<=101.166)tmp1=128.5;
        else if(FT1<=101.261)tmp1=128.6;
        else if(FT1<=101.357)tmp1=128.7;
        else if(FT1<=101.452)tmp1=128.8;
        else if(FT1<=101.547)tmp1=128.9;
        else if(FT1<=101.643)tmp1=129;
    }
    else if(FT1<=102.504){
        if(FT1==101.643)tmp1=129;
        else if(FT1<=101.738)tmp1=129.1;
        else if(FT1<=101.834)tmp1=129.2;
        else if(FT1<=101.93)tmp1=129.3;
        else if(FT1<=102.025)tmp1=129.4;
        else if(FT1<=102.121)tmp1=129.5;
        else if(FT1<=102.217)tmp1=129.6;
        else if(FT1<=102.312)tmp1=129.7;
        else if(FT1<=102.408)tmp1=129.8;
        else if(FT1<=102.504)tmp1=129.9;
        else if(FT1<=102.6)tmp1=130;
    }
    else if(FT1<=103.465){
        if(FT1==102.6)tmp1=130;
        else if(FT1<=102.696)tmp1=130.1;
        else if(FT1<=102.792)tmp1=130.2;
        else if(FT1<=102.888)tmp1=130.3;
        else if(FT1<=102.984)tmp1=130.4;
        else if(FT1<=103.08)tmp1=130.5;
        else if(FT1<=103.177)tmp1=130.6;
        else if(FT1<=103.273)tmp1=130.7;
        else if(FT1<=103.369)tmp1=130.8;
        else if(FT1<=103.465)tmp1=130.9;
        else if(FT1<=103.562)tmp1=131;
    }
    else if(FT1<=104.431){
        if(FT1==103.562)tmp1=131;
        else if(FT1<=103.658)tmp1=131.1;
        else if(FT1<=103.755)tmp1=131.2;
        else if(FT1<=103.851)tmp1=131.3;
        else if(FT1<=103.948)tmp1=131.4;
        else if(FT1<=104.044)tmp1=131.5;
        else if(FT1<=104.141)tmp1=131.6;
        else if(FT1<=104.238)tmp1=131.7;
        else if(FT1<=104.335)tmp1=131.8;
        else if(FT1<=104.431)tmp1=131.9;
        else if(FT1<=104.528)tmp1=132;
    }
    else if(FT1<=105.402){
        if(FT1==104.528)tmp1=132;
        else if(FT1<=104.625)tmp1=132.1;
        else if(FT1<=104.722)tmp1=132.2;
        else if(FT1<=104.819)tmp1=132.3;
        else if(FT1<=104.916)tmp1=132.4;
        else if(FT1<=105.013)tmp1=132.5;
        else if(FT1<=105.11)tmp1=132.6;
        else if(FT1<=105.207)tmp1=132.7;
        else if(FT1<=105.304)tmp1=132.8;
        else if(FT1<=105.402)tmp1=132.9;
        else if(FT1<=105.499)tmp1=133;
    }
    else if(FT1<=106.377){
        if(FT1==105.499)tmp1=133;
        else if(FT1<=105.596)tmp1=133.1;
        else if(FT1<=105.694)tmp1=133.2;
        else if(FT1<=105.791)tmp1=133.3;
        else if(FT1<=105.889)tmp1=133.4;
        else if(FT1<=105.986)tmp1=133.5;
        else if(FT1<=106.084)tmp1=133.6;
        else if(FT1<=106.181)tmp1=133.7;
        else if(FT1<=106.279)tmp1=133.8;
        else if(FT1<=106.377)tmp1=133.9;
        else if(FT1<=106.474)tmp1=134;
    }
    else if(FT1<=107.356){
        if(FT1==106.474)tmp1=134;
        else if(FT1<=106.572)tmp1=134.1;
        else if(FT1<=106.67)tmp1=134.2;
        else if(FT1<=106.768)tmp1=134.3;
        else if(FT1<=106.866)tmp1=134.4;
        else if(FT1<=106.964)tmp1=134.5;
        else if(FT1<=107.062)tmp1=134.6;
        else if(FT1<=107.16)tmp1=134.7;
        else if(FT1<=107.258)tmp1=134.8;
        else if(FT1<=107.356)tmp1=134.9;
        else if(FT1<=107.454)tmp1=135;
    }
    else if(FT1<=108.34){
        if(FT1==107.454)tmp1=135;
        else if(FT1<=107.552)tmp1=135.1;
        else if(FT1<=107.651)tmp1=135.2;
        else if(FT1<=107.749)tmp1=135.3;
        else if(FT1<=107.847)tmp1=135.4;
        else if(FT1<=107.946)tmp1=135.5;
        else if(FT1<=108.044)tmp1=135.6;
        else if(FT1<=108.143)tmp1=135.7;
        else if(FT1<=108.241)tmp1=135.8;
        else if(FT1<=108.34)tmp1=135.9;
        else if(FT1<=108.438)tmp1=136;
    }
    else if(FT1<=109.328){
        if(FT1==108.438)tmp1=136;
        else if(FT1<=108.537)tmp1=136.1;
        else if(FT1<=108.636)tmp1=136.2;
        else if(FT1<=108.735)tmp1=136.3;
        else if(FT1<=108.833)tmp1=136.4;
        else if(FT1<=108.932)tmp1=136.5;
        else if(FT1<=109.031)tmp1=136.6;
        else if(FT1<=109.13)tmp1=136.7;
        else if(FT1<=109.229)tmp1=136.8;
        else if(FT1<=109.328)tmp1=136.9;
        else if(FT1<=109.427)tmp1=137;
    }
    else if(FT1<=110.321){
        if(FT1==109.427)tmp1=137;
        else if(FT1<=109.526)tmp1=137.1;
        else if(FT1<=109.625)tmp1=137.2;
        else if(FT1<=109.725)tmp1=137.3;
        else if(FT1<=109.824)tmp1=137.4;
        else if(FT1<=109.923)tmp1=137.5;
        else if(FT1<=110.023)tmp1=137.6;
        else if(FT1<=110.122)tmp1=137.7;
        else if(FT1<=110.221)tmp1=137.8;
        else if(FT1<=110.321)tmp1=137.9;
        else if(FT1<=110.42)tmp1=138;
    }
    else if(FT1<=111.318){
        if(FT1==110.42)tmp1=138;
        else if(FT1<=110.52)tmp1=138.1;
        else if(FT1<=110.62)tmp1=138.2;
        else if(FT1<=110.719)tmp1=138.3;
        else if(FT1<=110.819)tmp1=138.4;
        else if(FT1<=110.919)tmp1=138.5;
        else if(FT1<=111.019)tmp1=138.6;
        else if(FT1<=111.118)tmp1=138.7;
        else if(FT1<=111.218)tmp1=138.8;
        else if(FT1<=111.318)tmp1=138.9;
        else if(FT1<=111.418)tmp1=139;
    }
    else if(FT1<=112.32){
        if(FT1==111.418)tmp1=139;
        else if(FT1<=111.518)tmp1=139.1;
        else if(FT1<=111.618)tmp1=139.2;
        else if(FT1<=111.718)tmp1=139.3;
        else if(FT1<=111.818)tmp1=139.4;
        else if(FT1<=111.919)tmp1=139.5;
        else if(FT1<=112.019)tmp1=139.6;
        else if(FT1<=112.119)tmp1=139.7;
        else if(FT1<=112.22)tmp1=139.8;
        else if(FT1<=112.32)tmp1=139.9;
        else if(FT1<=112.42)tmp1=140;
    }
    else if(FT1<=113.326){
        if(FT1==112.42)tmp1=140;
        else if(FT1<=112.521)tmp1=140.1;
        else if(FT1<=112.621)tmp1=140.2;
        else if(FT1<=112.722)tmp1=140.3;
        else if(FT1<=112.822)tmp1=140.4;
        else if(FT1<=112.923)tmp1=140.5;
        else if(FT1<=113.024)tmp1=140.6;
        else if(FT1<=113.124)tmp1=140.7;
        else if(FT1<=113.225)tmp1=140.8;
        else if(FT1<=113.326)tmp1=140.9;
        else if(FT1<=113.427)tmp1=141;
    }
    else if(FT1<=114.337){
        if(FT1==113.427)tmp1=141;
        else if(FT1<=113.528)tmp1=141.1;
        else if(FT1<=113.629)tmp1=141.2;
        else if(FT1<=113.73)tmp1=141.3;
        else if(FT1<=113.831)tmp1=141.4;
        else if(FT1<=113.932)tmp1=141.5;
        else if(FT1<=114.033)tmp1=141.6;
        else if(FT1<=114.134)tmp1=141.7;
        else if(FT1<=114.235)tmp1=141.8;
        else if(FT1<=114.337)tmp1=141.9;
        else if(FT1<=114.438)tmp1=142;
    }
    else if(FT1<=115.352){
        if(FT1==114.438)tmp1=142;
        else if(FT1<=114.539)tmp1=142.1;
        else if(FT1<=114.641)tmp1=142.2;
        else if(FT1<=114.742)tmp1=142.3;
        else if(FT1<=114.844)tmp1=142.4;
        else if(FT1<=114.945)tmp1=142.5;
        else if(FT1<=115.047)tmp1=142.6;
        else if(FT1<=115.148)tmp1=142.7;
        else if(FT1<=115.25)tmp1=142.8;
        else if(FT1<=115.352)tmp1=142.9;
        else if(FT1<=115.454)tmp1=143;
    }
    else if(FT1<=116.371){
        if(FT1==115.454)tmp1=143;
        else if(FT1<=115.555)tmp1=143.1;
        else if(FT1<=115.657)tmp1=143.2;
        else if(FT1<=115.759)tmp1=143.3;
        else if(FT1<=115.861)tmp1=143.4;
        else if(FT1<=115.963)tmp1=143.5;
        else if(FT1<=116.065)tmp1=143.6;
        else if(FT1<=116.167)tmp1=143.7;
        else if(FT1<=116.269)tmp1=143.8;
        else if(FT1<=116.371)tmp1=143.9;
        else if(FT1<=116.473)tmp1=144;
    }
    else if(FT1<=117.395){
        if(FT1==116.473)tmp1=144;
        else if(FT1<=116.576)tmp1=144.1;
        else if(FT1<=116.678)tmp1=144.2;
        else if(FT1<=116.78)tmp1=144.3;
        else if(FT1<=116.883)tmp1=144.4;
        else if(FT1<=116.985)tmp1=144.5;
        else if(FT1<=117.088)tmp1=144.6;
        else if(FT1<=117.19)tmp1=144.7;
        else if(FT1<=117.293)tmp1=144.8;
        else if(FT1<=117.395)tmp1=144.9;
        else if(FT1<=117.498)tmp1=145;
    }
    else if(FT1<=118.423){
        if(FT1==117.498)tmp1=145;
        else if(FT1<=117.6)tmp1=145.1;
        else if(FT1<=117.703)tmp1=145.2;
        else if(FT1<=117.806)tmp1=145.3;
        else if(FT1<=117.909)tmp1=145.4;
        else if(FT1<=118.012)tmp1=145.5;
        else if(FT1<=118.115)tmp1=145.6;
        else if(FT1<=118.217)tmp1=145.7;
        else if(FT1<=118.32)tmp1=145.8;
        else if(FT1<=118.423)tmp1=145.9;
        else if(FT1<=118.527)tmp1=146;
    }
    else if(FT1<=119.456){
        if(FT1==118.527)tmp1=146;
        else if(FT1<=118.63)tmp1=146.1;
        else if(FT1<=118.733)tmp1=146.2;
        else if(FT1<=118.836)tmp1=146.3;
        else if(FT1<=118.939)tmp1=146.4;
        else if(FT1<=119.043)tmp1=146.5;
        else if(FT1<=119.146)tmp1=146.6;
        else if(FT1<=119.249)tmp1=146.7;
        else if(FT1<=119.353)tmp1=146.8;
        else if(FT1<=119.456)tmp1=146.9;
        else if(FT1<=119.56)tmp1=147;
    }
    else if(FT1<=120.493){
        if(FT1==119.56)tmp1=147;
        else if(FT1<=119.663)tmp1=147.1;
        else if(FT1<=119.767)tmp1=147.2;
        else if(FT1<=119.87)tmp1=147.3;
        else if(FT1<=119.974)tmp1=147.4;
        else if(FT1<=120.078)tmp1=147.5;
        else if(FT1<=120.182)tmp1=147.6;
        else if(FT1<=120.286)tmp1=147.7;
        else if(FT1<=120.389)tmp1=147.8;
        else if(FT1<=120.493)tmp1=147.9;
        else if(FT1<=120.597)tmp1=148;
    }
    else if(FT1<=121.535){
        if(FT1==120.597)tmp1=148;
        else if(FT1<=120.701)tmp1=148.1;
        else if(FT1<=120.805)tmp1=148.2;
        else if(FT1<=120.909)tmp1=148.3;
        else if(FT1<=121.013)tmp1=148.4;
        else if(FT1<=121.118)tmp1=148.5;
        else if(FT1<=121.222)tmp1=148.6;
        else if(FT1<=121.326)tmp1=148.7;
        else if(FT1<=121.43)tmp1=148.8;
        else if(FT1<=121.535)tmp1=148.9;
        else if(FT1<=121.639)tmp1=149;
    }
    else if(FT1<=122.581){
        if(FT1==121.639)tmp1=149;
        else if(FT1<=121.744)tmp1=149.1;
        else if(FT1<=121.848)tmp1=149.2;
        else if(FT1<=121.953)tmp1=149.3;
        else if(FT1<=122.057)tmp1=149.4;
        else if(FT1<=122.162)tmp1=149.5;
        else if(FT1<=122.266)tmp1=149.6;
        else if(FT1<=122.371)tmp1=149.7;
        else if(FT1<=122.476)tmp1=149.8;
        else if(FT1<=122.581)tmp1=149.9;
        else if(FT1<=122.685)tmp1=150;
    }
    else if(FT1<=123.631){
        if(FT1==122.685)tmp1=150;
        else if(FT1<=122.79)tmp1=150.1;
        else if(FT1<=122.895)tmp1=150.2;
        else if(FT1<=123)tmp1=150.3;
        else if(FT1<=123.105)tmp1=150.4;
        else if(FT1<=123.21)tmp1=150.5;
        else if(FT1<=123.315)tmp1=150.6;
        else if(FT1<=123.42)tmp1=150.7;
        else if(FT1<=123.526)tmp1=150.8;
        else if(FT1<=123.631)tmp1=150.9;
        else if(FT1<=123.736)tmp1=151;
    }
    else if(FT1<=124.685){
        if(FT1==123.736)tmp1=151;
        else if(FT1<=123.841)tmp1=151.1;
        else if(FT1<=123.947)tmp1=151.2;
        else if(FT1<=124.052)tmp1=151.3;
        else if(FT1<=124.158)tmp1=151.4;
        else if(FT1<=124.263)tmp1=151.5;
        else if(FT1<=124.369)tmp1=151.6;
        else if(FT1<=124.474)tmp1=151.7;
        else if(FT1<=124.58)tmp1=151.8;
        else if(FT1<=124.685)tmp1=151.9;
        else if(FT1<=124.791)tmp1=152;
    }
    else if(FT1<=125.744){
        if(FT1==124.791)tmp1=152;
        else if(FT1<=124.897)tmp1=152.1;
        else if(FT1<=125.003)tmp1=152.2;
        else if(FT1<=125.108)tmp1=152.3;
        else if(FT1<=125.214)tmp1=152.4;
        else if(FT1<=125.32)tmp1=152.5;
        else if(FT1<=125.426)tmp1=152.6;
        else if(FT1<=125.532)tmp1=152.7;
        else if(FT1<=125.638)tmp1=152.8;
        else if(FT1<=125.744)tmp1=152.9;
        else if(FT1<=125.85)tmp1=153;
    }
    else if(FT1<=126.808){
        if(FT1==125.85)tmp1=153;
        else if(FT1<=125.957)tmp1=153.1;
        else if(FT1<=126.063)tmp1=153.2;
        else if(FT1<=126.169)tmp1=153.3;
        else if(FT1<=126.275)tmp1=153.4;
        else if(FT1<=126.382)tmp1=153.5;
        else if(FT1<=126.488)tmp1=153.6;
        else if(FT1<=126.595)tmp1=153.7;
        else if(FT1<=126.701)tmp1=153.8;
        else if(FT1<=126.808)tmp1=153.9;
        else if(FT1<=126.914)tmp1=154;
    }
    else if(FT1<=127.875){
        if(FT1==126.914)tmp1=154;
        else if(FT1<=127.021)tmp1=154.1;
        else if(FT1<=127.127)tmp1=154.2;
        else if(FT1<=127.234)tmp1=154.3;
        else if(FT1<=127.341)tmp1=154.4;
        else if(FT1<=127.448)tmp1=154.5;
        else if(FT1<=127.554)tmp1=154.6;
        else if(FT1<=127.661)tmp1=154.7;
        else if(FT1<=127.768)tmp1=154.8;
        else if(FT1<=127.875)tmp1=154.9;
        else if(FT1<=127.982)tmp1=155;
    }
    else if(FT1<=128.947){
        if(FT1==127.982)tmp1=155;
        else if(FT1<=128.089)tmp1=155.1;
        else if(FT1<=128.196)tmp1=155.2;
        else if(FT1<=128.303)tmp1=155.3;
        else if(FT1<=128.41)tmp1=155.4;
        else if(FT1<=128.518)tmp1=155.5;
        else if(FT1<=128.625)tmp1=155.6;
        else if(FT1<=128.732)tmp1=155.7;
        else if(FT1<=128.84)tmp1=155.8;
        else if(FT1<=128.947)tmp1=155.9;
        else if(FT1<=129.054)tmp1=156;
    }
    else if(FT1<=130.023){
        if(FT1==129.054)tmp1=156;
        else if(FT1<=129.162)tmp1=156.1;
        else if(FT1<=129.269)tmp1=156.2;
        else if(FT1<=129.377)tmp1=156.3;
        else if(FT1<=129.484)tmp1=156.4;
        else if(FT1<=129.592)tmp1=156.5;
        else if(FT1<=129.7)tmp1=156.6;
        else if(FT1<=129.808)tmp1=156.7;
        else if(FT1<=129.915)tmp1=156.8;
        else if(FT1<=130.023)tmp1=156.9;
        else if(FT1<=130.131)tmp1=157;
    }
    else if(FT1<=131.104){
        if(FT1==130.131)tmp1=157;
        else if(FT1<=130.239)tmp1=157.1;
        else if(FT1<=130.347)tmp1=157.2;
        else if(FT1<=130.455)tmp1=157.3;
        else if(FT1<=130.563)tmp1=157.4;
        else if(FT1<=130.671)tmp1=157.5;
        else if(FT1<=130.779)tmp1=157.6;
        else if(FT1<=130.887)tmp1=157.7;
        else if(FT1<=130.995)tmp1=157.8;
        else if(FT1<=131.104)tmp1=157.9;
        else if(FT1<=131.212)tmp1=158;
    }
    else if(FT1<=132.188){
        if(FT1==131.212)tmp1=158;
        else if(FT1<=131.32)tmp1=158.1;
        else if(FT1<=131.429)tmp1=158.2;
        else if(FT1<=131.537)tmp1=158.3;
        else if(FT1<=131.645)tmp1=158.4;
        else if(FT1<=131.754)tmp1=158.5;
        else if(FT1<=131.863)tmp1=158.6;
        else if(FT1<=131.971)tmp1=158.7;
        else if(FT1<=132.08)tmp1=158.8;
        else if(FT1<=132.188)tmp1=158.9;
        else if(FT1<=132.297)tmp1=159;
    }
    else if(FT1<=133.277){
        if(FT1==132.297)tmp1=159;
        else if(FT1<=132.406)tmp1=159.1;
        else if(FT1<=132.515)tmp1=159.2;
        else if(FT1<=132.624)tmp1=159.3;
        else if(FT1<=132.732)tmp1=159.4;
        else if(FT1<=132.841)tmp1=159.5;
        else if(FT1<=132.95)tmp1=159.6;
        else if(FT1<=133.059)tmp1=159.7;
        else if(FT1<=133.168)tmp1=159.8;
        else if(FT1<=133.277)tmp1=159.9;
        else if(FT1<=133.387)tmp1=160;
    }
    else if(FT1<=134.371){
        if(FT1==133.387)tmp1=160;
        else if(FT1<=133.496)tmp1=160.1;
        else if(FT1<=133.605)tmp1=160.2;
        else if(FT1<=133.714)tmp1=160.3;
        else if(FT1<=133.824)tmp1=160.4;
        else if(FT1<=133.933)tmp1=160.5;
        else if(FT1<=134.042)tmp1=160.6;
        else if(FT1<=134.152)tmp1=160.7;
        else if(FT1<=134.261)tmp1=160.8;
        else if(FT1<=134.371)tmp1=160.9;
        else if(FT1<=134.48)tmp1=161;
    }
    else if(FT1<=135.468){
        if(FT1==134.48)tmp1=161;
        else if(FT1<=134.59)tmp1=161.1;
        else if(FT1<=134.7)tmp1=161.2;
        else if(FT1<=134.809)tmp1=161.3;
        else if(FT1<=134.919)tmp1=161.4;
        else if(FT1<=135.029)tmp1=161.5;
        else if(FT1<=135.139)tmp1=161.6;
        else if(FT1<=135.249)tmp1=161.7;
        else if(FT1<=135.358)tmp1=161.8;
        else if(FT1<=135.468)tmp1=161.9;
        else if(FT1<=135.578)tmp1=162;
    }
    else if(FT1<=136.57){
        if(FT1==135.578)tmp1=162;
        else if(FT1<=135.688)tmp1=162.1;
        else if(FT1<=135.798)tmp1=162.2;
        else if(FT1<=135.909)tmp1=162.3;
        else if(FT1<=136.019)tmp1=162.4;
        else if(FT1<=136.129)tmp1=162.5;
        else if(FT1<=136.239)tmp1=162.6;
        else if(FT1<=136.349)tmp1=162.7;
        else if(FT1<=136.46)tmp1=162.8;
        else if(FT1<=136.57)tmp1=162.9;
        else if(FT1<=136.681)tmp1=163;
    }
    else if(FT1<=137.676){
        if(FT1==136.681)tmp1=163;
        else if(FT1<=136.791)tmp1=163.1;
        else if(FT1<=136.902)tmp1=163.2;
        else if(FT1<=137.012)tmp1=163.3;
        else if(FT1<=137.123)tmp1=163.4;
        else if(FT1<=137.233)tmp1=163.5;
        else if(FT1<=137.344)tmp1=163.6;
        else if(FT1<=137.455)tmp1=163.7;
        else if(FT1<=137.565)tmp1=163.8;
        else if(FT1<=137.676)tmp1=163.9;
        else if(FT1<=137.787)tmp1=164;
    }
    else if(FT1<=138.787){
        if(FT1==137.787)tmp1=164;
        else if(FT1<=137.898)tmp1=164.1;
        else if(FT1<=138.009)tmp1=164.2;
        else if(FT1<=138.12)tmp1=164.3;
        else if(FT1<=138.231)tmp1=164.4;
        else if(FT1<=138.342)tmp1=164.5;
        else if(FT1<=138.453)tmp1=164.6;
        else if(FT1<=138.564)tmp1=164.7;
        else if(FT1<=138.675)tmp1=164.8;
        else if(FT1<=138.787)tmp1=164.9;
        else if(FT1<=138.898)tmp1=165;
    }
    else if(FT1<=139.901){
        if(FT1==138.898)tmp1=165;
        else if(FT1<=139.009)tmp1=165.1;
        else if(FT1<=139.121)tmp1=165.2;
        else if(FT1<=139.232)tmp1=165.3;
        else if(FT1<=139.343)tmp1=165.4;
        else if(FT1<=139.455)tmp1=165.5;
        else if(FT1<=139.566)tmp1=165.6;
        else if(FT1<=139.678)tmp1=165.7;
        else if(FT1<=139.789)tmp1=165.8;
        else if(FT1<=139.901)tmp1=165.9;
        else if(FT1<=140.013)tmp1=166;
    }
    else if(FT1<=141.02){
        if(FT1==140.013)tmp1=166;
        else if(FT1<=140.125)tmp1=166.1;
        else if(FT1<=140.236)tmp1=166.2;
        else if(FT1<=140.348)tmp1=166.3;
        else if(FT1<=140.46)tmp1=166.4;
        else if(FT1<=140.572)tmp1=166.5;
        else if(FT1<=140.684)tmp1=166.6;
        else if(FT1<=140.796)tmp1=166.7;
        else if(FT1<=140.908)tmp1=166.8;
        else if(FT1<=141.02)tmp1=166.9;
        else if(FT1<=141.132)tmp1=167;
    }
    else if(FT1<=142.143){
        if(FT1==141.132)tmp1=167;
        else if(FT1<=141.244)tmp1=167.1;
        else if(FT1<=141.356)tmp1=167.2;
        else if(FT1<=141.469)tmp1=167.3;
        else if(FT1<=141.581)tmp1=167.4;
        else if(FT1<=141.693)tmp1=167.5;
        else if(FT1<=141.806)tmp1=167.6;
        else if(FT1<=141.918)tmp1=167.7;
        else if(FT1<=142.03)tmp1=167.8;
        else if(FT1<=142.143)tmp1=167.9;
        else if(FT1<=142.255)tmp1=168;
    }
    else if(FT1<=143.27){
        if(FT1==142.255)tmp1=168;
        else if(FT1<=142.368)tmp1=168.1;
        else if(FT1<=142.481)tmp1=168.2;
        else if(FT1<=142.593)tmp1=168.3;
        else if(FT1<=142.706)tmp1=168.4;
        else if(FT1<=142.819)tmp1=168.5;
        else if(FT1<=142.931)tmp1=168.6;
        else if(FT1<=143.044)tmp1=168.7;
        else if(FT1<=143.157)tmp1=168.8;
        else if(FT1<=143.27)tmp1=168.9;
        else if(FT1<=143.383)tmp1=169;
    }
    else if(FT1<=144.401){
        if(FT1==143.383)tmp1=169;
        else if(FT1<=143.496)tmp1=169.1;
        else if(FT1<=143.609)tmp1=169.2;
        else if(FT1<=143.722)tmp1=169.3;
        else if(FT1<=143.835)tmp1=169.4;
        else if(FT1<=143.948)tmp1=169.5;
        else if(FT1<=144.061)tmp1=169.6;
        else if(FT1<=144.175)tmp1=169.7;
        else if(FT1<=144.288)tmp1=169.8;
        else if(FT1<=144.401)tmp1=169.9;
        else if(FT1<=144.515)tmp1=170;
    }
    else if(FT1<=145.537){
        if(FT1==144.515)tmp1=170;
        else if(FT1<=144.628)tmp1=170.1;
        else if(FT1<=144.742)tmp1=170.2;
        else if(FT1<=144.855)tmp1=170.3;
        else if(FT1<=144.969)tmp1=170.4;
        else if(FT1<=145.082)tmp1=170.5;
        else if(FT1<=145.196)tmp1=170.6;
        else if(FT1<=145.309)tmp1=170.7;
        else if(FT1<=145.423)tmp1=170.8;
        else if(FT1<=145.537)tmp1=170.9;
        else if(FT1<=145.651)tmp1=171;
    }
    else if(FT1<=146.676){
        if(FT1==145.651)tmp1=171;
        else if(FT1<=145.764)tmp1=171.1;
        else if(FT1<=145.878)tmp1=171.2;
        else if(FT1<=145.992)tmp1=171.3;
        else if(FT1<=146.106)tmp1=171.4;
        else if(FT1<=146.22)tmp1=171.5;
        else if(FT1<=146.334)tmp1=171.6;
        else if(FT1<=146.448)tmp1=171.7;
        else if(FT1<=146.562)tmp1=171.8;
        else if(FT1<=146.676)tmp1=171.9;
        else if(FT1<=146.791)tmp1=172;
    }
    else if(FT1<=147.82){
        if(FT1==146.791)tmp1=172;
        else if(FT1<=146.905)tmp1=172.1;
        else if(FT1<=147.019)tmp1=172.2;
        else if(FT1<=147.133)tmp1=172.3;
        else if(FT1<=147.248)tmp1=172.4;
        else if(FT1<=147.362)tmp1=172.5;
        else if(FT1<=147.477)tmp1=172.6;
        else if(FT1<=147.591)tmp1=172.7;
        else if(FT1<=147.706)tmp1=172.8;
        else if(FT1<=147.82)tmp1=172.9;
        else if(FT1<=147.935)tmp1=173;
    }
    else if(FT1<=148.968){
        if(FT1==147.935)tmp1=173;
        else if(FT1<=148.05)tmp1=173.1;
        else if(FT1<=148.164)tmp1=173.2;
        else if(FT1<=148.279)tmp1=173.3;
        else if(FT1<=148.394)tmp1=173.4;
        else if(FT1<=148.509)tmp1=173.5;
        else if(FT1<=148.623)tmp1=173.6;
        else if(FT1<=148.738)tmp1=173.7;
        else if(FT1<=148.853)tmp1=173.8;
        else if(FT1<=148.968)tmp1=173.9;
        else if(FT1<=149.083)tmp1=174;
    }
    else if(FT1<=150.12){
        if(FT1==149.083)tmp1=174;
        else if(FT1<=149.198)tmp1=174.1;
        else if(FT1<=149.313)tmp1=174.2;
        else if(FT1<=149.429)tmp1=174.3;
        else if(FT1<=149.544)tmp1=174.4;
        else if(FT1<=149.659)tmp1=174.5;
        else if(FT1<=149.774)tmp1=174.6;
        else if(FT1<=149.89)tmp1=174.7;
        else if(FT1<=150.005)tmp1=174.8;
        else if(FT1<=150.12)tmp1=174.9;
        else if(FT1<=150.236)tmp1=175;
    }
    else if(FT1<=151.277){
        if(FT1==150.236)tmp1=175;
        else if(FT1<=150.351)tmp1=175.1;
        else if(FT1<=150.467)tmp1=175.2;
        else if(FT1<=150.582)tmp1=175.3;
        else if(FT1<=150.698)tmp1=175.4;
        else if(FT1<=150.814)tmp1=175.5;
        else if(FT1<=150.929)tmp1=175.6;
        else if(FT1<=151.045)tmp1=175.7;
        else if(FT1<=151.161)tmp1=175.8;
        else if(FT1<=151.277)tmp1=175.9;
        else if(FT1<=151.392)tmp1=176;
    }
    else if(FT1<=152.437){
        if(FT1==151.392)tmp1=176;
        else if(FT1<=151.508)tmp1=176.1;
        else if(FT1<=151.624)tmp1=176.2;
        else if(FT1<=151.74)tmp1=176.3;
        else if(FT1<=151.856)tmp1=176.4;
        else if(FT1<=151.972)tmp1=176.5;
        else if(FT1<=152.088)tmp1=176.6;
        else if(FT1<=152.205)tmp1=176.7;
        else if(FT1<=152.321)tmp1=176.8;
        else if(FT1<=152.437)tmp1=176.9;
        else if(FT1<=152.553)tmp1=177;
    }
    else if(FT1<=153.601){
        if(FT1==152.553)tmp1=177;
        else if(FT1<=152.669)tmp1=177.1;
        else if(FT1<=152.786)tmp1=177.2;
        else if(FT1<=152.902)tmp1=177.3;
        else if(FT1<=153.019)tmp1=177.4;
        else if(FT1<=153.135)tmp1=177.5;
        else if(FT1<=153.252)tmp1=177.6;
        else if(FT1<=153.368)tmp1=177.7;
        else if(FT1<=153.485)tmp1=177.8;
        else if(FT1<=153.601)tmp1=177.9;
        else if(FT1<=153.718)tmp1=178;
    }
    else if(FT1<=154.77){
        if(FT1==153.718)tmp1=178;
        else if(FT1<=153.835)tmp1=178.1;
        else if(FT1<=153.952)tmp1=178.2;
        else if(FT1<=154.068)tmp1=178.3;
        else if(FT1<=154.185)tmp1=178.4;
        else if(FT1<=154.302)tmp1=178.5;
        else if(FT1<=154.419)tmp1=178.6;
        else if(FT1<=154.536)tmp1=178.7;
        else if(FT1<=154.653)tmp1=178.8;
        else if(FT1<=154.77)tmp1=178.9;
        else if(FT1<=154.887)tmp1=179;
    }
    else if(FT1<=155.943){
        if(FT1==154.887)tmp1=179;
        else if(FT1<=155.004)tmp1=179.1;
        else if(FT1<=155.121)tmp1=179.2;
        else if(FT1<=155.239)tmp1=179.3;
        else if(FT1<=155.356)tmp1=179.4;
        else if(FT1<=155.473)tmp1=179.5;
        else if(FT1<=155.59)tmp1=179.6;
        else if(FT1<=155.708)tmp1=179.7;
        else if(FT1<=155.825)tmp1=179.8;
        else if(FT1<=155.943)tmp1=179.9;
        else if(FT1<=156.06)tmp1=180;
    }
    else if(FT1<=157.119){
        if(FT1==156.06)tmp1=180;
        else if(FT1<=156.178)tmp1=180.1;
        else if(FT1<=156.295)tmp1=180.2;
        else if(FT1<=156.413)tmp1=180.3;
        else if(FT1<=156.53)tmp1=180.4;
        else if(FT1<=156.648)tmp1=180.5;
        else if(FT1<=156.766)tmp1=180.6;
        else if(FT1<=156.884)tmp1=180.7;
        else if(FT1<=157.001)tmp1=180.8;
        else if(FT1<=157.119)tmp1=180.9;
        else if(FT1<=157.237)tmp1=181;
    }
    else if(FT1<=158.3){
        if(FT1==157.237)tmp1=181;
        else if(FT1<=157.355)tmp1=181.1;
        else if(FT1<=157.473)tmp1=181.2;
        else if(FT1<=157.591)tmp1=181.3;
        else if(FT1<=157.709)tmp1=181.4;
        else if(FT1<=157.827)tmp1=181.5;
        else if(FT1<=157.945)tmp1=181.6;
        else if(FT1<=158.064)tmp1=181.7;
        else if(FT1<=158.182)tmp1=181.8;
        else if(FT1<=158.3)tmp1=181.9;
        else if(FT1<=158.418)tmp1=182;
    }
    else if(FT1<=159.485){
        if(FT1==158.418)tmp1=182;
        else if(FT1<=158.537)tmp1=182.1;
        else if(FT1<=158.655)tmp1=182.2;
        else if(FT1<=158.774)tmp1=182.3;
        else if(FT1<=158.892)tmp1=182.4;
        else if(FT1<=159.011)tmp1=182.5;
        else if(FT1<=159.129)tmp1=182.6;
        else if(FT1<=159.248)tmp1=182.7;
        else if(FT1<=159.366)tmp1=182.8;
        else if(FT1<=159.485)tmp1=182.9;
        else if(FT1<=159.604)tmp1=183;
    }
    else if(FT1<=160.674){
        if(FT1==159.604)tmp1=183;
        else if(FT1<=159.722)tmp1=183.1;
        else if(FT1<=159.841)tmp1=183.2;
        else if(FT1<=159.96)tmp1=183.3;
        else if(FT1<=160.079)tmp1=183.4;
        else if(FT1<=160.198)tmp1=183.5;
        else if(FT1<=160.317)tmp1=183.6;
        else if(FT1<=160.436)tmp1=183.7;
        else if(FT1<=160.555)tmp1=183.8;
        else if(FT1<=160.674)tmp1=183.9;
        else if(FT1<=160.793)tmp1=184;
    }
    else if(FT1<=161.867){
        if(FT1==160.793)tmp1=184;
        else if(FT1<=160.912)tmp1=184.1;
        else if(FT1<=161.031)tmp1=184.2;
        else if(FT1<=161.15)tmp1=184.3;
        else if(FT1<=161.27)tmp1=184.4;
        else if(FT1<=161.389)tmp1=184.5;
        else if(FT1<=161.508)tmp1=184.6;
        else if(FT1<=161.628)tmp1=184.7;
        else if(FT1<=161.747)tmp1=184.8;
        else if(FT1<=161.867)tmp1=184.9;
        else if(FT1<=161.986)tmp1=185;
    }
    else if(FT1<=163.064){
        if(FT1==161.986)tmp1=185;
        else if(FT1<=162.106)tmp1=185.1;
        else if(FT1<=162.225)tmp1=185.2;
        else if(FT1<=162.345)tmp1=185.3;
        else if(FT1<=162.465)tmp1=185.4;
        else if(FT1<=162.584)tmp1=185.5;
        else if(FT1<=162.704)tmp1=185.6;
        else if(FT1<=162.824)tmp1=185.7;
        else if(FT1<=162.944)tmp1=185.8;
        else if(FT1<=163.064)tmp1=185.9;
        else if(FT1<=163.184)tmp1=186;
    }
    else if(FT1<=164.265){
        if(FT1==163.184)tmp1=186;
        else if(FT1<=163.304)tmp1=186.1;
        else if(FT1<=163.424)tmp1=186.2;
        else if(FT1<=163.544)tmp1=186.3;
        else if(FT1<=163.664)tmp1=186.4;
        else if(FT1<=163.784)tmp1=186.5;
        else if(FT1<=163.904)tmp1=186.6;
        else if(FT1<=164.024)tmp1=186.7;
        else if(FT1<=164.144)tmp1=186.8;
        else if(FT1<=164.265)tmp1=186.9;
        else if(FT1<=164.385)tmp1=187;
    }
    else if(FT1<=165.47){
        if(FT1==164.385)tmp1=187;
        else if(FT1<=164.505)tmp1=187.1;
        else if(FT1<=164.626)tmp1=187.2;
        else if(FT1<=164.746)tmp1=187.3;
        else if(FT1<=164.867)tmp1=187.4;
        else if(FT1<=164.987)tmp1=187.5;
        else if(FT1<=165.108)tmp1=187.6;
        else if(FT1<=165.228)tmp1=187.7;
        else if(FT1<=165.349)tmp1=187.8;
        else if(FT1<=165.47)tmp1=187.9;
        else if(FT1<=165.59)tmp1=188;
    }
    else if(FT1<=166.679){
        if(FT1==165.59)tmp1=188;
        else if(FT1<=165.711)tmp1=188.1;
        else if(FT1<=165.832)tmp1=188.2;
        else if(FT1<=165.953)tmp1=188.3;
        else if(FT1<=166.074)tmp1=188.4;
        else if(FT1<=166.195)tmp1=188.5;
        else if(FT1<=166.315)tmp1=188.6;
        else if(FT1<=166.436)tmp1=188.7;
        else if(FT1<=166.558)tmp1=188.8;
        else if(FT1<=166.679)tmp1=188.9;
        else if(FT1<=166.8)tmp1=189;
    }
    else if(FT1<=167.892){
        if(FT1==166.8)tmp1=189;
        else if(FT1<=166.921)tmp1=189.1;
        else if(FT1<=167.042)tmp1=189.2;
        else if(FT1<=167.163)tmp1=189.3;
        else if(FT1<=167.285)tmp1=189.4;
        else if(FT1<=167.406)tmp1=189.5;
        else if(FT1<=167.527)tmp1=189.6;
        else if(FT1<=167.649)tmp1=189.7;
        else if(FT1<=167.77)tmp1=189.8;
        else if(FT1<=167.892)tmp1=189.9;
        else if(FT1<=168.013)tmp1=190;
    }
    else if(FT1<=169.109){
        if(FT1==168.013)tmp1=190;
        else if(FT1<=168.135)tmp1=190.1;
        else if(FT1<=168.256)tmp1=190.2;
        else if(FT1<=168.378)tmp1=190.3;
        else if(FT1<=168.5)tmp1=190.4;
        else if(FT1<=168.621)tmp1=190.5;
        else if(FT1<=168.743)tmp1=190.6;
        else if(FT1<=168.865)tmp1=190.7;
        else if(FT1<=168.987)tmp1=190.8;
        else if(FT1<=169.109)tmp1=190.9;
        else if(FT1<=169.23)tmp1=191;
    }
    else if(FT1<=170.329){
        if(FT1==169.23)tmp1=191;
        else if(FT1<=169.352)tmp1=191.1;
        else if(FT1<=169.474)tmp1=191.2;
        else if(FT1<=169.596)tmp1=191.3;
        else if(FT1<=169.718)tmp1=191.4;
        else if(FT1<=169.841)tmp1=191.5;
        else if(FT1<=169.963)tmp1=191.6;
        else if(FT1<=170.085)tmp1=191.7;
        else if(FT1<=170.207)tmp1=191.8;
        else if(FT1<=170.329)tmp1=191.9;
        else if(FT1<=170.452)tmp1=192;
    }
    else if(FT1<=171.554){
        if(FT1==170.452)tmp1=192;
        else if(FT1<=170.574)tmp1=192.1;
        else if(FT1<=170.696)tmp1=192.2;
        else if(FT1<=170.819)tmp1=192.3;
        else if(FT1<=170.941)tmp1=192.4;
        else if(FT1<=171.064)tmp1=192.5;
        else if(FT1<=171.186)tmp1=192.6;
        else if(FT1<=171.309)tmp1=192.7;
        else if(FT1<=171.432)tmp1=192.8;
        else if(FT1<=171.554)tmp1=192.9;
        else if(FT1<=171.677)tmp1=193;
    }
    else if(FT1<=172.783){
        if(FT1==171.677)tmp1=193;
        else if(FT1<=171.8)tmp1=193.1;
        else if(FT1<=171.923)tmp1=193.2;
        else if(FT1<=172.045)tmp1=193.3;
        else if(FT1<=172.168)tmp1=193.4;
        else if(FT1<=172.291)tmp1=193.5;
        else if(FT1<=172.414)tmp1=193.6;
        else if(FT1<=172.537)tmp1=193.7;
        else if(FT1<=172.66)tmp1=193.8;
        else if(FT1<=172.783)tmp1=193.9;
        else if(FT1<=172.906)tmp1=194;
    }
    else if(FT1<=174.016){
        if(FT1==172.906)tmp1=194;
        else if(FT1<=173.029)tmp1=194.1;
        else if(FT1<=173.153)tmp1=194.2;
        else if(FT1<=173.276)tmp1=194.3;
        else if(FT1<=173.399)tmp1=194.4;
        else if(FT1<=173.522)tmp1=194.5;
        else if(FT1<=173.646)tmp1=194.6;
        else if(FT1<=173.769)tmp1=194.7;
        else if(FT1<=173.892)tmp1=194.8;
        else if(FT1<=174.016)tmp1=194.9;
        else if(FT1<=174.139)tmp1=195;
    }
    else if(FT1<=175.253){
        if(FT1==174.139)tmp1=195;
        else if(FT1<=174.263)tmp1=195.1;
        else if(FT1<=174.386)tmp1=195.2;
        else if(FT1<=174.51)tmp1=195.3;
        else if(FT1<=174.634)tmp1=195.4;
        else if(FT1<=174.757)tmp1=195.5;
        else if(FT1<=174.881)tmp1=195.6;
        else if(FT1<=175.005)tmp1=195.7;
        else if(FT1<=175.129)tmp1=195.8;
        else if(FT1<=175.253)tmp1=195.9;
        else if(FT1<=175.376)tmp1=196;
    }
    else if(FT1<=176.493){
        if(FT1==175.376)tmp1=196;
        else if(FT1<=175.5)tmp1=196.1;
        else if(FT1<=175.624)tmp1=196.2;
        else if(FT1<=175.748)tmp1=196.3;
        else if(FT1<=175.872)tmp1=196.4;
        else if(FT1<=175.996)tmp1=196.5;
        else if(FT1<=176.121)tmp1=196.6;
        else if(FT1<=176.245)tmp1=196.7;
        else if(FT1<=176.369)tmp1=196.8;
        else if(FT1<=176.493)tmp1=196.9;
        else if(FT1<=176.617)tmp1=197;
    }
    else if(FT1<=177.738){
        if(FT1==176.617)tmp1=197;
        else if(FT1<=176.742)tmp1=197.1;
        else if(FT1<=176.866)tmp1=197.2;
        else if(FT1<=176.99)tmp1=197.3;
        else if(FT1<=177.115)tmp1=197.4;
        else if(FT1<=177.239)tmp1=197.5;
        else if(FT1<=177.364)tmp1=197.6;
        else if(FT1<=177.488)tmp1=197.7;
        else if(FT1<=177.613)tmp1=197.8;
        else if(FT1<=177.738)tmp1=197.9;
        else if(FT1<=177.862)tmp1=198;
    }
    else if(FT1<=178.986){
        if(FT1==177.862)tmp1=198;
        else if(FT1<=177.987)tmp1=198.1;
        else if(FT1<=178.112)tmp1=198.2;
        else if(FT1<=178.237)tmp1=198.3;
        else if(FT1<=178.361)tmp1=198.4;
        else if(FT1<=178.486)tmp1=198.5;
        else if(FT1<=178.611)tmp1=198.6;
        else if(FT1<=178.736)tmp1=198.7;
        else if(FT1<=178.861)tmp1=198.8;
        else if(FT1<=178.986)tmp1=198.9;
        else if(FT1<=179.111)tmp1=199;
    }
    else if(FT1<=180.238){
        if(FT1==179.111)tmp1=199;
        else if(FT1<=179.236)tmp1=199.1;
        else if(FT1<=179.361)tmp1=199.2;
        else if(FT1<=179.486)tmp1=199.3;
        else if(FT1<=179.612)tmp1=199.4;
        else if(FT1<=179.737)tmp1=199.5;
        else if(FT1<=179.862)tmp1=199.6;
        else if(FT1<=179.988)tmp1=199.7;
        else if(FT1<=180.113)tmp1=199.8;
        else if(FT1<=180.238)tmp1=199.9;
        else if(FT1<=180.364)tmp1=200;
    }
    return tmp1;
}

double ShowImage::gettemperature_epsillon1(double FT1){

    double tmp1=0;

    if(FT1<=0)tmp1=-205;
    else if(FT1<=0.03868){
        if(FT1==0.03572)tmp1=-150;
        else if(FT1<=0.03604)tmp1=-149.9;
        else if(FT1<=0.03636)tmp1=-149.8;
        else if(FT1<=0.03668)tmp1=-149.7;
        else if(FT1<=0.03701)tmp1=-149.6;
        else if(FT1<=0.03734)tmp1=-149.5;
        else if(FT1<=0.03767)tmp1=-149.4;
        else if(FT1<=0.038)tmp1=-149.3;
        else if(FT1<=0.03834)tmp1=-149.2;
        else if(FT1<=0.03868)tmp1=-149.1;
        else if(FT1<=0.03902)tmp1=-149;
    }
    else if(FT1<=0.04221){
        if(FT1==0.03902)tmp1=-149;
        else if(FT1<=0.03937)tmp1=-148.9;
        else if(FT1<=0.03971)tmp1=-148.8;
        else if(FT1<=0.04006)tmp1=-148.7;
        else if(FT1<=0.04041)tmp1=-148.6;
        else if(FT1<=0.04077)tmp1=-148.5;
        else if(FT1<=0.04112)tmp1=-148.4;
        else if(FT1<=0.04148)tmp1=-148.3;
        else if(FT1<=0.04184)tmp1=-148.2;
        else if(FT1<=0.04221)tmp1=-148.1;
        else if(FT1<=0.04257)tmp1=-148;
    }
    else if(FT1<=0.046){
        if(FT1==0.04257)tmp1=-148;
        else if(FT1<=0.04294)tmp1=-147.9;
        else if(FT1<=0.04332)tmp1=-147.8;
        else if(FT1<=0.04369)tmp1=-147.7;
        else if(FT1<=0.04407)tmp1=-147.6;
        else if(FT1<=0.04445)tmp1=-147.5;
        else if(FT1<=0.04483)tmp1=-147.4;
        else if(FT1<=0.04522)tmp1=-147.3;
        else if(FT1<=0.0456)tmp1=-147.2;
        else if(FT1<=0.046)tmp1=-147.1;
        else if(FT1<=0.04639)tmp1=-147;
    }
    else if(FT1<=0.05006){
        if(FT1==0.04639)tmp1=-147;
        else if(FT1<=0.04679)tmp1=-146.9;
        else if(FT1<=0.04718)tmp1=-146.8;
        else if(FT1<=0.04759)tmp1=-146.7;
        else if(FT1<=0.04799)tmp1=-146.6;
        else if(FT1<=0.0484)tmp1=-146.5;
        else if(FT1<=0.04881)tmp1=-146.4;
        else if(FT1<=0.04922)tmp1=-146.3;
        else if(FT1<=0.04964)tmp1=-146.2;
        else if(FT1<=0.05006)tmp1=-146.1;
        else if(FT1<=0.05048)tmp1=-146;
    }
    else if(FT1<=0.05441){
        if(FT1==0.05048)tmp1=-146;
        else if(FT1<=0.05091)tmp1=-145.9;
        else if(FT1<=0.05133)tmp1=-145.8;
        else if(FT1<=0.05177)tmp1=-145.7;
        else if(FT1<=0.0522)tmp1=-145.6;
        else if(FT1<=0.05264)tmp1=-145.5;
        else if(FT1<=0.05308)tmp1=-145.4;
        else if(FT1<=0.05352)tmp1=-145.3;
        else if(FT1<=0.05397)tmp1=-145.2;
        else if(FT1<=0.05441)tmp1=-145.1;
        else if(FT1<=0.05487)tmp1=-145;
    }
    else if(FT1<=0.05908){
        if(FT1==0.05487)tmp1=-145;
        else if(FT1<=0.05532)tmp1=-144.9;
        else if(FT1<=0.05578)tmp1=-144.8;
        else if(FT1<=0.05624)tmp1=-144.7;
        else if(FT1<=0.05671)tmp1=-144.6;
        else if(FT1<=0.05717)tmp1=-144.5;
        else if(FT1<=0.05764)tmp1=-144.4;
        else if(FT1<=0.05812)tmp1=-144.3;
        else if(FT1<=0.0586)tmp1=-144.2;
        else if(FT1<=0.05908)tmp1=-144.1;
        else if(FT1<=0.05956)tmp1=-144;
    }
    else if(FT1<=0.06406){
        if(FT1==0.05956)tmp1=-144;
        else if(FT1<=0.06005)tmp1=-143.9;
        else if(FT1<=0.06054)tmp1=-143.8;
        else if(FT1<=0.06103)tmp1=-143.7;
        else if(FT1<=0.06153)tmp1=-143.6;
        else if(FT1<=0.06203)tmp1=-143.5;
        else if(FT1<=0.06253)tmp1=-143.4;
        else if(FT1<=0.06304)tmp1=-143.3;
        else if(FT1<=0.06355)tmp1=-143.2;
        else if(FT1<=0.06406)tmp1=-143.1;
        else if(FT1<=0.06458)tmp1=-143;
    }
    else if(FT1<=0.06938){
        if(FT1==0.06458)tmp1=-143;
        else if(FT1<=0.0651)tmp1=-142.9;
        else if(FT1<=0.06562)tmp1=-142.8;
        else if(FT1<=0.06615)tmp1=-142.7;
        else if(FT1<=0.06668)tmp1=-142.6;
        else if(FT1<=0.06721)tmp1=-142.5;
        else if(FT1<=0.06775)tmp1=-142.4;
        else if(FT1<=0.06829)tmp1=-142.3;
        else if(FT1<=0.06883)tmp1=-142.2;
        else if(FT1<=0.06938)tmp1=-142.1;
        else if(FT1<=0.06993)tmp1=-142;
    }
    else if(FT1<=0.07506){
        if(FT1==0.06993)tmp1=-142;
        else if(FT1<=0.07049)tmp1=-141.9;
        else if(FT1<=0.07105)tmp1=-141.8;
        else if(FT1<=0.07161)tmp1=-141.7;
        else if(FT1<=0.07218)tmp1=-141.6;
        else if(FT1<=0.07275)tmp1=-141.5;
        else if(FT1<=0.07332)tmp1=-141.4;
        else if(FT1<=0.0739)tmp1=-141.3;
        else if(FT1<=0.07448)tmp1=-141.2;
        else if(FT1<=0.07506)tmp1=-141.1;
        else if(FT1<=0.07565)tmp1=-141;
    }
    else if(FT1<=0.08111){
        if(FT1==0.07565)tmp1=-141;
        else if(FT1<=0.07624)tmp1=-140.9;
        else if(FT1<=0.07684)tmp1=-140.8;
        else if(FT1<=0.07744)tmp1=-140.7;
        else if(FT1<=0.07804)tmp1=-140.6;
        else if(FT1<=0.07865)tmp1=-140.5;
        else if(FT1<=0.07926)tmp1=-140.4;
        else if(FT1<=0.07987)tmp1=-140.3;
        else if(FT1<=0.08049)tmp1=-140.2;
        else if(FT1<=0.08111)tmp1=-140.1;
        else if(FT1<=0.08174)tmp1=-140;
    }
    else if(FT1<=0.08756){
        if(FT1==0.08174)tmp1=-140;
        else if(FT1<=0.08237)tmp1=-139.9;
        else if(FT1<=0.08301)tmp1=-139.8;
        else if(FT1<=0.08364)tmp1=-139.7;
        else if(FT1<=0.08429)tmp1=-139.6;
        else if(FT1<=0.08493)tmp1=-139.5;
        else if(FT1<=0.08558)tmp1=-139.4;
        else if(FT1<=0.08624)tmp1=-139.3;
        else if(FT1<=0.0869)tmp1=-139.2;
        else if(FT1<=0.08756)tmp1=-139.1;
        else if(FT1<=0.08823)tmp1=-139;
    }
    else if(FT1<=0.09442){
        if(FT1==0.08823)tmp1=-139;
        else if(FT1<=0.0889)tmp1=-138.9;
        else if(FT1<=0.08957)tmp1=-138.8;
        else if(FT1<=0.09025)tmp1=-138.7;
        else if(FT1<=0.09093)tmp1=-138.6;
        else if(FT1<=0.09162)tmp1=-138.5;
        else if(FT1<=0.09231)tmp1=-138.4;
        else if(FT1<=0.09301)tmp1=-138.3;
        else if(FT1<=0.09371)tmp1=-138.2;
        else if(FT1<=0.09442)tmp1=-138.1;
        else if(FT1<=0.09512)tmp1=-138;
    }
    else if(FT1<=0.1017){
        if(FT1==0.09512)tmp1=-138;
        else if(FT1<=0.09584)tmp1=-137.9;
        else if(FT1<=0.09655)tmp1=-137.8;
        else if(FT1<=0.09728)tmp1=-137.7;
        else if(FT1<=0.098)tmp1=-137.6;
        else if(FT1<=0.09873)tmp1=-137.5;
        else if(FT1<=0.09947)tmp1=-137.4;
        else if(FT1<=0.10021)tmp1=-137.3;
        else if(FT1<=0.10095)tmp1=-137.2;
        else if(FT1<=0.1017)tmp1=-137.1;
        else if(FT1<=0.10245)tmp1=-137;
    }
    else if(FT1<=0.10944){
        if(FT1==0.10245)tmp1=-137;
        else if(FT1<=0.10321)tmp1=-136.9;
        else if(FT1<=0.10397)tmp1=-136.8;
        else if(FT1<=0.10474)tmp1=-136.7;
        else if(FT1<=0.10551)tmp1=-136.6;
        else if(FT1<=0.10629)tmp1=-136.5;
        else if(FT1<=0.10707)tmp1=-136.4;
        else if(FT1<=0.10785)tmp1=-136.3;
        else if(FT1<=0.10864)tmp1=-136.2;
        else if(FT1<=0.10944)tmp1=-136.1;
        else if(FT1<=0.11023)tmp1=-136;
    }
    else if(FT1<=0.11764){
        if(FT1==0.11023)tmp1=-136;
        else if(FT1<=0.11104)tmp1=-135.9;
        else if(FT1<=0.11185)tmp1=-135.8;
        else if(FT1<=0.11266)tmp1=-135.7;
        else if(FT1<=0.11348)tmp1=-135.6;
        else if(FT1<=0.1143)tmp1=-135.5;
        else if(FT1<=0.11513)tmp1=-135.4;
        else if(FT1<=0.11596)tmp1=-135.3;
        else if(FT1<=0.1168)tmp1=-135.2;
        else if(FT1<=0.11764)tmp1=-135.1;
        else if(FT1<=0.11849)tmp1=-135;
    }
    else if(FT1<=0.12634){
        if(FT1==0.11849)tmp1=-135;
        else if(FT1<=0.11934)tmp1=-134.9;
        else if(FT1<=0.1202)tmp1=-134.8;
        else if(FT1<=0.12106)tmp1=-134.7;
        else if(FT1<=0.12193)tmp1=-134.6;
        else if(FT1<=0.1228)tmp1=-134.5;
        else if(FT1<=0.12367)tmp1=-134.4;
        else if(FT1<=0.12456)tmp1=-134.3;
        else if(FT1<=0.12544)tmp1=-134.2;
        else if(FT1<=0.12634)tmp1=-134.1;
        else if(FT1<=0.12723)tmp1=-134;
    }
    else if(FT1<=0.13554){
        if(FT1==0.12723)tmp1=-134;
        else if(FT1<=0.12814)tmp1=-133.9;
        else if(FT1<=0.12904)tmp1=-133.8;
        else if(FT1<=0.12996)tmp1=-133.7;
        else if(FT1<=0.13087)tmp1=-133.6;
        else if(FT1<=0.1318)tmp1=-133.5;
        else if(FT1<=0.13273)tmp1=-133.4;
        else if(FT1<=0.13366)tmp1=-133.3;
        else if(FT1<=0.1346)tmp1=-133.2;
        else if(FT1<=0.13554)tmp1=-133.1;
        else if(FT1<=0.13649)tmp1=-133;
    }
    else if(FT1<=0.14528){
        if(FT1==0.13649)tmp1=-133;
        else if(FT1<=0.13745)tmp1=-132.9;
        else if(FT1<=0.13841)tmp1=-132.8;
        else if(FT1<=0.13937)tmp1=-132.7;
        else if(FT1<=0.14035)tmp1=-132.6;
        else if(FT1<=0.14132)tmp1=-132.5;
        else if(FT1<=0.1423)tmp1=-132.4;
        else if(FT1<=0.14329)tmp1=-132.3;
        else if(FT1<=0.14428)tmp1=-132.2;
        else if(FT1<=0.14528)tmp1=-132.1;
        else if(FT1<=0.14629)tmp1=-132;
    }
    else if(FT1<=0.15558){
        if(FT1==0.14629)tmp1=-132;
        else if(FT1<=0.1473)tmp1=-131.9;
        else if(FT1<=0.14831)tmp1=-131.8;
        else if(FT1<=0.14933)tmp1=-131.7;
        else if(FT1<=0.15036)tmp1=-131.6;
        else if(FT1<=0.15139)tmp1=-131.5;
        else if(FT1<=0.15243)tmp1=-131.4;
        else if(FT1<=0.15348)tmp1=-131.3;
        else if(FT1<=0.15452)tmp1=-131.2;
        else if(FT1<=0.15558)tmp1=-131.1;
        else if(FT1<=0.15664)tmp1=-131;
    }
    else if(FT1<=0.16645){
        if(FT1==0.15664)tmp1=-131;
        else if(FT1<=0.15771)tmp1=-130.9;
        else if(FT1<=0.15878)tmp1=-130.8;
        else if(FT1<=0.15986)tmp1=-130.7;
        else if(FT1<=0.16094)tmp1=-130.6;
        else if(FT1<=0.16203)tmp1=-130.5;
        else if(FT1<=0.16313)tmp1=-130.4;
        else if(FT1<=0.16423)tmp1=-130.3;
        else if(FT1<=0.16534)tmp1=-130.2;
        else if(FT1<=0.16645)tmp1=-130.1;
        else if(FT1<=0.16757)tmp1=-130;
    }
    else if(FT1<=0.17793){
        if(FT1==0.16757)tmp1=-130;
        else if(FT1<=0.1687)tmp1=-129.9;
        else if(FT1<=0.16983)tmp1=-129.8;
        else if(FT1<=0.17097)tmp1=-129.7;
        else if(FT1<=0.17212)tmp1=-129.6;
        else if(FT1<=0.17327)tmp1=-129.5;
        else if(FT1<=0.17442)tmp1=-129.4;
        else if(FT1<=0.17559)tmp1=-129.3;
        else if(FT1<=0.17676)tmp1=-129.2;
        else if(FT1<=0.17793)tmp1=-129.1;
        else if(FT1<=0.17911)tmp1=-129;
    }
    else if(FT1<=0.19003){
        if(FT1==0.17911)tmp1=-129;
        else if(FT1<=0.1803)tmp1=-128.9;
        else if(FT1<=0.18149)tmp1=-128.8;
        else if(FT1<=0.1827)tmp1=-128.7;
        else if(FT1<=0.1839)tmp1=-128.6;
        else if(FT1<=0.18512)tmp1=-128.5;
        else if(FT1<=0.18633)tmp1=-128.4;
        else if(FT1<=0.18756)tmp1=-128.3;
        else if(FT1<=0.18879)tmp1=-128.2;
        else if(FT1<=0.19003)tmp1=-128.1;
        else if(FT1<=0.19128)tmp1=-128;
    }
    else if(FT1<=0.20278){
        if(FT1==0.19128)tmp1=-128;
        else if(FT1<=0.19253)tmp1=-127.9;
        else if(FT1<=0.19379)tmp1=-127.8;
        else if(FT1<=0.19505)tmp1=-127.7;
        else if(FT1<=0.19633)tmp1=-127.6;
        else if(FT1<=0.1976)tmp1=-127.5;
        else if(FT1<=0.19889)tmp1=-127.4;
        else if(FT1<=0.20018)tmp1=-127.3;
        else if(FT1<=0.20148)tmp1=-127.2;
        else if(FT1<=0.20278)tmp1=-127.1;
        else if(FT1<=0.2041)tmp1=-127;
    }
    else if(FT1<=0.21621){
        if(FT1==0.2041)tmp1=-127;
        else if(FT1<=0.20541)tmp1=-126.9;
        else if(FT1<=0.20674)tmp1=-126.8;
        else if(FT1<=0.20807)tmp1=-126.7;
        else if(FT1<=0.20941)tmp1=-126.6;
        else if(FT1<=0.21076)tmp1=-126.5;
        else if(FT1<=0.21211)tmp1=-126.4;
        else if(FT1<=0.21347)tmp1=-126.3;
        else if(FT1<=0.21484)tmp1=-126.2;
        else if(FT1<=0.21621)tmp1=-126.1;
        else if(FT1<=0.21759)tmp1=-126;
    }
    else if(FT1<=0.23033){
        if(FT1==0.21759)tmp1=-126;
        else if(FT1<=0.21898)tmp1=-125.9;
        else if(FT1<=0.22037)tmp1=-125.8;
        else if(FT1<=0.22177)tmp1=-125.7;
        else if(FT1<=0.22318)tmp1=-125.6;
        else if(FT1<=0.2246)tmp1=-125.5;
        else if(FT1<=0.22602)tmp1=-125.4;
        else if(FT1<=0.22745)tmp1=-125.3;
        else if(FT1<=0.22889)tmp1=-125.2;
        else if(FT1<=0.23033)tmp1=-125.1;
        else if(FT1<=0.23179)tmp1=-125;
    }
    else if(FT1<=0.24518){
        if(FT1==0.23179)tmp1=-125;
        else if(FT1<=0.23325)tmp1=-124.9;
        else if(FT1<=0.23471)tmp1=-124.8;
        else if(FT1<=0.23619)tmp1=-124.7;
        else if(FT1<=0.23767)tmp1=-124.6;
        else if(FT1<=0.23916)tmp1=-124.5;
        else if(FT1<=0.24065)tmp1=-124.4;
        else if(FT1<=0.24215)tmp1=-124.3;
        else if(FT1<=0.24367)tmp1=-124.2;
        else if(FT1<=0.24518)tmp1=-124.1;
        else if(FT1<=0.24671)tmp1=-124;
    }
    else if(FT1<=0.26078){
        if(FT1==0.24671)tmp1=-124;
        else if(FT1<=0.24824)tmp1=-123.9;
        else if(FT1<=0.24978)tmp1=-123.8;
        else if(FT1<=0.25133)tmp1=-123.7;
        else if(FT1<=0.25289)tmp1=-123.6;
        else if(FT1<=0.25445)tmp1=-123.5;
        else if(FT1<=0.25602)tmp1=-123.4;
        else if(FT1<=0.2576)tmp1=-123.3;
        else if(FT1<=0.25919)tmp1=-123.2;
        else if(FT1<=0.26078)tmp1=-123.1;
        else if(FT1<=0.26239)tmp1=-123;
    }
    else if(FT1<=0.27716){
        if(FT1==0.26239)tmp1=-123;
        else if(FT1<=0.264)tmp1=-122.9;
        else if(FT1<=0.26562)tmp1=-122.8;
        else if(FT1<=0.26724)tmp1=-122.7;
        else if(FT1<=0.26887)tmp1=-122.6;
        else if(FT1<=0.27052)tmp1=-122.5;
        else if(FT1<=0.27217)tmp1=-122.4;
        else if(FT1<=0.27382)tmp1=-122.3;
        else if(FT1<=0.27549)tmp1=-122.2;
        else if(FT1<=0.27716)tmp1=-122.1;
        else if(FT1<=0.27884)tmp1=-122;
    }
    else if(FT1<=0.29435){
        if(FT1==0.27884)tmp1=-122;
        else if(FT1<=0.28053)tmp1=-121.9;
        else if(FT1<=0.28223)tmp1=-121.8;
        else if(FT1<=0.28394)tmp1=-121.7;
        else if(FT1<=0.28565)tmp1=-121.6;
        else if(FT1<=0.28737)tmp1=-121.5;
        else if(FT1<=0.28911)tmp1=-121.4;
        else if(FT1<=0.29084)tmp1=-121.3;
        else if(FT1<=0.29259)tmp1=-121.2;
        else if(FT1<=0.29435)tmp1=-121.1;
        else if(FT1<=0.29611)tmp1=-121;
    }
    else if(FT1<=0.31236){
        if(FT1==0.29611)tmp1=-121;
        else if(FT1<=0.29788)tmp1=-120.9;
        else if(FT1<=0.29966)tmp1=-120.8;
        else if(FT1<=0.30145)tmp1=-120.7;
        else if(FT1<=0.30325)tmp1=-120.6;
        else if(FT1<=0.30505)tmp1=-120.5;
        else if(FT1<=0.30687)tmp1=-120.4;
        else if(FT1<=0.30869)tmp1=-120.3;
        else if(FT1<=0.31052)tmp1=-120.2;
        else if(FT1<=0.31236)tmp1=-120.1;
        else if(FT1<=0.31421)tmp1=-120;
    }
    else if(FT1<=0.33124){
        if(FT1==0.31421)tmp1=-120;
        else if(FT1<=0.31607)tmp1=-119.9;
        else if(FT1<=0.31793)tmp1=-119.8;
        else if(FT1<=0.31981)tmp1=-119.7;
        else if(FT1<=0.32169)tmp1=-119.6;
        else if(FT1<=0.32358)tmp1=-119.5;
        else if(FT1<=0.32548)tmp1=-119.4;
        else if(FT1<=0.32739)tmp1=-119.3;
        else if(FT1<=0.32931)tmp1=-119.2;
        else if(FT1<=0.33124)tmp1=-119.1;
        else if(FT1<=0.33317)tmp1=-119;
    }
    else if(FT1<=0.351){
        if(FT1==0.33317)tmp1=-119;
        else if(FT1<=0.33512)tmp1=-118.9;
        else if(FT1<=0.33707)tmp1=-118.8;
        else if(FT1<=0.33903)tmp1=-118.7;
        else if(FT1<=0.341)tmp1=-118.6;
        else if(FT1<=0.34298)tmp1=-118.5;
        else if(FT1<=0.34497)tmp1=-118.4;
        else if(FT1<=0.34697)tmp1=-118.3;
        else if(FT1<=0.34898)tmp1=-118.2;
        else if(FT1<=0.351)tmp1=-118.1;
        else if(FT1<=0.35302)tmp1=-118;
    }
    else if(FT1<=0.37168){
        if(FT1==0.35302)tmp1=-118;
        else if(FT1<=0.35506)tmp1=-117.9;
        else if(FT1<=0.3571)tmp1=-117.8;
        else if(FT1<=0.35916)tmp1=-117.7;
        else if(FT1<=0.36122)tmp1=-117.6;
        else if(FT1<=0.36329)tmp1=-117.5;
        else if(FT1<=0.36538)tmp1=-117.4;
        else if(FT1<=0.36747)tmp1=-117.3;
        else if(FT1<=0.36957)tmp1=-117.2;
        else if(FT1<=0.37168)tmp1=-117.1;
        else if(FT1<=0.3738)tmp1=-117;
    }
    else if(FT1<=0.3933){
        if(FT1==0.3738)tmp1=-117;
        else if(FT1<=0.37593)tmp1=-116.9;
        else if(FT1<=0.37806)tmp1=-116.8;
        else if(FT1<=0.38021)tmp1=-116.7;
        else if(FT1<=0.38237)tmp1=-116.6;
        else if(FT1<=0.38454)tmp1=-116.5;
        else if(FT1<=0.38671)tmp1=-116.4;
        else if(FT1<=0.3889)tmp1=-116.3;
        else if(FT1<=0.3911)tmp1=-116.2;
        else if(FT1<=0.3933)tmp1=-116.1;
        else if(FT1<=0.39552)tmp1=-116;
    }
    else if(FT1<=0.4159){
        if(FT1==0.39552)tmp1=-116;
        else if(FT1<=0.39774)tmp1=-115.9;
        else if(FT1<=0.39998)tmp1=-115.8;
        else if(FT1<=0.40222)tmp1=-115.7;
        else if(FT1<=0.40448)tmp1=-115.6;
        else if(FT1<=0.40674)tmp1=-115.5;
        else if(FT1<=0.40902)tmp1=-115.4;
        else if(FT1<=0.4113)tmp1=-115.3;
        else if(FT1<=0.4136)tmp1=-115.2;
        else if(FT1<=0.4159)tmp1=-115.1;
        else if(FT1<=0.41822)tmp1=-115;
    }
    else if(FT1<=0.43951){
        if(FT1==0.41822)tmp1=-115;
        else if(FT1<=0.42054)tmp1=-114.9;
        else if(FT1<=0.42288)tmp1=-114.8;
        else if(FT1<=0.42522)tmp1=-114.7;
        else if(FT1<=0.42758)tmp1=-114.6;
        else if(FT1<=0.42994)tmp1=-114.5;
        else if(FT1<=0.43232)tmp1=-114.4;
        else if(FT1<=0.43471)tmp1=-114.3;
        else if(FT1<=0.4371)tmp1=-114.2;
        else if(FT1<=0.43951)tmp1=-114.1;
        else if(FT1<=0.44193)tmp1=-114;
    }
    else if(FT1<=0.46415){
        if(FT1==0.44193)tmp1=-114;
        else if(FT1<=0.44435)tmp1=-113.9;
        else if(FT1<=0.44679)tmp1=-113.8;
        else if(FT1<=0.44924)tmp1=-113.7;
        else if(FT1<=0.4517)tmp1=-113.6;
        else if(FT1<=0.45417)tmp1=-113.5;
        else if(FT1<=0.45665)tmp1=-113.4;
        else if(FT1<=0.45914)tmp1=-113.3;
        else if(FT1<=0.46164)tmp1=-113.2;
        else if(FT1<=0.46415)tmp1=-113.1;
        else if(FT1<=0.46667)tmp1=-113;
    }
    else if(FT1<=0.48986){
        if(FT1==0.46667)tmp1=-113;
        else if(FT1<=0.4692)tmp1=-112.9;
        else if(FT1<=0.47175)tmp1=-112.8;
        else if(FT1<=0.4743)tmp1=-112.7;
        else if(FT1<=0.47687)tmp1=-112.6;
        else if(FT1<=0.47944)tmp1=-112.5;
        else if(FT1<=0.48203)tmp1=-112.4;
        else if(FT1<=0.48463)tmp1=-112.3;
        else if(FT1<=0.48724)tmp1=-112.2;
        else if(FT1<=0.48986)tmp1=-112.1;
        else if(FT1<=0.49249)tmp1=-112;
    }
    else if(FT1<=0.51666){
        if(FT1==0.49249)tmp1=-112;
        else if(FT1<=0.49513)tmp1=-111.9;
        else if(FT1<=0.49778)tmp1=-111.8;
        else if(FT1<=0.50044)tmp1=-111.7;
        else if(FT1<=0.50312)tmp1=-111.6;
        else if(FT1<=0.5058)tmp1=-111.5;
        else if(FT1<=0.5085)tmp1=-111.4;
        else if(FT1<=0.51121)tmp1=-111.3;
        else if(FT1<=0.51393)tmp1=-111.2;
        else if(FT1<=0.51666)tmp1=-111.1;
        else if(FT1<=0.5194)tmp1=-111;
    }
    else if(FT1<=0.54459){
        if(FT1==0.5194)tmp1=-111;
        else if(FT1<=0.52215)tmp1=-110.9;
        else if(FT1<=0.52492)tmp1=-110.8;
        else if(FT1<=0.52769)tmp1=-110.7;
        else if(FT1<=0.53048)tmp1=-110.6;
        else if(FT1<=0.53328)tmp1=-110.5;
        else if(FT1<=0.53609)tmp1=-110.4;
        else if(FT1<=0.53891)tmp1=-110.3;
        else if(FT1<=0.54175)tmp1=-110.2;
        else if(FT1<=0.54459)tmp1=-110.1;
        else if(FT1<=0.54745)tmp1=-110;
    }
    else if(FT1<=0.57368){
        if(FT1==0.54745)tmp1=-110;
        else if(FT1<=0.55031)tmp1=-109.9;
        else if(FT1<=0.55319)tmp1=-109.8;
        else if(FT1<=0.55609)tmp1=-109.7;
        else if(FT1<=0.55899)tmp1=-109.6;
        else if(FT1<=0.5619)tmp1=-109.5;
        else if(FT1<=0.56483)tmp1=-109.4;
        else if(FT1<=0.56777)tmp1=-109.3;
        else if(FT1<=0.57072)tmp1=-109.2;
        else if(FT1<=0.57368)tmp1=-109.1;
        else if(FT1<=0.57666)tmp1=-109;
    }
    else if(FT1<=0.60396){
        if(FT1==0.57666)tmp1=-109;
        else if(FT1<=0.57964)tmp1=-108.9;
        else if(FT1<=0.58264)tmp1=-108.8;
        else if(FT1<=0.58565)tmp1=-108.7;
        else if(FT1<=0.58867)tmp1=-108.6;
        else if(FT1<=0.59171)tmp1=-108.5;
        else if(FT1<=0.59475)tmp1=-108.4;
        else if(FT1<=0.59781)tmp1=-108.3;
        else if(FT1<=0.60088)tmp1=-108.2;
        else if(FT1<=0.60396)tmp1=-108.1;
        else if(FT1<=0.60706)tmp1=-108;
    }
    else if(FT1<=0.63547){
        if(FT1==0.60706)tmp1=-108;
        else if(FT1<=0.61017)tmp1=-107.9;
        else if(FT1<=0.61329)tmp1=-107.8;
        else if(FT1<=0.61642)tmp1=-107.7;
        else if(FT1<=0.61956)tmp1=-107.6;
        else if(FT1<=0.62272)tmp1=-107.5;
        else if(FT1<=0.62589)tmp1=-107.4;
        else if(FT1<=0.62907)tmp1=-107.3;
        else if(FT1<=0.63226)tmp1=-107.2;
        else if(FT1<=0.63547)tmp1=-107.1;
        else if(FT1<=0.63869)tmp1=-107;
    }
    else if(FT1<=0.66823){
        if(FT1==0.63869)tmp1=-107;
        else if(FT1<=0.64192)tmp1=-106.9;
        else if(FT1<=0.64516)tmp1=-106.8;
        else if(FT1<=0.64842)tmp1=-106.7;
        else if(FT1<=0.65169)tmp1=-106.6;
        else if(FT1<=0.65497)tmp1=-106.5;
        else if(FT1<=0.65827)tmp1=-106.4;
        else if(FT1<=0.66158)tmp1=-106.3;
        else if(FT1<=0.6649)tmp1=-106.2;
        else if(FT1<=0.66823)tmp1=-106.1;
        else if(FT1<=0.67158)tmp1=-106;
    }
    else if(FT1<=0.70228){
        if(FT1==0.67158)tmp1=-106;
        else if(FT1<=0.67494)tmp1=-105.9;
        else if(FT1<=0.67831)tmp1=-105.8;
        else if(FT1<=0.6817)tmp1=-105.7;
        else if(FT1<=0.68509)tmp1=-105.6;
        else if(FT1<=0.68851)tmp1=-105.5;
        else if(FT1<=0.69193)tmp1=-105.4;
        else if(FT1<=0.69537)tmp1=-105.3;
        else if(FT1<=0.69882)tmp1=-105.2;
        else if(FT1<=0.70228)tmp1=-105.1;
        else if(FT1<=0.70576)tmp1=-105;
    }
    else if(FT1<=0.73766){
        if(FT1==0.70576)tmp1=-105;
        else if(FT1<=0.70925)tmp1=-104.9;
        else if(FT1<=0.71275)tmp1=-104.8;
        else if(FT1<=0.71627)tmp1=-104.7;
        else if(FT1<=0.7198)tmp1=-104.6;
        else if(FT1<=0.72335)tmp1=-104.5;
        else if(FT1<=0.7269)tmp1=-104.4;
        else if(FT1<=0.73047)tmp1=-104.3;
        else if(FT1<=0.73406)tmp1=-104.2;
        else if(FT1<=0.73766)tmp1=-104.1;
        else if(FT1<=0.74127)tmp1=-104;
    }
    else if(FT1<=0.77439){
        if(FT1==0.74127)tmp1=-104;
        else if(FT1<=0.74489)tmp1=-103.9;
        else if(FT1<=0.74853)tmp1=-103.8;
        else if(FT1<=0.75218)tmp1=-103.7;
        else if(FT1<=0.75585)tmp1=-103.6;
        else if(FT1<=0.75953)tmp1=-103.5;
        else if(FT1<=0.76322)tmp1=-103.4;
        else if(FT1<=0.76693)tmp1=-103.3;
        else if(FT1<=0.77065)tmp1=-103.2;
        else if(FT1<=0.77439)tmp1=-103.1;
        else if(FT1<=0.77813)tmp1=-103;
    }
    else if(FT1<=0.8125){
        if(FT1==0.77813)tmp1=-103;
        else if(FT1<=0.7819)tmp1=-102.9;
        else if(FT1<=0.78567)tmp1=-102.8;
        else if(FT1<=0.78946)tmp1=-102.7;
        else if(FT1<=0.79327)tmp1=-102.6;
        else if(FT1<=0.79709)tmp1=-102.5;
        else if(FT1<=0.80092)tmp1=-102.4;
        else if(FT1<=0.80477)tmp1=-102.3;
        else if(FT1<=0.80863)tmp1=-102.2;
        else if(FT1<=0.8125)tmp1=-102.1;
        else if(FT1<=0.81639)tmp1=-102;
    }
    else if(FT1<=0.85205){
        if(FT1==0.81639)tmp1=-102;
        else if(FT1<=0.8203)tmp1=-101.9;
        else if(FT1<=0.82422)tmp1=-101.8;
        else if(FT1<=0.82815)tmp1=-101.7;
        else if(FT1<=0.8321)tmp1=-101.6;
        else if(FT1<=0.83606)tmp1=-101.5;
        else if(FT1<=0.84003)tmp1=-101.4;
        else if(FT1<=0.84402)tmp1=-101.3;
        else if(FT1<=0.84803)tmp1=-101.2;
        else if(FT1<=0.85205)tmp1=-101.1;
        else if(FT1<=0.85608)tmp1=-101;
    }
    else if(FT1<=0.89304){
        if(FT1==0.85608)tmp1=-101;
        else if(FT1<=0.86013)tmp1=-100.9;
        else if(FT1<=0.86419)tmp1=-100.8;
        else if(FT1<=0.86827)tmp1=-100.7;
        else if(FT1<=0.87236)tmp1=-100.6;
        else if(FT1<=0.87647)tmp1=-100.5;
        else if(FT1<=0.88059)tmp1=-100.4;
        else if(FT1<=0.88473)tmp1=-100.3;
        else if(FT1<=0.88888)tmp1=-100.2;
        else if(FT1<=0.89304)tmp1=-100.1;
        else if(FT1<=0.89723)tmp1=-100;
    }
    else if(FT1<=0.93553){
        if(FT1==0.89723)tmp1=-100;
        else if(FT1<=0.90142)tmp1=-99.9;
        else if(FT1<=0.90563)tmp1=-99.8;
        else if(FT1<=0.90986)tmp1=-99.7;
        else if(FT1<=0.9141)tmp1=-99.6;
        else if(FT1<=0.91836)tmp1=-99.5;
        else if(FT1<=0.92263)tmp1=-99.4;
        else if(FT1<=0.92692)tmp1=-99.3;
        else if(FT1<=0.93122)tmp1=-99.2;
        else if(FT1<=0.93553)tmp1=-99.1;
        else if(FT1<=0.93987)tmp1=-99;
    }
    else if(FT1<=0.97955){
        if(FT1==0.93987)tmp1=-99;
        else if(FT1<=0.94421)tmp1=-98.9;
        else if(FT1<=0.94858)tmp1=-98.8;
        else if(FT1<=0.95296)tmp1=-98.7;
        else if(FT1<=0.95735)tmp1=-98.6;
        else if(FT1<=0.96176)tmp1=-98.5;
        else if(FT1<=0.96618)tmp1=-98.4;
        else if(FT1<=0.97062)tmp1=-98.3;
        else if(FT1<=0.97508)tmp1=-98.2;
        else if(FT1<=0.97955)tmp1=-98.1;
        else if(FT1<=0.98404)tmp1=-98;
    }
    else if(FT1<=1.02513){
        if(FT1==0.98404)tmp1=-98;
        else if(FT1<=0.98854)tmp1=-97.9;
        else if(FT1<=0.99306)tmp1=-97.8;
        else if(FT1<=0.99759)tmp1=-97.7;
        else if(FT1<=1.00214)tmp1=-97.6;
        else if(FT1<=1.00671)tmp1=-97.5;
        else if(FT1<=1.01129)tmp1=-97.4;
        else if(FT1<=1.01589)tmp1=-97.3;
        else if(FT1<=1.0205)tmp1=-97.2;
        else if(FT1<=1.02513)tmp1=-97.1;
        else if(FT1<=1.02977)tmp1=-97;
    }
    else if(FT1<=1.0723){
        if(FT1==1.02977)tmp1=-97;
        else if(FT1<=1.03443)tmp1=-96.9;
        else if(FT1<=1.03911)tmp1=-96.8;
        else if(FT1<=1.0438)tmp1=-96.7;
        else if(FT1<=1.04851)tmp1=-96.6;
        else if(FT1<=1.05324)tmp1=-96.5;
        else if(FT1<=1.05798)tmp1=-96.4;
        else if(FT1<=1.06274)tmp1=-96.3;
        else if(FT1<=1.06751)tmp1=-96.2;
        else if(FT1<=1.0723)tmp1=-96.1;
        else if(FT1<=1.07711)tmp1=-96;
    }
    else if(FT1<=1.1211){
        if(FT1==1.07711)tmp1=-96;
        else if(FT1<=1.08193)tmp1=-95.9;
        else if(FT1<=1.08677)tmp1=-95.8;
        else if(FT1<=1.09162)tmp1=-95.7;
        else if(FT1<=1.0965)tmp1=-95.6;
        else if(FT1<=1.10138)tmp1=-95.5;
        else if(FT1<=1.10629)tmp1=-95.4;
        else if(FT1<=1.11121)tmp1=-95.3;
        else if(FT1<=1.11615)tmp1=-95.2;
        else if(FT1<=1.1211)tmp1=-95.1;
        else if(FT1<=1.12607)tmp1=-95;
    }
    else if(FT1<=1.17157){
        if(FT1==1.12607)tmp1=-95;
        else if(FT1<=1.13106)tmp1=-94.9;
        else if(FT1<=1.13607)tmp1=-94.8;
        else if(FT1<=1.14109)tmp1=-94.7;
        else if(FT1<=1.14613)tmp1=-94.6;
        else if(FT1<=1.15118)tmp1=-94.5;
        else if(FT1<=1.15625)tmp1=-94.4;
        else if(FT1<=1.16134)tmp1=-94.3;
        else if(FT1<=1.16645)tmp1=-94.2;
        else if(FT1<=1.17157)tmp1=-94.1;
        else if(FT1<=1.17671)tmp1=-94;
    }
    else if(FT1<=1.22374){
        if(FT1==1.17671)tmp1=-94;
        else if(FT1<=1.18187)tmp1=-93.9;
        else if(FT1<=1.18704)tmp1=-93.8;
        else if(FT1<=1.19223)tmp1=-93.7;
        else if(FT1<=1.19744)tmp1=-93.6;
        else if(FT1<=1.20267)tmp1=-93.5;
        else if(FT1<=1.20791)tmp1=-93.4;
        else if(FT1<=1.21317)tmp1=-93.3;
        else if(FT1<=1.21845)tmp1=-93.2;
        else if(FT1<=1.22374)tmp1=-93.1;
        else if(FT1<=1.22906)tmp1=-93;
    }
    else if(FT1<=1.27765){
        if(FT1==1.22906)tmp1=-93;
        else if(FT1<=1.23438)tmp1=-92.9;
        else if(FT1<=1.23973)tmp1=-92.8;
        else if(FT1<=1.2451)tmp1=-92.7;
        else if(FT1<=1.25048)tmp1=-92.6;
        else if(FT1<=1.25588)tmp1=-92.5;
        else if(FT1<=1.26129)tmp1=-92.4;
        else if(FT1<=1.26673)tmp1=-92.3;
        else if(FT1<=1.27218)tmp1=-92.2;
        else if(FT1<=1.27765)tmp1=-92.1;
        else if(FT1<=1.28314)tmp1=-92;
    }
    else if(FT1<=1.33333){
        if(FT1==1.28314)tmp1=-92;
        else if(FT1<=1.28864)tmp1=-91.9;
        else if(FT1<=1.29417)tmp1=-91.8;
        else if(FT1<=1.29971)tmp1=-91.7;
        else if(FT1<=1.30527)tmp1=-91.6;
        else if(FT1<=1.31084)tmp1=-91.5;
        else if(FT1<=1.31644)tmp1=-91.4;
        else if(FT1<=1.32205)tmp1=-91.3;
        else if(FT1<=1.32768)tmp1=-91.2;
        else if(FT1<=1.33333)tmp1=-91.1;
        else if(FT1<=1.339)tmp1=-91;
    }
    else if(FT1<=1.39082){
        if(FT1==1.339)tmp1=-91;
        else if(FT1<=1.34468)tmp1=-90.9;
        else if(FT1<=1.35039)tmp1=-90.8;
        else if(FT1<=1.35611)tmp1=-90.7;
        else if(FT1<=1.36185)tmp1=-90.6;
        else if(FT1<=1.3676)tmp1=-90.5;
        else if(FT1<=1.37338)tmp1=-90.4;
        else if(FT1<=1.37918)tmp1=-90.3;
        else if(FT1<=1.38499)tmp1=-90.2;
        else if(FT1<=1.39082)tmp1=-90.1;
        else if(FT1<=1.39667)tmp1=-90;
    }
    else if(FT1<=1.45015){
        if(FT1==1.39667)tmp1=-90;
        else if(FT1<=1.40254)tmp1=-89.9;
        else if(FT1<=1.40842)tmp1=-89.8;
        else if(FT1<=1.41433)tmp1=-89.7;
        else if(FT1<=1.42025)tmp1=-89.6;
        else if(FT1<=1.4262)tmp1=-89.5;
        else if(FT1<=1.43216)tmp1=-89.4;
        else if(FT1<=1.43814)tmp1=-89.3;
        else if(FT1<=1.44414)tmp1=-89.2;
        else if(FT1<=1.45015)tmp1=-89.1;
        else if(FT1<=1.45619)tmp1=-89;
    }
    else if(FT1<=1.51137){
        if(FT1==1.45619)tmp1=-89;
        else if(FT1<=1.46224)tmp1=-88.9;
        else if(FT1<=1.46832)tmp1=-88.8;
        else if(FT1<=1.47441)tmp1=-88.7;
        else if(FT1<=1.48052)tmp1=-88.6;
        else if(FT1<=1.48665)tmp1=-88.5;
        else if(FT1<=1.4928)tmp1=-88.4;
        else if(FT1<=1.49897)tmp1=-88.3;
        else if(FT1<=1.50516)tmp1=-88.2;
        else if(FT1<=1.51137)tmp1=-88.1;
        else if(FT1<=1.51759)tmp1=-88;
    }
    else if(FT1<=1.5745){
        if(FT1==1.51759)tmp1=-88;
        else if(FT1<=1.52384)tmp1=-87.9;
        else if(FT1<=1.5301)tmp1=-87.8;
        else if(FT1<=1.53639)tmp1=-87.7;
        else if(FT1<=1.54269)tmp1=-87.6;
        else if(FT1<=1.54901)tmp1=-87.5;
        else if(FT1<=1.55535)tmp1=-87.4;
        else if(FT1<=1.56171)tmp1=-87.3;
        else if(FT1<=1.5681)tmp1=-87.2;
        else if(FT1<=1.5745)tmp1=-87.1;
        else if(FT1<=1.58092)tmp1=-87;
    }
    else if(FT1<=1.63958){
        if(FT1==1.58092)tmp1=-87;
        else if(FT1<=1.58735)tmp1=-86.9;
        else if(FT1<=1.59381)tmp1=-86.8;
        else if(FT1<=1.60029)tmp1=-86.7;
        else if(FT1<=1.60679)tmp1=-86.6;
        else if(FT1<=1.61331)tmp1=-86.5;
        else if(FT1<=1.61985)tmp1=-86.4;
        else if(FT1<=1.6264)tmp1=-86.3;
        else if(FT1<=1.63298)tmp1=-86.2;
        else if(FT1<=1.63958)tmp1=-86.1;
        else if(FT1<=1.64619)tmp1=-86;
    }
    else if(FT1<=1.70665){
        if(FT1==1.64619)tmp1=-86;
        else if(FT1<=1.65283)tmp1=-85.9;
        else if(FT1<=1.65949)tmp1=-85.8;
        else if(FT1<=1.66617)tmp1=-85.7;
        else if(FT1<=1.67286)tmp1=-85.6;
        else if(FT1<=1.67958)tmp1=-85.5;
        else if(FT1<=1.68632)tmp1=-85.4;
        else if(FT1<=1.69307)tmp1=-85.3;
        else if(FT1<=1.69985)tmp1=-85.2;
        else if(FT1<=1.70665)tmp1=-85.1;
        else if(FT1<=1.71347)tmp1=-85;
    }
    else if(FT1<=1.77574){
        if(FT1==1.71347)tmp1=-85;
        else if(FT1<=1.7203)tmp1=-84.9;
        else if(FT1<=1.72716)tmp1=-84.8;
        else if(FT1<=1.73404)tmp1=-84.7;
        else if(FT1<=1.74094)tmp1=-84.6;
        else if(FT1<=1.74786)tmp1=-84.5;
        else if(FT1<=1.7548)tmp1=-84.4;
        else if(FT1<=1.76176)tmp1=-84.3;
        else if(FT1<=1.76874)tmp1=-84.2;
        else if(FT1<=1.77574)tmp1=-84.1;
        else if(FT1<=1.78277)tmp1=-84;
    }
    else if(FT1<=1.8469){
        if(FT1==1.78277)tmp1=-84;
        else if(FT1<=1.78981)tmp1=-83.9;
        else if(FT1<=1.79687)tmp1=-83.8;
        else if(FT1<=1.80396)tmp1=-83.7;
        else if(FT1<=1.81106)tmp1=-83.6;
        else if(FT1<=1.81819)tmp1=-83.5;
        else if(FT1<=1.82534)tmp1=-83.4;
        else if(FT1<=1.8325)tmp1=-83.3;
        else if(FT1<=1.83969)tmp1=-83.2;
        else if(FT1<=1.8469)tmp1=-83.1;
        else if(FT1<=1.85413)tmp1=-83;
    }
    else if(FT1<=1.92016){
        if(FT1==1.85413)tmp1=-83;
        else if(FT1<=1.86138)tmp1=-82.9;
        else if(FT1<=1.86866)tmp1=-82.8;
        else if(FT1<=1.87595)tmp1=-82.7;
        else if(FT1<=1.88326)tmp1=-82.6;
        else if(FT1<=1.8906)tmp1=-82.5;
        else if(FT1<=1.89796)tmp1=-82.4;
        else if(FT1<=1.90534)tmp1=-82.3;
        else if(FT1<=1.91274)tmp1=-82.2;
        else if(FT1<=1.92016)tmp1=-82.1;
        else if(FT1<=1.9276)tmp1=-82;
    }
    else if(FT1<=1.99555){
        if(FT1==1.9276)tmp1=-82;
        else if(FT1<=1.93506)tmp1=-81.9;
        else if(FT1<=1.94255)tmp1=-81.8;
        else if(FT1<=1.95005)tmp1=-81.7;
        else if(FT1<=1.95758)tmp1=-81.6;
        else if(FT1<=1.96513)tmp1=-81.5;
        else if(FT1<=1.9727)tmp1=-81.4;
        else if(FT1<=1.9803)tmp1=-81.3;
        else if(FT1<=1.98791)tmp1=-81.2;
        else if(FT1<=1.99555)tmp1=-81.1;
        else if(FT1<=2.0032)tmp1=-81;
    }
    else if(FT1<=2.07311){
        if(FT1==2.0032)tmp1=-81;
        else if(FT1<=2.01088)tmp1=-80.9;
        else if(FT1<=2.01858)tmp1=-80.8;
        else if(FT1<=2.02631)tmp1=-80.7;
        else if(FT1<=2.03405)tmp1=-80.6;
        else if(FT1<=2.04182)tmp1=-80.5;
        else if(FT1<=2.04961)tmp1=-80.4;
        else if(FT1<=2.05742)tmp1=-80.3;
        else if(FT1<=2.06525)tmp1=-80.2;
        else if(FT1<=2.07311)tmp1=-80.1;
        else if(FT1<=2.08098)tmp1=-80;
    }
    else if(FT1<=2.15287){
        if(FT1==2.08098)tmp1=-80;
        else if(FT1<=2.08888)tmp1=-79.9;
        else if(FT1<=2.0968)tmp1=-79.8;
        else if(FT1<=2.10475)tmp1=-79.7;
        else if(FT1<=2.11271)tmp1=-79.6;
        else if(FT1<=2.1207)tmp1=-79.5;
        else if(FT1<=2.12871)tmp1=-79.4;
        else if(FT1<=2.13674)tmp1=-79.3;
        else if(FT1<=2.14479)tmp1=-79.2;
        else if(FT1<=2.15287)tmp1=-79.1;
        else if(FT1<=2.16097)tmp1=-79;
    }
    else if(FT1<=2.23488){
        if(FT1==2.16097)tmp1=-79;
        else if(FT1<=2.16909)tmp1=-78.9;
        else if(FT1<=2.17724)tmp1=-78.8;
        else if(FT1<=2.1854)tmp1=-78.7;
        else if(FT1<=2.19359)tmp1=-78.6;
        else if(FT1<=2.20181)tmp1=-78.5;
        else if(FT1<=2.21004)tmp1=-78.4;
        else if(FT1<=2.2183)tmp1=-78.3;
        else if(FT1<=2.22658)tmp1=-78.2;
        else if(FT1<=2.23488)tmp1=-78.1;
        else if(FT1<=2.24321)tmp1=-78;
    }
    else if(FT1<=2.31917){
        if(FT1==2.24321)tmp1=-78;
        else if(FT1<=2.25156)tmp1=-77.9;
        else if(FT1<=2.25993)tmp1=-77.8;
        else if(FT1<=2.26832)tmp1=-77.7;
        else if(FT1<=2.27674)tmp1=-77.6;
        else if(FT1<=2.28518)tmp1=-77.5;
        else if(FT1<=2.29364)tmp1=-77.4;
        else if(FT1<=2.30213)tmp1=-77.3;
        else if(FT1<=2.31064)tmp1=-77.2;
        else if(FT1<=2.31917)tmp1=-77.1;
        else if(FT1<=2.32773)tmp1=-77;
    }
    else if(FT1<=2.40578){
        if(FT1==2.32773)tmp1=-77;
        else if(FT1<=2.3363)tmp1=-76.9;
        else if(FT1<=2.34491)tmp1=-76.8;
        else if(FT1<=2.35353)tmp1=-76.7;
        else if(FT1<=2.36218)tmp1=-76.6;
        else if(FT1<=2.37085)tmp1=-76.5;
        else if(FT1<=2.37955)tmp1=-76.4;
        else if(FT1<=2.38827)tmp1=-76.3;
        else if(FT1<=2.39701)tmp1=-76.2;
        else if(FT1<=2.40578)tmp1=-76.1;
        else if(FT1<=2.41456)tmp1=-76;
    }
    else if(FT1<=2.49473){
        if(FT1==2.41456)tmp1=-76;
        else if(FT1<=2.42338)tmp1=-75.9;
        else if(FT1<=2.43221)tmp1=-75.8;
        else if(FT1<=2.44107)tmp1=-75.7;
        else if(FT1<=2.44996)tmp1=-75.6;
        else if(FT1<=2.45886)tmp1=-75.5;
        else if(FT1<=2.4678)tmp1=-75.4;
        else if(FT1<=2.47675)tmp1=-75.3;
        else if(FT1<=2.48573)tmp1=-75.2;
        else if(FT1<=2.49473)tmp1=-75.1;
        else if(FT1<=2.50376)tmp1=-75;
    }
    else if(FT1<=2.58608){
        if(FT1==2.50376)tmp1=-75;
        else if(FT1<=2.51281)tmp1=-74.9;
        else if(FT1<=2.52188)tmp1=-74.8;
        else if(FT1<=2.53098)tmp1=-74.7;
        else if(FT1<=2.5401)tmp1=-74.6;
        else if(FT1<=2.54925)tmp1=-74.5;
        else if(FT1<=2.55842)tmp1=-74.4;
        else if(FT1<=2.56762)tmp1=-74.3;
        else if(FT1<=2.57683)tmp1=-74.2;
        else if(FT1<=2.58608)tmp1=-74.1;
        else if(FT1<=2.59534)tmp1=-74;
    }
    else if(FT1<=2.67985){
        if(FT1==2.59534)tmp1=-74;
        else if(FT1<=2.60464)tmp1=-73.9;
        else if(FT1<=2.61395)tmp1=-73.8;
        else if(FT1<=2.62329)tmp1=-73.7;
        else if(FT1<=2.63266)tmp1=-73.6;
        else if(FT1<=2.64205)tmp1=-73.5;
        else if(FT1<=2.65146)tmp1=-73.4;
        else if(FT1<=2.6609)tmp1=-73.3;
        else if(FT1<=2.67036)tmp1=-73.2;
        else if(FT1<=2.67985)tmp1=-73.1;
        else if(FT1<=2.68936)tmp1=-73;
    }
    else if(FT1<=2.77608){
        if(FT1==2.68936)tmp1=-73;
        else if(FT1<=2.69889)tmp1=-72.9;
        else if(FT1<=2.70846)tmp1=-72.8;
        else if(FT1<=2.71804)tmp1=-72.7;
        else if(FT1<=2.72765)tmp1=-72.6;
        else if(FT1<=2.73729)tmp1=-72.5;
        else if(FT1<=2.74695)tmp1=-72.4;
        else if(FT1<=2.75663)tmp1=-72.3;
        else if(FT1<=2.76634)tmp1=-72.2;
        else if(FT1<=2.77608)tmp1=-72.1;
        else if(FT1<=2.78584)tmp1=-72;
    }
    else if(FT1<=2.8748){
        if(FT1==2.78584)tmp1=-72;
        else if(FT1<=2.79562)tmp1=-71.9;
        else if(FT1<=2.80543)tmp1=-71.8;
        else if(FT1<=2.81527)tmp1=-71.7;
        else if(FT1<=2.82513)tmp1=-71.6;
        else if(FT1<=2.83501)tmp1=-71.5;
        else if(FT1<=2.84492)tmp1=-71.4;
        else if(FT1<=2.85486)tmp1=-71.3;
        else if(FT1<=2.86482)tmp1=-71.2;
        else if(FT1<=2.8748)tmp1=-71.1;
        else if(FT1<=2.88481)tmp1=-71;
    }
    else if(FT1<=2.97606){
        if(FT1==2.88481)tmp1=-71;
        else if(FT1<=2.89485)tmp1=-70.9;
        else if(FT1<=2.90491)tmp1=-70.8;
        else if(FT1<=2.915)tmp1=-70.7;
        else if(FT1<=2.92511)tmp1=-70.6;
        else if(FT1<=2.93525)tmp1=-70.5;
        else if(FT1<=2.94542)tmp1=-70.4;
        else if(FT1<=2.95561)tmp1=-70.3;
        else if(FT1<=2.96582)tmp1=-70.2;
        else if(FT1<=2.97606)tmp1=-70.1;
        else if(FT1<=2.98633)tmp1=-70;
    }
    else if(FT1<=3.07989){
        if(FT1==2.98633)tmp1=-70;
        else if(FT1<=2.99662)tmp1=-69.9;
        else if(FT1<=3.00694)tmp1=-69.8;
        else if(FT1<=3.01728)tmp1=-69.7;
        else if(FT1<=3.02765)tmp1=-69.6;
        else if(FT1<=3.03805)tmp1=-69.5;
        else if(FT1<=3.04847)tmp1=-69.4;
        else if(FT1<=3.05892)tmp1=-69.3;
        else if(FT1<=3.06939)tmp1=-69.2;
        else if(FT1<=3.07989)tmp1=-69.1;
        else if(FT1<=3.09041)tmp1=-69;
    }
    else if(FT1<=3.18632){
        if(FT1==3.09041)tmp1=-69;
        else if(FT1<=3.10097)tmp1=-68.9;
        else if(FT1<=3.11154)tmp1=-68.8;
        else if(FT1<=3.12215)tmp1=-68.7;
        else if(FT1<=3.13278)tmp1=-68.6;
        else if(FT1<=3.14343)tmp1=-68.5;
        else if(FT1<=3.15412)tmp1=-68.4;
        else if(FT1<=3.16482)tmp1=-68.3;
        else if(FT1<=3.17556)tmp1=-68.2;
        else if(FT1<=3.18632)tmp1=-68.1;
        else if(FT1<=3.19711)tmp1=-68;
    }
    else if(FT1<=3.29539){
        if(FT1==3.19711)tmp1=-68;
        else if(FT1<=3.20792)tmp1=-67.9;
        else if(FT1<=3.21876)tmp1=-67.8;
        else if(FT1<=3.22963)tmp1=-67.7;
        else if(FT1<=3.24052)tmp1=-67.6;
        else if(FT1<=3.25144)tmp1=-67.5;
        else if(FT1<=3.26239)tmp1=-67.4;
        else if(FT1<=3.27336)tmp1=-67.3;
        else if(FT1<=3.28436)tmp1=-67.2;
        else if(FT1<=3.29539)tmp1=-67.1;
        else if(FT1<=3.30645)tmp1=-67;
    }
    else if(FT1<=3.40714){
        if(FT1==3.30645)tmp1=-67;
        else if(FT1<=3.31753)tmp1=-66.9;
        else if(FT1<=3.32863)tmp1=-66.8;
        else if(FT1<=3.33977)tmp1=-66.7;
        else if(FT1<=3.35093)tmp1=-66.6;
        else if(FT1<=3.36212)tmp1=-66.5;
        else if(FT1<=3.37333)tmp1=-66.4;
        else if(FT1<=3.38457)tmp1=-66.3;
        else if(FT1<=3.39584)tmp1=-66.2;
        else if(FT1<=3.40714)tmp1=-66.1;
        else if(FT1<=3.41846)tmp1=-66;
    }
    else if(FT1<=3.52159){
        if(FT1==3.41846)tmp1=-66;
        else if(FT1<=3.42981)tmp1=-65.9;
        else if(FT1<=3.44119)tmp1=-65.8;
        else if(FT1<=3.45259)tmp1=-65.7;
        else if(FT1<=3.46402)tmp1=-65.6;
        else if(FT1<=3.47548)tmp1=-65.5;
        else if(FT1<=3.48697)tmp1=-65.4;
        else if(FT1<=3.49848)tmp1=-65.3;
        else if(FT1<=3.51003)tmp1=-65.2;
        else if(FT1<=3.52159)tmp1=-65.1;
        else if(FT1<=3.53319)tmp1=-65;
    }
    else if(FT1<=3.6388){
        if(FT1==3.53319)tmp1=-65;
        else if(FT1<=3.54481)tmp1=-64.9;
        else if(FT1<=3.55646)tmp1=-64.8;
        else if(FT1<=3.56814)tmp1=-64.7;
        else if(FT1<=3.57985)tmp1=-64.6;
        else if(FT1<=3.59158)tmp1=-64.5;
        else if(FT1<=3.60334)tmp1=-64.4;
        else if(FT1<=3.61513)tmp1=-64.3;
        else if(FT1<=3.62695)tmp1=-64.2;
        else if(FT1<=3.6388)tmp1=-64.1;
        else if(FT1<=3.65067)tmp1=-64;
    }
    else if(FT1<=3.75878){
        if(FT1==3.65067)tmp1=-64;
        else if(FT1<=3.66257)tmp1=-63.9;
        else if(FT1<=3.6745)tmp1=-63.8;
        else if(FT1<=3.68645)tmp1=-63.7;
        else if(FT1<=3.69844)tmp1=-63.6;
        else if(FT1<=3.71045)tmp1=-63.5;
        else if(FT1<=3.72249)tmp1=-63.4;
        else if(FT1<=3.73456)tmp1=-63.3;
        else if(FT1<=3.74665)tmp1=-63.2;
        else if(FT1<=3.75878)tmp1=-63.1;
    }
    else if(FT1<=3.88158){
        if(FT1==3.77093)tmp1=-63;
        else if(FT1<=3.78311)tmp1=-62.9;
        else if(FT1<=3.79532)tmp1=-62.8;
        else if(FT1<=3.80756)tmp1=-62.7;
        else if(FT1<=3.81982)tmp1=-62.6;
        else if(FT1<=3.83212)tmp1=-62.5;
        else if(FT1<=3.84444)tmp1=-62.4;
        else if(FT1<=3.85679)tmp1=-62.3;
        else if(FT1<=3.86917)tmp1=-62.2;
        else if(FT1<=3.88158)tmp1=-62.1;
    }
    else if(FT1<=4.00723){
        if(FT1==3.89401)tmp1=-62;
        else if(FT1<=3.90648)tmp1=-61.9;
        else if(FT1<=3.91897)tmp1=-61.8;
        else if(FT1<=3.93149)tmp1=-61.7;
        else if(FT1<=3.94404)tmp1=-61.6;
        else if(FT1<=3.95662)tmp1=-61.5;
        else if(FT1<=3.96923)tmp1=-61.4;
        else if(FT1<=3.98187)tmp1=-61.3;
        else if(FT1<=3.99453)tmp1=-61.2;
        else if(FT1<=4.00723)tmp1=-61.1;
    }
    else if(FT1<=4.13576){
        if(FT1==4.01995)tmp1=-61;
        else if(FT1<=4.0327)tmp1=-60.9;
        else if(FT1<=4.04548)tmp1=-60.8;
        else if(FT1<=4.05829)tmp1=-60.7;
        else if(FT1<=4.07113)tmp1=-60.6;
        else if(FT1<=4.084)tmp1=-60.5;
        else if(FT1<=4.09689)tmp1=-60.4;
        else if(FT1<=4.10982)tmp1=-60.3;
        else if(FT1<=4.12278)tmp1=-60.2;
        else if(FT1<=4.13576)tmp1=-60.1;
    }
    else if(FT1<=4.26721){
        if(FT1==4.14877)tmp1=-60;
        else if(FT1<=4.16182)tmp1=-59.9;
        else if(FT1<=4.17489)tmp1=-59.8;
        else if(FT1<=4.18799)tmp1=-59.7;
        else if(FT1<=4.20112)tmp1=-59.6;
        else if(FT1<=4.21428)tmp1=-59.5;
        else if(FT1<=4.22747)tmp1=-59.4;
        else if(FT1<=4.24069)tmp1=-59.3;
        else if(FT1<=4.25393)tmp1=-59.2;
        else if(FT1<=4.26721)tmp1=-59.1;
    }
    else if(FT1<=4.40162){
        if(FT1==4.28052)tmp1=-59;
        else if(FT1<=4.29386)tmp1=-58.9;
        else if(FT1<=4.30722)tmp1=-58.8;
        else if(FT1<=4.32062)tmp1=-58.7;
        else if(FT1<=4.33404)tmp1=-58.6;
        else if(FT1<=4.3475)tmp1=-58.5;
        else if(FT1<=4.36098)tmp1=-58.4;
        else if(FT1<=4.3745)tmp1=-58.3;
        else if(FT1<=4.38804)tmp1=-58.2;
        else if(FT1<=4.40162)tmp1=-58.1;
    }
    else if(FT1<=4.53901){
        if(FT1==4.41522)tmp1=-58;
        else if(FT1<=4.42886)tmp1=-57.9;
        else if(FT1<=4.44252)tmp1=-57.8;
        else if(FT1<=4.45622)tmp1=-57.7;
        else if(FT1<=4.46994)tmp1=-57.6;
        else if(FT1<=4.4837)tmp1=-57.5;
        else if(FT1<=4.49748)tmp1=-57.4;
        else if(FT1<=4.51129)tmp1=-57.3;
        else if(FT1<=4.52514)tmp1=-57.2;
        else if(FT1<=4.53901)tmp1=-57.1;
    }
    else if(FT1<=4.67943){
        if(FT1==4.55292)tmp1=-57;
        else if(FT1<=4.56685)tmp1=-56.9;
        else if(FT1<=4.58082)tmp1=-56.8;
        else if(FT1<=4.59482)tmp1=-56.7;
        else if(FT1<=4.60884)tmp1=-56.6;
        else if(FT1<=4.6229)tmp1=-56.5;
        else if(FT1<=4.63699)tmp1=-56.4;
        else if(FT1<=4.6511)tmp1=-56.3;
        else if(FT1<=4.66525)tmp1=-56.2;
        else if(FT1<=4.67943)tmp1=-56.1;
    }
    else if(FT1<=4.8229){
        if(FT1==4.69364)tmp1=-56;
        else if(FT1<=4.70788)tmp1=-55.9;
        else if(FT1<=4.72215)tmp1=-55.8;
        else if(FT1<=4.73645)tmp1=-55.7;
        else if(FT1<=4.75078)tmp1=-55.6;
        else if(FT1<=4.76514)tmp1=-55.5;
        else if(FT1<=4.77954)tmp1=-55.4;
        else if(FT1<=4.79396)tmp1=-55.3;
        else if(FT1<=4.80841)tmp1=-55.2;
        else if(FT1<=4.8229)tmp1=-55.1;
    }
    else if(FT1<=4.96946){
        if(FT1==4.83742)tmp1=-55;
        else if(FT1<=4.85196)tmp1=-54.9;
        else if(FT1<=4.86654)tmp1=-54.8;
        else if(FT1<=4.88115)tmp1=-54.7;
        else if(FT1<=4.89579)tmp1=-54.6;
        else if(FT1<=4.91046)tmp1=-54.5;
        else if(FT1<=4.92517)tmp1=-54.4;
        else if(FT1<=4.9399)tmp1=-54.3;
        else if(FT1<=4.95466)tmp1=-54.2;
        else if(FT1<=4.96946)tmp1=-54.1;
    }
    else if(FT1<=5.11914){
        if(FT1==4.98429)tmp1=-54;
        else if(FT1<=4.99915)tmp1=-53.9;
        else if(FT1<=5.01404)tmp1=-53.8;
        else if(FT1<=5.02896)tmp1=-53.7;
        else if(FT1<=5.04391)tmp1=-53.6;
        else if(FT1<=5.05889)tmp1=-53.5;
        else if(FT1<=5.07391)tmp1=-53.4;
        else if(FT1<=5.08895)tmp1=-53.3;
        else if(FT1<=5.10403)tmp1=-53.2;
        else if(FT1<=5.11914)tmp1=-53.1;
    }
    else if(FT1<=5.27198){
        if(FT1==5.13428)tmp1=-53;
        else if(FT1<=5.14946)tmp1=-52.9;
        else if(FT1<=5.16466)tmp1=-52.8;
        else if(FT1<=5.1799)tmp1=-52.7;
        else if(FT1<=5.19516)tmp1=-52.6;
        else if(FT1<=5.21046)tmp1=-52.5;
        else if(FT1<=5.22579)tmp1=-52.4;
        else if(FT1<=5.24116)tmp1=-52.3;
        else if(FT1<=5.25655)tmp1=-52.2;
        else if(FT1<=5.27198)tmp1=-52.1;
    }
    else if(FT1<=5.428){
        if(FT1==5.28744)tmp1=-52;
        else if(FT1<=5.30293)tmp1=-51.9;
        else if(FT1<=5.31845)tmp1=-51.8;
        else if(FT1<=5.334)tmp1=-51.7;
        else if(FT1<=5.34959)tmp1=-51.6;
        else if(FT1<=5.36521)tmp1=-51.5;
        else if(FT1<=5.38086)tmp1=-51.4;
        else if(FT1<=5.39654)tmp1=-51.3;
        else if(FT1<=5.41225)tmp1=-51.2;
        else if(FT1<=5.428)tmp1=-51.1;
    }
    else if(FT1<=5.58725){
        if(FT1==5.44378)tmp1=-51;
        else if(FT1<=5.45959)tmp1=-50.9;
        else if(FT1<=5.47544)tmp1=-50.8;
        else if(FT1<=5.49131)tmp1=-50.7;
        else if(FT1<=5.50722)tmp1=-50.6;
        else if(FT1<=5.52316)tmp1=-50.5;
        else if(FT1<=5.53913)tmp1=-50.4;
        else if(FT1<=5.55514)tmp1=-50.3;
        else if(FT1<=5.57118)tmp1=-50.2;
        else if(FT1<=5.58725)tmp1=-50.1;
    }
    else if(FT1<=5.74974){
        if(FT1==5.60335)tmp1=-50;
        else if(FT1<=5.61948)tmp1=-49.9;
        else if(FT1<=5.63565)tmp1=-49.8;
        else if(FT1<=5.65185)tmp1=-49.7;
        else if(FT1<=5.66809)tmp1=-49.6;
        else if(FT1<=5.68435)tmp1=-49.5;
        else if(FT1<=5.70065)tmp1=-49.4;
        else if(FT1<=5.71698)tmp1=-49.3;
        else if(FT1<=5.73334)tmp1=-49.2;
        else if(FT1<=5.74974)tmp1=-49.1;
    }
    else if(FT1<=5.91552){
        if(FT1==5.76617)tmp1=-49;
        else if(FT1<=5.78263)tmp1=-48.9;
        else if(FT1<=5.79913)tmp1=-48.8;
        else if(FT1<=5.81566)tmp1=-48.7;
        else if(FT1<=5.83222)tmp1=-48.6;
        else if(FT1<=5.84881)tmp1=-48.5;
        else if(FT1<=5.86544)tmp1=-48.4;
        else if(FT1<=5.8821)tmp1=-48.3;
        else if(FT1<=5.8988)tmp1=-48.2;
        else if(FT1<=5.91552)tmp1=-48.1;
    }
    else if(FT1<=6.08462){
        if(FT1==5.93228)tmp1=-48;
        else if(FT1<=5.94908)tmp1=-47.9;
        else if(FT1<=5.9659)tmp1=-47.8;
        else if(FT1<=5.98276)tmp1=-47.7;
        else if(FT1<=5.99965)tmp1=-47.6;
        else if(FT1<=6.01658)tmp1=-47.5;
        else if(FT1<=6.03354)tmp1=-47.4;
        else if(FT1<=6.05053)tmp1=-47.3;
        else if(FT1<=6.06756)tmp1=-47.2;
        else if(FT1<=6.08462)tmp1=-47.1;
    }
    else if(FT1<=6.25707){
        if(FT1==6.10171)tmp1=-47;
        else if(FT1<=6.11884)tmp1=-46.9;
        else if(FT1<=6.136)tmp1=-46.8;
        else if(FT1<=6.15319)tmp1=-46.7;
        else if(FT1<=6.17042)tmp1=-46.6;
        else if(FT1<=6.18768)tmp1=-46.5;
        else if(FT1<=6.20498)tmp1=-46.4;
        else if(FT1<=6.22231)tmp1=-46.3;
        else if(FT1<=6.23967)tmp1=-46.2;
        else if(FT1<=6.25707)tmp1=-46.1;
    }
    else if(FT1<=6.43289){
        if(FT1==6.2745)tmp1=-46;
        else if(FT1<=6.29196)tmp1=-45.9;
        else if(FT1<=6.30946)tmp1=-45.8;
        else if(FT1<=6.32699)tmp1=-45.7;
        else if(FT1<=6.34455)tmp1=-45.6;
        else if(FT1<=6.36215)tmp1=-45.5;
        else if(FT1<=6.37979)tmp1=-45.4;
        else if(FT1<=6.39745)tmp1=-45.3;
        else if(FT1<=6.41515)tmp1=-45.2;
        else if(FT1<=6.43289)tmp1=-45.1;
    }
    else if(FT1<=6.61212){
        if(FT1==6.45066)tmp1=-45;
        else if(FT1<=6.46846)tmp1=-44.9;
        else if(FT1<=6.4863)tmp1=-44.8;
        else if(FT1<=6.50417)tmp1=-44.7;
        else if(FT1<=6.52208)tmp1=-44.6;
        else if(FT1<=6.54002)tmp1=-44.5;
        else if(FT1<=6.55799)tmp1=-44.4;
        else if(FT1<=6.576)tmp1=-44.3;
        else if(FT1<=6.59405)tmp1=-44.2;
        else if(FT1<=6.61212)tmp1=-44.1;
    }
    else if(FT1<=6.7948){
        if(FT1==6.63024)tmp1=-44;
        else if(FT1<=6.64838)tmp1=-43.9;
        else if(FT1<=6.66656)tmp1=-43.8;
        else if(FT1<=6.68478)tmp1=-43.7;
        else if(FT1<=6.70303)tmp1=-43.6;
        else if(FT1<=6.72131)tmp1=-43.5;
        else if(FT1<=6.73963)tmp1=-43.4;
        else if(FT1<=6.75799)tmp1=-43.3;
        else if(FT1<=6.77638)tmp1=-43.2;
        else if(FT1<=6.7948)tmp1=-43.1;
    }
    else if(FT1<=6.98095){
        if(FT1==6.81326)tmp1=-43;
        else if(FT1<=6.83175)tmp1=-42.9;
        else if(FT1<=6.85028)tmp1=-42.8;
        else if(FT1<=6.86884)tmp1=-42.7;
        else if(FT1<=6.88744)tmp1=-42.6;
        else if(FT1<=6.90607)tmp1=-42.5;
        else if(FT1<=6.92474)tmp1=-42.4;
        else if(FT1<=6.94344)tmp1=-42.3;
        else if(FT1<=6.96218)tmp1=-42.2;
        else if(FT1<=6.98095)tmp1=-42.1;
    }
    else if(FT1<=7.1706){
        if(FT1==6.99975)tmp1=-42;
        else if(FT1<=7.0186)tmp1=-41.9;
        else if(FT1<=7.03747)tmp1=-41.8;
        else if(FT1<=7.05638)tmp1=-41.7;
        else if(FT1<=7.07533)tmp1=-41.6;
        else if(FT1<=7.09431)tmp1=-41.5;
        else if(FT1<=7.11333)tmp1=-41.4;
        else if(FT1<=7.13238)tmp1=-41.3;
        else if(FT1<=7.15147)tmp1=-41.2;
        else if(FT1<=7.1706)tmp1=-41.1;
    }
    else if(FT1<=7.36378){
        if(FT1==7.18975)tmp1=-41;
        else if(FT1<=7.20895)tmp1=-40.9;
        else if(FT1<=7.22818)tmp1=-40.8;
        else if(FT1<=7.24744)tmp1=-40.7;
        else if(FT1<=7.26674)tmp1=-40.6;
        else if(FT1<=7.28608)tmp1=-40.5;
        else if(FT1<=7.30545)tmp1=-40.4;
        else if(FT1<=7.32486)tmp1=-40.3;
        else if(FT1<=7.3443)tmp1=-40.2;
        else if(FT1<=7.36378)tmp1=-40.1;
    }
    else if(FT1<=7.56052){
        if(FT1==7.38329)tmp1=-40;
        else if(FT1<=7.40284)tmp1=-39.9;
        else if(FT1<=7.42242)tmp1=-39.8;
        else if(FT1<=7.44204)tmp1=-39.7;
        else if(FT1<=7.4617)tmp1=-39.6;
        else if(FT1<=7.48139)tmp1=-39.5;
        else if(FT1<=7.50112)tmp1=-39.4;
        else if(FT1<=7.52088)tmp1=-39.3;
        else if(FT1<=7.54068)tmp1=-39.2;
        else if(FT1<=7.56052)tmp1=-39.1;
    }
    else if(FT1<=7.76085){
        if(FT1==7.58039)tmp1=-39;
        else if(FT1<=7.6003)tmp1=-38.9;
        else if(FT1<=7.62024)tmp1=-38.8;
        else if(FT1<=7.64022)tmp1=-38.7;
        else if(FT1<=7.66024)tmp1=-38.6;
        else if(FT1<=7.68029)tmp1=-38.5;
        else if(FT1<=7.70037)tmp1=-38.4;
        else if(FT1<=7.7205)tmp1=-38.3;
        else if(FT1<=7.74066)tmp1=-38.2;
        else if(FT1<=7.76085)tmp1=-38.1;
    }
    else if(FT1<=7.96481){
        if(FT1==7.78108)tmp1=-38;
        else if(FT1<=7.80135)tmp1=-37.9;
        else if(FT1<=7.82166)tmp1=-37.8;
        else if(FT1<=7.842)tmp1=-37.7;
        else if(FT1<=7.86237)tmp1=-37.6;
        else if(FT1<=7.88279)tmp1=-37.5;
        else if(FT1<=7.90324)tmp1=-37.4;
        else if(FT1<=7.92372)tmp1=-37.3;
        else if(FT1<=7.94425)tmp1=-37.2;
        else if(FT1<=7.96481)tmp1=-37.1;
    }
    else if(FT1<=8.17241){
        if(FT1==7.9854)tmp1=-37;
        else if(FT1<=8.00603)tmp1=-36.9;
        else if(FT1<=8.0267)tmp1=-36.8;
        else if(FT1<=8.04741)tmp1=-36.7;
        else if(FT1<=8.06815)tmp1=-36.6;
        else if(FT1<=8.08893)tmp1=-36.5;
        else if(FT1<=8.10974)tmp1=-36.4;
        else if(FT1<=8.13059)tmp1=-36.3;
        else if(FT1<=8.15148)tmp1=-36.2;
        else if(FT1<=8.17241)tmp1=-36.1;
    }
    else if(FT1<=8.38369){
        if(FT1==8.19337)tmp1=-36;
        else if(FT1<=8.21437)tmp1=-35.9;
        else if(FT1<=8.2354)tmp1=-35.8;
        else if(FT1<=8.25648)tmp1=-35.7;
        else if(FT1<=8.27759)tmp1=-35.6;
        else if(FT1<=8.29873)tmp1=-35.5;
        else if(FT1<=8.31992)tmp1=-35.4;
        else if(FT1<=8.34114)tmp1=-35.3;
        else if(FT1<=8.36239)tmp1=-35.2;
        else if(FT1<=8.38369)tmp1=-35.1;
    }
    else if(FT1<=8.59867){
        if(FT1==8.40502)tmp1=-35;
        else if(FT1<=8.42639)tmp1=-34.9;
        else if(FT1<=8.44779)tmp1=-34.8;
        else if(FT1<=8.46923)tmp1=-34.7;
        else if(FT1<=8.49071)tmp1=-34.6;
        else if(FT1<=8.51223)tmp1=-34.5;
        else if(FT1<=8.53379)tmp1=-34.4;
        else if(FT1<=8.55538)tmp1=-34.3;
        else if(FT1<=8.57701)tmp1=-34.2;
        else if(FT1<=8.59867)tmp1=-34.1;
    }
    else if(FT1<=8.81739){
        if(FT1==8.62038)tmp1=-34;
        else if(FT1<=8.64212)tmp1=-33.9;
        else if(FT1<=8.66389)tmp1=-33.8;
        else if(FT1<=8.68571)tmp1=-33.7;
        else if(FT1<=8.70756)tmp1=-33.6;
        else if(FT1<=8.72945)tmp1=-33.5;
        else if(FT1<=8.75138)tmp1=-33.4;
        else if(FT1<=8.77335)tmp1=-33.3;
        else if(FT1<=8.79535)tmp1=-33.2;
        else if(FT1<=8.81739)tmp1=-33.1;
    }
    else if(FT1<=9.03987){
        if(FT1==8.83947)tmp1=-33;
        else if(FT1<=8.86158)tmp1=-32.9;
        else if(FT1<=8.88374)tmp1=-32.8;
        else if(FT1<=8.90593)tmp1=-32.7;
        else if(FT1<=8.92816)tmp1=-32.6;
        else if(FT1<=8.95043)tmp1=-32.5;
        else if(FT1<=8.97273)tmp1=-32.4;
        else if(FT1<=8.99507)tmp1=-32.3;
        else if(FT1<=9.01745)tmp1=-32.2;
        else if(FT1<=9.03987)tmp1=-32.1;
    }
    else if(FT1<=9.26614){
        if(FT1==9.06233)tmp1=-32;
        else if(FT1<=9.08482)tmp1=-31.9;
        else if(FT1<=9.10735)tmp1=-31.8;
        else if(FT1<=9.12992)tmp1=-31.7;
        else if(FT1<=9.15253)tmp1=-31.6;
        else if(FT1<=9.17517)tmp1=-31.5;
        else if(FT1<=9.19786)tmp1=-31.4;
        else if(FT1<=9.22058)tmp1=-31.3;
        else if(FT1<=9.24334)tmp1=-31.2;
        else if(FT1<=9.26614)tmp1=-31.1;
    }
    else if(FT1<=9.49622){
        if(FT1==9.28897)tmp1=-31;
        else if(FT1<=9.31185)tmp1=-30.9;
        else if(FT1<=9.33476)tmp1=-30.8;
        else if(FT1<=9.35771)tmp1=-30.7;
        else if(FT1<=9.3807)tmp1=-30.6;
        else if(FT1<=9.40373)tmp1=-30.5;
        else if(FT1<=9.42679)tmp1=-30.4;
        else if(FT1<=9.4499)tmp1=-30.3;
        else if(FT1<=9.47304)tmp1=-30.2;
        else if(FT1<=9.49622)tmp1=-30.1;
    }
    else if(FT1<=9.73015){
        if(FT1==9.51944)tmp1=-30;
        else if(FT1<=9.5427)tmp1=-29.9;
        else if(FT1<=9.566)tmp1=-29.8;
        else if(FT1<=9.58933)tmp1=-29.7;
        else if(FT1<=9.6127)tmp1=-29.6;
        else if(FT1<=9.63611)tmp1=-29.5;
        else if(FT1<=9.65957)tmp1=-29.4;
        else if(FT1<=9.68305)tmp1=-29.3;
        else if(FT1<=9.70658)tmp1=-29.2;
        else if(FT1<=9.73015)tmp1=-29.1;
    }
    else if(FT1<=9.96794){
        if(FT1==9.75375)tmp1=-29;
        else if(FT1<=9.7774)tmp1=-28.9;
        else if(FT1<=9.80108)tmp1=-28.8;
        else if(FT1<=9.8248)tmp1=-28.7;
        else if(FT1<=9.84856)tmp1=-28.6;
        else if(FT1<=9.87236)tmp1=-28.5;
        else if(FT1<=9.8962)tmp1=-28.4;
        else if(FT1<=9.92007)tmp1=-28.3;
        else if(FT1<=9.94399)tmp1=-28.2;
        else if(FT1<=9.96794)tmp1=-28.1;
    }
    else if(FT1<=10.2096){
        if(FT1==9.99194)tmp1=-28;
        else if(FT1<=10.016)tmp1=-27.9;
        else if(FT1<=10.04)tmp1=-27.8;
        else if(FT1<=10.0642)tmp1=-27.7;
        else if(FT1<=10.0883)tmp1=-27.6;
        else if(FT1<=10.1125)tmp1=-27.5;
        else if(FT1<=10.1367)tmp1=-27.4;
        else if(FT1<=10.161)tmp1=-27.3;
        else if(FT1<=10.1853)tmp1=-27.2;
        else if(FT1<=10.2096)tmp1=-27.1;
    }
    else if(FT1<=10.4552){
        if(FT1==10.234)tmp1=-27;
        else if(FT1<=10.2584)tmp1=-26.9;
        else if(FT1<=10.2829)tmp1=-26.8;
        else if(FT1<=10.3074)tmp1=-26.7;
        else if(FT1<=10.3319)tmp1=-26.6;
        else if(FT1<=10.3565)tmp1=-26.5;
        else if(FT1<=10.3811)tmp1=-26.4;
        else if(FT1<=10.4058)tmp1=-26.3;
        else if(FT1<=10.4305)tmp1=-26.2;
        else if(FT1<=10.4552)tmp1=-26.1;
    }
    else if(FT1<=10.7048){
        if(FT1==10.48)tmp1=-26;
        else if(FT1<=10.5048)tmp1=-25.9;
        else if(FT1<=10.5297)tmp1=-25.8;
        else if(FT1<=10.5546)tmp1=-25.7;
        else if(FT1<=10.5795)tmp1=-25.6;
        else if(FT1<=10.6045)tmp1=-25.5;
        else if(FT1<=10.6295)tmp1=-25.4;
        else if(FT1<=10.6546)tmp1=-25.3;
        else if(FT1<=10.6797)tmp1=-25.2;
        else if(FT1<=10.7048)tmp1=-25.1;
    }
    else if(FT1<=10.9583){
        if(FT1==10.73)tmp1=-25;
        else if(FT1<=10.7552)tmp1=-24.9;
        else if(FT1<=10.7804)tmp1=-24.8;
        else if(FT1<=10.8057)tmp1=-24.7;
        else if(FT1<=10.8311)tmp1=-24.6;
        else if(FT1<=10.8564)tmp1=-24.5;
        else if(FT1<=10.8819)tmp1=-24.4;
        else if(FT1<=10.9073)tmp1=-24.3;
        else if(FT1<=10.9328)tmp1=-24.2;
        else if(FT1<=10.9583)tmp1=-24.1;
    }
    else if(FT1<=11.2159){
        if(FT1==10.9839)tmp1=-24;
        else if(FT1<=11.0095)tmp1=-23.9;
        else if(FT1<=11.0352)tmp1=-23.8;
        else if(FT1<=11.0609)tmp1=-23.7;
        else if(FT1<=11.0866)tmp1=-23.6;
        else if(FT1<=11.1124)tmp1=-23.5;
        else if(FT1<=11.1382)tmp1=-23.4;
        else if(FT1<=11.164)tmp1=-23.3;
        else if(FT1<=11.1899)tmp1=-23.2;
        else if(FT1<=11.2159)tmp1=-23.1;
    }
    else if(FT1<=11.4774){
        if(FT1==11.2418)tmp1=-23;
        else if(FT1<=11.2678)tmp1=-22.9;
        else if(FT1<=11.2939)tmp1=-22.8;
        else if(FT1<=11.32)tmp1=-22.7;
        else if(FT1<=11.3461)tmp1=-22.6;
        else if(FT1<=11.3723)tmp1=-22.5;
        else if(FT1<=11.3985)tmp1=-22.4;
        else if(FT1<=11.4248)tmp1=-22.3;
        else if(FT1<=11.4511)tmp1=-22.2;
        else if(FT1<=11.4774)tmp1=-22.1;
    }
    else if(FT1<=11.743){
        if(FT1==11.5038)tmp1=-22;
        else if(FT1<=11.5302)tmp1=-21.9;
        else if(FT1<=11.5566)tmp1=-21.8;
        else if(FT1<=11.5831)tmp1=-21.7;
        else if(FT1<=11.6097)tmp1=-21.6;
        else if(FT1<=11.6363)tmp1=-21.5;
        else if(FT1<=11.6629)tmp1=-21.4;
        else if(FT1<=11.6895)tmp1=-21.3;
        else if(FT1<=11.7162)tmp1=-21.2;
        else if(FT1<=11.743)tmp1=-21.1;
    }
    else if(FT1<=12.0126){
        if(FT1==11.7698)tmp1=-21;
        else if(FT1<=11.7966)tmp1=-20.9;
        else if(FT1<=11.8234)tmp1=-20.8;
        else if(FT1<=11.8504)tmp1=-20.7;
        else if(FT1<=11.8773)tmp1=-20.6;
        else if(FT1<=11.9043)tmp1=-20.5;
        else if(FT1<=11.9313)tmp1=-20.4;
        else if(FT1<=11.9584)tmp1=-20.3;
        else if(FT1<=11.9855)tmp1=-20.2;
        else if(FT1<=12.0126)tmp1=-20.1;
    }
    else if(FT1<=12.2864){
        if(FT1==12.0398)tmp1=-20;
        else if(FT1<=12.0671)tmp1=-19.9;
        else if(FT1<=12.0943)tmp1=-19.8;
        else if(FT1<=12.1216)tmp1=-19.7;
        else if(FT1<=12.149)tmp1=-19.6;
        else if(FT1<=12.1764)tmp1=-19.5;
        else if(FT1<=12.2038)tmp1=-19.4;
        else if(FT1<=12.2313)tmp1=-19.3;
        else if(FT1<=12.2588)tmp1=-19.2;
        else if(FT1<=12.2864)tmp1=-19.1;
    }
    else if(FT1<=12.5642){
        if(FT1==12.314)tmp1=-19;
        else if(FT1<=12.3416)tmp1=-18.9;
        else if(FT1<=12.3693)tmp1=-18.8;
        else if(FT1<=12.397)tmp1=-18.7;
        else if(FT1<=12.4248)tmp1=-18.6;
        else if(FT1<=12.4526)tmp1=-18.5;
        else if(FT1<=12.4805)tmp1=-18.4;
        else if(FT1<=12.5083)tmp1=-18.3;
        else if(FT1<=12.5363)tmp1=-18.2;
        else if(FT1<=12.5642)tmp1=-18.1;
    }
    else if(FT1<=12.8462){
        if(FT1==12.5923)tmp1=-18;
        else if(FT1<=12.6203)tmp1=-17.9;
        else if(FT1<=12.6484)tmp1=-17.8;
        else if(FT1<=12.6766)tmp1=-17.7;
        else if(FT1<=12.7047)tmp1=-17.6;
        else if(FT1<=12.733)tmp1=-17.5;
        else if(FT1<=12.7612)tmp1=-17.4;
        else if(FT1<=12.7895)tmp1=-17.3;
        else if(FT1<=12.8179)tmp1=-17.2;
        else if(FT1<=12.8462)tmp1=-17.1;
    }
    else if(FT1<=13.1324){
        if(FT1==12.8747)tmp1=-17;
        else if(FT1<=12.9031)tmp1=-16.9;
        else if(FT1<=12.9317)tmp1=-16.8;
        else if(FT1<=12.9602)tmp1=-16.7;
        else if(FT1<=12.9888)tmp1=-16.6;
        else if(FT1<=13.0174)tmp1=-16.5;
        else if(FT1<=13.0461)tmp1=-16.4;
        else if(FT1<=13.0748)tmp1=-16.3;
        else if(FT1<=13.1036)tmp1=-16.2;
        else if(FT1<=13.1324)tmp1=-16.1;
    }
    else if(FT1<=13.4228){
        if(FT1==13.1613)tmp1=-16;
        else if(FT1<=13.1901)tmp1=-15.9;
        else if(FT1<=13.2191)tmp1=-15.8;
        else if(FT1<=13.248)tmp1=-15.7;
        else if(FT1<=13.2771)tmp1=-15.6;
        else if(FT1<=13.3061)tmp1=-15.5;
        else if(FT1<=13.3352)tmp1=-15.4;
        else if(FT1<=13.3643)tmp1=-15.3;
        else if(FT1<=13.3935)tmp1=-15.2;
        else if(FT1<=13.4228)tmp1=-15.1;
        else if(FT1<=13.452)tmp1=-15;
    }
    else if(FT1<=13.7173){
        if(FT1==13.452)tmp1=-15;
        else if(FT1<=13.4813)tmp1=-14.9;
        else if(FT1<=13.5107)tmp1=-14.8;
        else if(FT1<=13.5401)tmp1=-14.7;
        else if(FT1<=13.5695)tmp1=-14.6;
        else if(FT1<=13.599)tmp1=-14.5;
        else if(FT1<=13.6285)tmp1=-14.4;
        else if(FT1<=13.6581)tmp1=-14.3;
        else if(FT1<=13.6877)tmp1=-14.2;
        else if(FT1<=13.7173)tmp1=-14.1;
        else if(FT1<=13.747)tmp1=-14;
    }
    else if(FT1<=14.0161){
        if(FT1==13.747)tmp1=-14;
        else if(FT1<=13.7767)tmp1=-13.9;
        else if(FT1<=13.8065)tmp1=-13.8;
        else if(FT1<=13.8363)tmp1=-13.7;
        else if(FT1<=13.8662)tmp1=-13.6;
        else if(FT1<=13.8961)tmp1=-13.5;
        else if(FT1<=13.926)tmp1=-13.4;
        else if(FT1<=13.956)tmp1=-13.3;
        else if(FT1<=13.986)tmp1=-13.2;
        else if(FT1<=14.0161)tmp1=-13.1;
        else if(FT1<=14.0462)tmp1=-13;
    }
    else if(FT1<=14.3191){
        if(FT1==14.0462)tmp1=-13;
        else if(FT1<=14.0763)tmp1=-12.9;
        else if(FT1<=14.1065)tmp1=-12.8;
        else if(FT1<=14.1368)tmp1=-12.7;
        else if(FT1<=14.1671)tmp1=-12.6;
        else if(FT1<=14.1974)tmp1=-12.5;
        else if(FT1<=14.2277)tmp1=-12.4;
        else if(FT1<=14.2582)tmp1=-12.3;
        else if(FT1<=14.2886)tmp1=-12.2;
        else if(FT1<=14.3191)tmp1=-12.1;
        else if(FT1<=14.3496)tmp1=-12;
    }
    else if(FT1<=14.6264){
        if(FT1==14.3496)tmp1=-12;
        else if(FT1<=14.3802)tmp1=-11.9;
        else if(FT1<=14.4108)tmp1=-11.8;
        else if(FT1<=14.4415)tmp1=-11.7;
        else if(FT1<=14.4722)tmp1=-11.6;
        else if(FT1<=14.503)tmp1=-11.5;
        else if(FT1<=14.5338)tmp1=-11.4;
        else if(FT1<=14.5646)tmp1=-11.3;
        else if(FT1<=14.5955)tmp1=-11.2;
        else if(FT1<=14.6264)tmp1=-11.1;
        else if(FT1<=14.6574)tmp1=-11;
    }
    else if(FT1<=14.938){
        if(FT1==14.6574)tmp1=-11;
        else if(FT1<=14.6884)tmp1=-10.9;
        else if(FT1<=14.7194)tmp1=-10.8;
        else if(FT1<=14.7505)tmp1=-10.7;
        else if(FT1<=14.7816)tmp1=-10.6;
        else if(FT1<=14.8128)tmp1=-10.5;
        else if(FT1<=14.844)tmp1=-10.4;
        else if(FT1<=14.8753)tmp1=-10.3;
        else if(FT1<=14.9066)tmp1=-10.2;
        else if(FT1<=14.938)tmp1=-10.1;
        else if(FT1<=14.9694)tmp1=-10;
    }
    else if(FT1<=15.2539){
        if(FT1==14.9694)tmp1=-10;
        else if(FT1<=15.0008)tmp1=-9.9;
        else if(FT1<=15.0323)tmp1=-9.8;
        else if(FT1<=15.0638)tmp1=-9.7;
        else if(FT1<=15.0954)tmp1=-9.6;
        else if(FT1<=15.127)tmp1=-9.5;
        else if(FT1<=15.1586)tmp1=-9.4;
        else if(FT1<=15.1903)tmp1=-9.3;
        else if(FT1<=15.2221)tmp1=-9.2;
        else if(FT1<=15.2539)tmp1=-9.1;
        else if(FT1<=15.2857)tmp1=-9;
    }
    else if(FT1<=15.5741){
        if(FT1==15.2857)tmp1=-9;
        else if(FT1<=15.3176)tmp1=-8.9;
        else if(FT1<=15.3495)tmp1=-8.8;
        else if(FT1<=15.3814)tmp1=-8.7;
        else if(FT1<=15.4134)tmp1=-8.6;
        else if(FT1<=15.4455)tmp1=-8.5;
        else if(FT1<=15.4776)tmp1=-8.4;
        else if(FT1<=15.5097)tmp1=-8.3;
        else if(FT1<=15.5419)tmp1=-8.2;
        else if(FT1<=15.5741)tmp1=-8.1;
        else if(FT1<=15.6063)tmp1=-8;
    }
    else if(FT1<=15.8986){
        if(FT1==15.6063)tmp1=-8;
        else if(FT1<=15.6386)tmp1=-7.9;
        else if(FT1<=15.671)tmp1=-7.8;
        else if(FT1<=15.7034)tmp1=-7.7;
        else if(FT1<=15.7358)tmp1=-7.6;
        else if(FT1<=15.7683)tmp1=-7.5;
        else if(FT1<=15.8008)tmp1=-7.4;
        else if(FT1<=15.8334)tmp1=-7.3;
        else if(FT1<=15.866)tmp1=-7.2;
        else if(FT1<=15.8986)tmp1=-7.1;
        else if(FT1<=15.9313)tmp1=-7;
    }
    else if(FT1<=16.2276){
        if(FT1==15.9313)tmp1=-7;
        else if(FT1<=15.9641)tmp1=-6.9;
        else if(FT1<=15.9969)tmp1=-6.8;
        else if(FT1<=16.0297)tmp1=-6.7;
        else if(FT1<=16.0626)tmp1=-6.6;
        else if(FT1<=16.0955)tmp1=-6.5;
        else if(FT1<=16.1284)tmp1=-6.4;
        else if(FT1<=16.1614)tmp1=-6.3;
        else if(FT1<=16.1945)tmp1=-6.2;
        else if(FT1<=16.2276)tmp1=-6.1;
        else if(FT1<=16.2607)tmp1=-6;
    }
    else if(FT1<=16.5609){
        if(FT1==16.2607)tmp1=-6;
        else if(FT1<=16.2939)tmp1=-5.9;
        else if(FT1<=16.3271)tmp1=-5.8;
        else if(FT1<=16.3604)tmp1=-5.7;
        else if(FT1<=16.3937)tmp1=-5.6;
        else if(FT1<=16.4271)tmp1=-5.5;
        else if(FT1<=16.4604)tmp1=-5.4;
        else if(FT1<=16.4939)tmp1=-5.3;
        else if(FT1<=16.5274)tmp1=-5.2;
        else if(FT1<=16.5609)tmp1=-5.1;
        else if(FT1<=16.5945)tmp1=-5;
    }
    else if(FT1<=16.8986){
        if(FT1==16.5945)tmp1=-5;
        else if(FT1<=16.6281)tmp1=-4.9;
        else if(FT1<=16.6618)tmp1=-4.8;
        else if(FT1<=16.6955)tmp1=-4.7;
        else if(FT1<=16.7292)tmp1=-4.6;
        else if(FT1<=16.763)tmp1=-4.5;
        else if(FT1<=16.7969)tmp1=-4.4;
        else if(FT1<=16.8307)tmp1=-4.3;
        else if(FT1<=16.8647)tmp1=-4.2;
        else if(FT1<=16.8986)tmp1=-4.1;
        else if(FT1<=16.9327)tmp1=-4;
    }
    else if(FT1<=17.2408){
        if(FT1==16.9327)tmp1=-4;
        else if(FT1<=16.9667)tmp1=-3.9;
        else if(FT1<=17.0008)tmp1=-3.8;
        else if(FT1<=17.035)tmp1=-3.7;
        else if(FT1<=17.0692)tmp1=-3.6;
        else if(FT1<=17.1034)tmp1=-3.5;
        else if(FT1<=17.1377)tmp1=-3.4;
        else if(FT1<=17.172)tmp1=-3.3;
        else if(FT1<=17.2064)tmp1=-3.2;
        else if(FT1<=17.2408)tmp1=-3.1;
    }
    else if(FT1<=17.5874){
        if(FT1==17.2752)tmp1=-3;
        else if(FT1<=17.3098)tmp1=-2.9;
        else if(FT1<=17.3443)tmp1=-2.8;
        else if(FT1<=17.3789)tmp1=-2.7;
        else if(FT1<=17.4135)tmp1=-2.6;
        else if(FT1<=17.4482)tmp1=-2.5;
        else if(FT1<=17.4829)tmp1=-2.4;
        else if(FT1<=17.5177)tmp1=-2.3;
        else if(FT1<=17.5525)tmp1=-2.2;
        else if(FT1<=17.5874)tmp1=-2.1;
    }
    else if(FT1<=17.9384){
        if(FT1==17.6223)tmp1=-2;
        else if(FT1<=17.6572)tmp1=-1.9;
        else if(FT1<=17.6922)tmp1=-1.8;
        else if(FT1<=17.7273)tmp1=-1.7;
        else if(FT1<=17.7624)tmp1=-1.6;
        else if(FT1<=17.7975)tmp1=-1.5;
        else if(FT1<=17.8327)tmp1=-1.4;
        else if(FT1<=17.8679)tmp1=-1.3;
        else if(FT1<=17.9031)tmp1=-1.2;
        else if(FT1<=17.9384)tmp1=-1.1;
    }
    else if(FT1<=18.294){
        if(FT1==17.9738)tmp1=-1;
        else if(FT1<=18.0092)tmp1=-0.9;
        else if(FT1<=18.0446)tmp1=-0.8;
        else if(FT1<=18.0801)tmp1=-0.7;
        else if(FT1<=18.1156)tmp1=-0.6;
        else if(FT1<=18.1512)tmp1=-0.5;
        else if(FT1<=18.1868)tmp1=-0.4;
        else if(FT1<=18.2225)tmp1=-0.3;
        else if(FT1<=18.2582)tmp1=-0.2;
        else if(FT1<=18.294)tmp1=-0.1;
        else if(FT1<=18.3298)tmp1=-1.38778e-16;
    }
    else if(FT1<=18.654){
        if(FT1==18.3298)tmp1=0;
        else if(FT1<=18.3656)tmp1=0.1;
        else if(FT1<=18.4015)tmp1=0.2;
        else if(FT1<=18.4374)tmp1=0.3;
        else if(FT1<=18.4734)tmp1=0.4;
        else if(FT1<=18.5094)tmp1=0.5;
        else if(FT1<=18.5455)tmp1=0.6;
        else if(FT1<=18.5816)tmp1=0.7;
        else if(FT1<=18.6178)tmp1=0.8;
        else if(FT1<=18.654)tmp1=0.9;
        else if(FT1<=18.6902)tmp1=1;
    }
    else if(FT1<=19.0185){
        if(FT1==18.6902)tmp1=1;
        else if(FT1<=18.7265)tmp1=1.1;
        else if(FT1<=18.7629)tmp1=1.2;
        else if(FT1<=18.7993)tmp1=1.3;
        else if(FT1<=18.8357)tmp1=1.4;
        else if(FT1<=18.8722)tmp1=1.5;
        else if(FT1<=18.9087)tmp1=1.6;
        else if(FT1<=18.9453)tmp1=1.7;
        else if(FT1<=18.9819)tmp1=1.8;
        else if(FT1<=19.0185)tmp1=1.9;
    }
    else if(FT1<=19.3876){
        if(FT1==19.0552)tmp1=2;
        else if(FT1<=19.092)tmp1=2.1;
        else if(FT1<=19.1288)tmp1=2.2;
        else if(FT1<=19.1656)tmp1=2.3;
        else if(FT1<=19.2025)tmp1=2.4;
        else if(FT1<=19.2394)tmp1=2.5;
        else if(FT1<=19.2764)tmp1=2.6;
        else if(FT1<=19.3134)tmp1=2.7;
        else if(FT1<=19.3505)tmp1=2.8;
        else if(FT1<=19.3876)tmp1=2.9;
    }
    else if(FT1<=19.7612){
        if(FT1==19.4247)tmp1=3;
        else if(FT1<=19.4619)tmp1=3.1;
        else if(FT1<=19.4992)tmp1=3.2;
        else if(FT1<=19.5365)tmp1=3.3;
        else if(FT1<=19.5738)tmp1=3.4;
        else if(FT1<=19.6112)tmp1=3.5;
        else if(FT1<=19.6486)tmp1=3.6;
        else if(FT1<=19.6861)tmp1=3.7;
        else if(FT1<=19.7236)tmp1=3.8;
        else if(FT1<=19.7612)tmp1=3.9;
    }
    else if(FT1<=20.1394){
        if(FT1==19.7988)tmp1=4;
        else if(FT1<=19.8365)tmp1=4.1;
        else if(FT1<=19.8742)tmp1=4.2;
        else if(FT1<=19.9119)tmp1=4.3;
        else if(FT1<=19.9497)tmp1=4.4;
        else if(FT1<=19.9875)tmp1=4.5;
        else if(FT1<=20.0254)tmp1=4.6;
        else if(FT1<=20.0634)tmp1=4.7;
        else if(FT1<=20.1013)tmp1=4.8;
        else if(FT1<=20.1394)tmp1=4.9;
        else if(FT1<=20.1774)tmp1=5;
    }
    else if(FT1<=20.5221){
        if(FT1==20.1774)tmp1=5;
        else if(FT1<=20.2155)tmp1=5.1;
        else if(FT1<=20.2537)tmp1=5.2;
        else if(FT1<=20.2919)tmp1=5.3;
        else if(FT1<=20.3302)tmp1=5.4;
        else if(FT1<=20.3685)tmp1=5.5;
        else if(FT1<=20.4068)tmp1=5.6;
        else if(FT1<=20.4452)tmp1=5.7;
        else if(FT1<=20.4836)tmp1=5.8;
        else if(FT1<=20.5221)tmp1=5.9;
        else if(FT1<=20.5606)tmp1=6;
    }
    else if(FT1<=20.9094){
        if(FT1==20.5606)tmp1=6;
        else if(FT1<=20.5992)tmp1=6.1;
        else if(FT1<=20.6378)tmp1=6.2;
        else if(FT1<=20.6765)tmp1=6.3;
        else if(FT1<=20.7152)tmp1=6.4;
        else if(FT1<=20.7539)tmp1=6.5;
        else if(FT1<=20.7927)tmp1=6.6;
        else if(FT1<=20.8316)tmp1=6.7;
        else if(FT1<=20.8705)tmp1=6.8;
        else if(FT1<=20.9094)tmp1=6.9;
        else if(FT1<=20.9484)tmp1=7;
    }
    else if(FT1<=21.3014){
        if(FT1==20.9484)tmp1=7;
        else if(FT1<=20.9874)tmp1=7.1;
        else if(FT1<=21.0265)tmp1=7.2;
        else if(FT1<=21.0656)tmp1=7.3;
        else if(FT1<=21.1048)tmp1=7.4;
        else if(FT1<=21.144)tmp1=7.5;
        else if(FT1<=21.1833)tmp1=7.6;
        else if(FT1<=21.2226)tmp1=7.7;
        else if(FT1<=21.262)tmp1=7.8;
        else if(FT1<=21.3014)tmp1=7.9;
        else if(FT1<=21.3408)tmp1=8;
    }
    else if(FT1<=21.6979){
        if(FT1==21.3408)tmp1=8;
        else if(FT1<=21.3803)tmp1=8.1;
        else if(FT1<=21.4198)tmp1=8.2;
        else if(FT1<=21.4594)tmp1=8.3;
        else if(FT1<=21.4991)tmp1=8.4;
        else if(FT1<=21.5387)tmp1=8.5;
        else if(FT1<=21.5785)tmp1=8.6;
        else if(FT1<=21.6182)tmp1=8.7;
        else if(FT1<=21.658)tmp1=8.8;
        else if(FT1<=21.6979)tmp1=8.9;
        else if(FT1<=21.7378)tmp1=9;
    }
    else if(FT1<=22.0991){
        if(FT1==21.7378)tmp1=9;
        else if(FT1<=21.7778)tmp1=9.1;
        else if(FT1<=21.8178)tmp1=9.2;
        else if(FT1<=21.8578)tmp1=9.3;
        else if(FT1<=21.8979)tmp1=9.4;
        else if(FT1<=21.9381)tmp1=9.5;
        else if(FT1<=21.9782)tmp1=9.6;
        else if(FT1<=22.0185)tmp1=9.7;
        else if(FT1<=22.0588)tmp1=9.8;
        else if(FT1<=22.0991)tmp1=9.9;
        else if(FT1<=22.1395)tmp1=10;
    }
    else if(FT1<=22.5049){
        if(FT1==22.1395)tmp1=10;
        else if(FT1<=22.1799)tmp1=10.1;
        else if(FT1<=22.2203)tmp1=10.2;
        else if(FT1<=22.2609)tmp1=10.3;
        else if(FT1<=22.3014)tmp1=10.4;
        else if(FT1<=22.342)tmp1=10.5;
        else if(FT1<=22.3827)tmp1=10.6;
        else if(FT1<=22.4234)tmp1=10.7;
        else if(FT1<=22.4641)tmp1=10.8;
        else if(FT1<=22.5049)tmp1=10.9;
        else if(FT1<=22.5458)tmp1=11;
    }
    else if(FT1<=22.9154){
        if(FT1==22.5458)tmp1=11;
        else if(FT1<=22.5866)tmp1=11.1;
        else if(FT1<=22.6276)tmp1=11.2;
        else if(FT1<=22.6686)tmp1=11.3;
        else if(FT1<=22.7096)tmp1=11.4;
        else if(FT1<=22.7506)tmp1=11.5;
        else if(FT1<=22.7918)tmp1=11.6;
        else if(FT1<=22.8329)tmp1=11.7;
        else if(FT1<=22.8741)tmp1=11.8;
        else if(FT1<=22.9154)tmp1=11.9;
        else if(FT1<=22.9567)tmp1=12;
    }
    else if(FT1<=23.3306){
        if(FT1==22.9567)tmp1=12;
        else if(FT1<=22.9981)tmp1=12.1;
        else if(FT1<=23.0395)tmp1=12.2;
        else if(FT1<=23.0809)tmp1=12.3;
        else if(FT1<=23.1224)tmp1=12.4;
        else if(FT1<=23.1639)tmp1=12.5;
        else if(FT1<=23.2055)tmp1=12.6;
        else if(FT1<=23.2472)tmp1=12.7;
        else if(FT1<=23.2888)tmp1=12.8;
        else if(FT1<=23.3306)tmp1=12.9;
        else if(FT1<=23.3723)tmp1=13;
    }
    else if(FT1<=23.7504){
        if(FT1==23.3723)tmp1=13;
        else if(FT1<=23.4142)tmp1=13.1;
        else if(FT1<=23.456)tmp1=13.2;
        else if(FT1<=23.4979)tmp1=13.3;
        else if(FT1<=23.5399)tmp1=13.4;
        else if(FT1<=23.5819)tmp1=13.5;
        else if(FT1<=23.624)tmp1=13.6;
        else if(FT1<=23.6661)tmp1=13.7;
        else if(FT1<=23.7082)tmp1=13.8;
        else if(FT1<=23.7504)tmp1=13.9;
        else if(FT1<=23.7927)tmp1=14;
    }
    else if(FT1<=24.175){
        if(FT1==23.7927)tmp1=14;
        else if(FT1<=23.8349)tmp1=14.1;
        else if(FT1<=23.8773)tmp1=14.2;
        else if(FT1<=23.9197)tmp1=14.3;
        else if(FT1<=23.9621)tmp1=14.4;
        else if(FT1<=24.0046)tmp1=14.5;
        else if(FT1<=24.0471)tmp1=14.6;
        else if(FT1<=24.0897)tmp1=14.7;
        else if(FT1<=24.1323)tmp1=14.8;
        else if(FT1<=24.175)tmp1=14.9;
        else if(FT1<=24.2177)tmp1=15;
    }
    else if(FT1<=24.6042){
        if(FT1==24.2177)tmp1=15;
        else if(FT1<=24.2604)tmp1=15.1;
        else if(FT1<=24.3032)tmp1=15.2;
        else if(FT1<=24.3461)tmp1=15.3;
        else if(FT1<=24.389)tmp1=15.4;
        else if(FT1<=24.432)tmp1=15.5;
        else if(FT1<=24.4749)tmp1=15.6;
        else if(FT1<=24.518)tmp1=15.7;
        else if(FT1<=24.5611)tmp1=15.8;
        else if(FT1<=24.6042)tmp1=15.9;
        else if(FT1<=24.6474)tmp1=16;
    }
    else if(FT1<=25.0382){
        if(FT1==24.6474)tmp1=16;
        else if(FT1<=24.6906)tmp1=16.1;
        else if(FT1<=24.7339)tmp1=16.2;
        else if(FT1<=24.7772)tmp1=16.3;
        else if(FT1<=24.8206)tmp1=16.4;
        else if(FT1<=24.864)tmp1=16.5;
        else if(FT1<=24.9075)tmp1=16.6;
        else if(FT1<=24.951)tmp1=16.7;
        else if(FT1<=24.9946)tmp1=16.8;
        else if(FT1<=25.0382)tmp1=16.9;
    }
    else if(FT1<=25.4769){
        if(FT1==25.0819)tmp1=17;
        else if(FT1<=25.1256)tmp1=17.1;
        else if(FT1<=25.1693)tmp1=17.2;
        else if(FT1<=25.2131)tmp1=17.3;
        else if(FT1<=25.257)tmp1=17.4;
        else if(FT1<=25.3009)tmp1=17.5;
        else if(FT1<=25.3448)tmp1=17.6;
        else if(FT1<=25.3888)tmp1=17.7;
        else if(FT1<=25.4328)tmp1=17.8;
        else if(FT1<=25.4769)tmp1=17.9;
    }
    else if(FT1<=25.9204){
        if(FT1==25.5211)tmp1=18;
        else if(FT1<=25.5652)tmp1=18.1;
        else if(FT1<=25.6095)tmp1=18.2;
        else if(FT1<=25.6537)tmp1=18.3;
        else if(FT1<=25.6981)tmp1=18.4;
        else if(FT1<=25.7424)tmp1=18.5;
        else if(FT1<=25.7868)tmp1=18.6;
        else if(FT1<=25.8313)tmp1=18.7;
        else if(FT1<=25.8758)tmp1=18.8;
        else if(FT1<=25.9204)tmp1=18.9;
    }
    else if(FT1<=26.3686){
        if(FT1==25.965)tmp1=19;
        else if(FT1<=26.0096)tmp1=19.1;
        else if(FT1<=26.0543)tmp1=19.2;
        else if(FT1<=26.0991)tmp1=19.3;
        else if(FT1<=26.1439)tmp1=19.4;
        else if(FT1<=26.1887)tmp1=19.5;
        else if(FT1<=26.2336)tmp1=19.6;
        else if(FT1<=26.2786)tmp1=19.7;
        else if(FT1<=26.3236)tmp1=19.8;
        else if(FT1<=26.3686)tmp1=19.9;
    }
    else if(FT1<=26.8216){
        if(FT1==26.4137)tmp1=20;
        else if(FT1<=26.4588)tmp1=20.1;
        else if(FT1<=26.504)tmp1=20.2;
        else if(FT1<=26.5492)tmp1=20.3;
        else if(FT1<=26.5945)tmp1=20.4;
        else if(FT1<=26.6398)tmp1=20.5;
        else if(FT1<=26.6852)tmp1=20.6;
        else if(FT1<=26.7306)tmp1=20.7;
        else if(FT1<=26.7761)tmp1=20.8;
        else if(FT1<=26.8216)tmp1=20.9;
    }
    else if(FT1<=27.2794){
        if(FT1==26.8672)tmp1=21;
        else if(FT1<=26.9128)tmp1=21.1;
        else if(FT1<=26.9584)tmp1=21.2;
        else if(FT1<=27.0041)tmp1=21.3;
        else if(FT1<=27.0499)tmp1=21.4;
        else if(FT1<=27.0957)tmp1=21.5;
        else if(FT1<=27.1415)tmp1=21.6;
        else if(FT1<=27.1874)tmp1=21.7;
        else if(FT1<=27.2334)tmp1=21.8;
        else if(FT1<=27.2794)tmp1=21.9;
    }
    else if(FT1<=27.7419){
        if(FT1==27.3254)tmp1=22;
        else if(FT1<=27.3715)tmp1=22.1;
        else if(FT1<=27.4176)tmp1=22.2;
        else if(FT1<=27.4638)tmp1=22.3;
        else if(FT1<=27.51)tmp1=22.4;
        else if(FT1<=27.5563)tmp1=22.5;
        else if(FT1<=27.6026)tmp1=22.6;
        else if(FT1<=27.649)tmp1=22.7;
        else if(FT1<=27.6954)tmp1=22.8;
        else if(FT1<=27.7419)tmp1=22.9;
    }
    else if(FT1<=28.2093){
        if(FT1==27.7884)tmp1=23;
        else if(FT1<=27.835)tmp1=23.1;
        else if(FT1<=27.8816)tmp1=23.2;
        else if(FT1<=27.9283)tmp1=23.3;
        else if(FT1<=27.975)tmp1=23.4;
        else if(FT1<=28.0218)tmp1=23.5;
        else if(FT1<=28.0686)tmp1=23.6;
        else if(FT1<=28.1154)tmp1=23.7;
        else if(FT1<=28.1623)tmp1=23.8;
        else if(FT1<=28.2093)tmp1=23.9;
    }
    else if(FT1<=28.6814){
        if(FT1==28.2563)tmp1=24;
        else if(FT1<=28.3033)tmp1=24.1;
        else if(FT1<=28.3504)tmp1=24.2;
        else if(FT1<=28.3976)tmp1=24.3;
        else if(FT1<=28.4447)tmp1=24.4;
        else if(FT1<=28.492)tmp1=24.5;
        else if(FT1<=28.5393)tmp1=24.6;
        else if(FT1<=28.5866)tmp1=24.7;
        else if(FT1<=28.634)tmp1=24.8;
        else if(FT1<=28.6814)tmp1=24.9;
    }
    else if(FT1<=29.1584){
        if(FT1==28.7289)tmp1=25;
        else if(FT1<=28.7764)tmp1=25.1;
        else if(FT1<=28.824)tmp1=25.2;
        else if(FT1<=28.8716)tmp1=25.3;
        else if(FT1<=28.9193)tmp1=25.4;
        else if(FT1<=28.967)tmp1=25.5;
        else if(FT1<=29.0148)tmp1=25.6;
        else if(FT1<=29.0626)tmp1=25.7;
        else if(FT1<=29.1105)tmp1=25.8;
        else if(FT1<=29.1584)tmp1=25.9;
    }
    else if(FT1<=29.6402){
        if(FT1==29.2064)tmp1=26;
        else if(FT1<=29.2544)tmp1=26.1;
        else if(FT1<=29.3025)tmp1=26.2;
        else if(FT1<=29.3506)tmp1=26.3;
        else if(FT1<=29.3987)tmp1=26.4;
        else if(FT1<=29.4469)tmp1=26.5;
        else if(FT1<=29.4952)tmp1=26.6;
        else if(FT1<=29.5435)tmp1=26.7;
        else if(FT1<=29.5918)tmp1=26.8;
        else if(FT1<=29.6402)tmp1=26.9;
    }
    else if(FT1<=30.1269){
        if(FT1==29.6887)tmp1=27;
        else if(FT1<=29.7372)tmp1=27.1;
        else if(FT1<=29.7857)tmp1=27.2;
        else if(FT1<=29.8343)tmp1=27.3;
        else if(FT1<=29.8829)tmp1=27.4;
        else if(FT1<=29.9316)tmp1=27.5;
        else if(FT1<=29.9804)tmp1=27.6;
        else if(FT1<=30.0292)tmp1=27.7;
        else if(FT1<=30.078)tmp1=27.8;
        else if(FT1<=30.1269)tmp1=27.9;
    }
    else if(FT1<=30.6184){
        if(FT1==30.1758)tmp1=28;
        else if(FT1<=30.2248)tmp1=28.1;
        else if(FT1<=30.2738)tmp1=28.2;
        else if(FT1<=30.3229)tmp1=28.3;
        else if(FT1<=30.372)tmp1=28.4;
        else if(FT1<=30.4212)tmp1=28.5;
        else if(FT1<=30.4704)tmp1=28.6;
        else if(FT1<=30.5197)tmp1=28.7;
        else if(FT1<=30.569)tmp1=28.8;
        else if(FT1<=30.6184)tmp1=28.9;
    }
    else if(FT1<=31.1147){
        if(FT1==30.6678)tmp1=29;
        else if(FT1<=30.7172)tmp1=29.1;
        else if(FT1<=30.7668)tmp1=29.2;
        else if(FT1<=30.8163)tmp1=29.3;
        else if(FT1<=30.8659)tmp1=29.4;
        else if(FT1<=30.9156)tmp1=29.5;
        else if(FT1<=30.9653)tmp1=29.6;
        else if(FT1<=31.015)tmp1=29.7;
        else if(FT1<=31.0648)tmp1=29.8;
        else if(FT1<=31.1147)tmp1=29.9;
    }
    else if(FT1<=31.6159){
        if(FT1==31.1646)tmp1=30;
        else if(FT1<=31.2146)tmp1=30.1;
        else if(FT1<=31.2646)tmp1=30.2;
        else if(FT1<=31.3146)tmp1=30.3;
        else if(FT1<=31.3647)tmp1=30.4;
        else if(FT1<=31.4148)tmp1=30.5;
        else if(FT1<=31.465)tmp1=30.6;
        else if(FT1<=31.5153)tmp1=30.7;
        else if(FT1<=31.5656)tmp1=30.8;
        else if(FT1<=31.6159)tmp1=30.9;
    }
    else if(FT1<=32.122){
        if(FT1==31.6663)tmp1=31;
        else if(FT1<=31.7167)tmp1=31.1;
        else if(FT1<=31.7672)tmp1=31.2;
        else if(FT1<=31.8178)tmp1=31.3;
        else if(FT1<=31.8683)tmp1=31.4;
        else if(FT1<=31.919)tmp1=31.5;
        else if(FT1<=31.9696)tmp1=31.6;
        else if(FT1<=32.0204)tmp1=31.7;
        else if(FT1<=32.0712)tmp1=31.8;
        else if(FT1<=32.122)tmp1=31.9;
    }
    else if(FT1<=32.6329){
        if(FT1==32.1729)tmp1=32;
        else if(FT1<=32.2238)tmp1=32.1;
        else if(FT1<=32.2748)tmp1=32.2;
        else if(FT1<=32.3258)tmp1=32.3;
        else if(FT1<=32.3768)tmp1=32.4;
        else if(FT1<=32.428)tmp1=32.5;
        else if(FT1<=32.4791)tmp1=32.6;
        else if(FT1<=32.5303)tmp1=32.7;
        else if(FT1<=32.5816)tmp1=32.8;
        else if(FT1<=32.6329)tmp1=32.9;
    }
    else if(FT1<=33.1487){
        if(FT1==32.6843)tmp1=33;
        else if(FT1<=32.7357)tmp1=33.1;
        else if(FT1<=32.7872)tmp1=33.2;
        else if(FT1<=32.8387)tmp1=33.3;
        else if(FT1<=32.8902)tmp1=33.4;
        else if(FT1<=32.9418)tmp1=33.5;
        else if(FT1<=32.9935)tmp1=33.6;
        else if(FT1<=33.0452)tmp1=33.7;
        else if(FT1<=33.0969)tmp1=33.8;
        else if(FT1<=33.1487)tmp1=33.9;
    }
    else if(FT1<=33.6695){
        if(FT1==33.2006)tmp1=34;
        else if(FT1<=33.2525)tmp1=34.1;
        else if(FT1<=33.3045)tmp1=34.2;
        else if(FT1<=33.3565)tmp1=34.3;
        else if(FT1<=33.4085)tmp1=34.4;
        else if(FT1<=33.4606)tmp1=34.5;
        else if(FT1<=33.5128)tmp1=34.6;
        else if(FT1<=33.5649)tmp1=34.7;
        else if(FT1<=33.6172)tmp1=34.8;
        else if(FT1<=33.6695)tmp1=34.9;
    }
    else if(FT1<=34.1951){
        if(FT1==33.7218)tmp1=35;
        else if(FT1<=33.7742)tmp1=35.1;
        else if(FT1<=33.8267)tmp1=35.2;
        else if(FT1<=33.8791)tmp1=35.3;
        else if(FT1<=33.9317)tmp1=35.4;
        else if(FT1<=33.9843)tmp1=35.5;
        else if(FT1<=34.0369)tmp1=35.6;
        else if(FT1<=34.0896)tmp1=35.7;
        else if(FT1<=34.1423)tmp1=35.8;
        else if(FT1<=34.1951)tmp1=35.9;
    }
    else if(FT1<=34.7256){
        if(FT1==34.2479)tmp1=36;
        else if(FT1<=34.3008)tmp1=36.1;
        else if(FT1<=34.3537)tmp1=36.2;
        else if(FT1<=34.4067)tmp1=36.3;
        else if(FT1<=34.4597)tmp1=36.4;
        else if(FT1<=34.5128)tmp1=36.5;
        else if(FT1<=34.566)tmp1=36.6;
        else if(FT1<=34.6191)tmp1=36.7;
        else if(FT1<=34.6724)tmp1=36.8;
        else if(FT1<=34.7256)tmp1=36.9;
    }
    else if(FT1<=35.2611){
        if(FT1==34.7789)tmp1=37;
        else if(FT1<=34.8323)tmp1=37.1;
        else if(FT1<=34.8857)tmp1=37.2;
        else if(FT1<=34.9392)tmp1=37.3;
        else if(FT1<=34.9927)tmp1=37.4;
        else if(FT1<=35.0463)tmp1=37.5;
        else if(FT1<=35.0999)tmp1=37.6;
        else if(FT1<=35.1536)tmp1=37.7;
        else if(FT1<=35.2073)tmp1=37.8;
        else if(FT1<=35.2611)tmp1=37.9;
    }
    else if(FT1<=35.8014){
        if(FT1==35.3149)tmp1=38;
        else if(FT1<=35.3687)tmp1=38.1;
        else if(FT1<=35.4226)tmp1=38.2;
        else if(FT1<=35.4766)tmp1=38.3;
        else if(FT1<=35.5306)tmp1=38.4;
        else if(FT1<=35.5847)tmp1=38.5;
        else if(FT1<=35.6388)tmp1=38.6;
        else if(FT1<=35.6929)tmp1=38.7;
        else if(FT1<=35.7471)tmp1=38.8;
        else if(FT1<=35.8014)tmp1=38.9;
    }
    else if(FT1<=36.3467){
        if(FT1==35.8557)tmp1=39;
        else if(FT1<=35.9101)tmp1=39.1;
        else if(FT1<=35.9645)tmp1=39.2;
        else if(FT1<=36.0189)tmp1=39.3;
        else if(FT1<=36.0734)tmp1=39.4;
        else if(FT1<=36.128)tmp1=39.5;
        else if(FT1<=36.1826)tmp1=39.6;
        else if(FT1<=36.2372)tmp1=39.7;
        else if(FT1<=36.2919)tmp1=39.8;
        else if(FT1<=36.3467)tmp1=39.9;
    }
    else if(FT1<=36.8969){
        if(FT1==36.4015)tmp1=40;
        else if(FT1<=36.4563)tmp1=40.1;
        else if(FT1<=36.5112)tmp1=40.2;
        else if(FT1<=36.5662)tmp1=40.3;
        else if(FT1<=36.6212)tmp1=40.4;
        else if(FT1<=36.6762)tmp1=40.5;
        else if(FT1<=36.7313)tmp1=40.6;
        else if(FT1<=36.7864)tmp1=40.7;
        else if(FT1<=36.8416)tmp1=40.8;
        else if(FT1<=36.8969)tmp1=40.9;
    }
    else if(FT1<=37.452){
        if(FT1==36.9522)tmp1=41;
        else if(FT1<=37.0075)tmp1=41.1;
        else if(FT1<=37.0629)tmp1=41.2;
        else if(FT1<=37.1183)tmp1=41.3;
        else if(FT1<=37.1738)tmp1=41.4;
        else if(FT1<=37.2294)tmp1=41.5;
        else if(FT1<=37.2849)tmp1=41.6;
        else if(FT1<=37.3406)tmp1=41.7;
        else if(FT1<=37.3963)tmp1=41.8;
        else if(FT1<=37.452)tmp1=41.9;
    }
    else if(FT1<=38.0121){
        if(FT1==37.5078)tmp1=42;
        else if(FT1<=37.5636)tmp1=42.1;
        else if(FT1<=37.6195)tmp1=42.2;
        else if(FT1<=37.6754)tmp1=42.3;
        else if(FT1<=37.7314)tmp1=42.4;
        else if(FT1<=37.7874)tmp1=42.5;
        else if(FT1<=37.8435)tmp1=42.6;
        else if(FT1<=37.8997)tmp1=42.7;
        else if(FT1<=37.9558)tmp1=42.8;
        else if(FT1<=38.0121)tmp1=42.9;
    }
    else if(FT1<=38.5771){
        if(FT1==38.0683)tmp1=43;
        else if(FT1<=38.1247)tmp1=43.1;
        else if(FT1<=38.181)tmp1=43.2;
        else if(FT1<=38.2375)tmp1=43.3;
        else if(FT1<=38.2939)tmp1=43.4;
        else if(FT1<=38.3505)tmp1=43.5;
        else if(FT1<=38.407)tmp1=43.6;
        else if(FT1<=38.4637)tmp1=43.7;
        else if(FT1<=38.5203)tmp1=43.8;
        else if(FT1<=38.5771)tmp1=43.9;
    }
    else if(FT1<=39.147){
        if(FT1==38.6338)tmp1=44;
        else if(FT1<=38.6907)tmp1=44.1;
        else if(FT1<=38.7475)tmp1=44.2;
        else if(FT1<=38.8045)tmp1=44.3;
        else if(FT1<=38.8614)tmp1=44.4;
        else if(FT1<=38.9184)tmp1=44.5;
        else if(FT1<=38.9755)tmp1=44.6;
        else if(FT1<=39.0326)tmp1=44.7;
        else if(FT1<=39.0898)tmp1=44.8;
        else if(FT1<=39.147)tmp1=44.9;
    }
    else if(FT1<=39.7219){
        if(FT1==39.2043)tmp1=45;
        else if(FT1<=39.2616)tmp1=45.1;
        else if(FT1<=39.319)tmp1=45.2;
        else if(FT1<=39.3764)tmp1=45.3;
        else if(FT1<=39.4338)tmp1=45.4;
        else if(FT1<=39.4914)tmp1=45.5;
        else if(FT1<=39.5489)tmp1=45.6;
        else if(FT1<=39.6065)tmp1=45.7;
        else if(FT1<=39.6642)tmp1=45.8;
        else if(FT1<=39.7219)tmp1=45.9;
    }
    else if(FT1<=40.3018){
        if(FT1==39.7797)tmp1=46;
        else if(FT1<=39.8375)tmp1=46.1;
        else if(FT1<=39.8954)tmp1=46.2;
        else if(FT1<=39.9533)tmp1=46.3;
        else if(FT1<=40.0112)tmp1=46.4;
        else if(FT1<=40.0692)tmp1=46.5;
        else if(FT1<=40.1273)tmp1=46.6;
        else if(FT1<=40.1854)tmp1=46.7;
        else if(FT1<=40.2436)tmp1=46.8;
        else if(FT1<=40.3018)tmp1=46.9;
    }
    else if(FT1<=40.8866){
        if(FT1==40.36)tmp1=47;
        else if(FT1<=40.4183)tmp1=47.1;
        else if(FT1<=40.4767)tmp1=47.2;
        else if(FT1<=40.5351)tmp1=47.3;
        else if(FT1<=40.5936)tmp1=47.4;
        else if(FT1<=40.6521)tmp1=47.5;
        else if(FT1<=40.7106)tmp1=47.6;
        else if(FT1<=40.7692)tmp1=47.7;
        else if(FT1<=40.8279)tmp1=47.8;
        else if(FT1<=40.8866)tmp1=47.9;
    }
    else if(FT1<=41.4764){
        if(FT1==40.9453)tmp1=48;
        else if(FT1<=41.0041)tmp1=48.1;
        else if(FT1<=41.063)tmp1=48.2;
        else if(FT1<=41.1219)tmp1=48.3;
        else if(FT1<=41.1809)tmp1=48.4;
        else if(FT1<=41.2399)tmp1=48.5;
        else if(FT1<=41.2989)tmp1=48.6;
        else if(FT1<=41.358)tmp1=48.7;
        else if(FT1<=41.4172)tmp1=48.8;
        else if(FT1<=41.4764)tmp1=48.9;
    }
    else if(FT1<=42.0711){
        if(FT1==41.5356)tmp1=49;
        else if(FT1<=41.5949)tmp1=49.1;
        else if(FT1<=41.6543)tmp1=49.2;
        else if(FT1<=41.7137)tmp1=49.3;
        else if(FT1<=41.7731)tmp1=49.4;
        else if(FT1<=41.8326)tmp1=49.5;
        else if(FT1<=41.8922)tmp1=49.6;
        else if(FT1<=41.9518)tmp1=49.7;
        else if(FT1<=42.0114)tmp1=49.8;
        else if(FT1<=42.0711)tmp1=49.9;
    }
    else if(FT1<=42.6708){
        if(FT1==42.1308)tmp1=50;
        else if(FT1<=42.1906)tmp1=50.1;
        else if(FT1<=42.2505)tmp1=50.2;
        else if(FT1<=42.3104)tmp1=50.3;
        else if(FT1<=42.3703)tmp1=50.4;
        else if(FT1<=42.4303)tmp1=50.5;
        else if(FT1<=42.4904)tmp1=50.6;
        else if(FT1<=42.5505)tmp1=50.7;
        else if(FT1<=42.6106)tmp1=50.8;
        else if(FT1<=42.6708)tmp1=50.9;
    }
    else if(FT1<=43.2755){
        if(FT1==42.731)tmp1=51;
        else if(FT1<=42.7913)tmp1=51.1;
        else if(FT1<=42.8517)tmp1=51.2;
        else if(FT1<=42.9121)tmp1=51.3;
        else if(FT1<=42.9725)tmp1=51.4;
        else if(FT1<=43.033)tmp1=51.5;
        else if(FT1<=43.0936)tmp1=51.6;
        else if(FT1<=43.1541)tmp1=51.7;
        else if(FT1<=43.2148)tmp1=51.8;
        else if(FT1<=43.2755)tmp1=51.9;
    }
    else if(FT1<=43.8851){
        if(FT1==43.3362)tmp1=52;
        else if(FT1<=43.397)tmp1=52.1;
        else if(FT1<=43.4579)tmp1=52.2;
        else if(FT1<=43.5187)tmp1=52.3;
        else if(FT1<=43.5797)tmp1=52.4;
        else if(FT1<=43.6407)tmp1=52.5;
        else if(FT1<=43.7017)tmp1=52.6;
        else if(FT1<=43.7628)tmp1=52.7;
        else if(FT1<=43.8239)tmp1=52.8;
        else if(FT1<=43.8851)tmp1=52.9;
    }
    else if(FT1<=44.4998){
        if(FT1==43.9464)tmp1=53;
        else if(FT1<=44.0077)tmp1=53.1;
        else if(FT1<=44.069)tmp1=53.2;
        else if(FT1<=44.1304)tmp1=53.3;
        else if(FT1<=44.1918)tmp1=53.4;
        else if(FT1<=44.2533)tmp1=53.5;
        else if(FT1<=44.3148)tmp1=53.6;
        else if(FT1<=44.3764)tmp1=53.7;
        else if(FT1<=44.4381)tmp1=53.8;
        else if(FT1<=44.4998)tmp1=53.9;
    }
    else if(FT1<=45.1194){
        if(FT1==44.5615)tmp1=54;
        else if(FT1<=44.6233)tmp1=54.1;
        else if(FT1<=44.6851)tmp1=54.2;
        else if(FT1<=44.747)tmp1=54.3;
        else if(FT1<=44.8089)tmp1=54.4;
        else if(FT1<=44.8709)tmp1=54.5;
        else if(FT1<=44.933)tmp1=54.6;
        else if(FT1<=44.995)tmp1=54.7;
        else if(FT1<=45.0572)tmp1=54.8;
        else if(FT1<=45.1194)tmp1=54.9;
    }
    else if(FT1<=45.7439){
        if(FT1==45.1816)tmp1=55;
        else if(FT1<=45.2439)tmp1=55.1;
        else if(FT1<=45.3062)tmp1=55.2;
        else if(FT1<=45.3686)tmp1=55.3;
        else if(FT1<=45.431)tmp1=55.4;
        else if(FT1<=45.4935)tmp1=55.5;
        else if(FT1<=45.556)tmp1=55.6;
        else if(FT1<=45.6186)tmp1=55.7;
        else if(FT1<=45.6813)tmp1=55.8;
        else if(FT1<=45.7439)tmp1=55.9;
    }
    else if(FT1<=46.3735){
        if(FT1==45.8067)tmp1=56;
        else if(FT1<=45.8695)tmp1=56.1;
        else if(FT1<=45.9323)tmp1=56.2;
        else if(FT1<=45.9952)tmp1=56.3;
        else if(FT1<=46.0581)tmp1=56.4;
        else if(FT1<=46.1211)tmp1=56.5;
        else if(FT1<=46.1841)tmp1=56.6;
        else if(FT1<=46.2472)tmp1=56.7;
        else if(FT1<=46.3103)tmp1=56.8;
        else if(FT1<=46.3735)tmp1=56.9;
    }
    else if(FT1<=47.008){
        if(FT1==46.4367)tmp1=57;
        else if(FT1<=46.5)tmp1=57.1;
        else if(FT1<=46.5633)tmp1=57.2;
        else if(FT1<=46.6267)tmp1=57.3;
        else if(FT1<=46.6902)tmp1=57.4;
        else if(FT1<=46.7536)tmp1=57.5;
        else if(FT1<=46.8172)tmp1=57.6;
        else if(FT1<=46.8807)tmp1=57.7;
        else if(FT1<=46.9444)tmp1=57.8;
        else if(FT1<=47.008)tmp1=57.9;
    }
    else if(FT1<=47.6476){
        if(FT1==47.0718)tmp1=58;
        else if(FT1<=47.1356)tmp1=58.1;
        else if(FT1<=47.1994)tmp1=58.2;
        else if(FT1<=47.2633)tmp1=58.3;
        else if(FT1<=47.3272)tmp1=58.4;
        else if(FT1<=47.3912)tmp1=58.5;
        else if(FT1<=47.4552)tmp1=58.6;
        else if(FT1<=47.5193)tmp1=58.7;
        else if(FT1<=47.5834)tmp1=58.8;
        else if(FT1<=47.6476)tmp1=58.9;
    }
    else if(FT1<=48.2921){
        if(FT1==47.7118)tmp1=59;
        else if(FT1<=47.7761)tmp1=59.1;
        else if(FT1<=47.8404)tmp1=59.2;
        else if(FT1<=47.9048)tmp1=59.3;
        else if(FT1<=47.9692)tmp1=59.4;
        else if(FT1<=48.0337)tmp1=59.5;
        else if(FT1<=48.0982)tmp1=59.6;
        else if(FT1<=48.1628)tmp1=59.7;
        else if(FT1<=48.2274)tmp1=59.8;
        else if(FT1<=48.2921)tmp1=59.9;
    }
    else if(FT1<=48.9416){
        if(FT1==48.3568)tmp1=60;
        else if(FT1<=48.4216)tmp1=60.1;
        else if(FT1<=48.4864)tmp1=60.2;
        else if(FT1<=48.5513)tmp1=60.3;
        else if(FT1<=48.6162)tmp1=60.4;
        else if(FT1<=48.6812)tmp1=60.5;
        else if(FT1<=48.7462)tmp1=60.6;
        else if(FT1<=48.8113)tmp1=60.7;
        else if(FT1<=48.8764)tmp1=60.8;
        else if(FT1<=48.9416)tmp1=60.9;
    }
    else if(FT1<=49.5961){
        if(FT1==49.0068)tmp1=61;
        else if(FT1<=49.0721)tmp1=61.1;
        else if(FT1<=49.1374)tmp1=61.2;
        else if(FT1<=49.2028)tmp1=61.3;
        else if(FT1<=49.2682)tmp1=61.4;
        else if(FT1<=49.3337)tmp1=61.5;
        else if(FT1<=49.3992)tmp1=61.6;
        else if(FT1<=49.4648)tmp1=61.7;
        else if(FT1<=49.5304)tmp1=61.8;
        else if(FT1<=49.5961)tmp1=61.9;
    }
    else if(FT1<=50.2556){
        if(FT1==49.6618)tmp1=62;
        else if(FT1<=49.7276)tmp1=62.1;
        else if(FT1<=49.7934)tmp1=62.2;
        else if(FT1<=49.8593)tmp1=62.3;
        else if(FT1<=49.9252)tmp1=62.4;
        else if(FT1<=49.9912)tmp1=62.5;
        else if(FT1<=50.0572)tmp1=62.6;
        else if(FT1<=50.1233)tmp1=62.7;
        else if(FT1<=50.1894)tmp1=62.8;
        else if(FT1<=50.2556)tmp1=62.9;
    }
    else if(FT1<=50.92){
        if(FT1==50.3218)tmp1=63;
        else if(FT1<=50.388)tmp1=63.1;
        else if(FT1<=50.4544)tmp1=63.2;
        else if(FT1<=50.5207)tmp1=63.3;
        else if(FT1<=50.5872)tmp1=63.4;
        else if(FT1<=50.6536)tmp1=63.5;
        else if(FT1<=50.7202)tmp1=63.6;
        else if(FT1<=50.7867)tmp1=63.7;
        else if(FT1<=50.8533)tmp1=63.8;
        else if(FT1<=50.92)tmp1=63.9;
    }
    else if(FT1<=51.5895){
        if(FT1==50.9867)tmp1=64;
        else if(FT1<=51.0535)tmp1=64.1;
        else if(FT1<=51.1203)tmp1=64.2;
        else if(FT1<=51.1872)tmp1=64.3;
        else if(FT1<=51.2541)tmp1=64.4;
        else if(FT1<=51.3211)tmp1=64.5;
        else if(FT1<=51.3881)tmp1=64.6;
        else if(FT1<=51.4552)tmp1=64.7;
        else if(FT1<=51.5223)tmp1=64.8;
        else if(FT1<=51.5895)tmp1=64.9;
        else if(FT1<=51.6567)tmp1=65;
    }
    else if(FT1<=52.2639){
        if(FT1==51.6567)tmp1=65;
        else if(FT1<=51.724)tmp1=65.1;
        else if(FT1<=51.7913)tmp1=65.2;
        else if(FT1<=51.8586)tmp1=65.3;
        else if(FT1<=51.9261)tmp1=65.4;
        else if(FT1<=51.9935)tmp1=65.5;
        else if(FT1<=52.061)tmp1=65.6;
        else if(FT1<=52.1286)tmp1=65.7;
        else if(FT1<=52.1962)tmp1=65.8;
        else if(FT1<=52.2639)tmp1=65.9;
        else if(FT1<=52.3316)tmp1=66;
    }
    else if(FT1<=52.9433){
        if(FT1==52.3316)tmp1=66;
        else if(FT1<=52.3994)tmp1=66.1;
        else if(FT1<=52.4672)tmp1=66.2;
        else if(FT1<=52.5351)tmp1=66.3;
        else if(FT1<=52.603)tmp1=66.4;
        else if(FT1<=52.671)tmp1=66.5;
        else if(FT1<=52.739)tmp1=66.6;
        else if(FT1<=52.807)tmp1=66.7;
        else if(FT1<=52.8752)tmp1=66.8;
        else if(FT1<=52.9433)tmp1=66.9;
        else if(FT1<=53.0115)tmp1=67;
    }
    else if(FT1<=53.6277){
        if(FT1==53.0115)tmp1=67;
        else if(FT1<=53.0798)tmp1=67.1;
        else if(FT1<=53.1481)tmp1=67.2;
        else if(FT1<=53.2165)tmp1=67.3;
        else if(FT1<=53.2849)tmp1=67.4;
        else if(FT1<=53.3534)tmp1=67.5;
        else if(FT1<=53.4219)tmp1=67.6;
        else if(FT1<=53.4905)tmp1=67.7;
        else if(FT1<=53.5591)tmp1=67.8;
        else if(FT1<=53.6277)tmp1=67.9;
        else if(FT1<=53.6965)tmp1=68;
    }
    else if(FT1<=54.3171){
        if(FT1==53.6965)tmp1=68;
        else if(FT1<=53.7652)tmp1=68.1;
        else if(FT1<=53.834)tmp1=68.2;
        else if(FT1<=53.9029)tmp1=68.3;
        else if(FT1<=53.9718)tmp1=68.4;
        else if(FT1<=54.0408)tmp1=68.5;
        else if(FT1<=54.1098)tmp1=68.6;
        else if(FT1<=54.1789)tmp1=68.7;
        else if(FT1<=54.248)tmp1=68.8;
        else if(FT1<=54.3171)tmp1=68.9;
        else if(FT1<=54.3863)tmp1=69;
    }
    else if(FT1<=55.0115){
        if(FT1==54.3863)tmp1=69;
        else if(FT1<=54.4556)tmp1=69.1;
        else if(FT1<=54.5249)tmp1=69.2;
        else if(FT1<=54.5943)tmp1=69.3;
        else if(FT1<=54.6637)tmp1=69.4;
        else if(FT1<=54.7332)tmp1=69.5;
        else if(FT1<=54.8027)tmp1=69.6;
        else if(FT1<=54.8722)tmp1=69.7;
        else if(FT1<=54.9419)tmp1=69.8;
        else if(FT1<=55.0115)tmp1=69.9;
        else if(FT1<=55.0812)tmp1=70;
    }
    else if(FT1<=55.7109){
        if(FT1==55.0812)tmp1=70;
        else if(FT1<=55.151)tmp1=70.1;
        else if(FT1<=55.2208)tmp1=70.2;
        else if(FT1<=55.2907)tmp1=70.3;
        else if(FT1<=55.3606)tmp1=70.4;
        else if(FT1<=55.4305)tmp1=70.5;
        else if(FT1<=55.5005)tmp1=70.6;
        else if(FT1<=55.5706)tmp1=70.7;
        else if(FT1<=55.6407)tmp1=70.8;
        else if(FT1<=55.7109)tmp1=70.9;
        else if(FT1<=55.7811)tmp1=71;
    }
    else if(FT1<=56.4152){
        if(FT1==55.7811)tmp1=71;
        else if(FT1<=55.8514)tmp1=71.1;
        else if(FT1<=55.9217)tmp1=71.2;
        else if(FT1<=55.992)tmp1=71.3;
        else if(FT1<=56.0624)tmp1=71.4;
        else if(FT1<=56.1329)tmp1=71.5;
        else if(FT1<=56.2034)tmp1=71.6;
        else if(FT1<=56.274)tmp1=71.7;
        else if(FT1<=56.3446)tmp1=71.8;
        else if(FT1<=56.4152)tmp1=71.9;
        else if(FT1<=56.4859)tmp1=72;
    }
    else if(FT1<=57.1246){
        if(FT1==56.4859)tmp1=72;
        else if(FT1<=56.5567)tmp1=72.1;
        else if(FT1<=56.6275)tmp1=72.2;
        else if(FT1<=56.6984)tmp1=72.3;
        else if(FT1<=56.7693)tmp1=72.4;
        else if(FT1<=56.8402)tmp1=72.5;
        else if(FT1<=56.9112)tmp1=72.6;
        else if(FT1<=56.9823)tmp1=72.7;
        else if(FT1<=57.0534)tmp1=72.8;
        else if(FT1<=57.1246)tmp1=72.9;
        else if(FT1<=57.1958)tmp1=73;
    }
    else if(FT1<=57.8389){
        if(FT1==57.1958)tmp1=73;
        else if(FT1<=57.267)tmp1=73.1;
        else if(FT1<=57.3383)tmp1=73.2;
        else if(FT1<=57.4097)tmp1=73.3;
        else if(FT1<=57.4811)tmp1=73.4;
        else if(FT1<=57.5526)tmp1=73.5;
        else if(FT1<=57.6241)tmp1=73.6;
        else if(FT1<=57.6956)tmp1=73.7;
        else if(FT1<=57.7672)tmp1=73.8;
        else if(FT1<=57.8389)tmp1=73.9;
        else if(FT1<=57.9106)tmp1=74;
    }
    else if(FT1<=58.5582){
        if(FT1==57.9106)tmp1=74;
        else if(FT1<=57.9823)tmp1=74.1;
        else if(FT1<=58.0541)tmp1=74.2;
        else if(FT1<=58.126)tmp1=74.3;
        else if(FT1<=58.1979)tmp1=74.4;
        else if(FT1<=58.2699)tmp1=74.5;
        else if(FT1<=58.3419)tmp1=74.6;
        else if(FT1<=58.4139)tmp1=74.7;
        else if(FT1<=58.486)tmp1=74.8;
        else if(FT1<=58.5582)tmp1=74.9;
        else if(FT1<=58.6304)tmp1=75;
    }
    else if(FT1<=59.2824){
        if(FT1==58.6304)tmp1=75;
        else if(FT1<=58.7026)tmp1=75.1;
        else if(FT1<=58.7749)tmp1=75.2;
        else if(FT1<=58.8473)tmp1=75.3;
        else if(FT1<=58.9197)tmp1=75.4;
        else if(FT1<=58.9921)tmp1=75.5;
        else if(FT1<=59.0646)tmp1=75.6;
        else if(FT1<=59.1372)tmp1=75.7;
        else if(FT1<=59.2098)tmp1=75.8;
        else if(FT1<=59.2824)tmp1=75.9;
        else if(FT1<=59.3551)tmp1=76;
    }
    else if(FT1<=60.0117){
        if(FT1==59.3551)tmp1=76;
        else if(FT1<=59.4279)tmp1=76.1;
        else if(FT1<=59.5007)tmp1=76.2;
        else if(FT1<=59.5735)tmp1=76.3;
        else if(FT1<=59.6464)tmp1=76.4;
        else if(FT1<=59.7194)tmp1=76.5;
        else if(FT1<=59.7924)tmp1=76.6;
        else if(FT1<=59.8654)tmp1=76.7;
        else if(FT1<=59.9385)tmp1=76.8;
        else if(FT1<=60.0117)tmp1=76.9;
        else if(FT1<=60.0849)tmp1=77;
    }
    else if(FT1<=60.7459){
        if(FT1==60.0849)tmp1=77;
        else if(FT1<=60.1581)tmp1=77.1;
        else if(FT1<=60.2314)tmp1=77.2;
        else if(FT1<=60.3048)tmp1=77.3;
        else if(FT1<=60.3782)tmp1=77.4;
        else if(FT1<=60.4516)tmp1=77.5;
        else if(FT1<=60.5251)tmp1=77.6;
        else if(FT1<=60.5987)tmp1=77.7;
        else if(FT1<=60.6722)tmp1=77.8;
        else if(FT1<=60.7459)tmp1=77.9;
        else if(FT1<=60.8196)tmp1=78;
    }
    else if(FT1<=61.4851){
        if(FT1==60.8196)tmp1=78;
        else if(FT1<=60.8933)tmp1=78.1;
        else if(FT1<=60.9671)tmp1=78.2;
        else if(FT1<=61.041)tmp1=78.3;
        else if(FT1<=61.1149)tmp1=78.4;
        else if(FT1<=61.1888)tmp1=78.5;
        else if(FT1<=61.2628)tmp1=78.6;
        else if(FT1<=61.3368)tmp1=78.7;
        else if(FT1<=61.4109)tmp1=78.8;
        else if(FT1<=61.4851)tmp1=78.9;
        else if(FT1<=61.5593)tmp1=79;
    }
    else if(FT1<=62.2292){
        if(FT1==61.5593)tmp1=79;
        else if(FT1<=61.6335)tmp1=79.1;
        else if(FT1<=61.7078)tmp1=79.2;
        else if(FT1<=61.7821)tmp1=79.3;
        else if(FT1<=61.8565)tmp1=79.4;
        else if(FT1<=61.931)tmp1=79.5;
        else if(FT1<=62.0055)tmp1=79.6;
        else if(FT1<=62.08)tmp1=79.7;
        else if(FT1<=62.1546)tmp1=79.8;
        else if(FT1<=62.2292)tmp1=79.9;
        else if(FT1<=62.3039)tmp1=80;
    }
    else if(FT1<=62.9783){
        if(FT1==62.3039)tmp1=80;
        else if(FT1<=62.3787)tmp1=80.1;
        else if(FT1<=62.4534)tmp1=80.2;
        else if(FT1<=62.5283)tmp1=80.3;
        else if(FT1<=62.6032)tmp1=80.4;
        else if(FT1<=62.6781)tmp1=80.5;
        else if(FT1<=62.7531)tmp1=80.6;
        else if(FT1<=62.8281)tmp1=80.7;
        else if(FT1<=62.9032)tmp1=80.8;
        else if(FT1<=62.9783)tmp1=80.9;
        else if(FT1<=63.0535)tmp1=81;
    }
    else if(FT1<=63.7324){
        if(FT1==63.0535)tmp1=81;
        else if(FT1<=63.1288)tmp1=81.1;
        else if(FT1<=63.2041)tmp1=81.2;
        else if(FT1<=63.2794)tmp1=81.3;
        else if(FT1<=63.3548)tmp1=81.4;
        else if(FT1<=63.4302)tmp1=81.5;
        else if(FT1<=63.5057)tmp1=81.6;
        else if(FT1<=63.5812)tmp1=81.7;
        else if(FT1<=63.6568)tmp1=81.8;
        else if(FT1<=63.7324)tmp1=81.9;
        else if(FT1<=63.8081)tmp1=82;
    }
    else if(FT1<=64.4915){
        if(FT1==63.8081)tmp1=82;
        else if(FT1<=63.8838)tmp1=82.1;
        else if(FT1<=63.9596)tmp1=82.2;
        else if(FT1<=64.0354)tmp1=82.3;
        else if(FT1<=64.1113)tmp1=82.4;
        else if(FT1<=64.1873)tmp1=82.5;
        else if(FT1<=64.2632)tmp1=82.6;
        else if(FT1<=64.3393)tmp1=82.7;
        else if(FT1<=64.4153)tmp1=82.8;
        else if(FT1<=64.4915)tmp1=82.9;
        else if(FT1<=64.5676)tmp1=83;
    }
    else if(FT1<=65.2555){
        if(FT1==64.5676)tmp1=83;
        else if(FT1<=64.6439)tmp1=83.1;
        else if(FT1<=64.7201)tmp1=83.2;
        else if(FT1<=64.7965)tmp1=83.3;
        else if(FT1<=64.8728)tmp1=83.4;
        else if(FT1<=64.9493)tmp1=83.5;
        else if(FT1<=65.0257)tmp1=83.6;
        else if(FT1<=65.1023)tmp1=83.7;
        else if(FT1<=65.1788)tmp1=83.8;
        else if(FT1<=65.2555)tmp1=83.9;
        else if(FT1<=65.3321)tmp1=84;
    }
    else if(FT1<=66.0244){
        if(FT1==65.3321)tmp1=84;
        else if(FT1<=65.4089)tmp1=84.1;
        else if(FT1<=65.4856)tmp1=84.2;
        else if(FT1<=65.5624)tmp1=84.3;
        else if(FT1<=65.6393)tmp1=84.4;
        else if(FT1<=65.7162)tmp1=84.5;
        else if(FT1<=65.7932)tmp1=84.6;
        else if(FT1<=65.8702)tmp1=84.7;
        else if(FT1<=65.9473)tmp1=84.8;
        else if(FT1<=66.0244)tmp1=84.9;
        else if(FT1<=66.1016)tmp1=85;
    }
    else if(FT1<=66.7983){
        if(FT1==66.1016)tmp1=85;
        else if(FT1<=66.1788)tmp1=85.1;
        else if(FT1<=66.2561)tmp1=85.2;
        else if(FT1<=66.3334)tmp1=85.3;
        else if(FT1<=66.4107)tmp1=85.4;
        else if(FT1<=66.4882)tmp1=85.5;
        else if(FT1<=66.5656)tmp1=85.6;
        else if(FT1<=66.6431)tmp1=85.7;
        else if(FT1<=66.7207)tmp1=85.8;
        else if(FT1<=66.7983)tmp1=85.9;
        else if(FT1<=66.876)tmp1=86;
    }
    else if(FT1<=67.5771){
        if(FT1==66.876)tmp1=86;
        else if(FT1<=66.9537)tmp1=86.1;
        else if(FT1<=67.0314)tmp1=86.2;
        else if(FT1<=67.1093)tmp1=86.3;
        else if(FT1<=67.1871)tmp1=86.4;
        else if(FT1<=67.265)tmp1=86.5;
        else if(FT1<=67.343)tmp1=86.6;
        else if(FT1<=67.421)tmp1=86.7;
        else if(FT1<=67.499)tmp1=86.8;
        else if(FT1<=67.5771)tmp1=86.9;
        else if(FT1<=67.6553)tmp1=87;
    }
    else if(FT1<=68.3609){
        if(FT1==67.6553)tmp1=87;
        else if(FT1<=67.7335)tmp1=87.1;
        else if(FT1<=67.8118)tmp1=87.2;
        else if(FT1<=67.8901)tmp1=87.3;
        else if(FT1<=67.9684)tmp1=87.4;
        else if(FT1<=68.0468)tmp1=87.5;
        else if(FT1<=68.1253)tmp1=87.6;
        else if(FT1<=68.2038)tmp1=87.7;
        else if(FT1<=68.2823)tmp1=87.8;
        else if(FT1<=68.3609)tmp1=87.9;
        else if(FT1<=68.4396)tmp1=88;
    }
    else if(FT1<=69.1497){
        if(FT1==68.4396)tmp1=88;
        else if(FT1<=68.5183)tmp1=88.1;
        else if(FT1<=68.597)tmp1=88.2;
        else if(FT1<=68.6758)tmp1=88.3;
        else if(FT1<=68.7547)tmp1=88.4;
        else if(FT1<=68.8336)tmp1=88.5;
        else if(FT1<=68.9125)tmp1=88.6;
        else if(FT1<=68.9915)tmp1=88.7;
        else if(FT1<=69.0706)tmp1=88.8;
        else if(FT1<=69.1497)tmp1=88.9;
        else if(FT1<=69.2288)tmp1=89;
    }
    else if(FT1<=69.9433){
        if(FT1==69.2288)tmp1=89;
        else if(FT1<=69.308)tmp1=89.1;
        else if(FT1<=69.3872)tmp1=89.2;
        else if(FT1<=69.4665)tmp1=89.3;
        else if(FT1<=69.5459)tmp1=89.4;
        else if(FT1<=69.6253)tmp1=89.5;
        else if(FT1<=69.7047)tmp1=89.6;
        else if(FT1<=69.7842)tmp1=89.7;
        else if(FT1<=69.8637)tmp1=89.8;
        else if(FT1<=69.9433)tmp1=89.9;
        else if(FT1<=70.023)tmp1=90;
    }
    else if(FT1<=70.7419){
        if(FT1==70.023)tmp1=90;
        else if(FT1<=70.1026)tmp1=90.1;
        else if(FT1<=70.1824)tmp1=90.2;
        else if(FT1<=70.2622)tmp1=90.3;
        else if(FT1<=70.342)tmp1=90.4;
        else if(FT1<=70.4219)tmp1=90.5;
        else if(FT1<=70.5018)tmp1=90.6;
        else if(FT1<=70.5818)tmp1=90.7;
        else if(FT1<=70.6618)tmp1=90.8;
        else if(FT1<=70.7419)tmp1=90.9;
        else if(FT1<=70.822)tmp1=91;
    }
    else if(FT1<=71.5454){
        if(FT1==70.822)tmp1=91;
        else if(FT1<=70.9022)tmp1=91.1;
        else if(FT1<=70.9824)tmp1=91.2;
        else if(FT1<=71.0627)tmp1=91.3;
        else if(FT1<=71.143)tmp1=91.4;
        else if(FT1<=71.2234)tmp1=91.5;
        else if(FT1<=71.3038)tmp1=91.6;
        else if(FT1<=71.3843)tmp1=91.7;
        else if(FT1<=71.4648)tmp1=91.8;
        else if(FT1<=71.5454)tmp1=91.9;
        else if(FT1<=71.626)tmp1=92;
    }
    else if(FT1<=72.3538){
        if(FT1==71.626)tmp1=92;
        else if(FT1<=71.7067)tmp1=92.1;
        else if(FT1<=71.7874)tmp1=92.2;
        else if(FT1<=71.8682)tmp1=92.3;
        else if(FT1<=71.949)tmp1=92.4;
        else if(FT1<=72.0299)tmp1=92.5;
        else if(FT1<=72.1108)tmp1=92.6;
        else if(FT1<=72.1918)tmp1=92.7;
        else if(FT1<=72.2728)tmp1=92.8;
        else if(FT1<=72.3538)tmp1=92.9;
        else if(FT1<=72.435)tmp1=93;
    }
    else if(FT1<=73.1672){
        if(FT1==72.435)tmp1=93;
        else if(FT1<=72.5161)tmp1=93.1;
        else if(FT1<=72.5973)tmp1=93.2;
        else if(FT1<=72.6786)tmp1=93.3;
        else if(FT1<=72.7599)tmp1=93.4;
        else if(FT1<=72.8413)tmp1=93.5;
        else if(FT1<=72.9227)tmp1=93.6;
        else if(FT1<=73.0041)tmp1=93.7;
        else if(FT1<=73.0856)tmp1=93.8;
        else if(FT1<=73.1672)tmp1=93.9;
        else if(FT1<=73.2488)tmp1=94;
    }
    else if(FT1<=73.9855){
        if(FT1==73.2488)tmp1=94;
        else if(FT1<=73.3304)tmp1=94.1;
        else if(FT1<=73.4122)tmp1=94.2;
        else if(FT1<=73.4939)tmp1=94.3;
        else if(FT1<=73.5757)tmp1=94.4;
        else if(FT1<=73.6576)tmp1=94.5;
        else if(FT1<=73.7395)tmp1=94.6;
        else if(FT1<=73.8214)tmp1=94.7;
        else if(FT1<=73.9034)tmp1=94.8;
        else if(FT1<=73.9855)tmp1=94.9;
        else if(FT1<=74.0676)tmp1=95;
    }
    else if(FT1<=74.8086){
        if(FT1==74.0676)tmp1=95;
        else if(FT1<=74.1497)tmp1=95.1;
        else if(FT1<=74.2319)tmp1=95.2;
        else if(FT1<=74.3141)tmp1=95.3;
        else if(FT1<=74.3964)tmp1=95.4;
        else if(FT1<=74.4788)tmp1=95.5;
        else if(FT1<=74.5612)tmp1=95.6;
        else if(FT1<=74.6436)tmp1=95.7;
        else if(FT1<=74.7261)tmp1=95.8;
        else if(FT1<=74.8086)tmp1=95.9;
        else if(FT1<=74.8912)tmp1=96;
    }
    else if(FT1<=75.6367){
        if(FT1==74.8912)tmp1=96;
        else if(FT1<=74.9738)tmp1=96.1;
        else if(FT1<=75.0565)tmp1=96.2;
        else if(FT1<=75.1393)tmp1=96.3;
        else if(FT1<=75.2221)tmp1=96.4;
        else if(FT1<=75.3049)tmp1=96.5;
        else if(FT1<=75.3878)tmp1=96.6;
        else if(FT1<=75.4707)tmp1=96.7;
        else if(FT1<=75.5537)tmp1=96.8;
        else if(FT1<=75.6367)tmp1=96.9;
        else if(FT1<=75.7198)tmp1=97;
    }
    else if(FT1<=76.4697){
        if(FT1==75.7198)tmp1=97;
        else if(FT1<=75.8029)tmp1=97.1;
        else if(FT1<=75.8861)tmp1=97.2;
        else if(FT1<=75.9693)tmp1=97.3;
        else if(FT1<=76.0526)tmp1=97.4;
        else if(FT1<=76.1359)tmp1=97.5;
        else if(FT1<=76.2193)tmp1=97.6;
        else if(FT1<=76.3027)tmp1=97.7;
        else if(FT1<=76.3862)tmp1=97.8;
        else if(FT1<=76.4697)tmp1=97.9;
        else if(FT1<=76.5532)tmp1=98;
    }
    else if(FT1<=77.3075){
        if(FT1==76.5532)tmp1=98;
        else if(FT1<=76.6368)tmp1=98.1;
        else if(FT1<=76.7205)tmp1=98.2;
        else if(FT1<=76.8042)tmp1=98.3;
        else if(FT1<=76.888)tmp1=98.4;
        else if(FT1<=76.9718)tmp1=98.5;
        else if(FT1<=77.0556)tmp1=98.6;
        else if(FT1<=77.1396)tmp1=98.7;
        else if(FT1<=77.2235)tmp1=98.8;
        else if(FT1<=77.3075)tmp1=98.9;
        else if(FT1<=77.3916)tmp1=99;
    }
    else if(FT1<=78.1503){
        if(FT1==77.3916)tmp1=99;
        else if(FT1<=77.4757)tmp1=99.1;
        else if(FT1<=77.5598)tmp1=99.2;
        else if(FT1<=77.644)tmp1=99.3;
        else if(FT1<=77.7283)tmp1=99.4;
        else if(FT1<=77.8126)tmp1=99.5;
        else if(FT1<=77.8969)tmp1=99.6;
        else if(FT1<=77.9813)tmp1=99.7;
        else if(FT1<=78.0658)tmp1=99.8;
        else if(FT1<=78.1503)tmp1=99.9;
        else if(FT1<=78.2348)tmp1=100;
    }
    else if(FT1<=78.9979){
        if(FT1==78.2348)tmp1=100;
        else if(FT1<=78.3194)tmp1=100.1;
        else if(FT1<=78.4041)tmp1=100.2;
        else if(FT1<=78.4887)tmp1=100.3;
        else if(FT1<=78.5735)tmp1=100.4;
        else if(FT1<=78.6583)tmp1=100.5;
        else if(FT1<=78.7431)tmp1=100.6;
        else if(FT1<=78.828)tmp1=100.7;
        else if(FT1<=78.9129)tmp1=100.8;
        else if(FT1<=78.9979)tmp1=100.9;
        else if(FT1<=79.0829)tmp1=101;
    }
    else if(FT1<=79.8504){
        if(FT1==79.0829)tmp1=101;
        else if(FT1<=79.168)tmp1=101.1;
        else if(FT1<=79.2531)tmp1=101.2;
        else if(FT1<=79.3383)tmp1=101.3;
        else if(FT1<=79.4236)tmp1=101.4;
        else if(FT1<=79.5088)tmp1=101.5;
        else if(FT1<=79.5941)tmp1=101.6;
        else if(FT1<=79.6795)tmp1=101.7;
        else if(FT1<=79.7649)tmp1=101.8;
        else if(FT1<=79.8504)tmp1=101.9;
        else if(FT1<=79.9359)tmp1=102;
    }
    else if(FT1<=80.7078){
        if(FT1==79.9359)tmp1=102;
        else if(FT1<=80.0215)tmp1=102.1;
        else if(FT1<=80.1071)tmp1=102.2;
        else if(FT1<=80.1928)tmp1=102.3;
        else if(FT1<=80.2785)tmp1=102.4;
        else if(FT1<=80.3643)tmp1=102.5;
        else if(FT1<=80.4501)tmp1=102.6;
        else if(FT1<=80.5359)tmp1=102.7;
        else if(FT1<=80.6218)tmp1=102.8;
        else if(FT1<=80.7078)tmp1=102.9;
        else if(FT1<=80.7938)tmp1=103;
    }
    else if(FT1<=81.57){
        if(FT1==80.7938)tmp1=103;
        else if(FT1<=80.8798)tmp1=103.1;
        else if(FT1<=80.9659)tmp1=103.2;
        else if(FT1<=81.0521)tmp1=103.3;
        else if(FT1<=81.1383)tmp1=103.4;
        else if(FT1<=81.2245)tmp1=103.5;
        else if(FT1<=81.3108)tmp1=103.6;
        else if(FT1<=81.3972)tmp1=103.7;
        else if(FT1<=81.4836)tmp1=103.8;
        else if(FT1<=81.57)tmp1=103.9;
        else if(FT1<=81.6565)tmp1=104;
    }
    else if(FT1<=82.4371){
        if(FT1==81.6565)tmp1=104;
        else if(FT1<=81.7431)tmp1=104.1;
        else if(FT1<=81.8296)tmp1=104.2;
        else if(FT1<=81.9163)tmp1=104.3;
        else if(FT1<=82.003)tmp1=104.4;
        else if(FT1<=82.0897)tmp1=104.5;
        else if(FT1<=82.1765)tmp1=104.6;
        else if(FT1<=82.2633)tmp1=104.7;
        else if(FT1<=82.3502)tmp1=104.8;
        else if(FT1<=82.4371)tmp1=104.9;
        else if(FT1<=82.5241)tmp1=105;
    }
    else if(FT1<=83.3091){
        if(FT1==82.5241)tmp1=105;
        else if(FT1<=82.6111)tmp1=105.1;
        else if(FT1<=82.6982)tmp1=105.2;
        else if(FT1<=82.7853)tmp1=105.3;
        else if(FT1<=82.8725)tmp1=105.4;
        else if(FT1<=82.9597)tmp1=105.5;
        else if(FT1<=83.047)tmp1=105.6;
        else if(FT1<=83.1343)tmp1=105.7;
        else if(FT1<=83.2217)tmp1=105.8;
        else if(FT1<=83.3091)tmp1=105.9;
        else if(FT1<=83.3965)tmp1=106;
    }
    else if(FT1<=84.1859){
        if(FT1==83.3965)tmp1=106;
        else if(FT1<=83.484)tmp1=106.1;
        else if(FT1<=83.5716)tmp1=106.2;
        else if(FT1<=83.6592)tmp1=106.3;
        else if(FT1<=83.7469)tmp1=106.4;
        else if(FT1<=83.8346)tmp1=106.5;
        else if(FT1<=83.9223)tmp1=106.6;
        else if(FT1<=84.0101)tmp1=106.7;
        else if(FT1<=84.098)tmp1=106.8;
        else if(FT1<=84.1859)tmp1=106.9;
        else if(FT1<=84.2738)tmp1=107;
    }
    else if(FT1<=85.0675){
        if(FT1==84.2738)tmp1=107;
        else if(FT1<=84.3618)tmp1=107.1;
        else if(FT1<=84.4499)tmp1=107.2;
        else if(FT1<=84.5379)tmp1=107.3;
        else if(FT1<=84.6261)tmp1=107.4;
        else if(FT1<=84.7143)tmp1=107.5;
        else if(FT1<=84.8025)tmp1=107.6;
        else if(FT1<=84.8908)tmp1=107.7;
        else if(FT1<=84.9791)tmp1=107.8;
        else if(FT1<=85.0675)tmp1=107.9;
        else if(FT1<=85.1559)tmp1=108;
    }
    else if(FT1<=85.954){
        if(FT1==85.1559)tmp1=108;
        else if(FT1<=85.2444)tmp1=108.1;
        else if(FT1<=85.3329)tmp1=108.2;
        else if(FT1<=85.4215)tmp1=108.3;
        else if(FT1<=85.5101)tmp1=108.4;
        else if(FT1<=85.5988)tmp1=108.5;
        else if(FT1<=85.6875)tmp1=108.6;
        else if(FT1<=85.7763)tmp1=108.7;
        else if(FT1<=85.8651)tmp1=108.8;
        else if(FT1<=85.954)tmp1=108.9;
        else if(FT1<=86.0429)tmp1=109;
    }
    else if(FT1<=86.8453){
        if(FT1==86.0429)tmp1=109;
        else if(FT1<=86.1319)tmp1=109.1;
        else if(FT1<=86.2209)tmp1=109.2;
        else if(FT1<=86.3099)tmp1=109.3;
        else if(FT1<=86.399)tmp1=109.4;
        else if(FT1<=86.4882)tmp1=109.5;
        else if(FT1<=86.5774)tmp1=109.6;
        else if(FT1<=86.6666)tmp1=109.7;
        else if(FT1<=86.7559)tmp1=109.8;
        else if(FT1<=86.8453)tmp1=109.9;
        else if(FT1<=86.9347)tmp1=110;
    }
    else if(FT1<=87.7414){
        if(FT1==86.9347)tmp1=110;
        else if(FT1<=87.0241)tmp1=110.1;
        else if(FT1<=87.1136)tmp1=110.2;
        else if(FT1<=87.2032)tmp1=110.3;
        else if(FT1<=87.2927)tmp1=110.4;
        else if(FT1<=87.3824)tmp1=110.5;
        else if(FT1<=87.4721)tmp1=110.6;
        else if(FT1<=87.5618)tmp1=110.7;
        else if(FT1<=87.6516)tmp1=110.8;
        else if(FT1<=87.7414)tmp1=110.9;
        else if(FT1<=87.8313)tmp1=111;
    }
    else if(FT1<=88.6423){
        if(FT1==87.8313)tmp1=111;
        else if(FT1<=87.9212)tmp1=111.1;
        else if(FT1<=88.0112)tmp1=111.2;
        else if(FT1<=88.1012)tmp1=111.3;
        else if(FT1<=88.1913)tmp1=111.4;
        else if(FT1<=88.2814)tmp1=111.5;
        else if(FT1<=88.3716)tmp1=111.6;
        else if(FT1<=88.4618)tmp1=111.7;
        else if(FT1<=88.552)tmp1=111.8;
        else if(FT1<=88.6423)tmp1=111.9;
        else if(FT1<=88.7327)tmp1=112;
    }
    else if(FT1<=89.5481){
        if(FT1==88.7327)tmp1=112;
        else if(FT1<=88.8231)tmp1=112.1;
        else if(FT1<=88.9136)tmp1=112.2;
        else if(FT1<=89.0041)tmp1=112.3;
        else if(FT1<=89.0946)tmp1=112.4;
        else if(FT1<=89.1852)tmp1=112.5;
        else if(FT1<=89.2759)tmp1=112.6;
        else if(FT1<=89.3665)tmp1=112.7;
        else if(FT1<=89.4573)tmp1=112.8;
        else if(FT1<=89.5481)tmp1=112.9;
        else if(FT1<=89.6389)tmp1=113;
    }
    else if(FT1<=90.4586){
        if(FT1==89.6389)tmp1=113;
        else if(FT1<=89.7298)tmp1=113.1;
        else if(FT1<=89.8207)tmp1=113.2;
        else if(FT1<=89.9117)tmp1=113.3;
        else if(FT1<=90.0028)tmp1=113.4;
        else if(FT1<=90.0938)tmp1=113.5;
        else if(FT1<=90.185)tmp1=113.6;
        else if(FT1<=90.2761)tmp1=113.7;
        else if(FT1<=90.3674)tmp1=113.8;
        else if(FT1<=90.4586)tmp1=113.9;
        else if(FT1<=90.5499)tmp1=114;
    }
    else if(FT1<=91.374){
        if(FT1==90.5499)tmp1=114;
        else if(FT1<=90.6413)tmp1=114.1;
        else if(FT1<=90.7327)tmp1=114.2;
        else if(FT1<=90.8242)tmp1=114.3;
        else if(FT1<=90.9157)tmp1=114.4;
        else if(FT1<=91.0073)tmp1=114.5;
        else if(FT1<=91.0989)tmp1=114.6;
        else if(FT1<=91.1905)tmp1=114.7;
        else if(FT1<=91.2822)tmp1=114.8;
        else if(FT1<=91.374)tmp1=114.9;
        else if(FT1<=91.4658)tmp1=115;
    }
    else if(FT1<=92.2941){
        if(FT1==91.4658)tmp1=115;
        else if(FT1<=91.5576)tmp1=115.1;
        else if(FT1<=91.6495)tmp1=115.2;
        else if(FT1<=91.7414)tmp1=115.3;
        else if(FT1<=91.8334)tmp1=115.4;
        else if(FT1<=91.9255)tmp1=115.5;
        else if(FT1<=92.0176)tmp1=115.6;
        else if(FT1<=92.1097)tmp1=115.7;
        else if(FT1<=92.2019)tmp1=115.8;
        else if(FT1<=92.2941)tmp1=115.9;
        else if(FT1<=92.3864)tmp1=116;
    }
    else if(FT1<=93.219){
        if(FT1==92.3864)tmp1=116;
        else if(FT1<=92.4787)tmp1=116.1;
        else if(FT1<=92.5711)tmp1=116.2;
        else if(FT1<=92.6635)tmp1=116.3;
        else if(FT1<=92.7559)tmp1=116.4;
        else if(FT1<=92.8485)tmp1=116.5;
        else if(FT1<=92.941)tmp1=116.6;
        else if(FT1<=93.0336)tmp1=116.7;
        else if(FT1<=93.1263)tmp1=116.8;
        else if(FT1<=93.219)tmp1=116.9;
        else if(FT1<=93.3118)tmp1=117;
    }
    else if(FT1<=94.1487){
        if(FT1==93.3118)tmp1=117;
        else if(FT1<=93.4046)tmp1=117.1;
        else if(FT1<=93.4974)tmp1=117.2;
        else if(FT1<=93.5903)tmp1=117.3;
        else if(FT1<=93.6832)tmp1=117.4;
        else if(FT1<=93.7762)tmp1=117.5;
        else if(FT1<=93.8693)tmp1=117.6;
        else if(FT1<=93.9624)tmp1=117.7;
        else if(FT1<=94.0555)tmp1=117.8;
        else if(FT1<=94.1487)tmp1=117.9;
        else if(FT1<=94.2419)tmp1=118;
    }
    else if(FT1<=95.0831){
        if(FT1==94.2419)tmp1=118;
        else if(FT1<=94.3352)tmp1=118.1;
        else if(FT1<=94.4285)tmp1=118.2;
        else if(FT1<=94.5219)tmp1=118.3;
        else if(FT1<=94.6153)tmp1=118.4;
        else if(FT1<=94.7088)tmp1=118.5;
        else if(FT1<=94.8023)tmp1=118.6;
        else if(FT1<=94.8959)tmp1=118.7;
        else if(FT1<=94.9895)tmp1=118.8;
        else if(FT1<=95.0831)tmp1=118.9;
        else if(FT1<=95.1768)tmp1=119;
    }
    else if(FT1<=96.0223){
        if(FT1==95.1768)tmp1=119;
        else if(FT1<=95.2706)tmp1=119.1;
        else if(FT1<=95.3644)tmp1=119.2;
        else if(FT1<=95.4582)tmp1=119.3;
        else if(FT1<=95.5521)tmp1=119.4;
        else if(FT1<=95.6461)tmp1=119.5;
        else if(FT1<=95.7401)tmp1=119.6;
        else if(FT1<=95.8341)tmp1=119.7;
        else if(FT1<=95.9282)tmp1=119.8;
        else if(FT1<=96.0223)tmp1=119.9;
        else if(FT1<=96.1165)tmp1=120;
    }
    else if(FT1<=96.9663){
        if(FT1==96.1165)tmp1=120;
        else if(FT1<=96.2107)tmp1=120.1;
        else if(FT1<=96.305)tmp1=120.2;
        else if(FT1<=96.3993)tmp1=120.3;
        else if(FT1<=96.4937)tmp1=120.4;
        else if(FT1<=96.5881)tmp1=120.5;
        else if(FT1<=96.6826)tmp1=120.6;
        else if(FT1<=96.7771)tmp1=120.7;
        else if(FT1<=96.8717)tmp1=120.8;
        else if(FT1<=96.9663)tmp1=120.9;
        else if(FT1<=97.0609)tmp1=121;
    }
    else if(FT1<=97.915){
        if(FT1==97.0609)tmp1=121;
        else if(FT1<=97.1556)tmp1=121.1;
        else if(FT1<=97.2504)tmp1=121.2;
        else if(FT1<=97.3452)tmp1=121.3;
        else if(FT1<=97.44)tmp1=121.4;
        else if(FT1<=97.5349)tmp1=121.5;
        else if(FT1<=97.6299)tmp1=121.6;
        else if(FT1<=97.7249)tmp1=121.7;
        else if(FT1<=97.8199)tmp1=121.8;
        else if(FT1<=97.915)tmp1=121.9;
        else if(FT1<=98.0101)tmp1=122;
    }
    else if(FT1<=98.8684){
        if(FT1==98.0101)tmp1=122;
        else if(FT1<=98.1053)tmp1=122.1;
        else if(FT1<=98.2005)tmp1=122.2;
        else if(FT1<=98.2958)tmp1=122.3;
        else if(FT1<=98.3911)tmp1=122.4;
        else if(FT1<=98.4865)tmp1=122.5;
        else if(FT1<=98.5819)tmp1=122.6;
        else if(FT1<=98.6773)tmp1=122.7;
        else if(FT1<=98.7729)tmp1=122.8;
        else if(FT1<=98.8684)tmp1=122.9;
        else if(FT1<=98.964)tmp1=123;
    }
    else if(FT1<=99.8266){
        if(FT1==98.964)tmp1=123;
        else if(FT1<=99.0597)tmp1=123.1;
        else if(FT1<=99.1554)tmp1=123.2;
        else if(FT1<=99.2511)tmp1=123.3;
        else if(FT1<=99.3469)tmp1=123.4;
        else if(FT1<=99.4427)tmp1=123.5;
        else if(FT1<=99.5386)tmp1=123.6;
        else if(FT1<=99.6346)tmp1=123.7;
        else if(FT1<=99.7305)tmp1=123.8;
        else if(FT1<=99.8266)tmp1=123.9;
        else if(FT1<=99.9227)tmp1=124;
    }
    else if(FT1<=100.789){
        if(FT1==99.9227)tmp1=124;
        else if(FT1<=100.019)tmp1=124.1;
        else if(FT1<=100.115)tmp1=124.2;
        else if(FT1<=100.211)tmp1=124.3;
        else if(FT1<=100.307)tmp1=124.4;
        else if(FT1<=100.404)tmp1=124.5;
        else if(FT1<=100.5)tmp1=124.6;
        else if(FT1<=100.597)tmp1=124.7;
        else if(FT1<=100.693)tmp1=124.8;
        else if(FT1<=100.789)tmp1=124.9;
        else if(FT1<=100.886)tmp1=125;
    }
    else if(FT1<=101.757){
        if(FT1==100.886)tmp1=125;
        else if(FT1<=100.983)tmp1=125.1;
        else if(FT1<=101.079)tmp1=125.2;
        else if(FT1<=101.176)tmp1=125.3;
        else if(FT1<=101.273)tmp1=125.4;
        else if(FT1<=101.369)tmp1=125.5;
        else if(FT1<=101.466)tmp1=125.6;
        else if(FT1<=101.563)tmp1=125.7;
        else if(FT1<=101.66)tmp1=125.8;
        else if(FT1<=101.757)tmp1=125.9;
        else if(FT1<=101.854)tmp1=126;
    }
    else if(FT1<=102.729){
        if(FT1==101.854)tmp1=126;
        else if(FT1<=101.951)tmp1=126.1;
        else if(FT1<=102.048)tmp1=126.2;
        else if(FT1<=102.145)tmp1=126.3;
        else if(FT1<=102.243)tmp1=126.4;
        else if(FT1<=102.34)tmp1=126.5;
        else if(FT1<=102.437)tmp1=126.6;
        else if(FT1<=102.535)tmp1=126.7;
        else if(FT1<=102.632)tmp1=126.8;
        else if(FT1<=102.729)tmp1=126.9;
        else if(FT1<=102.827)tmp1=127;
    }
    else if(FT1<=103.706){
        if(FT1==102.827)tmp1=127;
        else if(FT1<=102.924)tmp1=127.1;
        else if(FT1<=103.022)tmp1=127.2;
        else if(FT1<=103.12)tmp1=127.3;
        else if(FT1<=103.217)tmp1=127.4;
        else if(FT1<=103.315)tmp1=127.5;
        else if(FT1<=103.413)tmp1=127.6;
        else if(FT1<=103.511)tmp1=127.7;
        else if(FT1<=103.608)tmp1=127.8;
        else if(FT1<=103.706)tmp1=127.9;
        else if(FT1<=103.804)tmp1=128;
    }
    else if(FT1<=104.688){
        if(FT1==103.804)tmp1=128;
        else if(FT1<=103.902)tmp1=128.1;
        else if(FT1<=104)tmp1=128.2;
        else if(FT1<=104.098)tmp1=128.3;
        else if(FT1<=104.197)tmp1=128.4;
        else if(FT1<=104.295)tmp1=128.5;
        else if(FT1<=104.393)tmp1=128.6;
        else if(FT1<=104.491)tmp1=128.7;
        else if(FT1<=104.59)tmp1=128.8;
        else if(FT1<=104.688)tmp1=128.9;
        else if(FT1<=104.786)tmp1=129;
    }
    else if(FT1<=105.674){
        if(FT1==104.786)tmp1=129;
        else if(FT1<=104.885)tmp1=129.1;
        else if(FT1<=104.983)tmp1=129.2;
        else if(FT1<=105.082)tmp1=129.3;
        else if(FT1<=105.181)tmp1=129.4;
        else if(FT1<=105.279)tmp1=129.5;
        else if(FT1<=105.378)tmp1=129.6;
        else if(FT1<=105.477)tmp1=129.7;
        else if(FT1<=105.576)tmp1=129.8;
        else if(FT1<=105.674)tmp1=129.9;
        else if(FT1<=105.773)tmp1=130;
    }
    else if(FT1<=106.665){
        if(FT1==105.773)tmp1=130;
        else if(FT1<=105.872)tmp1=130.1;
        else if(FT1<=105.971)tmp1=130.2;
        else if(FT1<=106.07)tmp1=130.3;
        else if(FT1<=106.169)tmp1=130.4;
        else if(FT1<=106.268)tmp1=130.5;
        else if(FT1<=106.368)tmp1=130.6;
        else if(FT1<=106.467)tmp1=130.7;
        else if(FT1<=106.566)tmp1=130.8;
        else if(FT1<=106.665)tmp1=130.9;
        else if(FT1<=106.765)tmp1=131;
    }
    else if(FT1<=107.661){
        if(FT1==106.765)tmp1=131;
        else if(FT1<=106.864)tmp1=131.1;
        else if(FT1<=106.964)tmp1=131.2;
        else if(FT1<=107.063)tmp1=131.3;
        else if(FT1<=107.163)tmp1=131.4;
        else if(FT1<=107.262)tmp1=131.5;
        else if(FT1<=107.362)tmp1=131.6;
        else if(FT1<=107.462)tmp1=131.7;
        else if(FT1<=107.561)tmp1=131.8;
        else if(FT1<=107.661)tmp1=131.9;
        else if(FT1<=107.761)tmp1=132;
    }
    else if(FT1<=108.662){
        if(FT1==107.761)tmp1=132;
        else if(FT1<=107.861)tmp1=132.1;
        else if(FT1<=107.961)tmp1=132.2;
        else if(FT1<=108.061)tmp1=132.3;
        else if(FT1<=108.161)tmp1=132.4;
        else if(FT1<=108.261)tmp1=132.5;
        else if(FT1<=108.361)tmp1=132.6;
        else if(FT1<=108.461)tmp1=132.7;
        else if(FT1<=108.561)tmp1=132.8;
        else if(FT1<=108.662)tmp1=132.9;
        else if(FT1<=108.762)tmp1=133;
    }
    else if(FT1<=109.667){
        if(FT1==108.762)tmp1=133;
        else if(FT1<=108.862)tmp1=133.1;
        else if(FT1<=108.963)tmp1=133.2;
        else if(FT1<=109.063)tmp1=133.3;
        else if(FT1<=109.163)tmp1=133.4;
        else if(FT1<=109.264)tmp1=133.5;
        else if(FT1<=109.365)tmp1=133.6;
        else if(FT1<=109.465)tmp1=133.7;
        else if(FT1<=109.566)tmp1=133.8;
        else if(FT1<=109.667)tmp1=133.9;
        else if(FT1<=109.767)tmp1=134;
    }
    else if(FT1<=110.676){
        if(FT1==109.767)tmp1=134;
        else if(FT1<=109.868)tmp1=134.1;
        else if(FT1<=109.969)tmp1=134.2;
        else if(FT1<=110.07)tmp1=134.3;
        else if(FT1<=110.171)tmp1=134.4;
        else if(FT1<=110.272)tmp1=134.5;
        else if(FT1<=110.373)tmp1=134.6;
        else if(FT1<=110.474)tmp1=134.7;
        else if(FT1<=110.575)tmp1=134.8;
        else if(FT1<=110.676)tmp1=134.9;
        else if(FT1<=110.777)tmp1=135;
    }
    else if(FT1<=111.69){
        if(FT1==110.777)tmp1=135;
        else if(FT1<=110.879)tmp1=135.1;
        else if(FT1<=110.98)tmp1=135.2;
        else if(FT1<=111.081)tmp1=135.3;
        else if(FT1<=111.183)tmp1=135.4;
        else if(FT1<=111.284)tmp1=135.5;
        else if(FT1<=111.386)tmp1=135.6;
        else if(FT1<=111.487)tmp1=135.7;
        else if(FT1<=111.589)tmp1=135.8;
        else if(FT1<=111.69)tmp1=135.9;
        else if(FT1<=111.792)tmp1=136;
    }
    else if(FT1<=112.709){
        if(FT1==111.792)tmp1=136;
        else if(FT1<=111.894)tmp1=136.1;
        else if(FT1<=111.996)tmp1=136.2;
        else if(FT1<=112.097)tmp1=136.3;
        else if(FT1<=112.199)tmp1=136.4;
        else if(FT1<=112.301)tmp1=136.5;
        else if(FT1<=112.403)tmp1=136.6;
        else if(FT1<=112.505)tmp1=136.7;
        else if(FT1<=112.607)tmp1=136.8;
        else if(FT1<=112.709)tmp1=136.9;
        else if(FT1<=112.812)tmp1=137;
    }
    else if(FT1<=113.733){
        if(FT1==112.812)tmp1=137;
        else if(FT1<=112.914)tmp1=137.1;
        else if(FT1<=113.016)tmp1=137.2;
        else if(FT1<=113.118)tmp1=137.3;
        else if(FT1<=113.221)tmp1=137.4;
        else if(FT1<=113.323)tmp1=137.5;
        else if(FT1<=113.425)tmp1=137.6;
        else if(FT1<=113.528)tmp1=137.7;
        else if(FT1<=113.63)tmp1=137.8;
        else if(FT1<=113.733)tmp1=137.9;
        else if(FT1<=113.835)tmp1=138;
    }
    else if(FT1<=114.761){
        if(FT1==113.835)tmp1=138;
        else if(FT1<=113.938)tmp1=138.1;
        else if(FT1<=114.041)tmp1=138.2;
        else if(FT1<=114.144)tmp1=138.3;
        else if(FT1<=114.246)tmp1=138.4;
        else if(FT1<=114.349)tmp1=138.5;
        else if(FT1<=114.452)tmp1=138.6;
        else if(FT1<=114.555)tmp1=138.7;
        else if(FT1<=114.658)tmp1=138.8;
        else if(FT1<=114.761)tmp1=138.9;
        else if(FT1<=114.864)tmp1=139;
    }
    else if(FT1<=115.794){
        if(FT1==114.864)tmp1=139;
        else if(FT1<=114.967)tmp1=139.1;
        else if(FT1<=115.07)tmp1=139.2;
        else if(FT1<=115.174)tmp1=139.3;
        else if(FT1<=115.277)tmp1=139.4;
        else if(FT1<=115.38)tmp1=139.5;
        else if(FT1<=115.483)tmp1=139.6;
        else if(FT1<=115.587)tmp1=139.7;
        else if(FT1<=115.69)tmp1=139.8;
        else if(FT1<=115.794)tmp1=139.9;
        else if(FT1<=115.897)tmp1=140;
    }
    else if(FT1<=116.831){
        if(FT1==115.897)tmp1=140;
        else if(FT1<=116.001)tmp1=140.1;
        else if(FT1<=116.104)tmp1=140.2;
        else if(FT1<=116.208)tmp1=140.3;
        else if(FT1<=116.312)tmp1=140.4;
        else if(FT1<=116.416)tmp1=140.5;
        else if(FT1<=116.519)tmp1=140.6;
        else if(FT1<=116.623)tmp1=140.7;
        else if(FT1<=116.727)tmp1=140.8;
        else if(FT1<=116.831)tmp1=140.9;
        else if(FT1<=116.935)tmp1=141;
    }
    else if(FT1<=117.873){
        if(FT1==116.935)tmp1=141;
        else if(FT1<=117.039)tmp1=141.1;
        else if(FT1<=117.143)tmp1=141.2;
        else if(FT1<=117.247)tmp1=141.3;
        else if(FT1<=117.351)tmp1=141.4;
        else if(FT1<=117.456)tmp1=141.5;
        else if(FT1<=117.56)tmp1=141.6;
        else if(FT1<=117.664)tmp1=141.7;
        else if(FT1<=117.769)tmp1=141.8;
        else if(FT1<=117.873)tmp1=141.9;
        else if(FT1<=117.977)tmp1=142;
    }
    else if(FT1<=118.919){
        if(FT1==117.977)tmp1=142;
        else if(FT1<=118.082)tmp1=142.1;
        else if(FT1<=118.186)tmp1=142.2;
        else if(FT1<=118.291)tmp1=142.3;
        else if(FT1<=118.396)tmp1=142.4;
        else if(FT1<=118.5)tmp1=142.5;
        else if(FT1<=118.605)tmp1=142.6;
        else if(FT1<=118.71)tmp1=142.7;
        else if(FT1<=118.815)tmp1=142.8;
        else if(FT1<=118.919)tmp1=142.9;
        else if(FT1<=119.024)tmp1=143;
    }
    else if(FT1<=119.97){
        if(FT1==119.024)tmp1=143;
        else if(FT1<=119.129)tmp1=143.1;
        else if(FT1<=119.234)tmp1=143.2;
        else if(FT1<=119.339)tmp1=143.3;
        else if(FT1<=119.444)tmp1=143.4;
        else if(FT1<=119.549)tmp1=143.5;
        else if(FT1<=119.655)tmp1=143.6;
        else if(FT1<=119.76)tmp1=143.7;
        else if(FT1<=119.865)tmp1=143.8;
        else if(FT1<=119.97)tmp1=143.9;
        else if(FT1<=120.076)tmp1=144;
    }
    else if(FT1<=121.026){
        if(FT1==120.076)tmp1=144;
        else if(FT1<=120.181)tmp1=144.1;
        else if(FT1<=120.287)tmp1=144.2;
        else if(FT1<=120.392)tmp1=144.3;
        else if(FT1<=120.498)tmp1=144.4;
        else if(FT1<=120.603)tmp1=144.5;
        else if(FT1<=120.709)tmp1=144.6;
        else if(FT1<=120.814)tmp1=144.7;
        else if(FT1<=120.92)tmp1=144.8;
        else if(FT1<=121.026)tmp1=144.9;
        else if(FT1<=121.132)tmp1=145;
    }
    else if(FT1<=122.086){
        if(FT1==121.132)tmp1=145;
        else if(FT1<=121.238)tmp1=145.1;
        else if(FT1<=121.344)tmp1=145.2;
        else if(FT1<=121.449)tmp1=145.3;
        else if(FT1<=121.555)tmp1=145.4;
        else if(FT1<=121.661)tmp1=145.5;
        else if(FT1<=121.768)tmp1=145.6;
        else if(FT1<=121.874)tmp1=145.7;
        else if(FT1<=121.98)tmp1=145.8;
        else if(FT1<=122.086)tmp1=145.9;
        else if(FT1<=122.192)tmp1=146;
    }
    else if(FT1<=123.151){
        if(FT1==122.192)tmp1=146;
        else if(FT1<=122.299)tmp1=146.1;
        else if(FT1<=122.405)tmp1=146.2;
        else if(FT1<=122.511)tmp1=146.3;
        else if(FT1<=122.618)tmp1=146.4;
        else if(FT1<=122.724)tmp1=146.5;
        else if(FT1<=122.831)tmp1=146.6;
        else if(FT1<=122.937)tmp1=146.7;
        else if(FT1<=123.044)tmp1=146.8;
        else if(FT1<=123.151)tmp1=146.9;
        else if(FT1<=123.257)tmp1=147;
    }
    else if(FT1<=124.22){
        if(FT1==123.257)tmp1=147;
        else if(FT1<=123.364)tmp1=147.1;
        else if(FT1<=123.471)tmp1=147.2;
        else if(FT1<=123.578)tmp1=147.3;
        else if(FT1<=123.685)tmp1=147.4;
        else if(FT1<=123.792)tmp1=147.5;
        else if(FT1<=123.899)tmp1=147.6;
        else if(FT1<=124.006)tmp1=147.7;
        else if(FT1<=124.113)tmp1=147.8;
        else if(FT1<=124.22)tmp1=147.9;
        else if(FT1<=124.327)tmp1=148;
    }
    else if(FT1<=125.294){
        if(FT1==124.327)tmp1=148;
        else if(FT1<=124.434)tmp1=148.1;
        else if(FT1<=124.542)tmp1=148.2;
        else if(FT1<=124.649)tmp1=148.3;
        else if(FT1<=124.756)tmp1=148.4;
        else if(FT1<=124.864)tmp1=148.5;
        else if(FT1<=124.971)tmp1=148.6;
        else if(FT1<=125.078)tmp1=148.7;
        else if(FT1<=125.186)tmp1=148.8;
        else if(FT1<=125.294)tmp1=148.9;
        else if(FT1<=125.401)tmp1=149;
    }
    else if(FT1<=126.372){
        if(FT1==125.401)tmp1=149;
        else if(FT1<=125.509)tmp1=149.1;
        else if(FT1<=125.617)tmp1=149.2;
        else if(FT1<=125.724)tmp1=149.3;
        else if(FT1<=125.832)tmp1=149.4;
        else if(FT1<=125.94)tmp1=149.5;
        else if(FT1<=126.048)tmp1=149.6;
        else if(FT1<=126.156)tmp1=149.7;
        else if(FT1<=126.264)tmp1=149.8;
        else if(FT1<=126.372)tmp1=149.9;
        else if(FT1<=126.48)tmp1=150;
    }
    else if(FT1<=127.454){
        if(FT1==126.48)tmp1=150;
        else if(FT1<=126.588)tmp1=150.1;
        else if(FT1<=126.696)tmp1=150.2;
        else if(FT1<=126.804)tmp1=150.3;
        else if(FT1<=126.913)tmp1=150.4;
        else if(FT1<=127.021)tmp1=150.5;
        else if(FT1<=127.129)tmp1=150.6;
        else if(FT1<=127.238)tmp1=150.7;
        else if(FT1<=127.346)tmp1=150.8;
        else if(FT1<=127.454)tmp1=150.9;
        else if(FT1<=127.563)tmp1=151;
    }
    else if(FT1<=128.542){
        if(FT1==127.563)tmp1=151;
        else if(FT1<=127.672)tmp1=151.1;
        else if(FT1<=127.78)tmp1=151.2;
        else if(FT1<=127.889)tmp1=151.3;
        else if(FT1<=127.997)tmp1=151.4;
        else if(FT1<=128.106)tmp1=151.5;
        else if(FT1<=128.215)tmp1=151.6;
        else if(FT1<=128.324)tmp1=151.7;
        else if(FT1<=128.433)tmp1=151.8;
        else if(FT1<=128.542)tmp1=151.9;
        else if(FT1<=128.651)tmp1=152;
    }
    else if(FT1<=129.633){
        if(FT1==128.651)tmp1=152;
        else if(FT1<=128.76)tmp1=152.1;
        else if(FT1<=128.869)tmp1=152.2;
        else if(FT1<=128.978)tmp1=152.3;
        else if(FT1<=129.087)tmp1=152.4;
        else if(FT1<=129.196)tmp1=152.5;
        else if(FT1<=129.305)tmp1=152.6;
        else if(FT1<=129.415)tmp1=152.7;
        else if(FT1<=129.524)tmp1=152.8;
        else if(FT1<=129.633)tmp1=152.9;
        else if(FT1<=129.743)tmp1=153;
    }
    else if(FT1<=130.729){
        if(FT1==129.743)tmp1=153;
        else if(FT1<=129.852)tmp1=153.1;
        else if(FT1<=129.962)tmp1=153.2;
        else if(FT1<=130.071)tmp1=153.3;
        else if(FT1<=130.181)tmp1=153.4;
        else if(FT1<=130.29)tmp1=153.5;
        else if(FT1<=130.4)tmp1=153.6;
        else if(FT1<=130.51)tmp1=153.7;
        else if(FT1<=130.62)tmp1=153.8;
        else if(FT1<=130.729)tmp1=153.9;
        else if(FT1<=130.839)tmp1=154;
    }
    else if(FT1<=131.83){
        if(FT1==130.839)tmp1=154;
        else if(FT1<=130.949)tmp1=154.1;
        else if(FT1<=131.059)tmp1=154.2;
        else if(FT1<=131.169)tmp1=154.3;
        else if(FT1<=131.279)tmp1=154.4;
        else if(FT1<=131.389)tmp1=154.5;
        else if(FT1<=131.499)tmp1=154.6;
        else if(FT1<=131.609)tmp1=154.7;
        else if(FT1<=131.72)tmp1=154.8;
        else if(FT1<=131.83)tmp1=154.9;
        else if(FT1<=131.94)tmp1=155;
    }
    else if(FT1<=132.935){
        if(FT1==131.94)tmp1=155;
        else if(FT1<=132.051)tmp1=155.1;
        else if(FT1<=132.161)tmp1=155.2;
        else if(FT1<=132.271)tmp1=155.3;
        else if(FT1<=132.382)tmp1=155.4;
        else if(FT1<=132.492)tmp1=155.5;
        else if(FT1<=132.603)tmp1=155.6;
        else if(FT1<=132.714)tmp1=155.7;
        else if(FT1<=132.824)tmp1=155.8;
        else if(FT1<=132.935)tmp1=155.9;
        else if(FT1<=133.046)tmp1=156;
    }
    else if(FT1<=134.044){
        if(FT1==133.046)tmp1=156;
        else if(FT1<=133.157)tmp1=156.1;
        else if(FT1<=133.267)tmp1=156.2;
        else if(FT1<=133.378)tmp1=156.3;
        else if(FT1<=133.489)tmp1=156.4;
        else if(FT1<=133.6)tmp1=156.5;
        else if(FT1<=133.711)tmp1=156.6;
        else if(FT1<=133.822)tmp1=156.7;
        else if(FT1<=133.933)tmp1=156.8;
        else if(FT1<=134.044)tmp1=156.9;
        else if(FT1<=134.156)tmp1=157;
    }
    else if(FT1<=135.158){
        if(FT1==134.156)tmp1=157;
        else if(FT1<=134.267)tmp1=157.1;
        else if(FT1<=134.378)tmp1=157.2;
        else if(FT1<=134.49)tmp1=157.3;
        else if(FT1<=134.601)tmp1=157.4;
        else if(FT1<=134.712)tmp1=157.5;
        else if(FT1<=134.824)tmp1=157.6;
        else if(FT1<=134.935)tmp1=157.7;
        else if(FT1<=135.047)tmp1=157.8;
        else if(FT1<=135.158)tmp1=157.9;
        else if(FT1<=135.27)tmp1=158;
    }
    else if(FT1<=136.277){
        if(FT1==135.27)tmp1=158;
        else if(FT1<=135.382)tmp1=158.1;
        else if(FT1<=135.493)tmp1=158.2;
        else if(FT1<=135.605)tmp1=158.3;
        else if(FT1<=135.717)tmp1=158.4;
        else if(FT1<=135.829)tmp1=158.5;
        else if(FT1<=135.941)tmp1=158.6;
        else if(FT1<=136.053)tmp1=158.7;
        else if(FT1<=136.165)tmp1=158.8;
        else if(FT1<=136.277)tmp1=158.9;
        else if(FT1<=136.389)tmp1=159;
    }
    else if(FT1<=137.399){
        if(FT1==136.389)tmp1=159;
        else if(FT1<=136.501)tmp1=159.1;
        else if(FT1<=136.613)tmp1=159.2;
        else if(FT1<=136.725)tmp1=159.3;
        else if(FT1<=136.838)tmp1=159.4;
        else if(FT1<=136.95)tmp1=159.5;
        else if(FT1<=137.062)tmp1=159.6;
        else if(FT1<=137.175)tmp1=159.7;
        else if(FT1<=137.287)tmp1=159.8;
        else if(FT1<=137.399)tmp1=159.9;
        else if(FT1<=137.512)tmp1=160;
    }
    else if(FT1<=138.527){
        if(FT1==137.512)tmp1=160;
        else if(FT1<=137.625)tmp1=160.1;
        else if(FT1<=137.737)tmp1=160.2;
        else if(FT1<=137.85)tmp1=160.3;
        else if(FT1<=137.962)tmp1=160.4;
        else if(FT1<=138.075)tmp1=160.5;
        else if(FT1<=138.188)tmp1=160.6;
        else if(FT1<=138.301)tmp1=160.7;
        else if(FT1<=138.414)tmp1=160.8;
        else if(FT1<=138.527)tmp1=160.9;
        else if(FT1<=138.64)tmp1=161;
    }
    else if(FT1<=139.658){
        if(FT1==138.64)tmp1=161;
        else if(FT1<=138.753)tmp1=161.1;
        else if(FT1<=138.866)tmp1=161.2;
        else if(FT1<=138.979)tmp1=161.3;
        else if(FT1<=139.092)tmp1=161.4;
        else if(FT1<=139.205)tmp1=161.5;
        else if(FT1<=139.318)tmp1=161.6;
        else if(FT1<=139.431)tmp1=161.7;
        else if(FT1<=139.545)tmp1=161.8;
        else if(FT1<=139.658)tmp1=161.9;
        else if(FT1<=139.772)tmp1=162;
    }
    else if(FT1<=140.794){
        if(FT1==139.772)tmp1=162;
        else if(FT1<=139.885)tmp1=162.1;
        else if(FT1<=139.998)tmp1=162.2;
        else if(FT1<=140.112)tmp1=162.3;
        else if(FT1<=140.226)tmp1=162.4;
        else if(FT1<=140.339)tmp1=162.5;
        else if(FT1<=140.453)tmp1=162.6;
        else if(FT1<=140.566)tmp1=162.7;
        else if(FT1<=140.68)tmp1=162.8;
        else if(FT1<=140.794)tmp1=162.9;
        else if(FT1<=140.908)tmp1=163;
    }
    else if(FT1<=141.934){
        if(FT1==140.908)tmp1=163;
        else if(FT1<=141.022)tmp1=163.1;
        else if(FT1<=141.136)tmp1=163.2;
        else if(FT1<=141.25)tmp1=163.3;
        else if(FT1<=141.364)tmp1=163.4;
        else if(FT1<=141.478)tmp1=163.5;
        else if(FT1<=141.592)tmp1=163.6;
        else if(FT1<=141.706)tmp1=163.7;
        else if(FT1<=141.82)tmp1=163.8;
        else if(FT1<=141.934)tmp1=163.9;
        else if(FT1<=142.049)tmp1=164;
    }
    else if(FT1<=143.079){
        if(FT1==142.049)tmp1=164;
        else if(FT1<=142.163)tmp1=164.1;
        else if(FT1<=142.277)tmp1=164.2;
        else if(FT1<=142.392)tmp1=164.3;
        else if(FT1<=142.506)tmp1=164.4;
        else if(FT1<=142.621)tmp1=164.5;
        else if(FT1<=142.735)tmp1=164.6;
        else if(FT1<=142.85)tmp1=164.7;
        else if(FT1<=142.964)tmp1=164.8;
        else if(FT1<=143.079)tmp1=164.9;
        else if(FT1<=143.194)tmp1=165;
    }
    else if(FT1<=144.228){
        if(FT1==143.194)tmp1=165;
        else if(FT1<=143.308)tmp1=165.1;
        else if(FT1<=143.423)tmp1=165.2;
        else if(FT1<=143.538)tmp1=165.3;
        else if(FT1<=143.653)tmp1=165.4;
        else if(FT1<=143.768)tmp1=165.5;
        else if(FT1<=143.883)tmp1=165.6;
        else if(FT1<=143.998)tmp1=165.7;
        else if(FT1<=144.113)tmp1=165.8;
        else if(FT1<=144.228)tmp1=165.9;
        else if(FT1<=144.343)tmp1=166;
    }
    else if(FT1<=145.381){
        if(FT1==144.343)tmp1=166;
        else if(FT1<=144.458)tmp1=166.1;
        else if(FT1<=144.574)tmp1=166.2;
        else if(FT1<=144.689)tmp1=166.3;
        else if(FT1<=144.804)tmp1=166.4;
        else if(FT1<=144.919)tmp1=166.5;
        else if(FT1<=145.035)tmp1=166.6;
        else if(FT1<=145.15)tmp1=166.7;
        else if(FT1<=145.266)tmp1=166.8;
        else if(FT1<=145.381)tmp1=166.9;
        else if(FT1<=145.497)tmp1=167;
    }
    else if(FT1<=146.539){
        if(FT1==145.497)tmp1=167;
        else if(FT1<=145.613)tmp1=167.1;
        else if(FT1<=145.728)tmp1=167.2;
        else if(FT1<=145.844)tmp1=167.3;
        else if(FT1<=145.96)tmp1=167.4;
        else if(FT1<=146.075)tmp1=167.5;
        else if(FT1<=146.191)tmp1=167.6;
        else if(FT1<=146.307)tmp1=167.7;
        else if(FT1<=146.423)tmp1=167.8;
        else if(FT1<=146.539)tmp1=167.9;
        else if(FT1<=146.655)tmp1=168;
    }
    else if(FT1<=147.701){
        if(FT1==146.655)tmp1=168;
        else if(FT1<=146.771)tmp1=168.1;
        else if(FT1<=146.887)tmp1=168.2;
        else if(FT1<=147.003)tmp1=168.3;
        else if(FT1<=147.119)tmp1=168.4;
        else if(FT1<=147.236)tmp1=168.5;
        else if(FT1<=147.352)tmp1=168.6;
        else if(FT1<=147.468)tmp1=168.7;
        else if(FT1<=147.585)tmp1=168.8;
        else if(FT1<=147.701)tmp1=168.9;
        else if(FT1<=147.817)tmp1=169;
    }
    else if(FT1<=148.867){
        if(FT1==147.817)tmp1=169;
        else if(FT1<=147.934)tmp1=169.1;
        else if(FT1<=148.05)tmp1=169.2;
        else if(FT1<=148.167)tmp1=169.3;
        else if(FT1<=148.284)tmp1=169.4;
        else if(FT1<=148.4)tmp1=169.5;
        else if(FT1<=148.517)tmp1=169.6;
        else if(FT1<=148.634)tmp1=169.7;
        else if(FT1<=148.75)tmp1=169.8;
        else if(FT1<=148.867)tmp1=169.9;
        else if(FT1<=148.984)tmp1=170;
    }
    else if(FT1<=150.038){
        if(FT1==148.984)tmp1=170;
        else if(FT1<=149.101)tmp1=170.1;
        else if(FT1<=149.218)tmp1=170.2;
        else if(FT1<=149.335)tmp1=170.3;
        else if(FT1<=149.452)tmp1=170.4;
        else if(FT1<=149.569)tmp1=170.5;
        else if(FT1<=149.686)tmp1=170.6;
        else if(FT1<=149.803)tmp1=170.7;
        else if(FT1<=149.921)tmp1=170.8;
        else if(FT1<=150.038)tmp1=170.9;
        else if(FT1<=150.155)tmp1=171;
    }
    else if(FT1<=151.213){
        if(FT1==150.155)tmp1=171;
        else if(FT1<=150.273)tmp1=171.1;
        else if(FT1<=150.39)tmp1=171.2;
        else if(FT1<=150.507)tmp1=171.3;
        else if(FT1<=150.625)tmp1=171.4;
        else if(FT1<=150.742)tmp1=171.5;
        else if(FT1<=150.86)tmp1=171.6;
        else if(FT1<=150.978)tmp1=171.7;
        else if(FT1<=151.095)tmp1=171.8;
        else if(FT1<=151.213)tmp1=171.9;
        else if(FT1<=151.331)tmp1=172;
    }
    else if(FT1<=152.392){
        if(FT1==151.331)tmp1=172;
        else if(FT1<=151.448)tmp1=172.1;
        else if(FT1<=151.566)tmp1=172.2;
        else if(FT1<=151.684)tmp1=172.3;
        else if(FT1<=151.802)tmp1=172.4;
        else if(FT1<=151.92)tmp1=172.5;
        else if(FT1<=152.038)tmp1=172.6;
        else if(FT1<=152.156)tmp1=172.7;
        else if(FT1<=152.274)tmp1=172.8;
        else if(FT1<=152.392)tmp1=172.9;
        else if(FT1<=152.51)tmp1=173;
    }
    else if(FT1<=153.576){
        if(FT1==152.51)tmp1=173;
        else if(FT1<=152.628)tmp1=173.1;
        else if(FT1<=152.747)tmp1=173.2;
        else if(FT1<=152.865)tmp1=173.3;
        else if(FT1<=152.983)tmp1=173.4;
        else if(FT1<=153.102)tmp1=173.5;
        else if(FT1<=153.22)tmp1=173.6;
        else if(FT1<=153.338)tmp1=173.7;
        else if(FT1<=153.457)tmp1=173.8;
        else if(FT1<=153.576)tmp1=173.9;
        else if(FT1<=153.694)tmp1=174;
    }
    else if(FT1<=154.763){
        if(FT1==153.694)tmp1=174;
        else if(FT1<=153.813)tmp1=174.1;
        else if(FT1<=153.931)tmp1=174.2;
        else if(FT1<=154.05)tmp1=174.3;
        else if(FT1<=154.169)tmp1=174.4;
        else if(FT1<=154.288)tmp1=174.5;
        else if(FT1<=154.406)tmp1=174.6;
        else if(FT1<=154.525)tmp1=174.7;
        else if(FT1<=154.644)tmp1=174.8;
        else if(FT1<=154.763)tmp1=174.9;
        else if(FT1<=154.882)tmp1=175;
    }
    else if(FT1<=155.955){
        if(FT1==154.882)tmp1=175;
        else if(FT1<=155.001)tmp1=175.1;
        else if(FT1<=155.12)tmp1=175.2;
        else if(FT1<=155.24)tmp1=175.3;
        else if(FT1<=155.359)tmp1=175.4;
        else if(FT1<=155.478)tmp1=175.5;
        else if(FT1<=155.597)tmp1=175.6;
        else if(FT1<=155.717)tmp1=175.7;
        else if(FT1<=155.836)tmp1=175.8;
        else if(FT1<=155.955)tmp1=175.9;
        else if(FT1<=156.075)tmp1=176;
    }
    else if(FT1<=157.151){
        if(FT1==156.075)tmp1=176;
        else if(FT1<=156.194)tmp1=176.1;
        else if(FT1<=156.314)tmp1=176.2;
        else if(FT1<=156.433)tmp1=176.3;
        else if(FT1<=156.553)tmp1=176.4;
        else if(FT1<=156.672)tmp1=176.5;
        else if(FT1<=156.792)tmp1=176.6;
        else if(FT1<=156.912)tmp1=176.7;
        else if(FT1<=157.032)tmp1=176.8;
        else if(FT1<=157.151)tmp1=176.9;
        else if(FT1<=157.271)tmp1=177;
    }
    else if(FT1<=158.352){
        if(FT1==157.271)tmp1=177;
        else if(FT1<=157.391)tmp1=177.1;
        else if(FT1<=157.511)tmp1=177.2;
        else if(FT1<=157.631)tmp1=177.3;
        else if(FT1<=157.751)tmp1=177.4;
        else if(FT1<=157.871)tmp1=177.5;
        else if(FT1<=157.991)tmp1=177.6;
        else if(FT1<=158.112)tmp1=177.7;
        else if(FT1<=158.232)tmp1=177.8;
        else if(FT1<=158.352)tmp1=177.9;
        else if(FT1<=158.472)tmp1=178;
    }
    else if(FT1<=159.557){
        if(FT1==158.472)tmp1=178;
        else if(FT1<=158.593)tmp1=178.1;
        else if(FT1<=158.713)tmp1=178.2;
        else if(FT1<=158.833)tmp1=178.3;
        else if(FT1<=158.954)tmp1=178.4;
        else if(FT1<=159.074)tmp1=178.5;
        else if(FT1<=159.195)tmp1=178.6;
        else if(FT1<=159.315)tmp1=178.7;
        else if(FT1<=159.436)tmp1=178.8;
        else if(FT1<=159.557)tmp1=178.9;
        else if(FT1<=159.677)tmp1=179;
    }
    else if(FT1<=160.766){
        if(FT1==159.677)tmp1=179;
        else if(FT1<=159.798)tmp1=179.1;
        else if(FT1<=159.919)tmp1=179.2;
        else if(FT1<=160.04)tmp1=179.3;
        else if(FT1<=160.161)tmp1=179.4;
        else if(FT1<=160.281)tmp1=179.5;
        else if(FT1<=160.402)tmp1=179.6;
        else if(FT1<=160.523)tmp1=179.7;
        else if(FT1<=160.644)tmp1=179.8;
        else if(FT1<=160.766)tmp1=179.9;
        else if(FT1<=160.887)tmp1=180;
    }
    else if(FT1<=161.979){
        if(FT1==160.887)tmp1=180;
        else if(FT1<=161.008)tmp1=180.1;
        else if(FT1<=161.129)tmp1=180.2;
        else if(FT1<=161.25)tmp1=180.3;
        else if(FT1<=161.372)tmp1=180.4;
        else if(FT1<=161.493)tmp1=180.5;
        else if(FT1<=161.614)tmp1=180.6;
        else if(FT1<=161.736)tmp1=180.7;
        else if(FT1<=161.857)tmp1=180.8;
        else if(FT1<=161.979)tmp1=180.9;
        else if(FT1<=162.1)tmp1=181;
    }
    else if(FT1<=163.196){
        if(FT1==162.1)tmp1=181;
        else if(FT1<=162.222)tmp1=181.1;
        else if(FT1<=162.343)tmp1=181.2;
        else if(FT1<=162.465)tmp1=181.3;
        else if(FT1<=162.587)tmp1=181.4;
        else if(FT1<=162.709)tmp1=181.5;
        else if(FT1<=162.83)tmp1=181.6;
        else if(FT1<=162.952)tmp1=181.7;
        else if(FT1<=163.074)tmp1=181.8;
        else if(FT1<=163.196)tmp1=181.9;
        else if(FT1<=163.318)tmp1=182;
    }
    else if(FT1<=164.417){
        if(FT1==163.318)tmp1=182;
        else if(FT1<=163.44)tmp1=182.1;
        else if(FT1<=163.562)tmp1=182.2;
        else if(FT1<=163.684)tmp1=182.3;
        else if(FT1<=163.806)tmp1=182.4;
        else if(FT1<=163.928)tmp1=182.5;
        else if(FT1<=164.051)tmp1=182.6;
        else if(FT1<=164.173)tmp1=182.7;
        else if(FT1<=164.295)tmp1=182.8;
        else if(FT1<=164.417)tmp1=182.9;
        else if(FT1<=164.54)tmp1=183;
    }
    else if(FT1<=165.643){
        if(FT1==164.54)tmp1=183;
        else if(FT1<=164.662)tmp1=183.1;
        else if(FT1<=164.785)tmp1=183.2;
        else if(FT1<=164.907)tmp1=183.3;
        else if(FT1<=165.03)tmp1=183.4;
        else if(FT1<=165.152)tmp1=183.5;
        else if(FT1<=165.275)tmp1=183.6;
        else if(FT1<=165.398)tmp1=183.7;
        else if(FT1<=165.52)tmp1=183.8;
        else if(FT1<=165.643)tmp1=183.9;
        else if(FT1<=165.766)tmp1=184;
    }
    else if(FT1<=166.873){
        if(FT1==165.766)tmp1=184;
        else if(FT1<=165.889)tmp1=184.1;
        else if(FT1<=166.012)tmp1=184.2;
        else if(FT1<=166.135)tmp1=184.3;
        else if(FT1<=166.257)tmp1=184.4;
        else if(FT1<=166.38)tmp1=184.5;
        else if(FT1<=166.504)tmp1=184.6;
        else if(FT1<=166.627)tmp1=184.7;
        else if(FT1<=166.75)tmp1=184.8;
        else if(FT1<=166.873)tmp1=184.9;
        else if(FT1<=166.996)tmp1=185;
    }
    else if(FT1<=168.107){
        if(FT1==166.996)tmp1=185;
        else if(FT1<=167.119)tmp1=185.1;
        else if(FT1<=167.243)tmp1=185.2;
        else if(FT1<=167.366)tmp1=185.3;
        else if(FT1<=167.489)tmp1=185.4;
        else if(FT1<=167.613)tmp1=185.5;
        else if(FT1<=167.736)tmp1=185.6;
        else if(FT1<=167.86)tmp1=185.7;
        else if(FT1<=167.983)tmp1=185.8;
        else if(FT1<=168.107)tmp1=185.9;
        else if(FT1<=168.231)tmp1=186;
    }
    else if(FT1<=169.345){
        if(FT1==168.231)tmp1=186;
        else if(FT1<=168.354)tmp1=186.1;
        else if(FT1<=168.478)tmp1=186.2;
        else if(FT1<=168.602)tmp1=186.3;
        else if(FT1<=168.725)tmp1=186.4;
        else if(FT1<=168.849)tmp1=186.5;
        else if(FT1<=168.973)tmp1=186.6;
        else if(FT1<=169.097)tmp1=186.7;
        else if(FT1<=169.221)tmp1=186.8;
        else if(FT1<=169.345)tmp1=186.9;
        else if(FT1<=169.469)tmp1=187;
    }
    else if(FT1<=170.587){
        if(FT1==169.469)tmp1=187;
        else if(FT1<=169.593)tmp1=187.1;
        else if(FT1<=169.717)tmp1=187.2;
        else if(FT1<=169.841)tmp1=187.3;
        else if(FT1<=169.966)tmp1=187.4;
        else if(FT1<=170.09)tmp1=187.5;
        else if(FT1<=170.214)tmp1=187.6;
        else if(FT1<=170.338)tmp1=187.7;
        else if(FT1<=170.463)tmp1=187.8;
        else if(FT1<=170.587)tmp1=187.9;
        else if(FT1<=170.712)tmp1=188;
    }
    else if(FT1<=171.834){
        if(FT1==170.712)tmp1=188;
        else if(FT1<=170.836)tmp1=188.1;
        else if(FT1<=170.961)tmp1=188.2;
        else if(FT1<=171.085)tmp1=188.3;
        else if(FT1<=171.21)tmp1=188.4;
        else if(FT1<=171.335)tmp1=188.5;
        else if(FT1<=171.459)tmp1=188.6;
        else if(FT1<=171.584)tmp1=188.7;
        else if(FT1<=171.709)tmp1=188.8;
        else if(FT1<=171.834)tmp1=188.9;
        else if(FT1<=171.958)tmp1=189;
    }
    else if(FT1<=173.084){
        if(FT1==171.958)tmp1=189;
        else if(FT1<=172.083)tmp1=189.1;
        else if(FT1<=172.208)tmp1=189.2;
        else if(FT1<=172.333)tmp1=189.3;
        else if(FT1<=172.458)tmp1=189.4;
        else if(FT1<=172.583)tmp1=189.5;
        else if(FT1<=172.709)tmp1=189.6;
        else if(FT1<=172.834)tmp1=189.7;
        else if(FT1<=172.959)tmp1=189.8;
        else if(FT1<=173.084)tmp1=189.9;
        else if(FT1<=173.209)tmp1=190;
    }
    else if(FT1<=174.339){
        if(FT1==173.209)tmp1=190;
        else if(FT1<=173.335)tmp1=190.1;
        else if(FT1<=173.46)tmp1=190.2;
        else if(FT1<=173.585)tmp1=190.3;
        else if(FT1<=173.711)tmp1=190.4;
        else if(FT1<=173.836)tmp1=190.5;
        else if(FT1<=173.962)tmp1=190.6;
        else if(FT1<=174.087)tmp1=190.7;
        else if(FT1<=174.213)tmp1=190.8;
        else if(FT1<=174.339)tmp1=190.9;
        else if(FT1<=174.464)tmp1=191;
    }
    else if(FT1<=175.597){
        if(FT1==174.464)tmp1=191;
        else if(FT1<=174.59)tmp1=191.1;
        else if(FT1<=174.716)tmp1=191.2;
        else if(FT1<=174.842)tmp1=191.3;
        else if(FT1<=174.967)tmp1=191.4;
        else if(FT1<=175.093)tmp1=191.5;
        else if(FT1<=175.219)tmp1=191.6;
        else if(FT1<=175.345)tmp1=191.7;
        else if(FT1<=175.471)tmp1=191.8;
        else if(FT1<=175.597)tmp1=191.9;
        else if(FT1<=175.723)tmp1=192;
    }
    else if(FT1<=176.86){
        if(FT1==175.723)tmp1=192;
        else if(FT1<=175.85)tmp1=192.1;
        else if(FT1<=175.976)tmp1=192.2;
        else if(FT1<=176.102)tmp1=192.3;
        else if(FT1<=176.228)tmp1=192.4;
        else if(FT1<=176.354)tmp1=192.5;
        else if(FT1<=176.481)tmp1=192.6;
        else if(FT1<=176.607)tmp1=192.7;
        else if(FT1<=176.734)tmp1=192.8;
        else if(FT1<=176.86)tmp1=192.9;
        else if(FT1<=176.987)tmp1=193;
    }
    else if(FT1<=178.127){
        if(FT1==176.987)tmp1=193;
        else if(FT1<=177.113)tmp1=193.1;
        else if(FT1<=177.24)tmp1=193.2;
        else if(FT1<=177.366)tmp1=193.3;
        else if(FT1<=177.493)tmp1=193.4;
        else if(FT1<=177.62)tmp1=193.5;
        else if(FT1<=177.746)tmp1=193.6;
        else if(FT1<=177.873)tmp1=193.7;
        else if(FT1<=178)tmp1=193.8;
        else if(FT1<=178.127)tmp1=193.9;
        else if(FT1<=178.254)tmp1=194;
    }
    else if(FT1<=179.398){
        if(FT1==178.254)tmp1=194;
        else if(FT1<=178.381)tmp1=194.1;
        else if(FT1<=178.508)tmp1=194.2;
        else if(FT1<=178.635)tmp1=194.3;
        else if(FT1<=178.762)tmp1=194.4;
        else if(FT1<=178.889)tmp1=194.5;
        else if(FT1<=179.016)tmp1=194.6;
        else if(FT1<=179.143)tmp1=194.7;
        else if(FT1<=179.271)tmp1=194.8;
        else if(FT1<=179.398)tmp1=194.9;
        else if(FT1<=179.525)tmp1=195;
    }
    else if(FT1<=180.673){
        if(FT1==179.525)tmp1=195;
        else if(FT1<=179.652)tmp1=195.1;
        else if(FT1<=179.78)tmp1=195.2;
        else if(FT1<=179.907)tmp1=195.3;
        else if(FT1<=180.035)tmp1=195.4;
        else if(FT1<=180.162)tmp1=195.5;
        else if(FT1<=180.29)tmp1=195.6;
        else if(FT1<=180.417)tmp1=195.7;
        else if(FT1<=180.545)tmp1=195.8;
        else if(FT1<=180.673)tmp1=195.9;
        else if(FT1<=180.8)tmp1=196;
    }
    else if(FT1<=181.952){
        if(FT1==180.8)tmp1=196;
        else if(FT1<=180.928)tmp1=196.1;
        else if(FT1<=181.056)tmp1=196.2;
        else if(FT1<=181.184)tmp1=196.3;
        else if(FT1<=181.312)tmp1=196.4;
        else if(FT1<=181.44)tmp1=196.5;
        else if(FT1<=181.568)tmp1=196.6;
        else if(FT1<=181.696)tmp1=196.7;
        else if(FT1<=181.824)tmp1=196.8;
        else if(FT1<=181.952)tmp1=196.9;
        else if(FT1<=182.08)tmp1=197;
    }
    else if(FT1<=183.235){
        if(FT1==182.08)tmp1=197;
        else if(FT1<=182.208)tmp1=197.1;
        else if(FT1<=182.336)tmp1=197.2;
        else if(FT1<=182.464)tmp1=197.3;
        else if(FT1<=182.593)tmp1=197.4;
        else if(FT1<=182.721)tmp1=197.5;
        else if(FT1<=182.849)tmp1=197.6;
        else if(FT1<=182.978)tmp1=197.7;
        else if(FT1<=183.106)tmp1=197.8;
        else if(FT1<=183.235)tmp1=197.9;
        else if(FT1<=183.363)tmp1=198;
    }
    else if(FT1<=184.522){
        if(FT1==183.363)tmp1=198;
        else if(FT1<=183.492)tmp1=198.1;
        else if(FT1<=183.62)tmp1=198.2;
        else if(FT1<=183.749)tmp1=198.3;
        else if(FT1<=183.878)tmp1=198.4;
        else if(FT1<=184.006)tmp1=198.5;
        else if(FT1<=184.135)tmp1=198.6;
        else if(FT1<=184.264)tmp1=198.7;
        else if(FT1<=184.393)tmp1=198.8;
        else if(FT1<=184.522)tmp1=198.9;
        else if(FT1<=184.651)tmp1=199;
    }
    else if(FT1<=185.813){
        if(FT1==184.651)tmp1=199;
        else if(FT1<=184.78)tmp1=199.1;
        else if(FT1<=184.909)tmp1=199.2;
        else if(FT1<=185.038)tmp1=199.3;
        else if(FT1<=185.167)tmp1=199.4;
        else if(FT1<=185.296)tmp1=199.5;
        else if(FT1<=185.425)tmp1=199.6;
        else if(FT1<=185.554)tmp1=199.7;
        else if(FT1<=185.683)tmp1=199.8;
        else if(FT1<=185.813)tmp1=199.9;
        else if(FT1<=185.942)tmp1=200;
    }
    return tmp1;
}

