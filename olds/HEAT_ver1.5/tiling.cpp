//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//

#include "tiling.h"
#include "ui_tiling.h"
#include "popwindow.h"
#include "showimage.h"
#include "mainwindow.h"
#include "dataset.h"
#include <QLabel>

#define W 494
#define H 276

int i=0;
int j=0;


Tiling::Tiling(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Tiling)
{
    ui->setupUi(this);
    Layout = new QGridLayout;
    this->setWindowTitle("Tiling");

}

Tiling::~Tiling()
{
    delete ui;
}

void Tiling::tilingWindow(QString name)
{
  ShowImage *a = new ShowImage(this);
  a->setFixedSize(W,H);
  a->loadFileName(name);
  if(j>2){i++;j=0;}
  Layout->addWidget(a,i,j++);
  this->setLayout(Layout);
}
