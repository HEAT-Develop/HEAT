#include "project.h"
#include "dataset.h"
#include <QApplication>
#include <QCoreApplication>

dataset xmldata;

project::project()
{

}
void project::parser(QString missionname){
    //path -> /HEAT.app/Contents/MacOS/heat.xml
    //Set xml file
    //QString filename = QApplication::applicationDirPath() + "/heat.xml";
    QString filename = "/Users/konnoryuji/Desktop/heat.xml";
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(0, QObject::tr("Project XML file"),
                             QObject::tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(file.fileName()),
                                  file.errorString()));
        return;
    }
    QIODevice *device(&file);
    QString errorStr;
    int errorLine;
    int errorColumn;
    QDomDocument doc;
    if (!doc.setContent(device, true, &errorStr, &errorLine,
                        &errorColumn))
    {
        QMessageBox::warning(0, QObject::tr("DOM Parser"),
                             QObject::tr("Parse error at line %1, "
                                         "column %2:\n%3")
                             .arg(errorLine)
                             .arg(errorColumn)
                             .arg(errorStr));
        return;
    }
    else qDebug()<<"can read";
    QDomElement root = doc.documentElement();
    if(root.tagName() != "projects"){
        QMessageBox::warning(0,QObject::tr("Checking project format"),QObject::tr("Ilegal format for HEAT."));
        qDebug()<<"Not read XML file.";
        return;
    }
    QDomNode node = root.firstChild();
    while(!node.isNull()){
        if(node.toElement().tagName() == "project"){
            if(node.toElement().attribute("name") == missionname){
                parseEntry(node.toElement());
            }
        }
        node = node.nextSibling();
    }
}

