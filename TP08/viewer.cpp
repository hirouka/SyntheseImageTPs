#include "viewer.h"

#include <math.h>
#include <iostream>
#include "meshLoader.h"
#include <QTime>

using namespace std;


Viewer::Viewer(char *filename,const QGLFormat &format)
  : QGLWidget(format),
    _timer(new QTimer(this)),
    _currentshader(0),
    _light(glm::vec3(0,0,1)),
    _mode(false),
    _showShadowMap(false),
    _depthResol(512) {

  setlocale(LC_ALL,"C");

  _mesh = new Mesh(filename);
  _cam  = new Camera(_mesh->radius,glm::vec3(_mesh->center[0],_mesh->center[1],_mesh->center[2]));

  _timer->setInterval(10);
  connect(_timer,SIGNAL(timeout()),this,SLOT(updateGL()));
}

Viewer::~Viewer() {
  delete _timer;
  delete _mesh;
  delete _cam;

  // delete all GPU objects  
  deleteShaders();
  deleteTextures();
  deleteVAO(); 
  deleteFBO();
}

void Viewer::deleteTextures() {
  // delete loaded textures 
  glDeleteTextures(2,_texColor);
  glDeleteTextures(2,_texNormal);
}

void Viewer::deleteVAO() {
  // delete your VAO here (function called in destructor)
  glDeleteBuffers(5,_object);
  glDeleteBuffers(4,_floor);
  glDeleteBuffers(1,&_quad);

  glDeleteVertexArrays(1,&_vaoObject);
  glDeleteVertexArrays(1,&_vaoFloor);
  glDeleteVertexArrays(1,&_vaoQuad);
}


void Viewer::deleteFBO() {
  // delete all FBO Ids
  glDeleteFramebuffers(1,&_fbo);
  glDeleteTextures(1,&_texDepth);
}

void Viewer::createFBO() {
  // generate fbo and associated textures
  glGenFramebuffers(1,&_fbo);
  glGenTextures(1,&_texDepth);

  // create the texture for rendering depth values
  glBindTexture(GL_TEXTURE_2D,_texDepth);
  glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24,_depthResol,_depthResol,0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // attach textures to framebuffer object 
  glBindFramebuffer(GL_FRAMEBUFFER,_fbo);
  glBindTexture(GL_TEXTURE_2D,_texDepth);
  glFramebufferTexture2D(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,_texDepth,0);

  // test if everything is ok
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    cout << "Warning: FBO not complete!" << endl;

  // disable FBO
  glBindFramebuffer(GL_FRAMEBUFFER,0);
}


void Viewer::loadTexture(GLuint id,const char *filename) {
  // load image 
  QImage image = QGLWidget::convertToGLFormat(QImage(filename));

  // activate texture 
  glBindTexture(GL_TEXTURE_2D,id);
  
  // set texture parameters 
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); 
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_MIRRORED_REPEAT);
  
  // store texture in the GPU
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,image.width(),image.height(),0,
  	       GL_RGBA,GL_UNSIGNED_BYTE,(const GLvoid *)image.bits());
  
  // generate mipmaps 
  glGenerateMipmap(GL_TEXTURE_2D);
}

void Viewer::createTextures() {

  // generate 3 texture ids 
  glGenTextures(2,_texColor);
  glGenTextures(2,_texNormal);

  // load all needed textures 
  loadTexture(_texColor[0],"textures/color01.jpg");
  loadTexture(_texNormal[0],"textures/normal01.jpg");
  loadTexture(_texColor[1],"textures/color02.jpg");
  loadTexture(_texNormal[1],"textures/normal02.jpg");
}

