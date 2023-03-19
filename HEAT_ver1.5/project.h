#ifndef PROJECT_H
#define PROJECT_H
#include <QtXml>
#include <QTreeWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QObject>
#include <QtCore>

class project
{
public:
    project();

    static void parser(QString);
    void setDocument();
    static void parseEntry(const QDomElement &element);
    static QStringList setMissionName();
private:
    QTreeWidget *treeWidget;
};

#endif // PROJECT_H
