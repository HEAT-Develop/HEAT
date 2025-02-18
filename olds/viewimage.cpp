#include "viewimage.h"
#include "mainwindow.h"
#include <fstream>
#include <iostream>
using namespace std;

ViewImage::ViewImage(QWidget *parent):
    QOpenGLWidget(parent)
{

}

void ViewImage::initializeGL()
{
    loadFile();
    glClearColor(1, 1, 1, 0);
    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);

}

void ViewImage::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
    glLoadIdentity();
    //視野指定
    glOrtho(0, 2, 0, 2, -1, 1);
}

void ViewImage::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   // glEnable(GL_DEPTH_TEST);
   // glEnable(GL_CULL_FACE);

    glColor3d(0,0,0);
    glBegin(GL_TRIANGLES);
    glVertex3d(0,1,0);
    glVertex3d(1,1,0);
    glVertex3d(1,0,0);
    glEnd();

}

/*
void ViewImage::loadFile(){

    char outfile[] = "file.txt";
       int k;
       ifstream fin ( outfile, ios::in | ios::binary );
        if (!fin){
           cout << "ファイル file.txt が開けません";
            return;
       }

       double d;  //文字列ではないデータ

       cout << "何番目のデータを読み込みますか"<<endl;
       cin >> k;
       fin.seekg ( k*sizeof ( double ) );  //ポインタの位置を移動
       fin.read( ( char * ) &d, sizeof( double ) );  //文字列ではないデータを読みこむ
       cout << k << '\t' << d << endl;

       fin.close();

       return;



}
*/
void ViewImage::loadFile()//上のgetPngImageSizeを使用する関数
{

    QFile file(tr("/Users/joker/test/TIR_Data/swingby/TIR_Earth_201512040406/TIR_8CF2_hrw.img"));
    if( !file.open(QIODevice::ReadOnly) )//読込のみで開けたかチェック
    {
       printf("error\n");//オープン失敗
    }

    QDataStream in(&file);

    in.setVersion(QDataStream::Qt_5_4);
    QString str; char *cp; qint32 n32; qint64 n64;
    in>>str>>cp>>n32>>n64;
    file.close();

    qDebug() << str << QString::fromLocal8Bit(cp) << n32 << n64;
    delete[] cp;

}

void ViewImage::mousePressEvent(QMouseEvent* event)
{
    if(event->buttons() & Qt::LeftButton){

    }else if(event->buttons() & Qt::RightButton){
    }

}