void Viewer::createVAO() {
  // data associated with the scene floor 
  const float pt  = _mesh->radius*10.0f;
  const float pty = _mesh->radius;
  const GLfloat floorData[] = { 
    -pt,-pty,pt, pt,-pty,pt, pt,-pty,-pt, pt,-pty,-pt, -pt,-pty,-pt, -pt,-pty,pt,                   // vertices 
    0.0f,1.0f,0.0f, 0.0f,1.0f,0.0f, 0.0f,1.0f,0.0f, 0.0f,1.0f,0.0f, 0.0f,1.0f,0.0f, 0.0f,1.0f,0.0f, // normals
    1.0f,0.0f,0.0f, 1.0f,0.0f,0.0f, 1.0f,0.0f,0.0f, 1.0f,0.0f,0.0f, 1.0f,0.0f,0.0f, 1.0f,0.0f,0.0f, // tangents 
    0.0f,0.0f, 1.0f,0.0f, 1.0f,1.0f, 1.0f,1.0f, 0.0f,1.0f, 0.0f,0.0f                                // coords 
  }; 
 
  // data associated with the screen quad
  const GLfloat quadData[] = { 
    -1.0f,-1.0f,0.0f, 1.0f,-1.0f,0.0f, -1.0f,1.0f,0.0f, -1.0f,1.0f,0.0f, 1.0f,-1.0f,0.0f, 1.0f,1.0f,0.0f
  }; 

  // create VAOs
  glGenVertexArrays(1,&_vaoObject);
  glGenVertexArrays(1,&_vaoFloor);
  glGenVertexArrays(1,&_vaoQuad);

  // VBO associated with the mesh 
  glGenBuffers(5,_object);
  glBindVertexArray(_vaoObject);
  glBindBuffer(GL_ARRAY_BUFFER,_object[0]); // vertices 
  glBufferData(GL_ARRAY_BUFFER,_mesh->nb_vertices*3*sizeof(float),_mesh->vertices,GL_STATIC_DRAW);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER,_object[1]); // normals
  glBufferData(GL_ARRAY_BUFFER,_mesh->nb_vertices*3*sizeof(float),_mesh->normals,GL_STATIC_DRAW);
  glVertexAttribPointer(1,3,GL_FLOAT,GL_TRUE,0,(void *)0);
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER,_object[2]); // tangents
  glBufferData(GL_ARRAY_BUFFER,_mesh->nb_vertices*3*sizeof(float),_mesh->tangents,GL_STATIC_DRAW);
  glVertexAttribPointer(2,3,GL_FLOAT,GL_TRUE,0,(void *)0);
  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER,_object[3]); // coords 
  glBufferData(GL_ARRAY_BUFFER,_mesh->nb_vertices*2*sizeof(float),_mesh->coords,GL_STATIC_DRAW);
  glVertexAttribPointer(3,2,GL_FLOAT,GL_FALSE,0,(void *)0);
  glEnableVertexAttribArray(3);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_object[4]); // indices 
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,_mesh->nb_faces*3*sizeof(unsigned int),_mesh->faces,GL_STATIC_DRAW); 

  // VBO associated with the scene floor
  glGenBuffers(4,_floor);
  glBindVertexArray(_vaoFloor);
  glBindBuffer(GL_ARRAY_BUFFER,_floor[0]); // vertices 
  glBufferData(GL_ARRAY_BUFFER, 18*sizeof(GLfloat),&floorData[ 0],GL_STATIC_DRAW);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER,_floor[1]); // normals
  glBufferData(GL_ARRAY_BUFFER, 18*sizeof(GLfloat),&floorData[18],GL_STATIC_DRAW);
  glVertexAttribPointer(1,3,GL_FLOAT,GL_TRUE,0,(void *)0);
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER,_floor[2]); // tangents
  glBufferData(GL_ARRAY_BUFFER, 18*sizeof(GLfloat),&floorData[36],GL_STATIC_DRAW);
  glVertexAttribPointer(2,3,GL_FLOAT,GL_TRUE,0,(void *)0);
  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER,_floor[3]); // coords
  glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat),&floorData[54],GL_STATIC_DRAW);
  glVertexAttribPointer(3,2,GL_FLOAT,GL_FALSE,0,(void *)0);
  glEnableVertexAttribArray(3);

  // create the VBO associated with the screen quad 
  glGenBuffers(1,&_quad);
  glBindVertexArray(_vaoQuad);
  glBindBuffer(GL_ARRAY_BUFFER,_quad);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadData),quadData,GL_STATIC_DRAW);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void *)0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
}

void Viewer::createShaders() {
  // create 3 shaders
  _shadowMapShader = new Shader(); // will create the shadow map
  _renderingShader = new Shader(); // render the scene, use shadow map
  _debugShader = new Shader(); // debug: show the computed shadow map

  _shadowMapShader->load("shaders/shadow-map.vert","shaders/shadow-map.frag");
  _renderingShader->load("shaders/rendering.vert","shaders/rendering.frag");
  _debugShader->load("shaders/show-shadow-map.vert","shaders/show-shadow-map.frag");
}


void Viewer::deleteShaders() {
  delete _shadowMapShader; _shadowMapShader = NULL;
  delete _renderingShader; _renderingShader = NULL;
  delete _debugShader; _debugShader = NULL;
}

