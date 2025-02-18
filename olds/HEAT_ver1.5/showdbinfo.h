//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//

#ifndef SHOWDBINFO_H
#define SHOWDBINFO_H

#include <QDialog>

namespace Ui {
class ShowDBInfo;
}

class ShowDBInfo : public QDialog
{
    Q_OBJECT

public:
    explicit ShowDBInfo(QWidget *parent = 0);
    ~ShowDBInfo();

public slots:
    void getInfo(QString*);

private:
    Ui::ShowDBInfo *ui;
};

#endif // SHOWDBINFO_H
