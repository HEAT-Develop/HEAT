//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//

#include "targetmodel.h"
#include "ui_targetmodel.h"

TargetModel::TargetModel(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TargetModel)
{
    ui->setupUi(this);
    this->setWindowTitle("3D Model");
    QObject::connect(ui->modelSelect, SIGNAL( currentIndexChanged(int) ),
                       ui->modelRendering , SLOT( changeModel(int) ));
}

TargetModel::~TargetModel()
{
    delete ui;
}