void Viewer::drawSceneFromCamera(GLuint id) {
  // mdv matrix from the light point of view 
  const float size = _mesh->radius*10;
  glm::vec3 l   = glm::transpose(_cam->normalMatrix())*_light;
  glm::mat4 p   = glm::ortho<float>(-size,size,-size,size,-size,2*size);
  glm::mat4 v   = glm::lookAt(l, glm::vec3(0,0,0), glm::vec3(0,1,0));
  glm::mat4 m   = glm::mat4(1.0);
  glm::mat4 mv  = v*m;

  // send uniform variables 
  glUniformMatrix4fv(glGetUniformLocation(id,"projMat"),1,GL_FALSE,&(_cam->projMatrix()[0][0]));
  glUniformMatrix3fv(glGetUniformLocation(id,"normalMat"),1,GL_FALSE,&(_cam->normalMatrix()[0][0]));
  glUniform3fv(glGetUniformLocation(id,"light"),1,&(_light[0]));

  // send textures
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,_texColor[0]);
  glUniform1i(glGetUniformLocation(id,"colormap"),0);

  glActiveTexture(GL_TEXTURE0+1);
  glBindTexture(GL_TEXTURE_2D,_texNormal[0]);
  glUniform1i(glGetUniformLocation(id,"normalmap"),1);

  // *** TODO: send the shadow map here ***
 glActiveTexture(GL_TEXTURE0+2);
  glBindTexture(GL_TEXTURE_2D,_texDepth);
  glUniform1i(glGetUniformLocation(id,"shadowmap"),2);

  glBindVertexArray(_vaoObject);

  // draw several objects
  const float r = _mesh->radius*2;
  const int   n = 2;
  for(int i=-n;i<=n;++i) {
    
    // send the modelview matrix (changes for each object)
    const glm::vec3 pos = glm::vec3(i*r,0,i*r);
    const glm::mat4 mdv = glm::translate(_cam->mdvMatrix(),pos);
    glUniformMatrix4fv(glGetUniformLocation(id,"mdvMat"),1,GL_FALSE,&(mdv[0][0]));

    // send the modelview projection depth matrix 
    const glm::mat4 mvpDepth = p*glm::translate(mv,pos);
    glUniformMatrix4fv(glGetUniformLocation(id,"mvpDepthMat"),1,GL_FALSE,&mvpDepth[0][0]);
    
    // draw faces 
    glDrawElements(GL_TRIANGLES,3*_mesh->nb_faces,GL_UNSIGNED_INT,(void *)0);
  }

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,_texColor[1]);
  glUniform1i(glGetUniformLocation(id,"colormap"),0);

  glActiveTexture(GL_TEXTURE0+1);
  glBindTexture(GL_TEXTURE_2D,_texNormal[1]);
  glUniform1i(glGetUniformLocation(id,"normalmap"),1);

  // send initial mdv matrix 
  glUniformMatrix4fv(glGetUniformLocation(id,"mdvMat"),1,GL_FALSE,&(_cam->mdvMatrix()[0][0]));

  // send initial mvp depth matrix
  const glm::mat4 mvpDepth = p*mv;
  glUniformMatrix4fv(glGetUniformLocation(id,"mvpDepthMat"),1,GL_FALSE,&mvpDepth[0][0]);

  // draw the floor
  glBindVertexArray(_vaoFloor);
  glDrawArrays(GL_TRIANGLES,0,6);

  // disable VAO
  glBindVertexArray(0);
}



void Viewer::drawSceneFromLight(GLuint id) {
  // mdv matrix from the light point of view 
  const float size = _mesh->radius*10;
  glm::vec3 l   = glm::transpose(_cam->normalMatrix())*_light;
  glm::mat4 p   = glm::ortho<float>(-size,size,-size,size,-size,2*size);
  glm::mat4 v   = glm::lookAt(l, glm::vec3(0,0,0), glm::vec3(0,1,0));
  glm::mat4 m   = glm::mat4(1.0);
  glm::mat4 mv  = v*m;

  // send uniform variables 

  // *** TODO: send the shadow map here ***
  


  glBindVertexArray(_vaoObject);

  // draw several objects
  const float r = _mesh->radius*2;
  const int   n = 2;
  for(int i=-n;i<=n;++i) {
    
    // send the modelview matrix (changes for each object)
    const glm::vec3 pos = glm::vec3(i*r,0,i*r);
    const glm::mat4 mdv = glm::translate(_cam->mdvMatrix(),pos);

    // send the modelview projection depth matrix 
    const glm::mat4 mvpDepth = p*glm::translate(mv,pos);
    glUniformMatrix4fv(glGetUniformLocation(id,"mvpMat"),1,GL_FALSE,&mvpDepth[0][0]);
    
    // draw faces 
    glDrawElements(GL_TRIANGLES,3*_mesh->nb_faces,GL_UNSIGNED_INT,(void *)0);
  }

  
  // send initial mdv matrix 

  // send initial mvp depth matrix
  const glm::mat4 mvpDepth = p*mv;
  glUniformMatrix4fv(glGetUniformLocation(id,"mvpMat"),1,GL_FALSE,&mvpDepth[0][0]);

  // draw the floor


  // disable VAO
  glBindVertexArray(0);
}

void Viewer::drawShadowMap(GLuint id) {
  // send depth texture 
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,_texDepth);
  glUniform1i(glGetUniformLocation(id,"shadowmap"),0);

  // draw the quad 
  glBindVertexArray(_vaoQuad);
  glDrawArrays(GL_TRIANGLES,0,6);

  // disable VAO
  glBindVertexArray(0);
}

