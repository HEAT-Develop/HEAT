//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//

#ifndef VTKMODEL_H
#define VTKMODEL_H

#include <QDialog>
namespace Ui {
class VtkModel;
}

class VtkModel : public QDialog
{
    Q_OBJECT

public:
    explicit VtkModel(QWidget *parent = 0);
    ~VtkModel();

private slots:

    void on_applyButton_clicked();

signals:
    void dirPathSignal(QString);
    void changeRangeVtkSignal(double, double);

private:
    Ui::VtkModel *ui;

public slots:
    void loadVtkData();
};

#endif // VTKMODEL_H
