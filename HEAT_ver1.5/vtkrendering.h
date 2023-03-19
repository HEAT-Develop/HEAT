//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//

#ifndef VTKRENDERING_H
#define VTKRENDERING_H

#include <QWidget>
#include <QOpenGLWidget>
#include <QGLWidget>
#include <QtOpenGL>
#include <QtGui>
#include <QMouseEvent>

#define NUM 49152

class VtkRendering : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit VtkRendering(QWidget *parent = 0);
    ~VtkRendering();

private:
    GLdouble point[(NUM+4)/2][3];
    GLint    facet[NUM][3];
    QPoint lastPos;
    double maxT[3];
    double RotX,RotY;
    int num;
    int facetN;
    double colorTable[256][3];
    double colorValue[256];
    double MAX[4];
    double MIN[4];

    void loadFile();
    void render();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

public slots:
      void loadVtkFile(QString vtkFileName);
      void changeData(int);
      void getDirPath(QString );
      void setColor(int);
      void setRangeVtk(double, double);

protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
    void makeColorTable();
    void colorBar();
    void unitBar();

};


#endif // VTKRENDERING_H
