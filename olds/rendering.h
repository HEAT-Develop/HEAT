//
// (c) 2022 The University of Aizu
// This software is released under the GNU General Public License.
//

#ifndef RENDERING
#define RENDERING
#include <QWidget>
#include <QOpenGLWidget>
#include <QGLWidget>
#include <QtOpenGL>
#include <QtGui>
#include <QMouseEvent>
#include <QDir>

#define NUM 49152
#define NN 49141

class Rendering : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit Rendering(QWidget *parent = 0);
    ~Rendering();
    GLdouble point[(NUM+4)/2][3];
    GLint    facet[NUM][3];
    QPoint lastPos;
    double maxT[3];
    double RotX,RotY;
    int num;
    QDir srcPath;

    void loadFile();
    void render();
    void setNormalVector();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    double colorDefine(double T,int i);
    void calcNormalVector(GLdouble a[3][3],int n);

public slots:
    void changeModel(int);

protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

};

#endif //RENDERING
