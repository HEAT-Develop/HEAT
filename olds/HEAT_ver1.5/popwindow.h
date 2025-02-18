//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//

#ifndef POPWINDOW_H
#define POPWINDOW_H

#include <QDialog>
#include <QGraphicsScene>
#include <QFileDialog>
#include <QLineEdit>

namespace Ui {
class PopWindow;
}

class PopWindow : public QDialog
{
    Q_OBJECT

public:
    explicit PopWindow(QWidget *parent = 0);
    ~PopWindow();
    PopWindow *t[50];
    QPoint position[50];
    int nameLength[50];

    int n;

signals:


private:
    Ui::PopWindow *ui;
    QString importName[50];

public slots:
    void getFileName(QString,int);
    void tilingWindow();
    void closeAll();
    void changeParameter(double,double,int);
    void substract();
private slots:
};

#endif // POPWINDOW_H
