//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//

#ifndef SHOWIMAGE
#define SHOWIMAGE

#include "pixcelgraph.h"
#include <CCfits/CCfits>
#include <QDir>
#include <QGLWidget>
#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QWidget>
#include <calibrationgraph.h>
#include <fitsio.h>

#define PI (4.0 * atan(1.0))
#define Max_Height 768
#define Max_Width 1024


using namespace CCfits;

class ShowImage : public QOpenGLWidget {
  Q_OBJECT

  public:
  explicit ShowImage(QWidget *parent = 0);
  ~ShowImage();
  double planck(double T);

  std::valarray<double> slope,intercept;
  void calibrationLutImage(double,double,int,int);
  void moveimage();
  void outputCurrentImage();
  void SetDarkImage(QString getdarkfilename);
  void initializeFITSarray();
  double csv_data[350][2];
  QPoint startPos_;
  QPoint endPos;
  void initializedarkimage();

  void subtractFITSImage(QString file1);
  int Width_i;
  int Height_i;
  double pkgt = 0;
  double cast = 0;
  double shtt = 0;
  double lent = 0;


  string origin;
  string date;
  string date_beg;
  string date_obs;
  string date_end;
  string telescop;
  string instrume;
  string object;
  string bunit;
  double xposure;
  double ifov;
  string filter;
  string oprgname;
  string oprgno;
  double roi_llx;
  double roi_lly;
  double roi_urx;
  double roi_ury;
  double datamax;
  double datamin;
  double mean;
  double stdev;
  double miss_val;
  double miss_num;
  double dead_val;
  double dead_num;
  double satu_val;
  double satu_num;
  string imgcmprv;
  string imgcmpal;
  string imgcmppr;
  double img_err;
  string imgseqc;
  double imgaccm;
  double bitdepth;
  string plt_pow;
  string plt_stat;
  string bol_stat;
  double bol_trgt;
  double bol_rang;
  double bol_temp;
  double pkg_temp;
  double cas_temp;
  double sht_temp;
  double len_temp;
  double bgr_vol;
  double vb1_vol;
  double adofsvol;
  double hce_temp;
  double pnl_temp;
  double ae_temp;
  double s_distht;
  double s_disthe;
  double s_disths;
  double s_distts;
  double s_tgradi;
  double s_appdia;
  double s_sollat;
  double s_sollon;
  double s_ssclat;
  double s_ssclon;
  double s_ssclst;
  double s_sscpx;
  double s_sscpy;
  double s_scxsan;
  double s_scysan;
  double s_sczsan;
  string naifname;
  double naifid;
  string mkname;
  double version;
  double rad2temp(double,int,QSqlDatabase);
  QSqlDatabase db;

  void OutputFITSDBsubtractImage(QString file1, QString file2);
  void calibrateLUTl2a(double g, double h, int y, int x);
private:
  void render();
  void renderunit();
  void loadBuffer();
  void mousePressEvent(QMouseEvent *event);
  void pixelDraw(double T);
  void makeColorTable();
  void connectDB();
  double epsilon = 1;
  //double epsilon=0.97;
  void readLUTfile(QString);
  void readLUTfile(QString,double *,double *,int,int);


  //QSqlDatabase db;
  QSqlQuery query;
  QRubberBand *rubberBand_;

  QDir filterpath;
  QString filename;
  double colorTable[256][3];
  double image[Max_Height][Max_Width];
  double FITSsubimage[Max_Height][Max_Width];
  double fitstemperature[Max_Width * Max_Height];
  double fitstemperature2[Max_Height][Max_Width];
  double darkimage[Max_Height][Max_Width];
  double imagetmp[100000][6];
  double calibrationImage[Max_Height][Max_Width];
  double tirfilterforshowimage[2000][3];
  double colorValue[256];
  double MAX_V;
  double MIN_V;
  bool judge;
  bool renderunitflag;
  int colorselect;

  double fitsimage[500][500];
  double fitsimagesub[500][500];
  int fitsflag = 0;
  void loadFilter();
  double h_planck;
  double kB;
  double c_speed;
  double c1;
  double c2;
  double SIGMA;

  double round3(double dIn);
  double round2(double dIn);
  double round1(double dIn);
  string ReplaceString(string String1, std::string String2,
                       std::string String3);

  PixcelGraph pg;


  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);

  void lsm_slope(double[], double[], double *, double *);
  void lsm_intercept(double[], double[], double *, double *);
  void lsm_slope_threeh(double[], double[], double *, double *);
  void lsm_intercept_threeh(double[], double[], double *, double *);
  void slope_300(double[], double[], double *, double *);
  void intercept_300(double[], double[], double *, double *);
protected:
  void initializeGL();
  void resizeGL(int width, int height);
  void paintGL();

signals:
  void valueChangedX(QString);
  void valueChangedY(QString);
  void valueChangedPixel(QString);

public slots:
  void setValueX(QString value);
  void setValueY(QString value);
  void setValuePixel(QString value);
  void loadFileName(QString name);
  void changeParameter(double, double, int);
  void subtractImage(QString, QString);
  double getPixelValue(int y, int x);
  void drawPixcelLineGraph(QString);
  void calibrateImage(QString s, int x, int y);
  void confirmation(QString s, int x, int y, QString subFileName1);
  void calibrateImageforBlackbodyAllPixel(QString s, int x, int y);
  void calibrateImagetoRadianceforBlackbodyAllPixel(QString s, int x, int y);
  void updateImage(int judge, QString dirpath, QString fitdirectory);
  void initializeCalibrateImage();
  void setHeight(int a);
  void setWidth(int a);
  int getHeight();
  int getWidth();
  QVector<double> getImageD();
  void loadImageD(QVector<double>);
  void loadTxtImageD(QString);
  void openTxtImageD(QString);

  int getMaxDN();
  int getMinDN();
};

#endif
