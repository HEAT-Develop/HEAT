//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <Qtsql>
#include <QFileDialog>
#include <QLineEdit>
#include <qtextedit.h>


#define MAXH 25555
#define MAXL -25555

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QPixmap pixmap;
    QGraphicsScene scene;
    QString exportFileName;

    QTextEdit *textEdit;

        QAction *loadAction;
        QAction *saveAction;
        QAction *exitAction;

        QMenu *fileMenu;


signals:
    void loadFileSignal(QString);
    void loadThumbnailSignal(QString);
    void exportDataSignal(QString);
    void tilingWindowSignal();

private slots:

    void exportWindow();
    void on_loadDataButton_clicked();
    void on_loadedDataList_clicked(const QModelIndex &index);
    void on_windowList_clicked(const QModelIndex &index);
    void makePixelInfoList(QString);
    void on_tilingWindowButton_clicked();



public slots:
    void on_action_Open_triggered();
    void loadFile(QString name);



};

#endif // MAINWINDOW_H