void Viewer::paintGL() {
  // *** TODO: compute the shadow map here ***
/*******************************************************/
  glBindFramebuffer(GL_FRAMEBUFFER,_fbo);
  glDrawBuffer(GL_NONE);
  glViewport(0,0,_depthResol,_depthResol); // fenetre ou on dessine 
  glClear(GL_DEPTH_BUFFER_BIT);
  glUseProgram(_shadowMapShader->id());
  drawSceneFromLight(_shadowMapShader->id());
  glUseProgram(0);
  glBindFramebuffer(GL_FRAMEBUFFER,0);




  // back to the screen viewport
  glViewport(0,0,width(),height());

  // activate the rendering shader 
  glUseProgram(_renderingShader->id());

  // clear buffers 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // draw the scene and apply the shadows 
  drawSceneFromCamera(_renderingShader->id());

  // show the shadow map (press S key) 
  if(_showShadowMap) {
    // activate the test shader  
    glUseProgram(_debugShader->id());

    // clear buffers 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // display the shadow map 
    drawShadowMap(_debugShader->id());
  }

  // disable shader 
  glUseProgram(0);
}

void Viewer::resizeGL(int width,int height) {
  _cam->initialize(width,height,false);
  glViewport(0,0,width,height);
  updateGL();
}

void Viewer::mousePressEvent(QMouseEvent *me) {
  const glm::vec2 p((float)me->x(),(float)(height()-me->y()));

  if(me->button()==Qt::LeftButton) {
    _cam->initRotation(p);
    _mode = false;
  } else if(me->button()==Qt::MidButton) {
    _cam->initMoveZ(p);
    _mode = false;
  } else if(me->button()==Qt::RightButton) {
    _light[0] = (p[0]-(float)(width()/2))/((float)(width()/2));
    _light[1] = (p[1]-(float)(height()/2))/((float)(height()/2));
    _light[2] = 1.0f-std::max(fabs(_light[0]),fabs(_light[1]));
    _light = glm::normalize(_light);
    _mode = true;
  } 

  updateGL();
}

void Viewer::mouseMoveEvent(QMouseEvent *me) {
  const glm::vec2 p((float)me->x(),(float)(height()-me->y()));
 
  if(_mode) {
    // light mode
    _light[0] = (p[0]-(float)(width()/2))/((float)(width()/2));
    _light[1] = (p[1]-(float)(height()/2))/((float)(height()/2));
    _light[2] = 1.0f-std::max(fabs(_light[0]),fabs(_light[1]));
    _light = glm::normalize(_light);
  } else {
    // camera mode
    _cam->move(p);
  }

  updateGL();
}

void Viewer::keyPressEvent(QKeyEvent *ke) {
  
  // key a: play/stop animation
  if(ke->key()==Qt::Key_A) {
    if(_timer->isActive()) 
      _timer->stop();
    else 
      _timer->start();
  }

  // key i: init camera
  if(ke->key()==Qt::Key_I) {
    _cam->initialize(width(),height(),true);
  }
  
  // key f: compute FPS
  if(ke->key()==Qt::Key_F) {
    int elapsed;
    QTime timer;
    timer.start();
    unsigned int nb = 500;
    for(unsigned int i=0;i<nb;++i) {
      paintGL();
    }
    elapsed = timer.elapsed();
    double t = (double)nb/((double)elapsed);
    cout << "FPS : " << t*1000.0 << endl;
  }

  // key r: reload shaders 
  if(ke->key()==Qt::Key_R) {
    _shadowMapShader->reload("shaders/shadow-map.vert","shaders/shadow-map.frag");
    _renderingShader->reload("shaders/rendering.vert","shaders/rendering.vert");
    _debugShader->reload("shaders/show-shadow-map.vert","shaders/show-shadow-map.frag");
  }

  // key S: show the shadow map 
  if(ke->key()==Qt::Key_S) {
    _showShadowMap = !_showShadowMap;
  }

  updateGL();
}

void Viewer::initializeGL() {
  // make this window the current one
  makeCurrent();

  // init and chack glew
  glewExperimental = GL_TRUE;

  if(glewInit()!=GLEW_OK) {
    cerr << "Warning: glewInit failed!" << endl;
  }

  // init OpenGL settings
  glClearColor(0.0,0.0,0.0,1.0);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);
  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  glViewport(0,0,width(),height());

  // initialize camera
  _cam->initialize(width(),height(),true);

  // load shader files
  createShaders();

  // init VAO
  createVAO();
  
  // init textures 
  createTextures();
  
  // create/init FBO
  createFBO();

  // starts the timer 
  _timer->start();
}

