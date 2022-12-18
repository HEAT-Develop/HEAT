//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//

#ifndef TARGETMODEL_H
#define TARGETMODEL_H

#include <QDialog>

namespace Ui {
class TargetModel;
}

class TargetModel : public QDialog
{
    Q_OBJECT

public:
    explicit TargetModel(QWidget *parent = 0);
    ~TargetModel();

private:
    Ui::TargetModel *ui;
};

#endif // TARGETMODEL_H