void project::parseEntry(const QDomElement &element){

    QDomNode node = element.firstChild();
    qDebug()<<element.toText().data();
    while(!node.isNull()){
        if(node.toElement().tagName() == "project"){
            parseEntry(node.toElement());
        }
        else{
            if(node.toElement().tagName() == "No"){
                qDebug()<<node.toElement().tagName();
                qDebug()<<node.firstChild().toText().data();
                xmldata.No = node.firstChild().toText().data().toInt();
            }
            else if(node.toElement().tagName() == "Name"){
                qDebug()<<node.toElement().tagName();
                qDebug()<<node.firstChild().toText().data();
                xmldata.scname = node.firstChild().toText().data();
            }
            else if(node.toElement().tagName() == "Camera"){
                qDebug()<<node.toElement().tagName();
                xmldata.Camera = node.firstChild().toText().data();
                qDebug()<<xmldata.Camera;
            }
            else if(node.toElement().tagName() == "data"){
                QDomNode datanode = node.firstChild();
                while(!datanode.isNull()){
                    if(datanode.toElement().tagName() == "fits"){
                        QDomNode fitsnode = datanode.firstChild();
                        while(!fitsnode.isNull()){
                            if(fitsnode.toElement().tagName() == "l1"){
                                QDomNode l1node = fitsnode.firstChild();
                                while(!l1node.isNull()){
                                    if(l1node.toElement().tagName() == "Height"){
                                        qDebug()<<l1node.firstChild().toText().data();
                                        xmldata.l1.Height_data = l1node.firstChild().toText().data().toInt();
                                    }
                                    else if(l1node.toElement().tagName() == "Width"){
                                        xmldata.l1.Width_data = l1node.firstChild().toText().data().toInt();
                                    }
                                    else if(l1node.toElement().tagName() == "Unit"){
                                        xmldata.l1.unit = l1node.firstChild().toText().data();
                                    }
                                    l1node = l1node.nextSibling();
                                }
                            }
                            else if(fitsnode.toElement().tagName() == "l2"){
                                QDomNode l2node = fitsnode.firstChild();
                                while(!l2node.isNull()){
                                    if(l2node.toElement().tagName() == "Height"){
                                        qDebug()<<l2node.firstChild().toText().data();
                                        xmldata.l2.Height_data = l2node.firstChild().toText().data().toInt();
                                    }
                                    else if(l2node.toElement().tagName() == "Width"){
                                        xmldata.l2.Width_data = l2node.firstChild().toText().data().toInt();
                                    }
                                    else if(l2node.toElement().tagName() == "Unit"){
                                        xmldata.l2.unit = l2node.firstChild().toText().data();
                                    }
                                    l2node = l2node.nextSibling();
                                }
                            }
                            else if(fitsnode.toElement().tagName() == "header"){

                            }
                            fitsnode = fitsnode.nextSibling();
                        }
                    }
                    else if(datanode.toElement().tagName() == "band"){
                        xmldata.band = datanode.firstChild().toText().data().toInt();
                    }
                    else if(datanode.toElement().tagName() == "path"){
                        xmldata.path = datanode.firstChild().toText().data();
                    }
                    else if(datanode.toElement().tagName() == "calidata"){
                        QDomNode calidatanode = datanode.firstChild();
                        while(!calidatanode.isNull()){
                            if(calidatanode.toElement().tagName() == "setting"){
                                QDomNode settingnode = calidatanode.firstChild();
                                while(!settingnode.isNull()){
                                    if(settingnode.toElement().tagName() == "h_plank"){
                                        xmldata.set.h_plank = settingnode.firstChild().toText().data().toFloat();
                                    }
                                    else if(settingnode.toElement().tagName() =="kb"){
                                        xmldata.set.kb = settingnode.firstChild().toText().data().toFloat();
                                    }
                                    else if(settingnode.toElement().tagName() == "c_speed"){
                                        xmldata.set.c_speed = settingnode.firstChild().toText().data().toInt();
                                    }
                                    else if(settingnode.toElement().tagName() == "SIGMA"){
                                        xmldata.set.SIGMA = settingnode.firstChild().toText().data().toFloat();
                                    }
                                    settingnode = settingnode.nextSibling();
                                }
                            }
                            else if(calidatanode.toElement().tagName() == "pixelxy"){
                                xmldata.pixelxy = calidatanode.firstChild().toText().data().toInt();
                            }
                            else if(calidatanode.toElement().tagName() == "database"){
                                QDomNode dbnode = calidatanode.firstChild();
                                while(!dbnode.isNull()){
                                    if(dbnode.toElement().tagName() == "user"){
                                        xmldata.db_info.user = dbnode.firstChild().toText().data();
                                    }
                                    else if(dbnode.toElement().tagName() == "host"){
                                        xmldata.db_info.host = dbnode.firstChild().toText().data();
                                    }
                                    else if(dbnode.toElement().tagName() == "dbname"){
                                        xmldata.db_info.dbname = dbnode.firstChild().toText().data();
                                    }
                                    else if(dbnode.toElement().tagName() == "pass"){
                                        xmldata.db_info.pass = dbnode.firstChild().toText().data();
                                    }
                                    dbnode = dbnode.nextSibling();
                                }
                            }
                            calidatanode = calidatanode.nextSibling();
                        }
                    }
                    else if(datanode.toElement().tagName() == "file"){

                    }
                    datanode = datanode.nextSibling();
                }
            }
        }
        node = node.nextSibling();
    }
}

QStringList project::setMissionName()
{
    QStringList list;
    QFile file("/Users/konnoryuji/Desktop/heat.xml");
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(0, QObject::tr("Project XML file"),
                             QObject::tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(file.fileName()),
                                  file.errorString()));
        return list;
    }
    QString errorStr;
    QIODevice *device(&file);
    int errorLine;
    int errorColumn;
    QDomDocument doc;
    if (!doc.setContent(device, true, &errorStr, &errorLine,
                        &errorColumn))
    {
        QMessageBox::warning(0, QObject::tr("DOM Parser"),
                             QObject::tr("Parse error at line %1, "
                                         "column %2:\n%3")
                             .arg(errorLine)
                             .arg(errorColumn)
                             .arg(errorStr));
        return list;
    }

    QDomElement root = doc.documentElement();
    QDomNode node = root.firstChild();
    while(!node.isNull()){
        if(node.toElement().tagName() == "project"){
            qDebug()<<"mission name:"<<node.toElement().attribute("name");
            list.append(node.toElement().attribute("name"));
        }
        node = node.nextSibling();
    }
    return list;
}
