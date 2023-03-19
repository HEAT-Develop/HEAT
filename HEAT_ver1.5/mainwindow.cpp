//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "rendering.h"
#include "showimage.h"
#include "popwindow.h"
#include "dataset.h"
#include <iostream>

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);    


    this->setWindowTitle("TIR_Viewer");
    ui->tabWidget->setTabText(0,"Viewer");
    ui->tabWidget->setTabText(1,"Database");

    QObject::connect(ui->pushButton_4, SIGNAL( clicked() ),
                        this, SLOT( on_action_Open_triggered()) );
    //x,y座標表示のためのスロットシグナル設定
    QObject::connect(ui->imageViewer, SIGNAL( valueChangedX(QString) ),
                       ui->coordinate_x, SLOT( setText(QString) ));
    QObject::connect(ui->imageViewer, SIGNAL( valueChangedY(QString) ),
                       ui->coordinate_y, SLOT( setText(QString) ));
    QObject::connect(ui->imageViewer, SIGNAL( valueChangedPixel(QString)),
                       this, SLOT( makePixelInfoList(QString )));
    //img表示用
    QObject::connect(this, SIGNAL( loadFileSignal(QString) ),
                       ui->imageViewer, SLOT( loadFileName(QString) ));
    //Windowに表示
    QObject::connect(ui->exportWindowButton, SIGNAL( clicked() ),
                       this , SLOT( exportWindow() ));




    //モデル選択
    QObject::connect(ui->modelSelect, SIGNAL( currentIndexChanged(int) ),
                       ui->modelRendering , SLOT( changeModel(int) ));



}

MainWindow::~MainWindow()
{

    delete ui;
}

void MainWindow::on_action_Open_triggered()
{
       QString fileName = QFileDialog::getOpenFileName(this,
                                      tr("Open Image"), ".", tr("Image Files (*.png *.jpg *.bmp *.img)"));
       pixmap.load(fileName);
       scene.addPixmap(pixmap);

       ui->loadedDataList->addItem(fileName);
}

void MainWindow::on_loadDataButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                      tr("Open Image"), ".", tr("Image Files (*.png *.jpg *.bmp *.img)"));
    loadFile(fileName);

    ui->loadedDataList->addItem(fileName);
    ui->loadedDataList->setCurrentRow(ui->loadedDataList->count()-1);
    exportFileName = ui->loadedDataList->item(ui->loadedDataList->count()-1)->text();


}


void MainWindow::on_loadedDataList_clicked(const QModelIndex &index)
{
    loadFile(ui->loadedDataList->item(index.row())->text());
    exportFileName = ui->loadedDataList->item(index.row())->text();
}

void MainWindow::loadFile(QString name){
    emit loadFileSignal(name);
}

void MainWindow::exportWindow()
{
    bool judge=true;
    for(int i=0;i<ui->windowList->count();i++){
        if(ui->loadedDataList->currentItem()->text() == ui->windowList->item(i)->text()){judge=false; break;}
    }


    if(ui->loadedDataList->currentItem() != NULL && judge){
    ui->windowList->addItem(exportFileName);
    ui->windowList->setCurrentRow(ui->windowList->count()-1);
    emit exportDataSignal(exportFileName);
    }
}



void MainWindow::on_windowList_clicked(const QModelIndex &index)
{
    emit exportDataSignal(ui->loadedDataList->item(index.row())->text());
}

void MainWindow::makePixelInfoList(QString value)
{
   ui->pixelInfoList->clear();
   ui->pixelInfoList->addItem("value: "+ value);
}

void MainWindow::on_tilingWindowButton_clicked()
{
    emit tilingWindowSignal();
}


