//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//

#include "vtkmodel.h"
#include "ui_vtkmodel.h"
#include <QDir>
VtkModel::VtkModel(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VtkModel)
{
    ui->setupUi(this);
    this->setWindowTitle("Thermal model");

    QObject::connect(ui->vtkDataSelect,SIGNAL(currentIndexChanged(int)),ui->vtkRender,SLOT(changeData(int)));
    QObject::connect(ui->colorSelect,SIGNAL(currentIndexChanged(int)),ui->vtkRender,SLOT(setColor(int)));
    QObject::connect(ui->loadVtkButton,SIGNAL(clicked()),this,SLOT(loadVtkData()));
    QObject::connect(this,SIGNAL(dirPathSignal(QString)),ui->vtkRender,SLOT(getDirPath(QString)));
    QObject::connect(ui->vtkList,SIGNAL(currentTextChanged(QString)),ui->vtkRender,SLOT(loadVtkFile(QString)));
    QObject::connect(this,SIGNAL(changeRangeVtkSignal(double,double)),ui->vtkRender,SLOT(setRangeVtk(double,double)));
}

VtkModel::~VtkModel()
{
    delete ui;
}



void VtkModel::loadVtkData()
{

    QDir vtkSrc = QFileDialog::getExistingDirectory(this,"Open Directory",QDir::homePath());
    ui->vtkList->clear();
     if(vtkSrc.exists()){
          QStringList filelist = vtkSrc.entryList();
          ui->vtkList->addItems(filelist);
     }
     ui->vtkList->takeItem(0);
     ui->vtkList->takeItem(0);
     ui->vtkList->takeItem(0);
     emit dirPathSignal(vtkSrc.path());

}

void VtkModel::on_applyButton_clicked()
{
    if(ui->setMin->text()!=NULL && ui->setMax->text()!=NULL)
      emit changeRangeVtkSignal(ui->setMin->text().toDouble(), ui->setMax->text().toDouble());
}
