//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//

#include "mainwindow.h"
#include "rendering.h"
#include "dataset.h"
#include <fstream>
#include <iostream>

#define NUM 49152
#define NN 49141
int facetN=0;
int model=0;
GLdouble normal[NUM][3];

GLfloat light0pos[] = {0.0,0.0,1.0,1};
GLfloat light0amb[] = {0.1,0.1,0.1,1.0 };
GLfloat light0dif[] = {0.18,0.18,0.18,1.0 };
GLfloat light0spe[] = {0.2,0.2,0.2,1.0 };

Rendering::Rendering(QWidget *parent) :
    QOpenGLWidget(parent)
{
    srcPath.cd(QCoreApplication::applicationDirPath());
    srcPath.cd("../../modelSrc");

}

Rendering::~Rendering()
{

}

void Rendering::initializeGL()
{
    RotX=0;
    RotY=0;
    num=0;
    loadFile();

    glClearColor(0, 0, 0, 0);
    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light0pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0dif);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0spe);
}

void Rendering::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
    glLoadIdentity();
    glOrtho(-247.0, 247.0, -138.0, 138.0,-700.0, 700.0);

}

void Rendering::paintGL()
{
    float ambient[] = { 0.01, 0.01, 0.01, 1.0};
    float diffuse[] = { 0.014, 0.014, 0.014, 1.0};
    float specular[]= { 0.01, 0.01, 0.01, 1.0};

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glRotated(2.0,RotX,RotY,0.0);
    glMaterialfv(GL_FRONT,GL_AMBIENT,ambient);
    glMaterialfv(GL_FRONT,GL_DIFFUSE,diffuse);
    glMaterialfv(GL_FRONT,GL_SPECULAR,specular);
    glMaterialf(GL_FRONT,GL_SHININESS,20);

    render();

}

void Rendering::render()
{


    glBegin(GL_TRIANGLES);

    for(int i=0;i<facetN;i++){
        glNormal3dv(normal[i]);
        for(int j=0;j<3;j++){
          glVertex3dv(point[facet[i][j]]);
        }
    }
    glEnd();
}

void Rendering::loadFile()
{
    double V;
    QString str;
    QFile filep;
    QFile filef;

    if(model==0){
        //Itokawa
        filep.setFileName(srcPath.path()+"/ItokawaPoly.txt");
        filef.setFileName(srcPath.path()+"/ItokawaPolyID.txt");
        V=0.6;
    }
    else if(model==1) {
        //Ryugoid
        filep.setFileName(srcPath.path() + "/RyuguPoly.txt");
        filef.setFileName(srcPath.path() + "/RyuguPolyID.txt");
        //Ryugoid
        //filep.setFileName(srcPath.path() + "/RyuguPolygon_02140919.txt");
        //filef.setFileName(srcPath.path() + "/RyuguPolygonID_02140919.txt");
        V=200;
    }


    if (!filep.open(QIODevice::ReadOnly)||!filef.open(QIODevice::ReadOnly))//読込のみでオープンできたかチェック
    {
        printf("error\n");
        return;
    }

    QTextStream in_p(&filep);
    QTextStream in_f(&filef);

    for(int i=0;!in_p.atEnd();i++){
        for(int j=0;j<3;j++){
        in_p >> str;
        point[i][j]=str.toDouble()*V;
        }
    }

    for(int i=0;!in_f.atEnd();i++){
        for(int j=0;j<3;j++){
        in_f >> str;
        facet[i][j]=str.toInt();
        }
        facetN++;
    }

    setNormalVector();

}

double Rendering::colorDefine(double T,int i)
{
   return T/maxT[i];
}

void Rendering::mousePressEvent(QMouseEvent* event)
{
    if(event->buttons() & Qt::LeftButton){
     lastPos = event->pos();
    }
    else if(event->buttons() & Qt::RightButton){
        num++;
     if(num==3)num=0;
     update();
    }

}

void Rendering::mouseMoveEvent(QMouseEvent *event)
{
     double dx = event->x() - lastPos.x();
     double dy = event->y() - lastPos.y();

     if(event->buttons() & Qt::LeftButton){

     if(dx>lastPos.x()){
         RotX=dx;
     }else{RotX=-dx;}

     if(dy>lastPos.y()){
         RotY=dy;
     }else{RotY=-dy;}

     }
   lastPos = event->pos();
   update();
}

void Rendering::changeModel(int x){
    model =x;
    facetN=0;
    loadFile();
    update();
}

void Rendering::setNormalVector()
{
    GLdouble tmp[3][3];

    for(int i=0;i<facetN;i++){
        for(int j=0;j<3;j++){
          tmp[j][0]= point[facet[i][j]][0];
          tmp[j][1]= point[facet[i][j]][1];
          tmp[j][2]= point[facet[i][j]][2];
        }
        calcNormalVector(tmp,i);

    }

}

void Rendering::calcNormalVector(GLdouble a[3][3],int n)
{
    GLdouble v1[3], v2[3], cross[3];
        for (int i = 0; i < 3; i++){ v1[i] = a[0][i] - a[1][i]; }
        for (int i = 0; i < 3; i++){ v2[i] = a[2][i] - a[1][i]; }
        for (int i = 0; i < 3; i++){ cross[i] = v2[(i+1)%3] * v1[(i+2)%3] - v2[(i+2)%3] * v1[(i+1)%3]; }
        double length = sqrt(cross[0] * cross[0] + cross[1] * cross[1] + cross[2] * cross[2]);

        for (int i = 0; i < 3; i++) {
            if(length !=0) normal[n][i] = cross[i] / length;
            else normal[n][i] = 0 ;
        }
}
