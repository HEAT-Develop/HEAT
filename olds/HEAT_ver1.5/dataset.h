#ifndef DATASET_H
#define DATASET_H

#include <QObject>
#include <QWidget>
#include <QtCore>
#include "project.h"

struct fitslevel{
    int Height_data;
    int Width_data;
    QString unit;
};
struct setting{
    float h_plank;
    float kb;
    int c_speed;
    float SIGMA;
};
struct dbinfo{
   QString user;
   QString host;
   QString dbname;
   QString pass;
};

class dataset
{
public:
    dataset();
    int No;
    QString scname;
    QString Camera;
    fitslevel l1;
    fitslevel l2;
    int band;
    QString path;
    setting set;
    int pixelxy;
    dbinfo db_info;
};

extern dataset xmldata;



#endif // DATASET_H
