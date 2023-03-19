//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//

#include "controlpanel.h"
#include "ui_controlpanel.h"
#include "database.h"
#include <QFileDialog>
#include "dataset.h"
#include "project.h"


Controlpanel::Controlpanel(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Controlpanel)
{
    ui->setupUi(this);
    this->setWindowTitle("Control");
    this->move(400,30);
    QStringList list = project::setMissionName();
    ui->comboBox->addItems(list);
}

Controlpanel::~Controlpanel()
{
    delete ui;
}

void Controlpanel::on_showModelButton_clicked()
{
    emit showTargetModelSignal();
}

void Controlpanel::on_showDBButton_clicked()
{
    emit showDatabaseSignal();
}

void Controlpanel::on_showImageButton_clicked()
{   printf("ggggggg\n");
    emit showImageSignal();
}

void Controlpanel::on_quitButton_clicked()
{
    emit quitSystemSignal();
}

void Controlpanel::on_showVtkButton_clicked()
{
    emit showVtkSignal();
}

void Controlpanel::on_showCaliButton_clicked()
{
    emit showCaliSignal();
  //  emit showControlGraphPanel();
}

void Controlpanel::on_showConversionButton_clicked(){
    emit showControlGraphPanel();
}

void Controlpanel::on_comboBox_currentTextChanged(const QString &arg1)
{
    project::parser(arg1);
}

