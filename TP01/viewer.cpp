#include "viewer.h"

#include <math.h>
#include <iostream>

using namespace std;

Viewer::Viewer(const QGLFormat &format,const QString &)
  : QGLWidget(format),
    _timer(this) {

  _timer.setInterval(30);
  connect(&_timer,SIGNAL(timeout()),this,SLOT(updateGL()));
}

void Viewer::generateRandomPoints() {

 srand (time(NULL));
  for (int i=0;i<40;i++){
    Point p1;
    p1.pos=randv2(-1.0,1.0);
    p1.dir=randv2(-0.1,0.1);
    p1.col=randv3(0.0,1.0);
    p1.size=randf(10.0,30.0); 
    tab[i] = p1;
  }
  // TODO
}

void Viewer::updatePointPositions() {
  for(int i = 0; i<40 ;i++){
    if(tab[i].pos[0]<=-1){
      if(tab[i].pos[1]<=-1 || tab[i].pos[1]>=1){
        tab[i].dir=-tab[i].dir;
      }else{
        tab[i].dir=glm::reflect(tab[i].dir,glm::vec2(1,0));
      }
    }else if(tab[i].pos[1]<=-1){
      if(tab[i].pos[0]<=-1 || tab[i].pos[0]>=1){
        tab[i].dir=-tab[i].dir;
      }else{
        tab[i].dir=glm::reflect(tab[i].dir,glm::vec2(0,1));
      }
    }else if(tab[i].pos[0]>=1){
      if(tab[i].pos[1]<=-1 || tab[i].pos[1]>=1){
        tab[i].dir=-tab[i].dir;
      }else{
        tab[i].dir=glm::reflect(tab[i].dir,glm::vec2(-1,0));
      }
    }else if(tab[i].pos[1]>=1){
      if(tab[i].pos[0]<=-1 || tab[i].pos[0]>=1){
        tab[i].dir=-tab[i].dir;
      }else{
        tab[i].dir=glm::reflect(tab[i].dir,glm::vec2(0,-1));
      }
    }
    //avance
    tab[i].pos=tab[i].pos+tab[i].dir;
  }
  // TODO 
}

void Viewer::paintGL() {
  // clear color and depth buffers 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 /* glBegin(GL_TRIANGLES);
  //ROUGE
    glColor3f(1.0,0.0,0.0); // set the current color
    glVertex3f(-1.0,-1.0,0.0); // draw a vertex
    glVertex3f(-1.0,1.0,0.0); // draw another vertex
    glVertex3f(-0.5,1.0,0.0); // draw another vertex
   //JAUNE
    glColor3f(1.0,1.0,0.0); // set the current color
    glVertex3f(-1.0,-1.0,0.0); // draw a vertex
    glVertex3f(-0.5,1.0,0.0); // draw another vertex
    glVertex3f(0.5,-1.0,0.0); // draw another vertex
   //VERT
    glColor3f(0.0,1.0,0.0); // set the current color
    glVertex3f(0.5,-1.0,0.0); // draw a vertex
    glVertex3f(-0.5,1.0,0.0); // draw another vertex
    glVertex3f(1.0,1.0,0.0); // draw another vertex
    
   //BLEU
    glColor3f(0.0,0.0,1.0); // set the current color
    glVertex3f(0.5,-1.0,0.0); // draw a vertex
    glVertex3f(1.0,1.0,0.0); // draw another vertex
    glVertex3f(1.0,-1.0,0.0); // draw another vertex
    
  glEnd();
*/
  glBegin(GL_TRIANGLE_STRIP);
    glColor3f(1.0,0.0,0.0); // set the current color
    
    glVertex3f(-1.0,1.0,0.0); // draw another vertex
    glVertex3f(-1.0,-1.0,0.0); // draw a vertex
    glVertex3f(-0.5,1.0,0.0); // draw another vertex

    glColor3f(1.0,1.0,0.0); // set the current color
    glVertex3f(0.5,-1.0,0.0); // draw a vertex
    glColor3f(0.0,1.0,0.0); // set the current color
    glVertex3f(1.0,1.0,0.0); // draw another vertex
    glColor3f(0.0,0.0,1.0); // set the current color
    glVertex3f(1.0,-1.0,0.0); // draw another vertex


  glEnd();
  for(int i =0 ; i<40; i++){
    glPointSize(tab[i].size);
    glBegin(GL_POINTS);
      glColor3f(tab[i].col[0],tab[i].col[1],tab[i].col[2]);
      glVertex2f(tab[i].pos[0],tab[i].pos[1]);  
    glEnd();
  }
    updatePointPositions();



  // TODO
}

void Viewer::resizeGL(int width,int height) {
  glViewport(0,0,width,height);
  // TODO 

}

void Viewer::keyPressEvent(QKeyEvent *ke) {
  
  // key a: play/stop animation
  if(ke->key()==Qt::Key_A) {
    if(_timer.isActive()) 
      _timer.stop();
    else 
      _timer.start();
  }
  

  updateGL();
}

void Viewer::initializeGL() {
  // make this window the current one
  makeCurrent();

  // init and chack glew
  if(glewInit()!=GLEW_OK) {
    cerr << "Warning: glewInit failed!" << endl;
  }

  if(!GLEW_ARB_vertex_program   ||
     !GLEW_ARB_fragment_program ||
     !GLEW_ARB_texture_float    ||
     !GLEW_ARB_draw_buffers     ||
     !GLEW_ARB_framebuffer_object) {
    cerr << "Warning: Shaders not supported!" << endl;
  }

  // init OpenGL settings
  glClearColor(0.0,0.0,0.0,1.0);
  glDisable(GL_DEPTH_TEST);
  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  glEnable(GL_POINT_SMOOTH);
  // initialize points 
  generateRandomPoints();
  resizeGL(200,200);
  // starts the timer 
  _timer.start();
}

