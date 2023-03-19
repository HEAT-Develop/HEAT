//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//

#include "pixcelgraph.h"
#include "ui_pixcelgraph.h"

PixcelGraph::PixcelGraph(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PixcelGraph)
{
    ui->setupUi(this);
    this->setWindowTitle("Pixcel Line Graph");
}

PixcelGraph::~PixcelGraph()
{
    delete ui;
}

void PixcelGraph::drawGraph(QVector<double> H, QVector<double> V){
    ui->widget->clearGraphs();

    QCPGraph *graph = new QCPGraph(ui->widget->xAxis, ui->widget->yAxis);
    ui->widget->addPlottable(graph);
    ui->widget->xAxis->setLabel("x-coordinate of image");
    ui->widget->yAxis->setLabel("digital number");
    graph->setData(H, V);
    ui->widget->rescaleAxes();
    ui->widget->replot();
}
