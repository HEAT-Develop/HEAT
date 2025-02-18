//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//

#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QDialog>
namespace Ui {
class Controlpanel;
}

class Controlpanel : public QDialog
{
    Q_OBJECT

public:
    explicit Controlpanel(QWidget *parent = 0);
    ~Controlpanel();

private slots:
    void on_showImageButton_clicked();
    void on_showModelButton_clicked();
    void on_showDBButton_clicked();

    void on_quitButton_clicked();

    void on_showVtkButton_clicked();

    void on_showCaliButton_clicked();
    void on_showConversionButton_clicked();

    void on_comboBox_currentTextChanged(const QString &arg1);

     void on_showReadTxtButton_clicked();

private:
    Ui::Controlpanel *ui;
    int judgeTableNameInt(int, int);

signals:
    void showImageSignal();
    void showDatabaseSignal();
    void showTargetModelSignal();
    void quitSystemSignal();
    void showVtkSignal();
    void showCaliSignal();
    void showControlGraphPanel();
    void showReadTxtSignal();
    void showCali_tiriSignal();
};

#endif // CONTROLPANEL_H
