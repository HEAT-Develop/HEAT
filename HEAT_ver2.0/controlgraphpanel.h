//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//

#ifndef CONTROLGRAPHPANEL_H
#define CONTROLGRAPHPANEL_H

#include "database.h"
#include <FITS.h>
#include <QDialog>
#include <QFileDialog>
#include <QProgressDialog>
#include <QtSql>


namespace Ui {
class ControlGraphPanel;
}

struct imageData {
  QVector<double> image;
  double bol_t;
  double pkg_t;
  double case_t;
  double shut_t;
  double lens_t;
  double target_t;
  double distance;
};

struct interpolateFunction {
  double DN;
  double targetT;
  double distAve;
};

class ControlGraphPanel : public QDialog {
  Q_OBJECT

public:
  explicit ControlGraphPanel(QWidget *parent = 0);
  ~ControlGraphPanel();
  ControlGraphPanel *t[50];

  int n;
  std::valarray<double> slope,intercept;
  int h;
  int w;


private:
  Ui::ControlGraphPanel *ui;
  QList<QString> position;
  int i, j;

  QString fileName;
  QString subFileName;

  QSqlDatabase db;
  QSqlQuery query;
  QSqlQuery pairQuery;
  QString databPath;
  std::string fitskeys;
  double fitsave =0;


  void readLUTfile(QString);
  void conversionLUT(QString);
  void DarkImage();
  void on_BlackbodycalibrationAllPixelButton_clicked();
  void calibrationLutFile();
  void on_pushButton_clicked();
  void on_directConversionButton_clicked();
  void on_BlackbodycalibrationAllPixelButton_repeat_clicked();
  void on_calibrationtoRadianceButton_clicked();
  void on_calibrationButton_clicked();
signals:
  void showSelectGrphSignal(int, int);
  void showCalibrationPanelSignal();
  void closeSelectGrphSignal(int, int);
  void changeX(QString);
  void changeY(QString);
  void FITSinfoSignal1(QString *);

  void exportfilename(QString);

public slots:
  void getDataPath(QString);

private slots:
  void popControlGraphPanel();
  void on_loadFileButton_clicked();
  void on_substructButton_clicked();
  //void on_confirmation_clicked();
  void setX(QString x);
  void setY(QString y);

  void connectDB();

  static bool compDist(const imageData e1, const imageData e2);
  void on_outputCurrentImageButton_clicked();
  void on_substructFITSButton_clicked();
  void on_StartConversionButton_clicked();
};

#endif
