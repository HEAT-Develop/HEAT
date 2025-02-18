//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//

#include "popwindow.h"
#include "ui_popwindow.h"
#include "showimage.h"
#include "mainwindow.h"
#include "dataset.h"
#include <iostream>

PopWindow::PopWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PopWindow)
{
    ui->setupUi(this);
    n=0;
}

PopWindow::~PopWindow()
{
    delete ui;
}

void PopWindow::getFileName(QString name,int x){

    t[n] = new PopWindow;

    importName[n] = name;

    nameLength[n] = x;
    ui->setupUi(t[n]);

    t[n]->setWindowTitle(importName[n].right(nameLength[n]));

    ui->anotherView->loadFileName(importName[n]);

    t[n]->show();
    t[n]->activateWindow();
    n++;

}

void PopWindow::tilingWindow(){
    int j=0,k=0;

    for(int i = 1; i<n+1;i++,k++){
        if(i%4==0){
            j++;
            k=0;
        }
    t[i-1]->move(0+k*470,0+j*330);
    }
}

void PopWindow::closeAll(){
    for(int i = 0; i<n;i++){
        t[i]->destroy();
    }

    n=0;
}
void PopWindow::changeParameter(double min, double max, int x)
{

    for(int i = 0; i<n;i++){
        position[i] = t[i]->pos();
        t[i]->destroy();
    }

    for(int i = 0; i<n;i++){
        t[i] = new PopWindow;
        ui->setupUi(t[i]);
        t[i]->move(position[i]);
        ui->anotherView->loadFileName(importName[i]);
        ui->anotherView->changeParameter(min,max,x);
        t[i]->setWindowTitle(importName[i].right(nameLength[i]));
        t[i]->update();
        t[i]->show();
    }
}

void PopWindow::substract()
{

        std::cout<<"pop"<<std::endl;
    t[n] = new PopWindow;
    ui->setupUi(t[n]);
    t[n]->setWindowTitle(importName[n-1].right(nameLength[n-1])+"sub");

   ui->anotherView->subtractImage(importName[n-1],importName[n-2]);

    t[n]->show();
    t[n]->activateWindow();
    n++;
}



